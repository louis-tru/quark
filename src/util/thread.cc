/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./thread.h"
#include <uv.h>
#include <pthread.h>

#ifndef Qk_ATEXIT_WAIT_TIMEOUT
# define Qk_ATEXIT_WAIT_TIMEOUT 1e6 // 1s
#endif

namespace qk {

	template<> uint64_t Compare<ThreadID>::hashCode(const ThreadID& key) {
		return *reinterpret_cast<const uint32_t*>(&key);
	}

	std::atomic_int                      __is_process_exit_atomic(0);
	bool                                 __is_process_exit(false);
	Mutex                               *__threads_mutex = nullptr;
	static Dict<ThreadID, Thread_INL*>  *__threads = nullptr;
	static pthread_key_t                 __specific_key = 0;
	static EventNoticer<Event<>, Mutex> *__on_process_safe_exit = nullptr;

	Qk_EXPORT bool is_process_exit() {
		return __is_process_exit;
	}

	void CondMutex::lock_wait_for(uint64_t timeoutUs) {
		Lock lock(mutex);
		if (timeoutUs) {
			cond.wait_for(lock, std::chrono::microseconds(timeoutUs));
		} else {
			cond.wait(lock);
		}
	}

	void CondMutex::lock_notify_one() {
		ScopeLock scope(mutex);
		cond.notify_one();
	}

	void CondMutex::lock_notify_all() {
		ScopeLock scope(mutex);
		cond.notify_all();
	}

	static Thread_INL* Thread_INL_New(cString& tag, void *arg, void (*exec)(cThread *t, void* arg)) {
		auto t = new Thread_INL;
		t->tag = tag;
		t->abort = 0;
		t->loop = nullptr;
		t->exec = exec;
		t->arg = arg;
		return t;
	}

	static void thread_set_specific_data(Thread_INL *t) {
		Qk_Assert(!pthread_getspecific(__specific_key));
		pthread_setspecific(__specific_key, t);
	}

	cThread* thread_self() {
		return reinterpret_cast<cThread*>(pthread_getspecific(__specific_key));
	}

	Thread_INL* thread_self_inl() {
		return reinterpret_cast<Thread_INL*>(pthread_getspecific(__specific_key));
	}

	ThreadID thread_self_id() {
		return std::this_thread::get_id();
	}

	ThreadID thread_new(void (*exec)(cThread *t, void* arg), void* arg, cString& tag) {
		if ( __is_process_exit_atomic != 0 ) {
			return ThreadID();
		}
		ThreadID id;
		Thread_INL *t = Thread_INL_New(tag, arg, exec);
		ScopeLock scope(*__threads_mutex);

		uv_thread_create((uv_thread_t*)&id, [](void* arg) {
			{ ScopeLock scope(*__threads_mutex); /* wait thread_new main call return*/ }
			auto t = static_cast<Thread_INL*>(arg);
#if Qk_ANDROID
			JNI::ScopeENV scope;
#endif
			thread_set_specific_data(t);
			if ( !t->abort ) {
				t->exec(t, t->arg);
			}
			{ // delete global handle
				ScopeLock scope(*__threads_mutex);
				__threads->erase(t->id);
			}
			{ // notify wait and release loop
				ScopeLock scope(t->mutex);
				if (!t->abort) {
					t->abort = -3; // exit abort
				}
				runloop_death(t->loop); // release loop object
				t->loop = nullptr;
				Qk_DEBUG("Thread end ..., %s", t->tag.c_str());
				for (auto& i : t->waitSelfEnd) {
					i->lock_notify_one();
				}
				Qk_DEBUG("Thread end  ok, %s", t->tag.c_str());
			}
			delete t;
		}, t);

		if (id != ThreadID()) {
			__threads->set((t->id = id), t);
		} else { // fail
			Qk_FATAL("id != ThreadID()");
			// delete thread;
		}
		return id;
	}

	typedef std::function<void(cThread *t)> ForkFunc;

	ThreadID thread_new(ForkFunc func, cString& tag) {
		auto funcp = new ForkFunc(func);
		return thread_new([](cThread *t, void* arg) {
			std::unique_ptr<ForkFunc> f( (ForkFunc*)arg );
			(*f)(t);
		}, funcp, tag);
	}

