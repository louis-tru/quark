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

#include "./loop_.h"
#include <uv.h>
#include <pthread.h>

#ifndef Qk_ATEXIT_WAIT_TIMEOUT
# define Qk_ATEXIT_WAIT_TIMEOUT 1e6 // 1s
#endif

namespace qk {

	template<> uint64_t Compare<ThreadID>::hashCode(const ThreadID& key) {
		return *reinterpret_cast<const uint32_t*>(&key);
	}

	std::atomic_int                      __is_process_exit_safe(0);
	bool                                 __is_process_exit(false);
	Mutex                               *__threads_mutex = nullptr;
	static Dict<ThreadID, Thread_INL*> * __threads = nullptr;
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

	static Thread_INL* Thread_INL_init(
		Thread_INL *t, cString& tag, void *arg,
		void (*exec)(void* arg)
	) {
		t->tag = tag;
		t->abort = 0;
		t->loop = nullptr;
		t->exec = exec;
		t->arg = arg;
		return t;
	}

	static void thread_set_specific_data(Thread *t) {
		Qk_ASSERT(!pthread_getspecific(__specific_key));
		pthread_setspecific(__specific_key, t);
	}

	ThreadID thread_self_id() {
		return std::this_thread::get_id();
	}

	const Thread* thread_current() {
		return reinterpret_cast<const Thread*>(pthread_getspecific(__specific_key));
	}

	Thread_INL* thread_current_inl() {
		return reinterpret_cast<Thread_INL*>(pthread_getspecific(__specific_key));
	}

	ThreadID thread_new(void (*exec)(void* arg), void* arg, cString& tag) {
		if ( __is_process_exit_safe != 0 ) {
			return ThreadID();
		}
		Thread_INL *t = Thread_INL_init(new Thread_INL, tag, arg, exec);
		ScopeLock scope(*__threads_mutex);
		ThreadID id;

		uv_thread_create((uv_thread_t*)&id, [](void* t) {
			{ // wait thread_new main call return
				ScopeLock scope(*__threads_mutex);
			}
			auto thread = (Thread_INL*)t;
#if Qk_ANDROID
			JNI::ScopeENV scope;
#endif
			thread_set_specific_data(thread);
			if ( !thread->abort ) {
				thread->exec(thread->arg);
			}
			{ // delete global handle
				ScopeLock scope(*__threads_mutex);
				__threads->erase(thread->id);
			}
			{ // notify wait and release loop
				ScopeLock scope(thread->mutex);
				if (!thread->abort) {
					thread->abort = -3; // exit abort
				}
				Release(thread->loop); // release loop object
				Qk_DEBUG("Thread end ..., %s", thread->tag.c_str());
				for (auto& i : thread->waitSelfEnd) {
					i->lock_notify_one();
				}
				Qk_DEBUG("Thread end  ok, %s", thread->tag.c_str());
			}
			delete thread;
		}, t);

		if (id != ThreadID()) {
			__threads->set(t->id = id, t);
		} else { // fail
			Qk_FATAL("id != ThreadID()");
			// delete thread;
		}
		return id;
	}

	typedef std::function<void()> ForkFunc;

	ThreadID thread_new(ForkFunc func, cString& tag) {
		auto funcp = new ForkFunc(func);
		return thread_new([](void* arg) {
			std::unique_ptr<ForkFunc> f( (ForkFunc*)arg );
			(*f)();
		}, funcp, tag);
	}

	void thread_sleep(uint64_t timeoutUs) {
		std::this_thread::sleep_for(std::chrono::microseconds(timeoutUs));
	}

	void thread_pause(uint64_t timeoutUs) {
		auto t = thread_current_inl();
		Qk_ASSERT(t, "Cannot find current qk::Thread handle, use Thread::sleep()");
		t->lock_wait_for(timeoutUs);
	}

	static void thread_resume_inl(Thread_INL *t, int abort) {
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
		auto i = __threads->find(id);
		if ( i != __threads->end() ) {
			thread_resume_inl(i->value, abort); // resume sleep status
		}
	}

	void thread_abort(ThreadID id) {
		thread_resume(id, -1);
	}

	void thread_join_for(ThreadID id, uint64_t timeoutUs) {
		if (id == thread_self_id()) {
			Qk_DEBUG("thread_join_for(), cannot wait self thread");
			return;
		}
		Lock lock(*__threads_mutex);
		auto i = __threads->find(id);
		if ( i != __threads->end() ) {
			auto t = i->value;
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

	static void thread_try_abort_all(int rc) {
		if (__is_process_exit_safe++)
			return; // exit
		__is_process_exit = true;

		Array<ThreadID> threads_id;

		Qk_DEBUG("thread_try_abort_and_exit_inl(), 0");
		Event<> ev(Int32(rc), rc);
		Qk_Trigger(ProcessExit, ev); // trigger event
		rc = ev.return_value;
		Qk_DEBUG("thread_try_abort_and_exit_inl(), 1");

		{
			ScopeLock scope(*__threads_mutex);
			Qk_DEBUG("threads count, %d", __threads->length());
			for ( auto& i : *__threads ) {
				Qk_DEBUG("thread_try_abort_and_exit_inl,tag, %p, %s", i.value->id, *i.value->tag);
				thread_resume_inl(i.value, -2); // resume sleep status and abort
				threads_id.push(i.value->id);
			}
		}
		for ( auto& i: threads_id ) {
			// CondMutex for the end of this thread here, this time defaults to 1 second
			Qk_DEBUG("thread_try_abort_and_exit_inl,join, %p", i);
			thread_join_for(i, Qk_ATEXIT_WAIT_TIMEOUT); // wait 1s
		}

		Qk_DEBUG("thread_try_abort_and_exit_inl() 2");
	}

	void thread_try_abort_and_exit(int rc) {
		if (!__is_process_exit_safe) {
			thread_try_abort_all(rc);
			::exit(rc); // exit process
		}
	}

	EventNoticer<Event<>, Mutex>& onProcessExit() {
		return *__on_process_safe_exit;
	}

	Qk_INIT_BLOCK(thread_init_once) {
		Qk_DEBUG("thread_init_once");
		__threads = new Dict<ThreadID, Thread_INL*>();
		__threads_mutex = new Mutex();
		__on_process_safe_exit = new EventNoticer<Event<>, Mutex>(nullptr);
		Qk_DEBUG("sizeof EventNoticer<Event<>, Mutex>,%d", sizeof(EventNoticer<Event<>, Mutex>));
		Qk_DEBUG("sizeof EventNoticer<>,%d", sizeof(EventNoticer<>));
		atexit([](){ thread_try_abort_all(0); });
		int err = pthread_key_create(&__specific_key, nullptr);
		Qk_ASSERT(err == 0);
	}
}
