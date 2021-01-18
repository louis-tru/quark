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

#ifndef __ftr__util__loop__
#define __ftr__util__loop__

#include <ftr/util/util.h>
#include <ftr/util/event.h>
#include <ftr/util/cb.h>
#include <map>
#include <list>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

typedef struct uv_loop_s uv_loop_t;
typedef struct uv_async_s uv_async_t;
typedef struct uv_timer_s uv_timer_t;

namespace ftr {

	typedef std::thread::id ThreadID;
	typedef std::mutex Mutex;
	typedef std::recursive_mutex RecursiveMutex;
	typedef std::lock_guard<Mutex> ScopeLock;
	typedef std::unique_lock<Mutex> Lock;
	typedef std::condition_variable Condition;

	class RunLoop;
	class KeepLoop;

	/**
	* @class Thread
	*/
	class FX_EXPORT Thread {
			FX_HIDDEN_ALL_COPY(Thread);
		public:
			typedef ThreadID ID;
			typedef NonObjectTraits Traits;
			typedef std::function<int(Thread&)> Exec;
			inline bool is_abort() const { return _abort; }
			inline ID id() const { return _id; }
			inline String name() const { return _name; }
			inline RunLoop* loop() const { return _loop; }
			static ID spawn(Exec exec, const String& name);
			static ID current_id();
			static Thread* current();
			static void sleep(int64_t timeoutUs = 0 /*小于1永久等待*/);
			static void join(ID id, int64_t timeoutUs = 0 /*小于1永久等待*/);
			static void awaken(ID id);
			static void abort(ID id);
			static EventNoticer<>& onProcessSafeExit();
		private:
			Thread();
			~Thread();
			FX_DEFINE_INLINE_CLASS(Inl);
			bool  _abort;
			Mutex _mutex;
			Condition _cond;
			ID    _id;
			SString  _name;
			void* _data[256];
			RunLoop* _loop;
	};

	/**
	* @class PostMessage
	*/
	class FX_EXPORT PostMessage {
		public:
			virtual uint32_t post_message(cCb& cb, uint64_t delay_us = 0) = 0;
	};

	/**
	* @class RunLoop
	*/
	class FX_EXPORT RunLoop: public Object, public PostMessage {
			FX_HIDDEN_ALL_COPY(RunLoop);
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
			uint32_t post(cCb& cb, uint64_t delay_us = 0);

			class PostSyncData: public Object {
				public:
				virtual void complete() = 0;
			};

			/**
			* @func post_sync(cb) TODO: 线程循环调用导致锁死
			*/
			void post_sync(const Callback<PostSyncData>& cb);

			/**
			* @overwrite
			*/
			virtual uint32_t post_message(cCb& cb, uint64_t delay_us = 0);
			
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
			uint32_t work(cCb& cb, cCb& done = 0, const String& name = String());
			
			/**
			* @func cancel_work(id)
			*/
			void cancel_work(uint32_t id);

			/**
			* 保持run状态并返回一个代理对像,只要不删除`KeepLoop`或调用`stop()`消息队列会一直保持run状态
			* @func keep_alive(declear)
			* @arg name {const String&} 名称
			* @arg [declear=true] {bool} KeepLoop 释放时是否清理由keekloop发起并未完成的`post`消息
			*/
			KeepLoop* keep_alive(const String& name, bool declear = true);

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
			static KeepLoop* keep_alive_current(const String& name, bool declear = true);
			
			/**
			* @func next_tick
			*/
			static void next_tick(cCb& cb) throw(Error);
			
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
			
			FX_DEFINE_INLINE_CLASS(Inl);
			FX_DEFINE_INLINE_CLASS(Inl2);
			friend class KeepLoop;
			struct Queue {
				uint32_t id, group;
				int64_t time;
				Callback<> resolve;
			};
			struct Work;
			std::list<Queue>     _queue;
			std::list<Work*>     _works;
			std::list<KeepLoop*> _keeps;
			RecursiveMutex*      _independent_mutex;
			Mutex                _mutex;
			Thread*              _thread;
			ThreadID             _tid;
			uv_loop_t*           _uv_loop;
			uv_async_t*          _uv_async;
			uv_timer_t*          _uv_timer;
			int64_t              _timeout, _record_timeout;
	};

	/**
	* 这个对像能保持RunLoop的循环不自动终止,除非调用`RunLoop::stop()`
	* @class KeepLoop
	*/
	class FX_EXPORT KeepLoop: public Object, public PostMessage {
			FX_HIDDEN_ALL_COPY(KeepLoop);
		public:
			FX_DEFAULT_ALLOCATOR();
			/**
			* @destructor `destructor_clear=true`时会取消通过它`post`的所有消息
			*/
			virtual ~KeepLoop();
			/**
			* @func post_message(cb[,delay_us])
			*/
			virtual uint32_t post_message(cCb& cb, uint64_t delay_us = 0);
			/**
			* @func post(cb[,delay_us])
			*/
			uint32_t post(cCb& cb, uint64_t delay_us = 0);
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
			typedef std::list<KeepLoop*>::iterator Iterator;
			/**
			* @constructor `declear=true`时表示析构时会进行清理
			*/
			KeepLoop(const String& name, bool destructor_clear);
			RunLoop*  _loop;
			uint32_t  _group;
			Iterator  _id;
			String    _name;
			bool      _declear;
			friend class RunLoop;
	};

}
#endif
