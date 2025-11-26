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

#ifndef __quark__util__thread__
#define __quark__util__thread__

#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "./util.h"
#include "./cb.h"
#include "./list.h"
#include "./string.h"
#include "./event.h"

typedef struct uv_loop_s uv_loop_t;
typedef struct uv_async_s uv_async_t;
typedef struct uv_timer_s uv_timer_t;
typedef struct uv_check_s uv_check_t;

namespace qk {
	typedef std::thread::id               ThreadID;
	typedef std::mutex                    Mutex;
	typedef std::recursive_mutex          RecursiveMutex;
	typedef std::unique_lock<Mutex>       Lock;
	typedef std::lock_guard<Mutex>        ScopeLock;
	typedef std::condition_variable       Condition;

	template<> Qk_EXPORT
	uint64_t Compare<ThreadID>::hashCode(const ThreadID& key);

	struct CondMutex {
		Mutex     mutex;
		Condition cond;
		struct Lock { Mutex &m; ~Lock(); };
		void lock();
		void unlock();
		Lock scope_lock(); // scope lock
		void lock_and_wait_for(uint64_t timeoutUs = 0);
		void lock_and_notify_one();
		void lock_and_notify_all();
	};
	struct Thread {
		ThreadID id; // thread id
		String   name; // new thread name string
		int      abort; // abort signal of thread and run loop
	};
	typedef const Thread cThread;

	Qk_EXPORT ThreadID thread_new(void exec(cThread *t, void* arg), void* arg, cString& tag = String());
	Qk_EXPORT ThreadID thread_new(std::function<void(cThread *t)> func, cString& tag = String());
	//!< sleep The current thread cannot be awakened
	Qk_EXPORT void     thread_sleep(uint64_t timeoutUs = 0);
	//!< Pause the current operation can be awakened by 'resume()'
	Qk_EXPORT void     thread_pause(uint64_t timeoutUs = 0 /*Less than 1 permanent wait*/);
	Qk_EXPORT void     thread_resume(ThreadID id, int abort = 0); //!< resume thread running and try abort
	Qk_EXPORT void     thread_try_abort(ThreadID id); // !< try abort thread, abort=-1
	//!< wait for the target 'id' thread to end, param `timeoutUs` less than 1 permanent wait
	Qk_EXPORT void     thread_join_for(ThreadID id, uint64_t timeoutUs = 0);
	Qk_EXPORT ThreadID thread_self_id();
	Qk_EXPORT cThread* thread_self(); // return the self thread object created by `thread_new`
	Qk_EXPORT void     thread_exit(int exit_rc); // !< try abort all thread and exit process, abort=-2

	Qk_EXPORT EventNoticer<Event<void, int>, Mutex>& onProcessExit();

	/**
	* @class PostMessage
	*/
	class Qk_EXPORT PostMessage {
	public:
		virtual void post_message(Cb cb) = 0;
	};

	/**
	* @class RunLoop
	*/
	class Qk_EXPORT RunLoop: public Object, public PostMessage {
		Qk_DISABLE_COPY(RunLoop);
	public:
		/**
		 * @class PostSyncData
		*/
		struct PostSyncData: Object {
			virtual void complete() = 0;
		};

		/**
		 * @method runing()
		*/
		bool runing() const;

		/**
		 * @method is_alive
		*/
		bool is_alive() const;

		/**
		 * @overwrite
		*/
		void post_message(Cb cb) override;

		/**
		 * Synchronously post a message callback to the run loop
		 *
		 * @note Circular calls by the same thread lead to deadlock
		*/
		void post_sync(Callback<PostSyncData> cb);

		/**
		* @method post(cb) post message
		* @param alwaysPost always post msg to queue
		*/
		void post(Cb cb, bool alwaysPost = false);

		/**
		 * @method timer() start timer and return handle id
		 * @param time time unit is millisecond
		 * @param repeat repeat < 0 the always repeating
		*/
		uint32_t timer(Cb cb, uint64_t timeMs, int64_t repeat = 0);

		/**
		 * @method timer_stop timer stop 
		*/
		void timer_stop(uint32_t id);

		/**
		 * @method work(cb[,done])
		*/
		uint32_t work(Cb cb, Cb done = 0);

		/**
		* @method work_cancel(id)
		*/
		void work_cancel(uint32_t id);

		/**
		 * @method tick() tick check, repeat < 0 the always repeating
		*/
		uint32_t tick(Cb cb, int64_t repeat = 0);

		/**
		 * @method tick_stop tick stop
		*/
		void tick_stop(uint32_t id);

		/**
		 * Running the message loop
		*/
		void run();

		/**
		 * Stop running message loop
		 *
		 * @note When calling from multiple threads, please make sure that the `RunLoop` handle is still valid
		 * before calling, and make sure that `KeepLoop` has been released,
		 * and `keep_count` will be checked when `RunLoop` is destroyed
		*/
		void stop();

		/**
		 * Returns the libuv C library uv loop object for current run loop
		*/
		inline uv_loop_t* uv_loop() { return _uv_loop; }

		/**
		 * Returns the thread id for run loop
		 */
		inline ThreadID thread_id() const { return _tid; }

		/**
		 * Returns the run loop for current thrend
		*/
		static RunLoop* current();

		/**
		 * Returns the process first quark run loop
		 *
		 * @note Be careful with thread safety. It's best to ensure that `current()` has been invoked first.
		*/
		static RunLoop* first();

		/**
		 * Return if current is the first run loop of the process
		 */
		static bool is_first();

		/**
		* @method clear(), immediately stop all timer and msg,
		* only allowed to be called on the self thread
		*/
		void clear();

	private:
		/**
		 * @note Privately construct each thread,
		 * only one can be created and the current entity can be obtained through `RunLoop::current()`
		 * @constructor
		*/
		RunLoop(uv_loop_t* uv);
		~RunLoop();

		struct Msg { uv_timer_t *timer; Cb cb; };
		struct work_t;
		struct check_t;
		List<Msg>       _msg;
		Dict<uint32_t, uv_timer_t*> _timer;
		Dict<uint32_t, check_t*> _check;
		Dict<uint32_t, work_t*> _work;
		Mutex       _mutex;
		Thread*     _thread;
		ThreadID    _tid;
		uv_loop_t*  _uv_loop;
		uv_async_t* _uv_async;
		uv_timer_t* _uv_timer;

		Qk_DEFINE_INLINE_CLASS(Inl);
	};

	inline RunLoop* current_loop() {
		return RunLoop::current();
	}

	inline RunLoop* first_loop() {
		return RunLoop::first();
	}

	Qk_EXPORT void check_is_first_loop();
}
#endif
