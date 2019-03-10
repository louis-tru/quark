
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

#include "qgr/utils/loop.h"
#include "qgr/utils/loop-1.h"
#if XX_ANDROID
# include "qgr/utils/android-jni.h"
#endif
#include <uv.h>
#include <pthread.h>
#include "depe/node/deps/uv/src/queue.h"

#ifndef XX_ATEXIT_WAIT_TIMEOUT
# define XX_ATEXIT_WAIT_TIMEOUT 1e6
#endif

XX_NS(qgr)

template<> uint Compare<ThreadID>::hash(const ThreadID& key) {
	ThreadID* p = const_cast<ThreadID*>(&key);
	size_t* t = reinterpret_cast<size_t*>(p);
	return (*t) % Uint::max;
}
template<> bool Compare<ThreadID>::equals(
	const ThreadID& a, const ThreadID& b, uint ha, uint hb) {
	return a == b;
}

// ----------------------------------------------------------

struct ListenSignal {
	Thread* thread;
	Mutex mutex;
	Condition cond;
};

static Mutex* threads_mutex;
static Map<ThreadID, Thread*>* threads = nullptr;
static List<ListenSignal*>* threads_end_listens = nullptr;
static RunLoop* main_loop_obj = nullptr;
static ThreadID main_loop_id;
static pthread_key_t specific_key;
int __is_process_exit = 0;
int (*__xx_exit_app_hook)(int rc) = nullptr;
int (*__xx_exit_hook)(int rc) = nullptr;

