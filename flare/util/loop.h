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

#ifndef __flare__util__loop__
#define __flare__util__loop__

#include "./util.h"
#include "./cb.h"
#include "./thread.h"
#include "./list.h"
#include <functional>

typedef struct uv_loop_s uv_loop_t;
typedef struct uv_async_s uv_async_t;
typedef struct uv_timer_s uv_timer_t;

namespace flare {

	class KeepLoop;

	/**
	* @class PostMessage
	*/
	class F_EXPORT PostMessage {
	 public:
		virtual uint32_t post_message(Cb cb, uint64_t delay_us = 0) = 0;
	};

	/**
	* @class RunLoop
	*/
	class F_EXPORT RunLoop: public Object, public PostMessage {
		F_HIDDEN_ALL_COPY(RunLoop);
	 public:
		
		/**
		* @func runing()
		*/
		bool runing() const;
		
		/**
		* @func is_alive
		*/
		bool is_alive() const;
		
		/**
		* @func post(cb[,delay_us]) post message and setting delay
		*/
		uint32_t post(Cb cb, uint64_t delay_us = 0);

		class PostSyncData: public Object {
		 public:
			virtual void complete() = 0;
		};

		/**
		* @func post_sync(cb) TODO: 线程循环调用导致锁死
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
		* @func run() 运行消息循环
		* @arg [timeout=0] {int64_t} 超时时间(微妙us),等于0没有新消息立即结束
		*/
		void run(uint64_t timeout = 0);
		
		/**
		* @func stop() 停止循环
		* TODO 同由其在多线程中调用时,调用前请确保`RunLoop`句柄是否还有效,
		*   并且确保`KeepLoop`都已释放,`RunLoop`在销毁时会检查`keep_count`
		*/
		void stop();

		/**
		* @func work(cb[,done[,name]])
		*/
		uint32_t work(Cb cb, Cb done = 0, cString& name = String());
		
		/**
		* @func cancel_work(id)
		*/
		void cancel_work(uint32_t id);

		/**
		* 保持run状态并返回一个代理对像,只要不删除`KeepLoop`或调用`stop()`消息队列会一直保持run状态
		* @func keep_alive(declear)
		* @arg name {cString&} 名称
		* @arg [declear=true] {bool} KeepLoop 释放时是否清理由keekloop发起并未完成的`post`消息
		*/
		KeepLoop* keep_alive(cString& name, bool declear = true);

		/**
		* @func uv_loop()
		*/
		inline uv_loop_t* uv_loop() { return _uv_loop; }

		/**
		* @func thread_id()
		*/
		inline ThreadID thread_id() const { return _tid; }
		
		/**
		* @func current() 获取当前线程消息队列
		*/
		static RunLoop* current();
		
		/**
		* @func keep_alive_current(declear) 保持当前循环活跃并返回`KeepLoop`实体
		*/
		static KeepLoop* keep_alive_current(cString& name, bool declear = true);
		
		/**
		* @func next_tick
		*/
		static void next_tick(Cb cb) throw(Error);
		
		/**
		* Be careful with thread safety. It's best to ensure that `current()` has been invoked first.
		* @func main_loop()
		*/
		static RunLoop* main_loop();
		
		/**
		* @func is_main_loop() 当前线程是为主循环
		*/
		static bool is_main_loop();

		/**
		* @func stop() 停止循环
		*/
		static void stop(ThreadID id);

		/**
		* @func is_alive()
		*/
		static bool is_alive(ThreadID id);
		
	 private:
		/**
		* @constructor 私有构造每个线程只能创建一个通过`current()`来获取当前实体
		*/
		RunLoop(Thread* t);
		/**
		* @destructor
		*/
		virtual ~RunLoop();
		
		F_DEFINE_INLINE_CLASS(Inl);
		F_DEFINE_INLINE_CLASS(Inl2);
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
		RecursiveMutex* _independent_mutex;
		Mutex       _mutex;
		Thread*     _thread;
		ThreadID    _tid;
		uv_loop_t*  _uv_loop;
		uv_async_t* _uv_async;
		uv_timer_t* _uv_timer;
		int64_t     _timeout, _record_timeout;
	};

	/**
	* 这个对像能保持RunLoop的循环不自动终止,除非调用`RunLoop::stop()`
	* @class KeepLoop
	*/
	class F_EXPORT KeepLoop: public Object, public PostMessage {
		F_HIDDEN_ALL_COPY(KeepLoop);
	 public:
		F_DEFAULT_ALLOCATOR();
		/**
		* @destructor `destructor_clear=true`时会取消通过它`post`的所有消息
		*/
		virtual ~KeepLoop();
		/**
		* @func post_message(cb[,delay_us])
		*/
		virtual uint32_t post_message(Cb cb, uint64_t delay_us = 0);
		/**
		* @func post(cb[,delay_us])
		*/
		uint32_t post(Cb cb, uint64_t delay_us = 0);
		/**
		* @func cancel_all() 取消之前`post`的所有消息
		*/
		void cancel_all();
		/**
		* @func cancel(id)
		*/
		void cancel(uint32_t id);
		/**
		* @func host() 如果目标线程已经结束会返回`nullptr`
		*/
		inline RunLoop* host() { return _loop; }

	 private:
		typedef List<KeepLoop*>::Iterator Iterator;
		/**
		* @constructor `declear=true`时表示析构时会进行清理
		*/
		KeepLoop(cString& name, bool destructor_clear);
		RunLoop*  _loop;
		uint32_t  _group;
		Iterator  _id;
		String    _name;
		bool      _declear;
		friend class RunLoop;
	};

}
#endif
