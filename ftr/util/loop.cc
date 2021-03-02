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
#include "./os.h"
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

namespace ftr {

	struct ListenSignal {
		Thread* thread;
		Mutex mutex;
		Condition cond;
	};

	static Mutex* threads_mutex;
	static Dict<ThreadID, Thread*>* threads = nullptr;
	static List<ListenSignal*>* threads_end_listens = nullptr;
	static RunLoop* main_loop_obj = nullptr;
	static ThreadID main_loop_id;
	static pthread_key_t specific_key;
	static int is_process_exit = 0;
	static EventNoticer<>* on_process_safe_exit = nullptr;

	FX_DEFINE_INLINE_MEMBERS(Thread, Inl) {
		public:
		#define _inl_t(self) static_cast<Thread::Inl*>(self)

		static void thread_destructor(void* ptr) {
			auto thread = reinterpret_cast<Thread*>(ptr);
			if (is_process_exit == 0) { // no process exit
				if (main_loop_obj == thread->_loop) {
					main_loop_obj = nullptr;
					main_loop_id = ThreadID();
				}
				Release(thread->_loop);
			}
			delete thread;
		}

		static void thread_initialize() {
			threads = new Dict<ID, Thread*>();
			threads_mutex = new Mutex();
			threads_end_listens = new List<ListenSignal*>();
			on_process_safe_exit = new EventNoticer<>("ProcessSafeExit", nullptr);
			int err = pthread_key_create(&specific_key, thread_destructor);
			ASSERT(err == 0);
		}

		static void set_thread_specific_data(Thread* thread) {
			ASSERT(!pthread_getspecific(specific_key));
			pthread_setspecific(specific_key, thread);
		}

		static inline Thread* get_thread_specific_data() {
			return reinterpret_cast<Thread*>(pthread_getspecific(specific_key));
		}

		void set_run_loop(RunLoop* loop) {
			ASSERT(!_loop);
			_loop = loop;
		}

		void del_run_loop(RunLoop* loop) {
			ASSERT(_loop);
			ASSERT(_loop == loop);
			_loop = nullptr;
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
				ScopeLock scope(*threads_mutex);
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
				ScopeLock scope(*threads_mutex);
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
					ScopeLock scope(*threads_mutex);
					DLOG("threads count, %d", threads->length());
					for ( auto& i : *threads ) {
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

				KeepLoop* keep = nullptr;
				if (main_loop_obj && main_loop_obj->runing()) {
					keep = main_loop_obj->keep_alive("Thread::Inl::exit()"); // keep main loop
				}

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
		Lock lock(*threads_mutex);
		auto i = threads->find(id);
		if ( i != threads->end() ) {
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
			FX_WARN("Cannot find current ftr::Thread handle, use std::this_thread::sleep_for()");
			if (timeoutUs > 0) {
				std::this_thread::sleep_for(std::chrono::microseconds(timeoutUs));
			}
		}
	}

	/**
	 * @func awaken
	 */
	void Thread::awaken(ID id) {
		ScopeLock lock(*threads_mutex);
		auto i = threads->find(id);
		if ( i != threads->end() ) {
			_inl_t(i->value)->awaken(); // awaken sleep status
		}
	}

	void Thread::abort(ID id) {
		ScopeLock lock(*threads_mutex);
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
		DLOG("thread_init_once");
		atexit(Thread::Inl::before_exit);
		Thread::Inl::thread_initialize();
	}

	// --------------------- ThreadRunLoop ---------------------

	class RunLoop::Inl: public RunLoop {
	 public:
	 #define _inl(self) static_cast<RunLoop::Inl*>(self)
		
		void run(int64_t timeout) {
			if (is_process_exit) {
				DLOG("cannot run RunLoop, is_process_exit != 0");
				return;
			}
			if (_thread->is_abort()) {
				DLOG("cannot run RunLoop, _thread->is_abort() == true");
				return;
			}
			uv_async_t uv_async;
			uv_timer_t uv_timer;
			{ //
				ScopeLock lock(_mutex);
				ASSERT(Thread::current_id() == _tid, "Must run on the target thread");
				ASSERT(!_uv_async);
				_timeout = FX_MAX(timeout, 0);
				_record_timeout = 0;
				_uv_async = &uv_async; uv_async.data = this;
				_uv_timer = &uv_timer; uv_timer.data = this;
				uv_async_init(_uv_loop, _uv_async, (uv_async_cb)(resolve_queue_before));
				uv_timer_init(_uv_loop, _uv_timer);
				activate_loop();
			}
			uv_run(_uv_loop, UV_RUN_DEFAULT); // run uv loop
			close_uv_async();
			{ // loop end
				ScopeLock lock(_mutex);
				_uv_async = nullptr;
				_uv_timer = nullptr;
				_timeout = 0;
				_record_timeout = 0;
			}
			stop_after_print_message();
		}

		void stop_after_print_message();
		
		static void resolve_queue_before(uv_handle_t* handle) {
			bool Continue;
			do {
				Continue = ((Inl*)handle->data)->resolve_queue();
			} while (Continue);
		}
		
		bool is_alive() {
			// _uv_async 外是否还有活着的`handle`与请求
			uv_loop_t* loop = _uv_loop;
			//		return _uv_loop->active_handles > 1 ||
			//					 QUEUE_EMPTY(&(loop)->active_reqs) == 0 ||
			//					 loop->closing_handles != NULL;
			return uv_loop_alive(_uv_loop);
		}
		
		void close_uv_async() {
			if (!uv_is_closing((uv_handle_t*)_uv_async))
				uv_close((uv_handle_t*)_uv_async, nullptr); // close async
			uv_timer_stop(_uv_timer);
			if (!uv_is_closing((uv_handle_t*)_uv_timer))
				uv_close((uv_handle_t*)_uv_timer, nullptr);
		}
		
		inline void uv_timer_req(int64_t timeout_ms) {
			uv_timer_start(_uv_timer, (uv_timer_cb)resolve_queue_before, timeout_ms, 0);
		}
		
		bool resolve_queue_after(int64_t timeout_ms) {
			bool Continue = 0;
			if (_uv_loop->stop_flag != 0) { // 循环停止标志
				close_uv_async();
			}
			else if (timeout_ms == -1) { //
				if (_keeps.length() == 0 && _works.length() == 0) {
					// RunLoop 已经没有需要处理的消息
					if (is_alive()) { // 如果uv还有其它活着,那么间隔一秒测试一次
						uv_timer_req(1000);
						_record_timeout = 0; // 取消超时记录
					} else { // 已没有活着的其它uv请求
						if (_record_timeout) { // 如果已开始记录继续等待
							int64_t timeout = (os::time_monotonic() - _record_timeout - _timeout) / 1000;
							if (timeout >= 0) { // 已经超时
								close_uv_async();
							} else { // 继续等待超时
								uv_timer_req(-timeout);
							}
						} else {
							int64_t timeout = _timeout / 1000;
							if (timeout > 0) { // 需要等待超时
								_record_timeout = os::time_monotonic(); // 开始记录超时
								uv_timer_req(timeout);
							} else {
								close_uv_async();
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
					// Continue = 1; // continue
				}
				_record_timeout = 0; // 取消超时记录
			}
			return Continue;
		}
		
		void resolve_queue(List<Queue>& queue) {
			int64_t now = os::time_monotonic();
			for (auto i = queue.begin(), e = queue.end(); i != e; ) {
				auto t = i++;
				if (now >= t->time) { //
					t->resolve->resolve(this);
					queue.erase(t);
				}
			}
		}
		
		bool resolve_queue() {
			List<Queue> queue;
			{ ScopeLock lock(_mutex);
				if (_queue.length()) {
					queue = std::move(_queue);
				} else {
					return resolve_queue_after(-1);
				}
			}
			if (queue.length()) {
				if (_independent_mutex) {
					std::lock_guard<RecursiveMutex> lock(*_independent_mutex);
					resolve_queue(queue);
				} else {
					resolve_queue(queue);
				}
			}
			{ ScopeLock lock(_mutex);
				_queue.splice(_queue.begin(), queue);
				if (_queue.length() == 0) {
					return resolve_queue_after(-1);
				}
				int64_t now = os::time_monotonic();
				int64_t duration = Int64::max;
				for ( auto& i : _queue ) {
					int64_t du = i.time - now;
					if (du <= 0) {
						duration = 0; break;
					} else if (du < duration) {
						duration = du;
					}
				}
				return resolve_queue_after(duration / 1e3);
			}
		}
		
		/**
		 * @func post()
		 */
		uint32_t post(Cb exec, uint32_t group, uint64_t delay_us) {
			if (_thread->is_abort()) {
				DLOG("RunLoop::post, _thread->is_abort() == true");
				return 0;
			}
			ScopeLock lock(_mutex);
			uint32_t id = getId32();
			if (delay_us) {
				int64_t time = os::time_monotonic() + delay_us;
				_queue.push_back({ id, group, time, exec });
			} else {
				_queue.push_back({ id, group, 0, exec });
			}
			activate_loop(); // 通知继续
			return id;
		}

		void post_sync(Callback<RunLoop::PostSyncData> cb, uint32_t group, uint64_t delay_us) {
			ASSERT(!_thread->is_abort(), "RunLoop::post_sync, _thread->is_abort() == true");

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
				activate_loop(); // 通知继续
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
			activate_loop(); // 通知继续
		}
		
		inline void activate_loop() {
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
		Callback<> work;
		Callback<> done;
		uv_work_t uv_req;
		String name;
		static void uv_work_cb(uv_work_t* req) {
			Work* self = (Work*)req->data;
			self->work->resolve(self->host);
		}
		static void uv_after_work_cb(uv_work_t* req, int status) {
			Handle<Work> self = (Work*)req->data;
			if (self->host->_independent_mutex) {
				std::lock_guard<RecursiveMutex> lock(*self->host->_independent_mutex);
				self->done_work(status);
			} else {
				self->done_work(status);
			}
		}
		void done_work(int status) {
			_inl(host)->delete_work(it);
			if (UV_ECANCELED != status) { // cancel
				done->resolve(host);
			}
			_inl(host)->activate_loop();
		}
	};

	void RunLoop::Inl::stop_after_print_message() {
		ScopeLock lock(_mutex);
		for (auto& i: _keeps) {
			DLOG("Print: RunLoop keep not release \"%s\"", i->_name.c_str());
		}
		for (auto& i: _works) {
			DLOG("Print: RunLoop work not complete: \"%s\"", i->name.c_str());
		}
	}

	/**
	 * @constructor
	 */
	RunLoop::RunLoop(Thread* t)
	: _independent_mutex(nullptr)
	, _thread(t)
	, _tid(t->id())
	, _uv_loop(nullptr)
	, _uv_async(nullptr)
	, _uv_timer(nullptr)
	, _timeout(0)
	, _record_timeout(0)
	{
		_inl_t(t)->set_run_loop(this);
		_uv_loop = uv_loop_new();
	}

	/**
	 * @destructor
	 */
	RunLoop::~RunLoop() {
		ScopeLock lock(*threads_mutex);
		ASSERT(_uv_async == nullptr, "Secure deletion must ensure that the run loop has exited");

		{
			ScopeLock lock(_mutex);
			for (auto& i: _keeps) {
				FX_WARN("RunLoop keep not release \"%s\"", i->_name.c_str());
				i->_loop = nullptr;
			}
			for (auto& i: _works) {
				FX_WARN("RunLoop work not complete: \"%s\"", i->name.c_str());
				delete i;
			}
		}

		if (_uv_loop != uv_default_loop()) {
			uv_loop_delete(_uv_loop);
		}
		_inl_t(_thread)->del_run_loop(this);
	}

	/**
	 * @func current() 获取当前线程消息队列
	 */
	RunLoop* RunLoop::current() {
		auto t = Thread::Inl::get_thread_specific_data();
		ASSERT(t, "Can't get thread specific data");
		auto loop = t->loop();
		if (!loop) {
			ScopeLock scope(*threads_mutex);
			loop = new RunLoop(t);
			if (!main_loop_obj) {
				main_loop_obj = loop;
				main_loop_id = t->id();
				uv_loop_delete(loop->_uv_loop);
				loop->_uv_loop = uv_default_loop();
			}
		}
		return loop;
	}

	/**
	 * @func main_loop();
	 */
	RunLoop* RunLoop::main_loop() {
		// TODO: 小心线程安全,最好先确保已调用过`current()`
		if (!main_loop_obj) {
			current();
			ASSERT(main_loop_obj);
		}
		return main_loop_obj;
	}

	/**
	 * @func is_main_loop 当前线程是为主循环
	 */
	bool RunLoop::is_main_loop() {
		return main_loop_id == Thread::current_id();
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
			DLOG("RunLoop::work, _thread->is_abort() == true");
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
			ASSERT(!r);
			work->it = _works.insert(_works.end(), work);
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
					ASSERT(!r);
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
		_inl(this)->activate_loop();
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
	KeepLoop* RunLoop::keep_alive(cString& name, bool declear) {
		ScopeLock lock(_mutex);
		auto keep = new KeepLoop(name, declear);
		keep->_id = _keeps.insert(_keeps.end(), keep);
		keep->_loop = this;
		return keep;
	}

	static RunLoop* loop_2(ThreadID id) {
		auto i = threads->find(id);
		if (i == threads->end()) {
			return nullptr;
		}
		return i->value->loop();
	}

	/**
	 * @func get_loop_with_id(id) 通过线程获取,目标线程没有创建过实体返回`nullptr`
	 * @ret {RunLoop*}
	 */
	static RunLoop* get_loop_with_id(ThreadID id) {
		ScopeLock scope(*threads_mutex);
		return loop_2(id);
	}

	/**
	 * @func keep_alive_current 保持当前循环活跃并返回代理
	 */
	KeepLoop* RunLoop::keep_alive_current(cString& name, bool declear) {
		RunLoop* loop = current();
		if ( loop ) {
			return loop->keep_alive(name, declear);
		}
		return nullptr;
	}

	/**
	 * @func next_tick
	 */
	void RunLoop::next_tick(Cb cb) throw(Error) {
		RunLoop* loop = RunLoop::current();
		if ( loop ) {
			loop->post(cb);
		} else { // 没有消息队列 post to io loop
			FX_THROW(ERR_NOT_RUN_LOOP, "Unable to obtain thread io run loop");
		}
	}

	/**
	 * @func stop() 停止循环
	 */
	void RunLoop::stop(ThreadID id) {
		ScopeLock scope(*threads_mutex);
		auto loop = loop_2(id);
		if (loop) {
			loop->stop();
		}
	}

	/**
	 * @func is_alive()
	 */
	bool RunLoop::is_alive(ThreadID id) {
		ScopeLock scope(*threads_mutex);
		auto loop = loop_2(id);
		DLOG("RunLoop::is_alive, %p, %p", loop, id);
		if (loop) {
			return loop->is_alive();
		}
		return false;
	}

	// ************** KeepLoop **************

	KeepLoop::KeepLoop(cString& name, bool destructor_clear)
	: _group(getId32()), _name(name), _declear(destructor_clear) {
	}

	KeepLoop::~KeepLoop() {
		ScopeLock lock(*threads_mutex);

		if (_loop) {
			ScopeLock lock(_loop->_mutex);
			if ( _declear ) {
				_inl(_loop)->cancel_group_non_lock(_group);
			}
			ASSERT(_loop->_keeps.length());

			_loop->_keeps.erase(_id); // 减少一个引用计数

			if (_loop->_keeps.length() == 0 && !_loop->_uv_loop->stop_flag) { // 可以结束了
				_inl(_loop)->activate_loop(); // 激活循环状态,不再等待
			}
		} else {
			DLOG("Keep already invalid \"%s\", RunLoop already stop and release", *_name);
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

