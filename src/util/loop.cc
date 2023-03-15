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

#include "./loop.h"
#include "./list"
#include "./dict.h"
#include <uv.h>
#include <pthread.h>

#ifndef Qk_ATEXIT_WAIT_TIMEOUT
# define Qk_ATEXIT_WAIT_TIMEOUT 1e6
#endif

namespace qk {
	// ---------------------------------------------------------

	template<> uint64_t Compare<ThreadID>::hash_code(const ThreadID& key) {
		return *reinterpret_cast<const uint32_t*>(&key);
	}

	struct Thread_INL: Thread, Wait {
		RunLoop*    _loop;
		List<Wait*> _wait_ends; // external wait thread end
		void        (*_exec)(void* arg);
	};

	static RunLoop *__first_loop = nullptr;
	static Mutex *__threads_mutex = nullptr;
	static Dict<ThreadID, Thread_INL*> *__threads = nullptr;
	static pthread_key_t __specific_key = 0;
	static std::atomic_int __is_process_exit(0);
	static EventNoticer<Event<>, Mutex> *__on_process_safe_exit = nullptr;

	void Wait::wait_for(uint64_t timeoutUs) {
		Lock lock(mutex);
		if (timeoutUs) {
			cond.wait_for(lock, std::chrono::microseconds(timeoutUs));
		} else {
			cond.wait(lock);
		}
	}

	void Wait::notify_one() {
		ScopeLock scope(mutex);
		cond.notify_one();
	}

	void Wait::notify_all() {
		ScopeLock scope(mutex);
		cond.notify_all();
	}

	// --------------------------------- T H R E A D ---------------------------------

	static Thread_INL* Thread_INL_init(Thread_INL *t, void (*exec)(void* arg), cString& tag)
	{
		t->tag = tag;
		t->abort = 0;
		t->_loop = nullptr;
		t->_exec = exec;
		return t;
	}

	static void thread_destructor(void* ptr) {
		auto thread = reinterpret_cast<Thread_INL*>(ptr);
		Release(thread->_loop);
		delete thread;
	}

	static void thread_set_specific_data(Thread* thread) {
		Qk_ASSERT(!pthread_getspecific(__specific_key));
		pthread_setspecific(__specific_key, thread);
	}

	ThreadID thread_current_id() {
		return std::this_thread::get_id();
	}

	const Thread* thread_current() {
		return reinterpret_cast<const Thread*>(pthread_getspecific(__specific_key));
	}

	static Thread_INL* thread_current_inl() {
		return reinterpret_cast<Thread_INL*>(pthread_getspecific(__specific_key));
	}

	ThreadID thread_fork(void (*exec)(void* arg), void* arg, cString& name) {
		if ( __is_process_exit != 0 )
			return ThreadID();
		Thread_INL* thread = Thread_INL_init(new Thread_INL, exec, name);

		ThreadID id;
		uv_thread_create((uv_thread_t*)&id, [](void* arg) {
			auto thread = (Thread_INL*)arg;
#if Qk_ANDROID
			JNI::ScopeENV scope;
#endif
			thread_set_specific_data(thread);
			if ( !thread->abort ) {
				thread->_exec(arg);
			}
			{ // delete global handle
				ScopeLock scope(*__threads_mutex);
				__threads->erase(thread->id);
			}
			{ // notify wait
				ScopeLock scope(thread->mutex);
				if (!thread->abort)
					thread->abort = -3; // abort
				Qk_DEBUG("Thread end ..., %s", thread->tag.c_str());
				for (auto& i : thread->_wait_ends)
					i->notify_one();
				Qk_DEBUG("Thread end  ok, %s", thread->tag.c_str());
			}
		}, thread);

		if (id != ThreadID()) {
			ScopeLock scope(*__threads_mutex);
			__threads->set(thread->id = id, thread);
		} else { // fail
			delete thread;
		}
		return id;
	}

	typedef std::function<void()> ForkFunc;