XX_DEFINE_INLINE_MEMBERS(Thread, Inl) {
 public:
	#define _inl_t(self) static_cast<Thread::Inl*>(self)

	static void thread_destructor(void* ptr) {
		auto thread = reinterpret_cast<Thread*>(ptr);
		if (__is_process_exit == 0) { // no process exit
			if (main_loop_obj == thread->m_loop) {
				main_loop_obj = nullptr;
				main_loop_id = ThreadID();
			}
			Release(thread->m_loop);
		}
		delete thread;
	}

	static void thread_initialize() {
		threads = new Map<ID, Thread*>();
		threads_mutex = new Mutex();
		threads_end_listens = new List<ListenSignal*>();
		int err = pthread_key_create(&specific_key, thread_destructor);
		XX_CHECK(err == 0);
	}

	static void set_thread_specific_data(Thread* thread) {
		XX_CHECK(!pthread_getspecific(specific_key));
		pthread_setspecific(specific_key, thread);
	}

	static inline Thread* get_thread_specific_data() {
		return reinterpret_cast<Thread*>(pthread_getspecific(specific_key));
	}

	void set_run_loop(RunLoop* loop) {
		XX_CHECK(!m_loop);
		m_loop = loop;
	}

	void del_run_loop(RunLoop* loop) {
		XX_CHECK(m_loop);
		XX_CHECK(m_loop == loop);
		m_loop = nullptr;
	}

	static void run_2(Exec exec, Thread* thread) {
#if XX_ANDROID
		JNI::ScopeENV scope;
#endif
		set_thread_specific_data(thread);
		if ( !thread->m_abort ) {
			int rc = exec(*thread);
			thread->m_abort = true;
		}
		{
			ScopeLock scope(*threads_mutex);
			DLOG("Thread end ..., %s", *thread->name());
			for (auto& i : *threads_end_listens) {
				ListenSignal* s = i.value();
				if (s->thread == thread) {
					ScopeLock scope(s->mutex);
					s->cond.notify_one();
				}
			}
			DLOG("Thread end  ok, %s", *thread->name());
			threads->del(thread->id());
		}
	}

	static ID run(Exec exec, cString& name) {
		if ( __is_process_exit ) {
			return ID();
		} else {
			ScopeLock scope(*threads_mutex);
			Thread* thread = new Thread();
			std::thread t(run_2, exec, thread);
			thread->m_id = t.get_id();
			thread->m_name = name;
			thread->m_abort = false;
			thread->m_loop = nullptr;
			memset(thread->m_data, 0, sizeof(void*[256]));
			threads->set(thread->m_id, thread);
			t.detach();
			return thread->m_id;
		}
	}

	void awaken(bool abort = 0) {
		ScopeLock scope(m_mutex);
		if (abort) {
			if (m_loop)
				m_loop->stop();
			m_abort = true;
		}
		m_cond.notify_one(); // awaken sleep status
	}

	static void atexit_exec() {
		if (!__is_process_exit++) { // exit
			Array<ID> threads_id;
			{
				ScopeLock scope(*threads_mutex);
				DLOG("threads count, %d", threads->length());
				for ( auto& i : *threads ) {
					DLOG("atexit_exec,name, %p, %s", i.value()->id(), *i.value()->name());
					_inl_t(i.value())->awaken(true); // awaken sleep status and abort
					threads_id.push(i.value()->id());
				}
			}
			for ( auto& i: threads_id ) {
				// 在这里等待这个线程的结束,这个时间默认为1秒钟
				DLOG("atexit_exec,join, %p", i.value());
				join(i.value(), XX_ATEXIT_WAIT_TIMEOUT); // wait 1s
			}
		}
	}

	static void _reallyExit(int rc, bool reallyExit) {
		if (!__is_process_exit) {
			atexit_exec();
			DLOG("Inl::reallyExit()");
			if (reallyExit)
				::exit(rc);
		}
	}

	static void exit(int rc, bool reallyExit) {
		static int is_exited = 0;
		if (!is_exited++) {

			KeepLoop* keep = nullptr;
			if (main_loop_obj && main_loop_obj->runing()) {
				keep = main_loop_obj->keep_alive("Thread::Inl::exit()"); // keep main loop
			}
			DLOG("Inl::exit(), 0");
			if (__xx_exit_app_hook)
				rc = __xx_exit_app_hook(rc);
			DLOG("Inl::exit(), 1");
			if (__xx_exit_hook) 
				rc = __xx_exit_hook(rc);
			DLOG("Inl::exit(), 2");

			Release(keep); keep = nullptr;

			if (main_loop_obj && current_id() == main_loop_id && main_loop_obj->runing()) {
				main_loop_obj->post(Cb([rc, reallyExit](Se& e) {
					_reallyExit(rc, reallyExit);
				}));
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				// _reallyExit(rc, reallyExit);
			} else {
				_reallyExit(rc, reallyExit);
			}
		} else {
			DLOG("The program has exited");
		}
	}

};

Thread::Thread(){}
Thread::~Thread(){}

ThreadID Thread::spawn(Exec exec, cString& name) {
	return Inl::run(exec, name);
}

void Thread::join(ID id, int64 timeoutUs) {
	if (id == current_id()) {
		DLOG("Thread::join(), cannot join self");
		return;
	}
	Lock lock(*threads_mutex);
	auto i = threads->find(id);
	if ( !i.is_null() ) {
		ListenSignal signal = { i.value() };
		auto it = threads_end_listens->push(&signal);
		{ //
			Lock l(signal.mutex);
			lock.unlock();
			String name = i.value()->name();
			DLOG("Thread::join, ..., %p, %s", id, *name);
			if (timeoutUs > 0) {
				signal.cond.wait_for(l, std::chrono::microseconds(timeoutUs)); // wait
			} else {
				signal.cond.wait(l); // permanent wait
			}
			DLOG("Thread::join, end, %p, %s", id, *name);
		}
		lock.lock();
		threads_end_listens->del(it);
	}
}

/**
 * @func sleep
 */
