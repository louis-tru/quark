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

#include "./loop.h"
#include "./list"
#include "./dict.h"
#include <uv.h>
#include <pthread.h>

#ifndef F_ATEXIT_WAIT_TIMEOUT
# define F_ATEXIT_WAIT_TIMEOUT 1e6
#endif

namespace flare {

	// ---------------------------------------------------------

	template<> uint64_t Compare<ThreadID>::hash_code(const ThreadID& key) {
		return *reinterpret_cast<const uint32_t*>(&key);
	}

	struct ListenSignal {
		Thread* thread;
		Mutex mutex;
		Condition cond;
	};

	static RunLoop* __first_loop = nullptr;
	static Mutex* __threads_mutex = nullptr;
	static Dict<ThreadID, Thread*>* __threads = nullptr;
	static List<ListenSignal*>* __wait_end_listens = nullptr;
	static pthread_key_t __specific_key;
	static int __is_process_exit = 0;
	static EventNoticer<>* __on_process_safe_exit = nullptr;

	F_DEFINE_INLINE_MEMBERS(Thread, Inl) {
	 public:
		#define _inl_t(self) static_cast<Thread::Inl*>(self)

		static void destructor(void* ptr) {
			auto thread = reinterpret_cast<Thread::Inl*>(ptr);
			if (__is_process_exit == 0) { // no process exit
				Release(thread->_loop);
			}
			delete thread;
		}

		static void initialize() {
			F_DEBUG("thread_init_once");
			atexit(Thread::Inl::before_exit);
			__threads = new Dict<ID, Thread*>();
			__threads_mutex = new Mutex();
			__wait_end_listens = new List<ListenSignal*>();
			__on_process_safe_exit = new EventNoticer<>("ProcessSafeExit", nullptr);
			int err = pthread_key_create(&__specific_key, destructor);
			F_ASSERT(err == 0);
		}

		static void set_thread_specific_data(Thread* thread) {
			F_ASSERT(!pthread_getspecific(__specific_key));
			pthread_setspecific(__specific_key, thread);
		}

		static inline Thread* get_thread_specific_data() {
			return reinterpret_cast<Thread*>(pthread_getspecific(__specific_key));
		}

		static void run(Exec exec, Thread* thread) {
			#if F_ANDROID
				JNI::ScopeENV scope;
			#endif
			set_thread_specific_data(thread);
			if ( !thread->_abort ) {
				int rc = exec(*thread);
				thread->_abort = true;
			}
			{
				ScopeLock scope(*__threads_mutex);
				F_DEBUG("Thread end ..., %s", *thread->name());
				for (auto& i : *__wait_end_listens) {
					if (i->thread == thread) {
						ScopeLock scope(i->mutex);
						i->cond.notify_one();
					}
				}
				F_DEBUG("Thread end  ok, %s", *thread->name());
				__threads->erase(thread->id());
			}
		}

		static ID fork(Exec exec, cString& name) {
			if ( __is_process_exit ) {
				return ID();
			} else {
				ScopeLock scope(*__threads_mutex);
				Thread* thread = new Thread();
				std::thread t(run, exec, thread);
				thread->_id = t.get_id();
				thread->_abort = false;
				thread->_loop = nullptr;
				thread->_name = name;
				__threads->set(thread->_id, thread);
				t.detach();
				return thread->_id;
			}
		}

		void resume(bool abort = 0) {
			ScopeLock scope(_mutex);
			if (abort) {
				if (_loop)
					_loop->stop();
				_abort = true;
			}
			_cond.notify_one(); // resume sleep status
		}

		static void before_exit() {
			if (!__is_process_exit++) { // exit
				Array<ID> threads_id;
				{
					ScopeLock scope(*__threads_mutex);
					F_DEBUG("threads count, %d", __threads->length());
					for ( auto& i : *__threads ) {
						F_DEBUG("atexit_exec,name, %p, %s", i.value->id(), *i.value->name());
						_inl_t(i.value)->resume(true); // resume sleep status and abort
						threads_id.push(i.value->id());
					}
				}
				for ( auto& i: threads_id ) {
					// 在这里等待这个线程的结束,这个时间默认为1秒钟
					F_DEBUG("atexit_exec,join, %p", i);
					join(i, F_ATEXIT_WAIT_TIMEOUT); // wait 1s
				}
			}
		}

		static void safe_exit(int rc, bool forceExit = false) {
			static int is_exited = 0;
			if (!is_exited++ && !__is_process_exit) {

				// KeepLoop* keep = nullptr;
				// if (__first_loop && __first_loop->runing()) {
				// 	keep = __first_loop->keep_alive("Thread::Inl::exit()"); // keep main loop
				// }
				F_DEBUG("Inl::exit(), 0");
				Event<> ev(Int32(rc), nullptr, rc);
				F_Trigger(ProcessSafeExit, ev);
				rc = ev.return_value;
				F_DEBUG("Inl::exit(), 1");

				// Release(keep); keep = nullptr;
				before_exit();

				F_DEBUG("Inl::reallyExit()");
				if (forceExit)
					::exit(rc); // foece reallyExit
			} else {
				F_DEBUG("The program has exited");
			}
		}

	};

