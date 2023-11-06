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

	template<> uint64_t Compare<ThreadID>::hashCode(const ThreadID& key) {
		return *reinterpret_cast<const uint32_t*>(&key);
	}

	struct Thread {
		int      abort; // abort signal of run loop
		ThreadID id;
		String   tag;
	};

	struct Thread_INL: Thread, CondMutex {
		RunLoop*    _loop;
		List<CondMutex*> _waitSelfEnd; // external wait thread end
		void        (*_exec)(void* arg);
		void        *_arg;
	};

	static RunLoop *__first_loop = nullptr;
	static Mutex *__threads_mutex = nullptr;
	static Dict<ThreadID, Thread_INL*> *__threads = nullptr;
	static pthread_key_t __specific_key = 0;
	static std::atomic_int __is_process_exit(0);
	static EventNoticer<Event<>, Mutex> *__on_process_safe_exit = nullptr;

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

	// --------------------------------- T H R E A D ---------------------------------

	static Thread_INL* Thread_INL_init(
		Thread_INL *t, cString& tag, void *arg,
		void (*exec)(void* arg)
	) {
		t->tag = tag;
		t->abort = 0;
		t->_loop = nullptr;
		t->_exec = exec;
		t->_arg = arg;
		return t;
	}

	static void thread_set_specific_data(Thread *t) {
		Qk_ASSERT(!pthread_getspecific(__specific_key));
		pthread_setspecific(__specific_key, t);
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

	ThreadID thread_new(void (*exec)(void* arg), void* arg, cString& tag) {
		if ( __is_process_exit != 0 ) {
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
				thread->_exec(thread->_arg);
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
				Release(thread->_loop); // release loop object
				Qk_DEBUG("Thread end ..., %s", thread->tag.c_str());
				for (auto& i : thread->_waitSelfEnd) {
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

	void thread_join_for(ThreadID id, uint64_t timeoutUs) {
		if (id == thread_current_id()) {
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
			t->_waitSelfEnd.pushBack(&wait);
			t->mutex.unlock();
			Qk_DEBUG("thread_join_for(), ..., %p, %s", id, *t->tag);
			wait.lock_wait_for(timeoutUs); // permanent wait
			Qk_DEBUG("thread_join_for(), end, %p, %s", id, *t->tag);
		}
	}

	static void thread_try_abort_all(int rc) {
		if (__is_process_exit++)
			return; // exit

		Array<ThreadID> threads_id;

		Qk_DEBUG("thread_try_abort_and_exit_inl(), 0");
		Event<> ev(Int32(rc), nullptr, rc);
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
		if (!__is_process_exit) {
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

	// --------------------- R u n L o o p ---------------------

	Qk_DEFINE_INLINE_MEMBERS(RunLoop, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<RunLoop::Inl*>(self)

		static void before_resolve_msg(uv_handle_t* handle) {
			_inl(handle->data)->resolve_msg();
		}

		void stop_after_print_message();

		bool is_alive() {
			return uv_loop_alive(_uv_loop);
		}

		void close_uv_req_handles() {
			if (!uv_is_closing((uv_handle_t*)_uv_async)) {
				uv_close((uv_handle_t*)_uv_async, nullptr); // close async
			}
			uv_timer_stop(_uv_timer);
			if (!uv_is_closing((uv_handle_t*)_uv_timer)) {
				uv_close((uv_handle_t*)_uv_timer, nullptr);
			}
		}

		void timer_req(int64_t timeout_ms) {
			uv_timer_start(_uv_timer, (uv_timer_cb)before_resolve_msg, timeout_ms, 0);
		}

		void after_resolve_msg(int64_t timeout_ms) {
			if (_uv_loop->stop_flag != 0) { // 循环停止标志
				close_uv_req_handles();
			}
			else if (timeout_ms == -1) { // -1 end loop, 没有更多需要处理的工作
				if (_keep.length() == 0 && _work.length() == 0) {
					// RunLoop 已经没有需要处理的消息
					if (is_alive()) { // 如果uv还有其它活着,那么间隔一秒测试一次
						timer_req(1000);
						_record_timeout = 0; // 取消超时记录
					} else { // 已没有活着的其它uv请求
						if (_record_timeout) { // 如果已开始记录继续等待
							int64_t timeout = (time_monotonic() - _record_timeout - _timeout) / 1000;
							if (timeout >= 0) { // 已经超时
								close_uv_req_handles();
							} else { // 继续等待超时
								timer_req(-timeout);
							}
						} else {
							int64_t timeout = _timeout / 1000;
							if (timeout > 0) { // 需要等待超时
								_record_timeout = time_monotonic(); // 开始记录超时
								timer_req(timeout);
							} else {
								close_uv_req_handles();
							}
						}
					}
				}
			} else {
				if (timeout_ms > 0) { // > 0, delay
					timer_req(timeout_ms);
				} else { // == 0
					/* Do a cheap read first. */
					uv_async_send(_uv_async);
				}
				_record_timeout = 0; // 取消超时记录
			}
		}

		void resolve_msg() {
			List<Msg> msg;
			{
				ScopeLock lock(_mutex);
				if (_msg.length()) {
					msg = std::move(_msg);
				} else {
					after_resolve_msg(-1);
					return;
				}
			}

			// exec message
			if (msg.length()) {
				int64_t now = time_monotonic();
				auto i = msg.begin(), e = msg.end();
				do {
					auto t = i++;
					if (now >= t->time) { //
						t->cb->resolve(this);
						msg.erase(t);
					}
				} while(i != e);
			}

			// after resolve queue message
			_mutex.lock();
			_msg.splice(_msg.begin(), msg); // add
			if (_msg.length()) {
				int64_t now = time_monotonic();
				int64_t duration = Int64::limit_max;
				for ( auto& i : _msg ) {
					int64_t du = i.time - now;
					if (du <= 0) {
						duration = 0; break;
					} else if (du < duration) {
						duration = du;
					}
				}
				after_resolve_msg(duration / 1e3);
			} else {
				after_resolve_msg(-1);
			}
			_mutex.unlock();
		}

		void activate() {
			if (_uv_async)
				uv_async_send(_uv_async);
		}

		void erase_work(List<Work*>::Iterator it) {
			_work.erase(it);
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
			_inl(host)->erase_work(it);
			if (UV_ECANCELED != status) { // cancel
				done->resolve(host);
			}
			_inl(host)->activate();
		}
	};

	void RunLoop::Inl::stop_after_print_message() {
		ScopeLock lock(_mutex);
		for (auto& i: _keep) {
			Qk_DEBUG("Print: RunLoop keep not release \"%s\"", i->_name.c_str());
		}
		for (auto& i: _work) {
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
    Qk_ASSERT(_tid != ThreadID());
		Qk_ASSERT(!static_cast<Thread_INL*>(t)->_loop);
		// set run loop
		static_cast<Thread_INL*>(t)->_loop = this;
	}

	/**
	 * @destructor
	 */
	RunLoop::~RunLoop() {
		ScopeLock lock(*__threads_mutex);
		Qk_STRICT_ASSERT(_uv_async == nullptr, "Secure deletion must ensure that the run loop has exited");

		{
			ScopeLock lock(_mutex);
			for (auto& i: _keep) {
				Qk_WARN("RunLoop keep not release \"%s\"", i->_name.c_str());
				i->_loop = nullptr;
			}
			for (auto& i: _work) {
				Qk_WARN("RunLoop work not complete: \"%s\"", i->name.c_str());
				delete i;
			}
		}

		for (auto &i: _msg) {
			i.cb->resolve(this); // resolve last message
		}

		if (__first_loop == this) {
			__first_loop = nullptr;
		}

		if (_uv_loop != uv_default_loop()) {
			uv_loop_delete(_uv_loop);
		}

		// delete run loop
		auto t = static_cast<Thread_INL*>(_thread);
		Qk_ASSERT(t->_loop);
		Qk_ASSERT(t->_loop == this);
		t->_loop = nullptr;
		_thread = nullptr;
	}

	/**
	 * @func current() 获取当前线程消息队列
	 */
	RunLoop* RunLoop::current() {
		auto t = thread_current_inl();
		if (!t) {
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
		return uv_loop_alive(_uv_loop);
	}

	/**
	 * NOTE: Be careful about thread security issues
	 * @func sync # 延时
	 */
	void RunLoop::post(Cb cb, uint64_t delay_us) {
		Lock lock(_mutex);
		if (_thread->abort) {
			Qk_WARN("RunLoop::post, _thread->abort == true"); return;
		}
		if (!delay_us && _tid == thread_current_id()) { //is current
			lock.unlock();
			cb->resolve();
			return;
		}

		if (delay_us) {
			int64_t time = time_monotonic() + delay_us;
			_msg.pushBack({ time, cb });
		} else {
			_msg.pushBack({ 0, cb });
		}
		_this->activate(); // 通知继续
	}

	/**
	 * NOTE: Be careful about thread security issues
	 * @func post_sync(cb)
	 */
	void RunLoop::post_sync(Callback<PostSyncData> cb) {
		Lock lock(_mutex);
		if (_thread->abort) {
			Qk_WARN("RunLoop::post_sync, _thread->abort != 0"); return;
		}
		struct Data: public RunLoop::PostSyncData {
			virtual void complete() {
				ScopeLock scope(ctx->_mutex);
				ok = true;
				cond.notify_all();
			}
			RunLoop* ctx;
			bool ok;
			Condition cond;
		} data;

		Data* datap = &data;
		data.ctx = this;
		data.ok = false;

		if (thread_current_id() == _thread->id) { // is current
			lock.unlock();
			cb->resolve(datap);
			if (data.ok) {
				return;
			}
			lock.lock();
		} else {
			_msg.pushBack({
				0, Cb([cb, datap, this](Cb::Data& e) {
					cb->resolve(datap);
				})
			});
			_this->activate(); // 通知继续
		}

		while (!data.ok) {
			data.cond.wait(lock); // wait
		}
	}

	/**
	 * NOTE: Be careful about thread security issues
	 * @func work()
	 */
	uint32_t RunLoop::work(Cb cb, Cb done, cString& name) {
		if (_thread->abort) {
			Qk_WARN("RunLoop::work, _thread->abort != 0"); return 0;
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
			work->it = _work.pushBack(work);
		}));

		return work->id;
	}

	/**
	 * NOTE: Be careful about thread security issues
	 * @func cancel_work(id)
	 */
	void RunLoop::cancel_work(uint32_t id) {
		post(Cb([=](Cb::Data& ev) {
			for (auto& i : _work) {
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
	void RunLoop::post_message(Cb cb, uint64_t delay_us) {
		post(cb, delay_us);
	}

	/**
	 * @func run() 运行消息循环
	 */
	void RunLoop::run(uint64_t timeout) {
		if (__is_process_exit) {
			Qk_WARN("cannot run RunLoop::run(), __is_process_exit != 0"); return;
		}
		if (_thread->abort) {
			Qk_WARN("cannot run RunLoop::run(), _thread->abort != 0"); return;
		}

		uv_async_t uv_async;
		uv_timer_t uv_timer;

		// init run
		_mutex.lock();
		Qk_ASSERT(!_uv_async, "It is running and cannot be called repeatedly");
		Qk_ASSERT(thread_current_id() == _tid, "Must run on the target thread");
		_timeout = Qk_MAX(timeout, 0);
		_record_timeout = 0;
		_uv_async = &uv_async; uv_async.data = this;
		_uv_timer = &uv_timer; uv_timer.data = this;
		uv_async_init(_uv_loop, _uv_async, (uv_async_cb)(RunLoop::Inl::before_resolve_msg));
		uv_timer_init(_uv_loop, _uv_timer);
		_this->activate();
		_mutex.unlock();

		// run uv loop
		uv_run(_uv_loop, UV_RUN_DEFAULT);
		_this->close_uv_req_handles();

		// loop end
		_mutex.lock();
		_uv_async = nullptr;
		_uv_timer = nullptr;
		_timeout = 0;
		_record_timeout = 0;
		_mutex.unlock();

		_this->stop_after_print_message();
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
	KeepLoop* RunLoop::keep_alive(cString& name) {
		ScopeLock lock(_mutex);
		auto keep = new KeepLoop(name);
		keep->_id = _keep.pushBack(keep);
		keep->_loop = this;
		return keep;
	}

	KeepLoop::KeepLoop(cString& name): _name(name) {
	}

	KeepLoop::~KeepLoop() {
		ScopeLock lock(*__threads_mutex);
		if (_loop) {
			ScopeLock lock(_loop->_mutex);
			Qk_ASSERT(_loop->_keep.length());

			_loop->_keep.erase(_id); // delete keep object for runloop

			if (_loop->_keep.length() == 0 && !_loop->_uv_loop->stop_flag) { // 可以结束了
				_inl(_loop)->activate(); // 激活循环状态,不再等待
			}
		} else {
			Qk_DEBUG("Keep already invalid \"%s\", RunLoop already stop and release", *_name);
		}
	}

	void KeepLoop::post_message(Cb cb, uint64_t delay_us) {
		// NOTE: Be careful about thread security issues
		if (_loop)
			_inl(_loop)->post(cb, delay_us);
	}
}

