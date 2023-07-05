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

namespace qk {
	class RunLoop;
	class KeepLoop;

	typedef std::thread::id         ThreadID;
	typedef std::mutex              Mutex;
	typedef std::recursive_mutex    RecursiveMutex;
	typedef std::lock_guard<Mutex>  ScopeLock;
	typedef std::unique_lock<Mutex> Lock;
	typedef std::condition_variable Condition;

	template<> Qk_EXPORT uint64_t Compare<ThreadID>::hashCode(const ThreadID& key);

	struct Thread {
		ThreadID id;
		String   tag;
		int      abort; // abort signal
	};

	struct Wait {
		Mutex     mutex;
		Condition cond;
		void wait_for(uint64_t timeoutUs = 0);
		void notify_one();
		void notify_all();
	};

	Qk_EXPORT ThreadID thread_fork(void exec(void* arg), void* arg = nullptr, cString& tag = String());
	Qk_EXPORT ThreadID thread_fork(std::function<void()> func, cString& tag = String());
	//!< sleep The current thread cannot be awakened
	Qk_EXPORT void     thread_sleep(uint64_t timeoutUs = 0);
	//!< Pause the current operation can be awakened by 'resume()'
	Qk_EXPORT void     thread_pause(uint64_t timeoutUs = 0 /*Less than 1 permanent wait*/);
	Qk_EXPORT void     thread_resume(ThreadID id, int abort = 0); //!< resume thread running and send abort signal
	Qk_EXPORT void     thread_abort(ThreadID id); //!< send abort, signal=-1
	//!< Wait for the target 'id' thread to end, param `timeoutUs` less than 1 permanent wait
	Qk_EXPORT void     thread_wait_for(ThreadID id, uint64_t timeoutUs = 0);
	Qk_EXPORT void     thread_try_abort_and_exit(int exit_rc); //!< try abort, signal=-2

	Qk_EXPORT ThreadID      thread_current_id();
	Qk_EXPORT const Thread* thread_current();

	Qk_EXPORT EventNoticer<Event<>, Mutex>& onExit();

	/**
	* @class PostMessage
	*/
	class Qk_EXPORT PostMessage {
	public:
		virtual uint32_t post_message(Cb cb, uint64_t delay_us = 0) = 0;
	};

	/**
	* @class RunLoop
	*/
	class Qk_EXPORT RunLoop: public Object, public PostMessage {
		Qk_HIDDEN_ALL_COPY(RunLoop);
	public:
		
		/**
		 * @method runing()
		*/
		bool runing() const;
		
		/**
		 * @method is_alive
		*/
		bool is_alive() const;
		
		/**
		* @func post(cb[,delay_us]) post message and setting delay
		*/
		uint32_t post(Cb cb, uint64_t delay_us = 0);

		/**
		 * @class PostSyncData
		*/
		class PostSyncData: public Object {
		public:
			virtual void complete() = 0;
		};

		/**
		 * Synchronously post a message callback to the run loop
		 *
		 * @note Circular calls by the same thread lead to deadlock
		*/
		void post_sync(Callback<PostSyncData> cb);

		/**
		 * @overwrite
		*/
		virtual uint32_t post_message(Cb cb, uint64_t delay_us = 0);
		
		/**
		* @func cancel(id) cancel message with id
		*/
		void cancel(uint32_t id);
		
		/**
		 * Running the message loop
		 * @arg [timeout=0] {uint64_t} Timeout (subtle us), when it is equal to 0, no new message will end immediately
		*/
		void run(uint64_t timeout = 0);
		
		/**
		 * Stop running message loop
		 *
		 * @note When calling from multiple threads, please make sure that the `RunLoop` handle is still valid
		 * before calling, and make sure that `KeepLoop` has been released,
		 * and `keep_count` will be checked when `RunLoop` is destroyed
		*/
		void stop();

		/**
		 * @method work(cb[,done[,name]])
		*/
		uint32_t work(Cb cb, Cb done = 0, cString& name = String());
		
		/**
		* @func cancel_work(id)
		*/
		void cancel_work(uint32_t id);

		/**
		 * Keep the running state and return a proxy object, as long as you don't delete `KeepLoop` or call `stop()`,
		 * the message queue will always remain in the running state
		 *
		 * @arg name {cString&} alias
		 * @arg [clean=true] {bool} Whether to clean up the unfinished `post` message initiated by keekloop when releasing
		*/
		KeepLoop* keep_alive(cString& name = String(), bool clean = true);

		/**
		 * Returns the libuv C library uv loop object for current run loop
		*/
		inline uv_loop_t* uv_loop() { return _uv_loop; }
		
		/**
		 * Returns the thread id for run loop
		 */
		inline ThreadID thread_id() const { return _tid; }

		/**
		 * Returns the thread object for run loop
		 */
		inline const Thread* thread() const { return _thread; }
		
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
		
		Qk_DEFINE_INLINE_CLASS(Inl);

		friend class KeepLoop;
		struct Queue {
			uint32_t id, group;
			int64_t time;
			Cb resolve;
		};
		struct Work;
		List<Queue>     _queue;
		List<Work*>     _works;
		List<KeepLoop*> _keeps;
		Mutex       _mutex;
		Thread*     _thread;
		ThreadID    _tid;
		uv_loop_t*  _uv_loop;
		uv_async_t* _uv_async;
		uv_timer_t* _uv_timer;
		int64_t     _timeout, _record_timeout;
	};

	/**
	 * This object keeps the RunLoop loop from automatically terminating unless `RunLoop::stop()` is called
	 *
	 * @class KeepLoop
	 */
	class Qk_EXPORT KeepLoop: public PostMessage {
		Qk_HIDDEN_ALL_COPY(KeepLoop);
	public:
		/**
		 * @destructor `destructor_clear=true` will cancel all messages `post` through it
		*/
		virtual ~KeepLoop();
		/**
		 * @func post_message(cb[,delay_us])
		*/
		virtual uint32_t post_message(Cb cb, uint64_t delay_us = 0);
		/**
		 * @method post(cb[,delay_us])
		*/
		uint32_t post(Cb cb, uint64_t delay_us = 0);
		/**
		 * @method cancel_all() Cancel all messages from the previous `post`
		*/
		void cancel_all();
		/**
		 * @method cancel(id)
		*/
		void cancel(uint32_t id);
		/**
		 * @method host() Returns `nullptr` if the target thread has ended
		*/
		inline RunLoop* host() { return _loop; }

	private:
		typedef List<KeepLoop*>::Iterator Iterator;
		/**
		* @constructor `declear=true` means that it will be cleaned up when it is destructed
		*/
		KeepLoop(cString& name, bool destructor_clean);
		RunLoop*  _loop;
		uint32_t  _group;
		Iterator  _id;
		String    _name;
		bool      _de_clean;
		friend class RunLoop;
	};

}
#endif