	ThreadID Thread::fork(Exec exec, cString& name) {
		return Inl::fork(exec, name);
	}

	void Thread::sleep(uint64_t timeoutUs) {
		std::this_thread::sleep_for(std::chrono::microseconds(timeoutUs));
	}

	/**
	 * @func pause
	 */
	void Thread::pause(uint64_t timeoutUs) {
		auto cur = current();
		F_ASSERT(cur, "Cannot find current flare::Thread handle, use Thread::sleep()");

		Lock lock(cur->_mutex);
		if ( !cur->_abort ) {
			if (timeoutUs) {
				cur->_cond.wait_for(lock, std::chrono::microseconds(timeoutUs));
			} else {
				cur->_cond.wait(lock); // wait
			}
		} else {
			F_WARN("Thread aborted, cannot wait");
		}
	}

	/**
	 * @func resume
	 */
	void Thread::resume(ID id) {
		ScopeLock lock(*__threads_mutex);
		auto i = __threads->find(id);
		if ( i != __threads->end() ) {
			_inl_t(i->value)->resume(false); // resume sleep status
		}
	}

	void Thread::abort(ID id) {
		ScopeLock lock(*__threads_mutex);
		auto i = __threads->find(id);
		if ( i != __threads->end() ) {
			_inl_t(i->value)->resume(true); // resume sleep status
		}
	}

