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

#ifndef __quark__util__loop__
#define __quark__util__loop__

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
	class KeepLoop;

	typedef std::thread::id         ThreadID;
	typedef std::mutex              Mutex;
	typedef std::recursive_mutex    RecursiveMutex;
	typedef std::lock_guard<Mutex>  ScopeLock;
	typedef std::unique_lock<Mutex> Lock;
	typedef std::condition_variable Condition;

	template<> Qk_EXPORT uint64_t Compare<ThreadID>::hashCode(const ThreadID& key);

	struct Thread;
	struct CondMutex {
		Mutex     mutex;
		Condition cond;
		void lock_wait_for(uint64_t timeoutUs = 0);
		void lock_notify_one();
		void lock_notify_all();
	};

	Qk_EXPORT ThreadID thread_new(void exec(void* arg), void* arg = nullptr, cString& tag = String());
	Qk_EXPORT ThreadID thread_new(std::function<void()> func, cString& tag = String());
	//!< sleep The current thread cannot be awakened
	Qk_EXPORT void     thread_sleep(uint64_t timeoutUs = 0);
	//!< Pause the current operation can be awakened by 'resume()'
	Qk_EXPORT void     thread_pause(uint64_t timeoutUs = 0 /*Less than 1 permanent wait*/);
	Qk_EXPORT void     thread_resume(ThreadID id, int abort = 0); //!< resume thread running and send abort signal
	Qk_EXPORT void     thread_abort(ThreadID id); //!< send abort to run loop, signal=-1
	//!< wait for the target 'id' thread to end, param `timeoutUs` less than 1 permanent wait
	Qk_EXPORT void     thread_join_for(ThreadID id, uint64_t timeoutUs = 0);
	Qk_EXPORT void     thread_try_abort_and_exit(int exit_rc); //!< try abort all run loop, signal=-2
	Qk_EXPORT ThreadID thread_self_id();

	Qk_EXPORT EventNoticer<Event<>, Mutex>& onProcessExit();

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
		Qk_HIDDEN_ALL_COPY(RunLoop);
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
		 * Synchronously post a message callback to the run loop
		 *
		 * @note Circular calls by the same thread lead to deadlock
		*/
		void post_sync(Callback<PostSyncData> cb);

		/**
		* @func post(cb) post message
		*/
		void post(Cb cb);

		/**
		 * @method timer() start timer and return handle id
		*/
		uint32_t timer(Cb cb, uint64_t timeUs, uint64_t repeat = 0);

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
		 * @method next_tick() next tick check
		*/
		void next_tick(Cb cb);

		/**
		 * @overwrite
		*/
		virtual void post_message(Cb cb);

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
		 * Keep the running state and return a proxy object, as long as you don't delete `KeepLoop` or call `stop()`,
		 * the message queue will always remain in the running state
		*/
		KeepLoop* keep_alive(void (*check)(void *ctx) = 0, void* ctx = 0);

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
		 * is it the current run loop
		*/
		static bool is_current(RunLoop* loop);

		/**
		 * Returns the process first main run loop
		 *
		 * @note Be careful with thread safety. It's best to ensure that `current()` has been invoked first.
		*/
		static RunLoop* first();

	private:
		/**
		 * @note Privately construct each thread,
		 * only one can be created and the current entity can be obtained through `RunLoop::current()`
		 * @constructor
		*/
		RunLoop(Thread* t, uv_loop_t* uv);

		/**
		 * @destructor
		*/
		virtual ~RunLoop();

		struct Msg {
			Cb cb;
			uv_timer_t* timer;
		};
		struct Work;
		List<KeepLoop*> _keep;
		List<Msg>       _msg;
		Dict<uint32_t, Work*> _work;
		Dict<uint32_t, uv_timer_t*> _timer;
		Mutex       _mutex;
		Thread*     _thread;
		ThreadID    _tid;
		uv_loop_t*  _uv_loop;
		uv_async_t* _uv_async;
		uv_timer_t* _uv_timer;

		Qk_DEFINE_INLINE_CLASS(Inl);
	};

	/**
	 * This object keeps the RunLoop loop from automatically terminating unless `RunLoop::stop()` is called
	 *
	 * @class KeepLoop
	 */
	class Qk_EXPORT KeepLoop: public PostMessage {
	public:
		virtual ~KeepLoop() = default;
		RunLoop* loop();
	};

	inline RunLoop* current_loop() {
		return RunLoop::current();
	}

	inline RunLoop* first_loop() {
		return RunLoop::first();
	}
}
#endif