void Thread::sleep(int64 timeoutUs) {
	if ( timeoutUs > 0 && timeoutUs < 5e5 /*500ms*/ ) {
		std::this_thread::sleep_for(std::chrono::microseconds(timeoutUs));
		return;
	}
	auto cur = current();
	if ( cur ) {
		Lock lock(cur->m_mutex);
		if ( !cur->m_abort ) {
			if (timeoutUs < 1) {
				cur->m_cond.wait(lock); // wait
			} else {
				cur->m_cond.wait_for(lock, std::chrono::microseconds(timeoutUs));
			}
		} else {
			XX_WARN("Thread aborted, cannot sleep");
		}
	} else {
		XX_WARN("Cannot find current qgr::Thread handle, use std::this_thread::sleep_for()");
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
	if ( !i.is_null() ) {
		_inl_t(i.value())->awaken(); // awaken sleep status
	}
}

void Thread::abort(ID id) {
	ScopeLock lock(*threads_mutex);
	auto i = threads->find(id);
	if ( !i.is_null() ) {
		_inl_t(i.value())->awaken(true); // awaken sleep status and abort
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

void _exit(int rc, bool reallyExit) {
	Thread::Inl::exit(rc, reallyExit);
}

void exit(int rc) {
	Thread::Inl::exit(rc, 1);
}

bool is_exited() {
	return __is_process_exit;
}

XX_INIT_BLOCK(thread_init_once) {
	DLOG("thread_init_once");
	atexit(Thread::Inl::atexit_exec);
	Thread::Inl::thread_initialize();
}

// --------------------- ThreadRunLoop ---------------------

class RunLoop::Inl: public RunLoop {
 public:
 #define _inl(self) static_cast<RunLoop::Inl*>(self)
	
	void run(int64 timeout) {
		if (__is_process_exit) {
			DLOG("cannot run RunLoop, __is_process_exit != 0");
			return;
		}
		if (m_thread->is_abort()) {
			DLOG("cannot run RunLoop, m_thread->is_abort() == true");
			return;
		}
		uv_async_t uv_async;
		uv_timer_t uv_timer;
		{ //
			ScopeLock lock(m_mutex);
			XX_CHECK(Thread::current_id() == m_tid, "Must run on the target thread");
			XX_CHECK(!m_uv_async);
			m_timeout = XX_MAX(timeout, 0);
			m_record_timeout = 0;
			m_uv_async = &uv_async; uv_async.data = this;
			m_uv_timer = &uv_timer; uv_timer.data = this;
			uv_async_init(m_uv_loop, m_uv_async, (uv_async_cb)(resolve_queue_before));
			uv_timer_init(m_uv_loop, m_uv_timer);
			activate_loop();
		}
		uv_run(m_uv_loop, UV_RUN_DEFAULT); // run uv loop
		close_uv_async();
		{ // loop end
			ScopeLock lock(m_mutex);
			m_uv_async = nullptr;
			m_uv_timer = nullptr;
			m_timeout = 0;
			m_record_timeout = 0;
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
		// m_uv_async 外是否还有活着的`handle`与请求
		// uv_loop_alive(uv_loop*)
		uv_loop_t* loop = m_uv_loop;
		return m_uv_loop->active_handles > 1 ||
					 QUEUE_EMPTY(&(loop)->active_reqs) == 0 ||
					 loop->closing_handles != NULL;
	}
	
	void close_uv_async() {
		if (!uv_is_closing((uv_handle_t*)m_uv_async))
			uv_close((uv_handle_t*)m_uv_async, nullptr); // close async
		uv_timer_stop(m_uv_timer);
		if (!uv_is_closing((uv_handle_t*)m_uv_timer))
			uv_close((uv_handle_t*)m_uv_timer, nullptr);
	}
	
	inline void uv_timer_req(int64 timeout_ms) {
		uv_timer_start(m_uv_timer, (uv_timer_cb)resolve_queue_before, timeout_ms, 0);
	}
	
	bool resolve_queue_after(int64 timeout_ms) {
		bool Continue = 0;
		if (m_uv_loop->stop_flag != 0) { // 循环停止标志
			close_uv_async();
		}
		else if (timeout_ms == -1) { //
			if (m_keeps.length() == 0 && m_works.length() == 0) {
				// RunLoop 已经没有需要处理的消息
				if (is_alive()) { // 如果uv还有其它活着,那么间隔一秒测试一次
					uv_timer_req(1000);
					m_record_timeout = 0; // 取消超时记录
				} else { // 已没有活着的其它uv请求
					if (m_record_timeout) { // 如果已开始记录继续等待
						int64 timeout = (sys::time_monotonic() - m_record_timeout - m_timeout) / 1000;
						if (timeout >= 0) { // 已经超时
							close_uv_async();
						} else { // 继续等待超时
							uv_timer_req(-timeout);
						}
					} else {
						int64 timeout = m_timeout / 1000;
						if (timeout > 0) { // 需要等待超时
							m_record_timeout = sys::time_monotonic(); // 开始记录超时
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
				uv_async_send(m_uv_async);
				// Continue = 1; // continue
			}
			m_record_timeout = 0; // 取消超时记录
		}
		return Continue;
	}
	
	void resolve_queue(List<Queue>& queue) {
		int64 now = sys::time_monotonic();
		for (auto i = queue.begin(), e = queue.end(); i != e; ) {
			auto t = i++;
			if (now >= t.value().time) { //
				sync_callback(t.value().resolve, nullptr, this);
				queue.del(t);
			}
		}
	}
	
	bool resolve_queue() {
		List<Queue> queue;
		{ ScopeLock lock(m_mutex);
			if (m_queue.length()) {
				queue = move(m_queue);
			} else {
				return resolve_queue_after(-1);
			}
		}
		if (queue.length()) {
			if (m_independent_mutex) {
				std::lock_guard<RecursiveMutex> lock(*m_independent_mutex);
				resolve_queue(queue);
			} else {
				resolve_queue(queue);
			}
		}
		{ ScopeLock lock(m_mutex);
			m_queue.unshift(move(queue));
			if (m_queue.length() == 0) {
				return resolve_queue_after(-1);
			}
			int64 now = sys::time_monotonic();
			int64 duration = Int64::max;
			for ( auto& i : m_queue ) {
				int64 du = i.value().time - now;
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
	uint post(cCb& exec, uint group, uint64 delay_us) {
		if (m_thread->is_abort()) {
			DLOG("RunLoop::post, m_thread->is_abort() == true");
			return 0;
		}
		ScopeLock lock(m_mutex);
		uint id = iid32();
		if (delay_us) {
			int64 time = sys::time_monotonic() + delay_us;
			m_queue.push({ id, group, time, exec });
		} else {
			m_queue.push({ id, group, 0, exec });
		}
		activate_loop(); // 通知继续
		return id;
	}

	void post_sync(cCb& exec, uint group, uint64 delay_us) {
		if (Thread::current_id() == m_thread->id()) { // 相同的线程立即执行
			sync_callback(exec, nullptr, this);
		} else {
			if (m_thread->is_abort()) {
				DLOG("RunLoop::post_sync, m_thread->is_abort() == true");
				return;
			}

			struct Ctx { bool ok; Condition cond; } ctx = {false};

			Lock lock(m_mutex);
			Ctx* ctxp = &ctx;

			m_queue.push({
				0, group, 0,
				Callback([exec, ctxp, this](Se& e) {
					exec->call(e);
					ScopeLock scope(m_mutex);
					ctxp->ok = true;
					ctxp->cond.notify_all();
				})
			});
			activate_loop(); // 通知继续

			do {
				ctx.cond.wait(lock);
			} while(!ctx.ok);
		}
	}
	
	inline void cancel_group(uint group) {
		ScopeLock lock(m_mutex);
		cancel_group_non_lock(group);
	}

	void cancel_group_non_lock(uint group) {
		for (auto i = m_queue.begin(), e = m_queue.end(); i != e; ) {
			auto j = i++;
			if (j.value().group == group) {
				// TODO: 删除时如果析构函数间接调用锁定`m_mutex`的函数会锁死线程
				m_queue.del(j);
			}
		}
		activate_loop(); // 通知继续
	}
	
	inline void activate_loop() {
		if (m_uv_async)
			uv_async_send(m_uv_async);
	}
	
	inline void delete_work(List<Work*>::Iterator it) {
		m_works.del(it);
	}
};

/**
 * @struct RunLoop::Work
 */
struct RunLoop::Work {
	typedef NonObjectTraits Traits;
	RunLoop* host;
	uint id;
	List<Work*>::Iterator it;
	Callback work;
	Callback done;
	uv_work_t uv_req;
	String name;
	static void uv_work_cb(uv_work_t* req) {
		Work* self = (Work*)req->data;
		sync_callback(self->work, nullptr, self->host);
	}
	static void uv_after_work_cb(uv_work_t* req, int status) {
		Handle<Work> self = (Work*)req->data;
		if (self->host->m_independent_mutex) {
			std::lock_guard<RecursiveMutex> lock(*self->host->m_independent_mutex);
			self->done_work(status);
		} else {
			self->done_work(status);
		}
	}
	void done_work(int status) {
		_inl(host)->delete_work(it);
		if (UV_ECANCELED != status) { // cancel
			sync_callback(done, nullptr, host);
		}
		_inl(host)->activate_loop();
	}
};

void RunLoop::Inl::stop_after_print_message() {
	ScopeLock lock(m_mutex);
	for (auto& i: m_keeps) {
		DLOG("Print: RunLoop keep not release \"%s\"", *i.value()->m_name);
	}
	for (auto& i: m_works) {
		DLOG("Print: RunLoop work not complete: \"%s\"", *i.value()->name);
	}
}

/**
 * @constructor
 */
RunLoop::RunLoop(Thread* t)
: m_independent_mutex(nullptr)
, m_thread(t)
, m_tid(t->id())
, m_uv_loop(nullptr)
, m_uv_async(nullptr)
, m_uv_timer(nullptr)
, m_timeout(0)
, m_record_timeout(0)
{
	_inl_t(t)->set_run_loop(this);
	m_uv_loop = uv_loop_new();
}

/**
 * @destructor
 */
RunLoop::~RunLoop() {
	ScopeLock lock(*threads_mutex);
	XX_CHECK(m_uv_async == nullptr, "Secure deletion must ensure that the run loop has exited");

	{
		ScopeLock lock(m_mutex);
		for (auto& i: m_keeps) {
			XX_WARN("RunLoop keep not release \"%s\"", *i.value()->m_name);
			i.value()->m_loop = nullptr;
		}
		for (auto& i: m_works) {
			XX_WARN("RunLoop work not complete: \"%s\"", *i.value()->name);
			delete i.value();
		}
	}

	if (m_uv_loop != uv_default_loop()) {
		uv_loop_delete(m_uv_loop);
	}
	_inl_t(m_thread)->del_run_loop(this);
}

/**
 * @func current() 获取当前线程消息队列
 */
RunLoop* RunLoop::current() {
	auto t = Thread::Inl::get_thread_specific_data();
	XX_CHECK(t, "Can't get thread specific data");
	auto loop = t->loop();
	if (!loop) {
		ScopeLock scope(*threads_mutex);
		loop = new RunLoop(t);
		if (!main_loop_obj) {
			main_loop_obj = loop;
			main_loop_id = t->id();
			uv_loop_delete(loop->m_uv_loop);
			loop->m_uv_loop = uv_default_loop();
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
		XX_CHECK(main_loop_obj);
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
	return m_uv_async;
}

/**
 * TODO: Be careful about thread security issues
 * @func is_alive
 */
bool RunLoop::is_alive() const {
	return uv_loop_alive(m_uv_loop) /*|| m_keeps.length() || m_works.length()*/;
}

/**
 * TODO: Be careful about thread security issues
 * @func sync # 延时
 */
uint RunLoop::post(cCb& cb, uint64 delay_us) {
	return _inl(this)->post(cb, 0, delay_us);
}

/**
 * TODO: Be careful about thread security issues
 * @func post_sync(cb)
 */
void RunLoop::post_sync(cCb& cb) {
	_inl(this)->post_sync(cb, 0, 0);
}

/**
 * TODO: Be careful about thread security issues
 * @func work()
 */
uint RunLoop::work(cCb& cb, cCb& done, cString& name) {
	if (m_thread->is_abort()) {
		DLOG("RunLoop::work, m_thread->is_abort() == true");
		return 0;
	}

	Work* work = new Work();
	work->id = iid32();
	work->work = cb;
	work->done = done;
	work->uv_req.data = work;
	work->host = this;
	work->name = name;

	post(Cb([work, this](Se& ev) {
		int r = uv_queue_work(m_uv_loop, &work->uv_req,
													Work::uv_work_cb, Work::uv_after_work_cb);
		XX_ASSERT(!r);
		work->it = m_works.push(work);
	}));

	return work->id;
}

/**
 * TODO: Be careful about thread security issues
 * @func cancel_work(id)
 */
void RunLoop::cancel_work(uint id) {
	post(Cb([=](Se& ev) {
		for (auto& i : m_works) {
			if (i.value()->id == id) {
				int r = uv_cancel((uv_req_t*)&i.value()->uv_req);
				XX_ASSERT(!r);
				break;
			}
		}
	}));
}

/**
 * TODO: Be careful about thread security issues
 * @overwrite
 */
uint RunLoop::post_message(cCb& cb, uint64 delay_us) {
	return _inl(this)->post(cb, 0, delay_us);
}

/**
 * TODO: Be careful about thread security issues
 * @func cancel # 取消同步
 */
void RunLoop::cancel(uint id) {
	ScopeLock lock(m_mutex);
	for (auto& i : m_queue) {
		if (i.value().id == id) {
			// TODO: 删除时如果析构函数间接调用锁定`m_mutex`的函数会锁死线程
			m_queue.del(i); break;
		}
	}
	_inl(this)->activate_loop();
}

/**
 * @func run() 运行消息循环
 */
void RunLoop::run(uint64 timeout) {
	_inl(this)->run(timeout);
}

/**
 * TODO: Be careful about thread security issues
 * @func stop
 */
void RunLoop::stop() {
	if ( runing() ) {
		post(Cb([this](Se& se) {
			uv_stop(m_uv_loop);
		}));
	}
}

/**
 * TODO: Be careful about thread security issues
 * 保持活动状态,并返回一个代理,只要不删除返回的代理对像,消息队列会一直保持活跃状态
 * @func keep_alive
 */
KeepLoop* RunLoop::keep_alive(cString& name, bool declear) {
	ScopeLock lock(m_mutex);
	auto keep = new KeepLoop(name, declear);
	keep->m_id = m_keeps.push(keep);
	keep->m_loop = this;
	return keep;
}

static RunLoop* loop_2(ThreadID id) {
	auto i = threads->find(id);
	if (i.is_null()) {
		return nullptr;
	}
	return i.value()->loop();
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
void RunLoop::next_tick(cCb& cb) throw(Error) {
	RunLoop* loop = RunLoop::current();
	if ( loop ) {
		loop->post(cb);
	} else { // 没有消息队列 post to io loop
		XX_THROW(ERR_NOT_RUN_LOOP, "Unable to obtain thread io run loop");
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
: m_group(iid32()), m_name(name), m_declear(destructor_clear) {
}

KeepLoop::~KeepLoop() {
	ScopeLock lock(*threads_mutex);

	if (m_loop) {
		ScopeLock lock(m_loop->m_mutex);
		if ( m_declear ) {
			_inl(m_loop)->cancel_group_non_lock(m_group);
		}
		XX_CHECK(m_loop->m_keeps.length());

		m_loop->m_keeps.del(m_id); // 减少一个引用计数

		if (m_loop->m_keeps.length() == 0 && !m_loop->m_uv_loop->stop_flag) { // 可以结束了
			_inl(m_loop)->activate_loop(); // 激活循环状态,不再等待
		}
	} else {
		DLOG("Keep already invalid \"%s\", RunLoop already stop and release", *m_name);
	}
}

uint KeepLoop::post(cCb& exec, uint64 delay_us) {
	// TODO: Be careful about thread security issues
	if (m_loop)
		return _inl(m_loop)->post(exec, m_group, delay_us);
	else
		return 0;
}

uint KeepLoop::post_message(cCb& cb, uint64 delay_us) {
	// TODO: Be careful about thread security issues
	if (m_loop)
		return _inl(m_loop)->post(cb, m_group, delay_us);
	else
		return 0;
}

void KeepLoop::cancel_all() {
	// TODO: Be careful about thread security issues
	if (m_loop)
		_inl(m_loop)->cancel_group(m_group); // abort all
}

void KeepLoop::cancel(uint id) {
	// TODO: Be careful about thread security issues
	if (m_loop)
		m_loop->cancel(id);
}

// ************** ParallelWorking **************

/**
 * @constructor
 */
ParallelWorking::ParallelWorking(): ParallelWorking(RunLoop::current()) {}

ParallelWorking::ParallelWorking(RunLoop* loop) : m_proxy(nullptr) {
	XX_CHECK(loop, "Can not find current thread run loop.");
	m_proxy = loop->keep_alive("ParallelWorking()");
}

/**
 * @destructor
 */
ParallelWorking::~ParallelWorking() {
	abort_child();
	Release(m_proxy); m_proxy = nullptr;
}

/**
 * @func run
 */
ThreadID ParallelWorking::spawn_child(Exec exec, cString& name) {
	ScopeLock scope(m_mutex2);
	auto id = Thread::Inl::run([this, exec](Thread& t) {
		int rc = exec(t);
		ScopeLock scope(m_mutex2);
		m_childs.del(t.id());
		return rc;
	}, name);
	m_childs.set(id, 1);
	return id;
}

/**
 * @func abort_child
 */
void ParallelWorking::abort_child(ThreadID id) {
	if ( id == ThreadID() ) {
		Map<ThreadID, int> childs;
		{
			ScopeLock scope(m_mutex2);
			childs = m_childs;
		}
		for (auto& i : childs) {
			Thread::abort(i.key());
		}
		for (auto& i : childs) {
			Thread::join(i.key());
		}
		DLOG("ParallelWorking::abort_child() ok, count: %d", childs.length());
	} else {
		{
			ScopeLock scope(m_mutex2);
			XX_CHECK(m_childs.has(id), 
				"Only subthreads belonging to \"ParallelWorking\" can be aborted");
		}
		Thread::abort(id);
		Thread::join(id);
		DLOG("ParallelWorking::abort_child(id) ok");
	}
}

/**
 * @func awaken
 */
void ParallelWorking::awaken_child(ThreadID id) {
	ScopeLock scope(m_mutex2);
	if ( id == ThreadID() ) {
		for (auto& i : m_childs) {
			Thread::awaken(i.key());
		}
	} else {
		XX_CHECK(m_childs.has(id), 
			"Only subthreads belonging to \"ParallelWorking\" can be awaken");
		Thread::awaken(id);
	}
}

/**
 * @func post message to main thread
 */
uint ParallelWorking::post(cCb& exec) {
	return m_proxy->post(exec);
}

/**
 * @func post
 */
uint ParallelWorking::post(cCb& exec, uint64 delayUs) {
	return m_proxy->post(exec, delayUs);
}

/**
 * @func cancel
 */
void ParallelWorking::cancel(uint id) {
	if ( id ) {
		m_proxy->cancel(id);
	} else {
		m_proxy->cancel_all();
	}
}

XX_END

