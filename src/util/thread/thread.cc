/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./inl.h"
#include <uv.h>
#include <dlfcn.h>
#include <exception>
#include <cstdio>

#if Qk_ANDROID
#include "../util/jni.h"
#endif

#ifndef Qk_ATEXIT_WAIT_TIMEOUT
# define Qk_ATEXIT_WAIT_TIMEOUT 1e6 // 1s
#endif

namespace qk {

	template<> uint64_t Compare<ThreadID>::hashCode(const ThreadID& key) {
		union Id {
			ThreadID key; uint64_t hash;
		} id = {.key=key};
		return id.hash;
	}

	std::atomic_bool                    __is_process_exit_flag{false};
	Mutex                               *__threads_mutex = nullptr;
	static Dict<ThreadID, Thread_INL*>  *__threads = nullptr;
	static EventNoticer<Event<void, int>, Mutex> *__on_process_exit = nullptr;
	static EventNoticer<Event<void>, Mutex> *__on_foreground = nullptr;
	static EventNoticer<Event<void>, Mutex> *__on_background = nullptr;

	Qk_EXPORT bool is_process_exit() {
		return __is_process_exit_flag;
	}

	CondMutex::Lock::~Lock() {
		m.unlock();
	}

	void CondMutex::lock() {
		mutex.lock();
	}

	void CondMutex::unlock() {
		mutex.unlock();
	}

	CondMutex::Lock CondMutex::scope_lock() {
		mutex.lock();
		return Lock{.m=mutex};
	}

	void CondMutex::lock_and_wait_for(uint64_t timeoutUs) {
		qk::Lock lock(mutex);
		if (timeoutUs) {
			cond.wait_for(lock, std::chrono::microseconds(timeoutUs));
		} else {
			cond.wait(lock);
		}
	}

	void CondMutex::lock_and_notify_one() {
		mutex.lock();
		cond.notify_one();
		mutex.unlock();
	}

	void CondMutex::lock_and_notify_all() {
		mutex.lock();
		cond.notify_all();
		mutex.unlock();
	}

	static Thread_INL* Thread_INL_New(cString& name, void *arg, void (*exec)(cThread *t, void* arg)) {
		auto t = new Thread_INL;
		t->name = name;
		t->abort = 0;
		t->loop = nullptr;
		t->exec = exec;
		t->arg = arg;
		return t;
	}

	thread_local Thread_INL* thread_inl = nullptr;

	static void thread_set_thread_data(Thread_INL *t) {
		thread_inl = t;
	}

	cThread* thread_self() {
		return thread_inl;
	}

	Thread_INL* thread_self_inl() {
		return thread_inl;
	}

	ThreadID thread_self_id() {
		thread_local ThreadID tid = std::this_thread::get_id();
		return tid;
	}

	static void SetThreadName(cString &name) {
#if defined(__DragonFly__) || defined(__FreeBSD__) || defined(__OpenBSD__)
		pthread_set_name_np(pthread_self(), *name);
#elif defined(__NetBSD__)
		Qk_ASSERT(name.length() <= PTHREAD_MAX_NAMELEN_NP);
		pthread_setname_np(pthread_self(), "%s", *name);
#elif Qk_APPLE
		// Mac OS X does not expose the length limit of the name, so hardcode it.
		Qk_ASSERT(name.length() <= 63);
		pthread_setname_np(*name);
#elif defined(PR_SET_NAME)
		prctl(PR_SET_NAME,
					reinterpret_cast<unsigned long>(*name), // NOLINT
					0, 0, 0);
#endif
	}

	ThreadID thread_new(void (*exec)(cThread *t, void* arg), void* arg, cString& tag) {
		if ( __is_process_exit_flag ) {
			return ThreadID();
		}
		ThreadID id;
		Thread_INL *t = Thread_INL_New(tag, arg, exec);
		ScopeLock scope(*__threads_mutex);

		// uv_thread_create_ex
		// uv_thread_create((uv_thread_t*)&id, [](void* arg) {
		std::thread th([](void *arg) {
			{ ScopeLock scope(*__threads_mutex); /* wait thread_new main call return*/ }
			auto t = static_cast<Thread_INL*>(arg);
#if Qk_ANDROID
			JNI::ScopeENV scope;
#endif
			SetThreadName(t->name);
			thread_set_thread_data(t);
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
				Qk_DLog("Thread end ..., %s", t->name.c_str());
				for (auto& i : t->waitSelfEnd) {
					i->end = true;
					i->lock_and_notify_one();
				}
				Qk_DLog("Thread end  ok, %s", t->name.c_str());
			}
			delete t;
		}, t);