	void thread_sleep(uint64_t timeoutUs) {
		std::this_thread::sleep_for(std::chrono::microseconds(timeoutUs));
	}

	void thread_pause(uint64_t timeoutUs) {
		auto t = thread_self_inl();
		Qk_Assert(t, "Cannot find current qk::Thread handle, use Thread::sleep()");
		t->lock_wait_for(timeoutUs);
	}

	static void thread_resume_(Thread_INL *t, int abort) {
		ScopeLock scope(t->mutex);
		if (abort) {
			if (t->loop)
				t->loop->stop();
			t->abort = abort;
		}
		t->cond.notify_one(); // resume sleep status
	}

	void thread_resume(ThreadID id, int abort) {
		ScopeLock lock(*__threads_mutex);
		Thread_INL *t;
		if ( __threads->get(id, t) ) {
			thread_resume_(t, abort); // resume sleep status
		}
	}

	void thread_try_abort(ThreadID id) {
		thread_resume(id, -1);
	}

	void thread_join_for(ThreadID id, uint64_t timeoutUs) {
		if (id == thread_self_id()) {
			Qk_DEBUG("thread_join_for(), cannot wait self thread");
			return;
		}
		Lock lock(*__threads_mutex);
		Thread_INL *t;
		if ( __threads->get(id, t) ) {
			t->mutex.lock();
			lock.unlock();
			CondMutex wait;
			t->waitSelfEnd.pushBack(&wait);
			t->mutex.unlock();
			Qk_DEBUG("thread_join_for(), ..., %p, %s", id, *t->tag);
			wait.lock_wait_for(timeoutUs); // permanent wait
			Qk_DEBUG("thread_join_for(), end, %p, %s", id, *t->tag);
		}
	}

	static void thread_process_exit(int rc) {
		if (__is_process_exit_atomic++)
			return; // exit
		__is_process_exit = true;

		Array<ThreadID> threads_id;

		Qk_DEBUG("thread_process_exit(), 0");
		Event<> ev(Int32(rc), rc);
		Qk_Trigger(ProcessExit, ev); // trigger event
		rc = ev.return_value;
		Qk_DEBUG("thread_process_exit(), 1");

		{
			ScopeLock scope(*__threads_mutex);
			Qk_DEBUG("threads count, %d", __threads->length());
			for ( auto& i : *__threads ) {
				Qk_DEBUG("thread_process_exit(), tag, %p, %s", i.value->id, *i.value->tag);
				thread_resume_(i.value, -2); // resume sleep status and abort
				threads_id.push(i.value->id);
			}
		}
		for ( auto& i: threads_id ) {
			// CondMutex for the end of this thread here, this time defaults to 1 second
			Qk_DEBUG("thread_process_exit(), join, %p", i);
			thread_join_for(i, Qk_ATEXIT_WAIT_TIMEOUT); // wait 1s
		}

		Qk_DEBUG("thread_process_exit() 2");
	}

	void thread_exit(int rc) {
		if (!__is_process_exit_atomic) {
			thread_process_exit(rc);
			::exit(rc); // exit process
		}
	}

	EventNoticer<Event<>, Mutex>& onProcessExit() {
		return *__on_process_safe_exit;
	}

	Qk_INIT_BLOCK(thread_init_once) {
		Qk_DEBUG("thread_init_once");
		// Object heap allocator may not have been initialized yet
		__threads = new(::malloc(sizeof(Dict<ThreadID, Thread_INL*>))) Dict<ThreadID, Thread_INL*>();
		__threads_mutex = new Mutex();
		__on_process_safe_exit = new EventNoticer<Event<>, Mutex>(nullptr);
		Qk_DEBUG("sizeof EventNoticer<Event<>, Mutex>,%d", sizeof(EventNoticer<Event<>, Mutex>));
		Qk_DEBUG("sizeof EventNoticer<>,%d", sizeof(EventNoticer<>));
		atexit([](){ thread_process_exit(0); });
		int err = pthread_key_create(&__specific_key, nullptr);
		Qk_Assert(err == 0);
	}
}