	ThreadID thread_fork(ForkFunc func, cString& name) {
		auto funcp = new ForkFunc(func);
		return thread_fork([](void* arg) {
			std::unique_ptr<ForkFunc> f( (ForkFunc*)arg );
			(*f)();
		}, funcp, name);
	}

	void thread_sleep(uint64_t timeoutUs) {
		std::this_thread::sleep_for(std::chrono::microseconds(timeoutUs));
	}

	void thread_pause(uint64_t timeoutUs) {
		auto t = thread_current_inl();
		Qk_ASSERT(t, "Cannot find current qk::Thread handle, use Thread::sleep()");
		t->wait_for(timeoutUs);
	}

	static void thread_resume_inl(Thread_INL *t, int abort) {
		ScopeLock scope(t->mutex);
		if (abort) {
			if (t->_loop)
				t->_loop->stop();
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

	void thread_wait_for(ThreadID id, uint64_t timeoutUs) {
		if (id == thread_current_id()) {
			Qk_DEBUG("thread_wait_for(), cannot wait self thread");
			return;
		}
		Lock lock(*__threads_mutex);
		auto i = __threads->find(id);
		if ( i != __threads->end() ) {
			i->value->mutex.lock();
			lock.unlock();
			Wait wait;
			auto &tag = i->value->tag;
			auto it = i->value->_wait_ends.push_back(&wait);
			i->value->mutex.unlock();
			Qk_DEBUG("thread_wait_for(), ..., %p, %s", id, *tag);
			wait.wait_for(timeoutUs); // permanent wait
			Qk_DEBUG("thread_wait_for(), end, %p, %s", id, *tag);
		}
	}

	static void thread_try_abort_all(int rc) {
		if (__is_process_exit++)
			return; // exit

		Array<ThreadID> threads_id;

		Qk_DEBUG("thread_try_abort_and_exit_inl(), 0");
		Event<> ev(Int32(rc), nullptr, rc);
		Qk_Trigger(Exit, ev);
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
			// Wait for the end of this thread here, this time defaults to 1 second
			Qk_DEBUG("thread_try_abort_and_exit_inl,join, %p", i);
			thread_wait_for(i, Qk_ATEXIT_WAIT_TIMEOUT); // wait 1s
		}

		Qk_DEBUG("thread_try_abort_and_exit_inl() 2");
	}

	void thread_try_abort_and_exit(int rc) {
		if (!__is_process_exit) {
			thread_try_abort_all(rc);
			::exit(rc);
		}
	}

	EventNoticer<Event<>, Mutex>& onExit() {
		return *__on_process_safe_exit;
	}

	Qk_INIT_BLOCK(thread_init_once) {
		Qk_DEBUG("thread_init_once");
		__threads = new Dict<ThreadID, Thread_INL*>();
		__threads_mutex = new Mutex();
		__on_process_safe_exit = new EventNoticer<Event<>, Mutex>(nullptr);
		//Qk_DEBUG("sizeof EventNoticer<Event<>, Mutex>,%d", sizeof(EventNoticer<Event<>, Mutex>));
		//Qk_DEBUG("sizeof EventNoticer<>,%d", sizeof(EventNoticer<>));
		atexit([](){ thread_try_abort_all(0); });
		int err = pthread_key_create(&__specific_key, thread_destructor);
		Qk_ASSERT(err == 0);
	}

	// --------------------- R u n L o o p ---------------------

	Qk_DEFINE_INLINE_MEMBERS(RunLoop, Inl) {
		#define _inl(self) static_cast<RunLoop::Inl*>(self)
	public:

		void stop_after_print_message();

		bool is_alive() {
			// _uv_async 外是否还有活着的`handle`与请求
			// return _uv_loop->active_handles > 1 ||
			// 				QUEUE_EMPTY(&(_uv_loop)->active_reqs) == 0 ||
			// 				_uv_loop->closing_handles != NULL;
			return uv_loop_alive(_uv_loop);
		}
		
		void close_uv_req_handles() {
			if (!uv_is_closing((uv_handle_t*)_uv_async))
				uv_close((uv_handle_t*)_uv_async, nullptr); // close async
			uv_timer_stop(_uv_timer);
			if (!uv_is_closing((uv_handle_t*)_uv_timer))
				uv_close((uv_handle_t*)_uv_timer, nullptr);
		}
		
		static void before_resolve_queue(uv_handle_t* handle) {
			((Inl*)handle->data)->resolve_queue();
		}
		
		inline void uv_timer_req(int64_t timeout_ms) {
			uv_timer_start(_uv_timer, (uv_timer_cb)before_resolve_queue, timeout_ms, 0);
		}

		void after_resolve_queue(int64_t timeout_ms) {
			if (_uv_loop->stop_flag != 0) { // 循环停止标志
				close_uv_req_handles();
			}
			else if (timeout_ms == -1) { // -1 end loop, 没有更多需要处理的工作
				if (_keeps.length() == 0 && _works.length() == 0) {
					// RunLoop 已经没有需要处理的消息
					if (is_alive()) { // 如果uv还有其它活着,那么间隔一秒测试一次
						uv_timer_req(1000);
						_record_timeout = 0; // 取消超时记录
					} else { // 已没有活着的其它uv请求
						if (_record_timeout) { // 如果已开始记录继续等待
							int64_t timeout = (time_monotonic() - _record_timeout - _timeout) / 1000;
							if (timeout >= 0) { // 已经超时
								close_uv_req_handles();
							} else { // 继续等待超时
								uv_timer_req(-timeout);
							}
						} else {
							int64_t timeout = _timeout / 1000;
							if (timeout > 0) { // 需要等待超时
								_record_timeout = time_monotonic(); // 开始记录超时
								uv_timer_req(timeout);
							} else {
								close_uv_req_handles();
							}
						}
					}
				}
			} else {
				if (timeout_ms > 0) { // > 0, delay
					uv_timer_req(timeout_ms);
				} else { // == 0
					/* Do a cheap read first. */
					uv_async_send(_uv_async);
				}
				_record_timeout = 0; // 取消超时记录
			}
		}

		void resolve_queue() {
			List<Queue> queue;
			{
				ScopeLock lock(_mutex);
				if (_queue.length()) {
					queue = std::move(_queue);
				} else {
					after_resolve_queue(-1);
				}
			}

			if (queue.length()) {
				int64_t now = time_monotonic();
				for (auto i = queue.begin(), e = queue.end(); i != e; ) {
					auto t = i++;
					if (now >= t->time) { //
						t->resolve->resolve(this);
						queue.erase(t);
					}
				}
			}

			{
				ScopeLock lock(_mutex);
				_queue.splice(_queue.begin(), queue);
				if (_queue.length() == 0) {
					after_resolve_queue(-1);
				}
				int64_t now = time_monotonic();
				int64_t duration = Int64::limit_max;
				for ( auto& i : _queue ) {
					int64_t du = i.time - now;
					if (du <= 0) {
						duration = 0; break;
					} else if (du < duration) {
						duration = du;
					}
				}
				after_resolve_queue(duration / 1e3);
			}
		}

		/**
		 * @func post()
		 */
		uint32_t post(Cb exec, uint32_t group, uint64_t delay_us) {
			if (_thread->abort) {
				Qk_DEBUG("RunLoop::post, _thread->is_abort() == true");
				return 0;
			}
			ScopeLock lock(_mutex);
			uint32_t id = getId32();
			if (delay_us) {
				int64_t time = time_monotonic() + delay_us;
				_queue.push_back({ id, group, time, exec });
			} else {
				_queue.push_back({ id, group, 0, exec });
			}
			activate(); // 通知继续
			return id;
		}

		void post_sync(Callback<RunLoop::PostSyncData> cb, uint32_t group, uint64_t delay_us) {
			//Qk_ASSERT(!_thread->abort, "RunLoop::post_sync, _thread->abort != 0");
			if (_thread->abort) {
				Qk_DEBUG("RunLoop::post_sync, _thread->abort != 0");
				return;
			}

			struct Data: public RunLoop::PostSyncData {
				virtual void complete() {
					ScopeLock scope(inl->_mutex);
					ok = true;
					cond.notify_all();
				}
				Inl* inl;
				bool ok;
				Condition cond;
			} data;

			Data* datap = &data;
			data.inl = this;
			data.ok = false;

			bool isCur = thread_current_id() == _thread->id;
			if (isCur) { // 立即调用
				cb->resolve(datap);
			}

			Lock lock(_mutex);

			if (!isCur) {
				_queue.push_back({
					0, group, 0,
					Cb([cb, datap, this](Cb::Data& e) {
						cb->resolve(datap);
					})
				});
				activate(); // 通知继续
			}

			while(!data.ok) {
				data.cond.wait(lock); // wait
			}
		}
		
		inline void cancel_group(uint32_t group) {
			ScopeLock lock(_mutex);
			cancel_group_non_lock(group);
		}

		void cancel_group_non_lock(uint32_t group) {
			for (auto i = _queue.begin(), e = _queue.end(); i != e; ) {
				auto j = i++;
				if (j->group == group) {
					// NOTE: 删除时如果析构函数间接调用锁定`_mutex`的函数会锁死线程
					_queue.erase(j);
				}
			}
			activate(); // 通知继续
		}
		
		inline void activate() {
			if (_uv_async)
				uv_async_send(_uv_async);
		}
		
		inline void delete_work(List<Work*>::Iterator it) {
			_works.erase(it);
		}
	};

	/**
	 * @struct RunLoop::Work
	 */
	struct RunLoop::Work {
		typedef NonObjectTraits Traits;
		RunLoop* host;
		uint32_t id;
		List<Work*>::Iterator it;
		Cb work, done;
		uv_work_t uv_req;
		String name;
		static void uv_work_cb(uv_work_t* req) {
			Work* self = (Work*)req->data;
			self->work->resolve(self->host);
		}
		static void uv_after_work_cb(uv_work_t* req, int status) {
			Handle<Work> self = (Work*)req->data;
			self->done_work(status);
		}
		void done_work(int status) {
			_inl(host)->delete_work(it);
			if (UV_ECANCELED != status) { // cancel
				done->resolve(host);
			}
			_inl(host)->activate();
		}
	};

	void RunLoop::Inl::stop_after_print_message() {
		ScopeLock lock(_mutex);
		for (auto& i: _keeps) {
			Qk_DEBUG("Print: RunLoop keep not release \"%s\"", i->_name.c_str());
		}
		for (auto& i: _works) {
			Qk_DEBUG("Print: RunLoop work not complete: \"%s\"", i->name.c_str());
		}
	}


	/**
	 * @constructor
	 */
	RunLoop::RunLoop(Thread* t, uv_loop_t* uv)
		: _thread(t)
		, _tid(t->id)
		, _uv_loop(uv)
		, _uv_async(nullptr)
		, _uv_timer(nullptr)
		, _timeout(0)
		, _record_timeout(0)
	{
		Qk_ASSERT(!static_cast<Thread_INL*>(t)->_loop);
		// set run loop
		static_cast<Thread_INL*>(t)->_loop = this;
	}

	/**
	 * @destructor
	 */
	RunLoop::~RunLoop() {
		ScopeLock lock(*__threads_mutex);
		Qk_ASSERT(_uv_async == nullptr, "Secure deletion must ensure that the run loop has exited");
		
		{
			ScopeLock lock(_mutex);
			for (auto& i: _keeps) {
				Qk_WARN("RunLoop keep not release \"%s\"", i->_name.c_str());
				i->_loop = nullptr;
			}
			for (auto& i: _works) {
				Qk_WARN("RunLoop work not complete: \"%s\"", i->name.c_str());
				delete i;
			}
		}

		if (__first_loop == this) {
			__first_loop = nullptr;
		}

		if (_uv_loop != uv_default_loop()) {
			uv_loop_delete(_uv_loop);
		}

		// delete run loop
		Qk_ASSERT(static_cast<Thread_INL*>(_thread)->_loop);
		Qk_ASSERT(static_cast<Thread_INL*>(_thread)->_loop == this);
		static_cast<Thread_INL*>(_thread)->_loop = nullptr;
	}

	/**
	 * @func current() 获取当前线程消息队列
	 */
	RunLoop* RunLoop::current() {
		auto t = thread_current_inl();
		if (!t) {
			Qk_WARN("Can't get thread specific data");
			return nullptr;
		}
		auto loop = t->_loop;
		if (!loop) {
			ScopeLock scope(*__threads_mutex);
			if (__first_loop) {
				loop = new RunLoop(t, uv_loop_new());
			} else { // this is main loop
				loop = new RunLoop(t, uv_default_loop());
				__first_loop = loop;
			}
		}
		return loop;
	}

	/**
	 * @func is_current 当前线程是否为第一循环
	 */
	bool RunLoop::is_current(RunLoop* loop) {
		return loop && loop->_tid == thread_current_id();
	}

	/**
	 * @func first();
	 */
	RunLoop* RunLoop::first() {
		// NOTE: 小心线程安全,最好先确保已调用过`current()`
		if (!__first_loop) {
			current();
			Qk_ASSERT(__first_loop); // asset
		}
		return __first_loop;
	}

	/**
	 * NOTE: Be careful about thread security issues
	 * @func runing()
	 */
	bool RunLoop::runing() const {
		return _uv_async;
	}

	/**
	 * NOTE: Be careful about thread security issues
	 * @func is_alive
	 */
	bool RunLoop::is_alive() const {
		return uv_loop_alive(_uv_loop) /*|| _keeps.length() || _works.length()*/;
	}

	/**
	 * NOTE: Be careful about thread security issues
	 * @func sync # 延时
	 */
	uint32_t RunLoop::post(Cb cb, uint64_t delay_us) {
		return _inl(this)->post(cb, 0, delay_us);
	}

	/**
	 * NOTE: Be careful about thread security issues
	 * @func post_sync(cb)
	 */
	void RunLoop::post_sync(Callback<PostSyncData> cb) {
		_inl(this)->post_sync(cb, 0, 0);
	}

	/**
	 * NOTE: Be careful about thread security issues
	 * @func work()
	 */
	uint32_t RunLoop::work(Cb cb, Cb done, cString& name) {
		if (_thread->abort) {
			Qk_DEBUG("RunLoop::work, _thread->abort != 0");
			return 0;
		}

		Work* work = new Work();
		work->id = getId32();
		work->work = cb;
		work->done = done;
		work->uv_req.data = work;
		work->host = this;
		work->name = name;

		post(Cb([work, this](Cb::Data& ev) {
			int r = uv_queue_work(_uv_loop, &work->uv_req,
														Work::uv_work_cb, Work::uv_after_work_cb);
			Qk_ASSERT(!r);
			work->it = _works.push_back(work);
		}));

		return work->id;
	}

	/**
	 * NOTE: Be careful about thread security issues
	 * @func cancel_work(id)
	 */
	void RunLoop::cancel_work(uint32_t id) {
		post(Cb([=](Cb::Data& ev) {
			for (auto& i : _works) {
				if (i->id == id) {
					int r = uv_cancel((uv_req_t*)&i->uv_req);
					Qk_ASSERT(!r);
					break;
				}
			}
		}));
	}

	/**
	 * NOTE: Be careful about thread security issues
	 * @overwrite
	 */
	uint32_t RunLoop::post_message(Cb cb, uint64_t delay_us) {
		return _inl(this)->post(cb, 0, delay_us);
	}

	/**
	 * NOTE: Be careful about thread security issues
	 * @func cancel # 取消同步
	 */
	void RunLoop::cancel(uint32_t id) {
		ScopeLock lock(_mutex);
		for (auto i = _queue.begin(), e = _queue.end(); i != e; i++) {
			if (i->id == id) {
				// NOTE: 删除时如果析构函数间接调用锁定`_mutex`的函数会锁死线程
				_queue.erase(i);
				break;
			}
		}
		_inl(this)->activate();
	}

	/**
	 * @func run() 运行消息循环
	 */
	void RunLoop::run(uint64_t timeout) {
		if (__is_process_exit) {
			Qk_DEBUG("cannot run RunLoop, __is_process_exit != 0");
			return;
		}
		if (_thread->abort) {
			Qk_DEBUG("cannot run RunLoop, _thread->abort != 0");
			return;
		}

		uv_async_t uv_async;
		uv_timer_t uv_timer;
		{ //
			ScopeLock lock(_mutex);
			Qk_ASSERT(!_uv_async, "It is running and cannot be called repeatedly");
			Qk_ASSERT(thread_current_id() == _tid, "Must run on the target thread");
			_timeout = Qk_MAX(timeout, 0);
			_record_timeout = 0;
			_uv_async = &uv_async; uv_async.data = this;
			_uv_timer = &uv_timer; uv_timer.data = this;
			uv_async_init(_uv_loop, _uv_async, (uv_async_cb)(RunLoop::Inl::before_resolve_queue));
			uv_timer_init(_uv_loop, _uv_timer);
			_inl(this)->activate();
		}

		uv_run(_uv_loop, UV_RUN_DEFAULT); // run uv loop
		_inl(this)->close_uv_req_handles();

		{ // loop end
			ScopeLock lock(_mutex);
			_uv_async = nullptr;
			_uv_timer = nullptr;
			_timeout = 0;
			_record_timeout = 0;
		}

		_inl(this)->stop_after_print_message();
	}

	/**
	 * NOTE: Be careful about thread security issues
	 * @func stop
	 */
	void RunLoop::stop() {
		if ( runing() ) {
			post(Cb([this](Cb::Data& se) {
				uv_stop(_uv_loop);
			}));
		}
	}

	/**
	 * NOTE: Be careful about thread security issues
	 * 保持活动状态,并返回一个代理,只要不删除返回的代理对像,消息队列会一直保持活跃状态
	 * @func keep_alive
	 */
	KeepLoop* RunLoop::keep_alive(cString& name, bool clean) {
		ScopeLock lock(_mutex);
		auto keep = new KeepLoop(name, clean);
		keep->_id = _keeps.push_back(keep);
		keep->_loop = this;
		return keep;
	}

	// ************** KeepLoop **************

	KeepLoop::KeepLoop(cString& name, bool clean)
		: _group(getId32()), _name(name), _de_clean(clean) {
	}

	KeepLoop::~KeepLoop() {
		ScopeLock lock(*__threads_mutex);

		if (_loop) {
			ScopeLock lock(_loop->_mutex);
			if ( _de_clean ) {
				_inl(_loop)->cancel_group_non_lock(_group);
			}
			Qk_ASSERT(_loop->_keeps.length());

			_loop->_keeps.erase(_id); // 减少一个引用计数

			if (_loop->_keeps.length() == 0 && !_loop->_uv_loop->stop_flag) { // 可以结束了
				_inl(_loop)->activate(); // 激活循环状态,不再等待
			}
		} else {
			Qk_DEBUG("Keep already invalid \"%s\", RunLoop already stop and release", *_name);
		}
	}

	uint32_t KeepLoop::post(Cb exec, uint64_t delay_us) {
		// NOTE: Be careful about thread security issues
		if (_loop)
			return _inl(_loop)->post(exec, _group, delay_us);
		else
			return 0;
	}

	uint32_t KeepLoop::post_message(Cb cb, uint64_t delay_us) {
		// NOTE: Be careful about thread security issues
		if (_loop)
			return _inl(_loop)->post(cb, _group, delay_us);
		else
			return 0;
	}

	void KeepLoop::cancel_all() {
		// NOTE: Be careful about thread security issues
		if (_loop)
			_inl(_loop)->cancel_group(_group); // abort all
	}

	void KeepLoop::cancel(uint32_t id) {
		// NOTE: Be careful about thread security issues
		if (_loop)
			_loop->cancel(id);
	}

}