		id = th.get_id();
		th.detach();

		Qk_CHECK(id != ThreadID(), "id != ThreadID()");

		return __threads->set((t->id = id), t)->id;
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
		if (t) {
			t->lock_and_wait_for(timeoutUs);
		} else {
			thread_sleep(timeoutUs);
		}
	}

	static void thread_resume_(Thread_INL *t, int abort) {
		ScopeLock scope(t->mutex);
		if (abort) {
			if (t->loop)
				t->loop->stop();
			t->abort = abort;
		}
		t->cond.notify_one(); // resume sleep status @ thread_pause()
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
			Qk_DLog("thread_join_for(), cannot wait self thread");
			return;
		}
		Lock lock(*__threads_mutex);
		Thread_INL *t;
		if ( __threads->get(id, t) ) {
			t->mutex.lock();
			lock.unlock();
			WaitSelfEnd wait;
			String name(t->name);
			t->waitSelfEnd.push_back(&wait);
			t->mutex.unlock();
			Qk_DLog("thread_join_for(), ..., %p, %s", id, name.c_str());
			if (!wait.end)
				wait.lock_and_wait_for(timeoutUs); // permanent wait
			Qk_DLog("thread_join_for(), end, %p, %s", id, name.c_str());
		}
	}

	static void thread_process_exit(int rc) {
		if (__is_process_exit_flag.exchange(true))
			return; // exit

		Array<ThreadID> threads_id;

		Qk_DLog("thread_process_exit(), 0");
		Event<void, int> ev(rc, rc);
		Qk_Trigger(ProcessExit, ev); // trigger event
		rc = ev.return_value;
		Qk_DLog("thread_process_exit(), 1");

		{
			ScopeLock scope(*__threads_mutex);
			Qk_DLog("threads count, %d", __threads->length());
			for ( auto& i : *__threads ) {
				Qk_DLog("thread_process_exit(), tag, %p, %s", i.second->id, *i.second->name);
				thread_resume_(i.second, -2); // resume sleep status and abort
				threads_id.push(i.second->id);
			}
		}
		for ( auto& i: threads_id ) {
			// CondMutex for the end of this thread here, this time defaults to 1 second
			Qk_DLog("thread_process_exit(), join, %p", i);
			thread_join_for(i, Qk_ATEXIT_WAIT_TIMEOUT); // wait 1s
		}

		Qk_DLog("thread_process_exit() 2");
	}

	void thread_exit(int rc) {
		if (!__is_process_exit_flag.load()) {
			thread_process_exit(rc);
			::exit(rc); // exit process
		}
	}

	EventNoticer<Event<void, int>, Mutex>& onProcessExit() {
		return *__on_process_exit;
	}
	EventNoticer<Event<void, Object*>, Mutex>& onForeground() {
		return *__on_foreground;
	}
	EventNoticer<Event<void, Object*>, Mutex>& onBackground() {
		return *__on_background;
	}

	static void onUnexpectedTerminate() {
		if (auto eptr = std::current_exception()) {
			try {
				std::rethrow_exception(eptr);
			}
			catch (const qk::Error& e) {
				Qk_ELog("Uncaught qk::Error: %s", e.message().c_str());
			}
			catch (const std::exception& e) {
				Qk_ELog("Uncaught std::exception: %s", e.what());
			}
			catch (...) {
				Qk_ELog("Unknown exception");
			}
		}
		Qk_Log("Thread name: %s", thread_inl ? thread_inl->name.c_str(): "unknown");
		abort();
	}

	Qk_Init_Func(thread_init_once) {
		Qk_DLog("thread_init_once");
		// Object heap allocator may not have been initialized yet
		__threads = new(::malloc(sizeof(Dict<ThreadID, Thread_INL*>))) Dict<ThreadID, Thread_INL*>();
		__threads_mutex = new Mutex();
		__on_process_exit = new EventNoticer<Event<void, int>, Mutex>(nullptr);
		__on_foreground = new EventNoticer<Event<void>, Mutex>(nullptr);
		__on_background = new EventNoticer<Event<void>, Mutex>(nullptr);
		atexit([](){ thread_process_exit(0); });
		std::set_terminate(onUnexpectedTerminate);
		std::set_unexpected(onUnexpectedTerminate);
	}
}
