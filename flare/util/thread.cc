/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./thread.h"
#include "./loop.h"
#include "./list"
#include "./dict.h"
#include <uv.h>
#include <pthread.h>

#if FX_ANDROID
# include "./_android-jni.h"
#endif

#ifndef FX_ATEXIT_WAIT_TIMEOUT
# define FX_ATEXIT_WAIT_TIMEOUT 1e6
#endif

namespace flare {

	template<> uint64_t Compare<ThreadID>::hash_code(const ThreadID& key) {
		return *reinterpret_cast<const uint32_t*>(&key);
	}

	struct ListenSignal {
		Thread* thread;
		Mutex mutex;
		Condition cond;
	};

	Mutex* __Thread_threads_mutex = nullptr;
	Dict<ThreadID, Thread*>* __Thread_threads = nullptr;
	static List<ListenSignal*>* threads_end_listens = nullptr;
	static pthread_key_t specific_key;
	static int is_process_exit = 0;
	static EventNoticer<>* on_process_safe_exit = nullptr;

	FX_DEFINE_INLINE_MEMBERS(Thread, Inl) {
	 public:
		#define _inl_t(self) static_cast<Thread::Inl*>(self)

		static void destructor(void* ptr) {
			auto thread = reinterpret_cast<Thread::Inl*>(ptr);
			if (is_process_exit == 0) { // no process exit
				Release(thread->_loop);
			}
			delete thread;
		}

		static void initialize() {
			DLOG("thread_init_once");
			atexit(Thread::Inl::before_exit);
			__Thread_threads = new Dict<ID, Thread*>();
			__Thread_threads_mutex = new Mutex();
			threads_end_listens = new List<ListenSignal*>();
			on_process_safe_exit = new EventNoticer<>("ProcessSafeExit", nullptr);
			int err = pthread_key_create(&specific_key, destructor);
			ASSERT(err == 0);
		}

		static void set_thread_specific_data(Thread* thread) {
			ASSERT(!pthread_getspecific(specific_key));
			pthread_setspecific(specific_key, thread);
		}

		static inline Thread* get_thread_specific_data() {
			return reinterpret_cast<Thread*>(pthread_getspecific(specific_key));
		}

		static void run_2(Exec exec, Thread* thread) {
			#if FX_ANDROID
				JNI::ScopeENV scope;
			#endif
			set_thread_specific_data(thread);
			if ( !thread->_abort ) {
				int rc = exec(*thread);
				thread->_abort = true;
			}
			{
				ScopeLock scope(*__Thread_threads_mutex);
				DLOG("Thread end ..., %s", *thread->name());
				for (auto& i : *threads_end_listens) {
					if (i->thread == thread) {
						ScopeLock scope(i->mutex);
						i->cond.notify_one();
					}
				}
				DLOG("Thread end  ok, %s", *thread->name());
				threads->erase(thread->id());
			}
		}

		static ID run(Exec exec, cString& name) {
			if ( is_process_exit ) {
				return ID();
			} else {
				ScopeLock scope(*__Thread_threads_mutex);
				Thread* thread = new Thread();
				std::thread t(run_2, exec, thread);
				thread->_id = t.get_id();
				thread->_name = name;
				thread->_abort = false;
				thread->_loop = nullptr;
				memset(thread->_data, 0, sizeof(void*[256]));
				(*threads)[thread->_id] = thread;
				t.detach();
				return thread->_id;
			}
		}

		void awaken(bool abort = 0) {
			ScopeLock scope(_mutex);
			if (abort) {
				if (_loop)
					_loop->stop();
				_abort = true;
			}
			_cond.notify_one(); // awaken sleep status
		}

		static void before_exit() {
			if (!is_process_exit++) { // exit
				Array<ID> threads_id;
				{
					ScopeLock scope(*__Thread_threads_mutex);
					DLOG("threads count, %d", __Thread_threads->length());
					for ( auto& i : *__Thread_threads ) {
						DLOG("atexit_exec,name, %p, %s", i.value->id(), *i.value->name());
						_inl_t(i.value)->awaken(true); // awaken sleep status and abort
						threads_id.push(i.value->id());
					}
				}
				for ( auto& i: threads_id ) {
					// 在这里等待这个线程的结束,这个时间默认为1秒钟
					DLOG("atexit_exec,join, %p", i);
					join(i, FX_ATEXIT_WAIT_TIMEOUT); // wait 1s
				}
			}
		}

		static void safe_exit(int rc, bool forceExit = false) {
			static int is_exited = 0;
			if (!is_exited++ && !is_process_exit) {

				// KeepLoop* keep = nullptr;
				// if (__Loop_main_loop_obj && __Loop_main_loop_obj->runing()) {
				// 	keep = __Loop_main_loop_obj->keep_alive("Thread::Inl::exit()"); // keep main loop
				// }
				DLOG("Inl::exit(), 0");
				rc = Thread::FX_Trigger(ProcessSafeExit, Event<>(Int32(rc), std::move(rc)));
				DLOG("Inl::exit(), 1");

				Release(keep); keep = nullptr;
				before_exit();

				DLOG("Inl::reallyExit()");
				if (forceExit)
					::exit(rc); // foece reallyExit
			} else {
				DLOG("The program has exited");
			}
		}

	};