	void Thread::join(ID id, uint64_t timeoutUs) {
		if (id == current_id()) {
			F_DEBUG("Thread::join(), cannot wait_end self thread");
			return;
		}
		Lock lock(*__threads_mutex);
		auto i = __threads->find(id);
		if ( i != __threads->end() ) {
			ListenSignal signal = { i->value };
			auto it = __wait_end_listens->push_back(&signal);
			{ //
				Lock l(signal.mutex);
				lock.unlock();
				String name = i->value->_name;
				F_DEBUG("Thread::wait_end, ..., %p, %s", id, *name);
				if (timeoutUs) {
					signal.cond.wait_for(l, std::chrono::microseconds(timeoutUs)); // wait
				} else {
					signal.cond.wait(l); // permanent wait
				}
				F_DEBUG("Thread::wait_end, end, %p, %s", id, *name);
			}
			lock.lock();
			__wait_end_listens->erase(it);
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

	F_EXPORT void safe_exit(int rc) {
		Thread::Inl::safe_exit(rc);
	}

	void exit(int rc) {
		Thread::Inl::safe_exit(rc, true);
	}

	bool is_exited() {
		return __is_process_exit;
	}

	EventNoticer<>& onProcessSafeExit() {
		return *__on_process_safe_exit;
	}

	F_INIT_BLOCK(thread_init_once) {
		Thread::Inl::initialize();
	}

	// --------------------- RunLoop ---------------------

	F_DEFINE_INLINE_MEMBERS(RunLoop, Inl) {
		#define _inl(self) static_cast<RunLoop::Inl*>(self)
	 public:

		void stop_after_print_message();
		
		void run(int64_t timeout) {
			if (is_exited()) {
				F_DEBUG("cannot run RunLoop, __is_process_exit != 0");
				return;
			}
			if (_thread->is_abort()) {
				F_DEBUG("cannot run RunLoop, _thread->is_abort() == true");
				return;
			}
			uv_async_t uv_async;
			uv_timer_t uv_timer;
			{ //
				ScopeLock lock(_mutex);
				F_ASSERT(!_uv_async, "It is running and cannot be called repeatedly");
				F_ASSERT(Thread::current_id() == _tid, "Must run on the target thread");
				_timeout = F_MAX(timeout, 0);
				_record_timeout = 0;
				_uv_async = &uv_async; uv_async.data = this;
				_uv_timer = &uv_timer; uv_timer.data = this;
				uv_async_init(_uv_loop, _uv_async, (uv_async_cb)(resolve_queue_before));
				uv_timer_init(_uv_loop, _uv_timer);
				activate();
			}
			uv_run(_uv_loop, UV_RUN_DEFAULT); // run uv loop
			{ // loop end
				ScopeLock lock(_mutex);
				close_uv_req_handles();
				_uv_async = nullptr;
				_uv_timer = nullptr;
				_timeout = 0;
				_record_timeout = 0;
			}
			stop_after_print_message();
		}

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
		
		static void resolve_queue_before(uv_handle_t* handle) {
			((Inl*)handle->data)->resolve_queue();
		}
		
		inline void uv_timer_req(int64_t timeout_ms) {
			uv_timer_start(_uv_timer, (uv_timer_cb)resolve_queue_before, timeout_ms, 0);
		}
		
		void resolve_queue_after(int64_t timeout_ms) {
			if (_uv_loop->stop_flag != 0) { // 循环停止标志
				close_uv_req_handles();
			}
			else if (timeout_ms == -1) { //
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
			{ ScopeLock lock(_mutex);
				if (_queue.length()) {
					queue = std::move(_queue);
				} else {
					resolve_queue_after(-1);
				}
			}
			if (queue.length()) {
				exec_queue(queue);
			}
			{ ScopeLock lock(_mutex);
				_queue.splice(_queue.begin(), queue);
				if (_queue.length() == 0) {
					resolve_queue_after(-1);
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
				resolve_queue_after(duration / 1e3);
			}
		}

		void exec_queue(List<Queue>& queue) {
			int64_t now = time_monotonic();
			for (auto i = queue.begin(), e = queue.end(); i != e; ) {
				auto t = i++;
				if (now >= t->time) { //
					t->resolve->resolve(this);
					queue.erase(t);
				}
			}
		}
		
		/**
		 * @func post()
		 */
		uint32_t post(Cb exec, uint32_t group, uint64_t delay_us) {
			if (_thread->is_abort()) {
				F_DEBUG("RunLoop::post, _thread->is_abort() == true");
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
			F_ASSERT(!_thread->is_abort(), "RunLoop::post_sync, _thread->is_abort() == true");

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

			typedef CallbackData<RunLoop::PostSyncData> PCbData;

			bool isCur = Thread::current_id() == _thread->id();
			if (isCur) { // 立即调用
				cb->resolve(datap);
			}

			Lock lock(_mutex);

			if (!isCur) {
				_queue.push_back({
					0, group, 0,
					Cb([cb, datap, this](CbData& e) {
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
					// TODO: 删除时如果析构函数间接调用锁定`_mutex`的函数会锁死线程
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

	void  RunLoop::Inl::stop_after_print_message() {
		ScopeLock lock(_mutex);
		for (auto& i: _keeps) {
			F_DEBUG("Print: RunLoop keep not release \"%s\"", i->_name.c_str());
		}
		for (auto& i: _works) {
			F_DEBUG("Print: RunLoop work not complete: \"%s\"", i->name.c_str());
		}
	}
	
	/**
	 * @constructor
	 */
	RunLoop::RunLoop(Thread* t, uv_loop_t* uv)
		: _thread(t)
		, _tid(t->id())
		, _uv_loop(uv)
		, _uv_async(nullptr)
		, _uv_timer(nullptr)
		, _timeout(0)
		, _record_timeout(0)
	{
		F_ASSERT(!t->_loop);
		// set run loop
		t->_loop = this;
	}

	/**
	 * @destructor
	 */
	RunLoop::~RunLoop() {
		ScopeLock lock(*__threads_mutex);
		F_ASSERT(_uv_async == nullptr, "Secure deletion must ensure that the run loop has exited");
		
		{
			ScopeLock lock(_mutex);
			for (auto& i: _keeps) {
				F_WARN("RunLoop keep not release \"%s\"", i->_name.c_str());
				i->_loop = nullptr;
			}
			for (auto& i: _works) {
				F_WARN("RunLoop work not complete: \"%s\"", i->name.c_str());
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
		F_ASSERT(_thread->_loop);
		F_ASSERT(_thread->_loop == this);
		_thread->_loop = nullptr;
	}

	/**
	 * @func current() 获取当前线程消息队列
	 */
	RunLoop* RunLoop::current() {
		auto t = Thread::current();
		if (!t) {
			F_WARN(t, "Can't get thread specific data");
			return nullptr;
		}
		auto loop = t->loop();
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
		return loop && loop->_tid == Thread::current_id();
	}

	/**
	 * @func first();
	 */
	RunLoop* RunLoop::first() {
		// TODO: 小心线程安全,最好先确保已调用过`current()`
		if (!__first_loop) {
			current();
			F_ASSERT(__first_loop); // asset
		}
		return __first_loop;
	}

	/**
	 * TODO: Be careful about thread security issues
	 * @func runing()
	 */
	bool RunLoop::runing() const {
		return _uv_async;
	}

	/**
	 * TODO: Be careful about thread security issues
	 * @func is_alive
	 */
	bool RunLoop::is_alive() const {
		return uv_loop_alive(_uv_loop) /*|| _keeps.length() || _works.length()*/;
	}

	/**
	 * TODO: Be careful about thread security issues
	 * @func sync # 延时
	 */
	uint32_t RunLoop::post(Cb cb, uint64_t delay_us) {
		return _inl(this)->post(cb, 0, delay_us);
	}

	/**
	 * TODO: Be careful about thread security issues
	 * @func post_sync(cb)
	 */
	void RunLoop::post_sync(Callback<PostSyncData> cb) {
		_inl(this)->post_sync(cb, 0, 0);
	}

	/**
	 * TODO: Be careful about thread security issues
	 * @func work()
	 */
	uint32_t RunLoop::work(Cb cb, Cb done, cString& name) {
		if (_thread->is_abort()) {
			F_DEBUG("RunLoop::work, _thread->is_abort() == true");
			return 0;
		}

		Work* work = new Work();
		work->id = getId32();
		work->work = cb;
		work->done = done;
		work->uv_req.data = work;
		work->host = this;
		work->name = name;

		post(Cb([work, this](CbData& ev) {
			int r = uv_queue_work(_uv_loop, &work->uv_req,
														Work::uv_work_cb, Work::uv_after_work_cb);
			F_ASSERT(!r);
			work->it = _works.push_back(work);
		}));

		return work->id;
	}

	/**
	 * TODO: Be careful about thread security issues
	 * @func cancel_work(id)
	 */
	void RunLoop::cancel_work(uint32_t id) {
		post(Cb([=](CbData& ev) {
			for (auto& i : _works) {
				if (i->id == id) {
					int r = uv_cancel((uv_req_t*)&i->uv_req);
					F_ASSERT(!r);
					break;
				}
			}
		}));
	}

	/**
	 * TODO: Be careful about thread security issues
	 * @overwrite
	 */
	uint32_t RunLoop::post_message(Cb cb, uint64_t delay_us) {
		return _inl(this)->post(cb, 0, delay_us);
	}

	/**
	 * TODO: Be careful about thread security issues
	 * @func cancel # 取消同步
	 */
	void RunLoop::cancel(uint32_t id) {
		ScopeLock lock(_mutex);
		for (auto i = _queue.begin(), e = _queue.end(); i != e; i++) {
			if (i->id == id) {
				// TODO: 删除时如果析构函数间接调用锁定`_mutex`的函数会锁死线程
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
		_inl(this)->run(timeout);
	}

	/**
	 * TODO: Be careful about thread security issues
	 * @func stop
	 */
	void RunLoop::stop() {
		if ( runing() ) {
			post(Cb([this](CbData& se) {
				uv_stop(_uv_loop);
			}));
		}
	}

	/**
	 * TODO: Be careful about thread security issues
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
			F_ASSERT(_loop->_keeps.length());

			_loop->_keeps.erase(_id); // 减少一个引用计数

			if (_loop->_keeps.length() == 0 && !_loop->_uv_loop->stop_flag) { // 可以结束了
				_inl(_loop)->activate(); // 激活循环状态,不再等待
			}
		} else {
			F_DEBUG("Keep already invalid \"%s\", RunLoop already stop and release", *_name);
		}
	}

	uint32_t KeepLoop::post(Cb exec, uint64_t delay_us) {
		// TODO: Be careful about thread security issues
		if (_loop)
			return _inl(_loop)->post(exec, _group, delay_us);
		else
			return 0;
	}

	uint32_t KeepLoop::post_message(Cb cb, uint64_t delay_us) {
		// TODO: Be careful about thread security issues
		if (_loop)
			return _inl(_loop)->post(cb, _group, delay_us);
		else
			return 0;
	}

	void KeepLoop::cancel_all() {
		// TODO: Be careful about thread security issues
		if (_loop)
			_inl(_loop)->cancel_group(_group); // abort all
	}

	void KeepLoop::cancel(uint32_t id) {
		// TODO: Be careful about thread security issues
		if (_loop)
			_loop->cancel(id);
	}

}