	EventNoticer<>& Thread::onProcessSafeExit() {
		return *on_process_safe_exit;
	}

	Thread::Thread(){}
	Thread::~Thread(){}

	ThreadID Thread::spawn(Exec exec, cString& name) {
		return Inl::run(exec, name);
	}

	void Thread::join(ID id, int64_t timeoutUs) {
		if (id == current_id()) {
			DLOG("Thread::join(), cannot join self");
			return;
		}
		Lock lock(*__Thread_threads_mutex);
		auto i = __Thread_threads->find(id);
		if ( i != __Thread_threads->end() ) {
			ListenSignal signal = { i->value };
			auto it = threads_end_listens->insert(threads_end_listens->end(), &signal);//(&signal);
			{ //
				Lock l(signal.mutex);
				lock.unlock();
				String name = i->value->name();
				DLOG("Thread::join, ..., %p, %s", id, *name);
				if (timeoutUs > 0) {
					signal.cond.wait_for(l, std::chrono::microseconds(timeoutUs)); // wait
				} else {
					signal.cond.wait(l); // permanent wait
				}
				DLOG("Thread::join, end, %p, %s", id, *name);
			}
			lock.lock();
			threads_end_listens->erase(it);
		}
	}

	/**
	 * @func sleep
	 */
	void Thread::sleep(int64_t timeoutUs) {
		if ( timeoutUs > 0 && timeoutUs < 5e5 /*500ms*/ ) {
			std::this_thread::sleep_for(std::chrono::microseconds(timeoutUs));
			return;
		}
		auto cur = current();
		if ( cur ) {
			Lock lock(cur->_mutex);
			if ( !cur->_abort ) {
				if (timeoutUs < 1) {
					cur->_cond.wait(lock); // wait
				} else {
					cur->_cond.wait_for(lock, std::chrono::microseconds(timeoutUs));
				}
			} else {
				FX_WARN("Thread aborted, cannot sleep");
			}
		} else {
			FX_WARN("Cannot find current flare::Thread handle, use std::this_thread::sleep_for()");
			if (timeoutUs > 0) {
				std::this_thread::sleep_for(std::chrono::microseconds(timeoutUs));
			}
		}
	}

	/**
	 * @func awaken
	 */
	void Thread::awaken(ID id) {
		ScopeLock lock(*__Thread_threads_mutex);
		auto i = threads->find(id);
		if ( i != threads->end() ) {
			_inl_t(i->value)->awaken(); // awaken sleep status
		}
	}

	void Thread::abort(ID id) {
		ScopeLock lock(*__Thread_threads_mutex);
		auto i = threads->find(id);
		if ( i != threads->end() ) {
			_inl_t(i->value)->awaken(true); // awaken sleep status and abort
		}
	}

	/**
	 * @func current_id
	 */
	ThreadID Thread::current_id() {
		return std::this_thread::get_id();
	}

	/**
	 * @func current
	 */
	Thread* Thread::current() {
		return Inl::get_thread_specific_data();
	}

	FX_EXPORT void safe_exit(int rc) {
		Thread::Inl::safe_exit(rc);
	}

	void exit(int rc) {
		Thread::Inl::safe_exit(rc, true);
	}

	bool is_exited() {
		return is_process_exit;
	}

	FX_INIT_BLOCK(thread_init_once) {
		Thread::Inl::initialize();
	}

}
