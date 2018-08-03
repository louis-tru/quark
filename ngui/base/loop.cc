
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

#include "loop.h"
#include "loop-1.h"
#include "sys.h"
#if XX_ANDROID
# include "os/android-jni.h"
#endif
#include <uv.h>
#include <pthread.h>
#include "../../node/deps/uv/src/queue.h"

#ifndef XX_ATEXIT_WAIT_TIMEOUT
# define XX_ATEXIT_WAIT_TIMEOUT 1e6
#endif

XX_NS(ngui)

template<> uint Compare<ThreadID>::hash(const ThreadID& key) {
	ThreadID* p = const_cast<ThreadID*>(&key);
	size_t* t = reinterpret_cast<size_t*>(p);
	return (*t) % Uint::max;
}
template<> bool Compare<ThreadID>::equals(const ThreadID& a,
																					const ThreadID& b, uint ha, uint hb) {
	return a == b;
}

// ----------------------------------------------------------

struct ListenSignal {
	SimpleThread* thread;
	Mutex mutex;
	Condition cond;
};

static Mutex* all_threads_mutex;
static Map<ThreadID, SimpleThread*>* all_threads = nullptr;
static List<ListenSignal*>* all_thread_end_listens = nullptr;
static RunLoop* main_loop_obj = nullptr;
static pthread_key_t specific_key;
int process_exit = 0;

XX_DEFINE_INLINE_MEMBERS(SimpleThread, Inl) {
public:
#define _inl_t(self) static_cast<SimpleThread::Inl*>(self)
	
	static void thread_destructor(void* ptr) {
		auto thread = reinterpret_cast<SimpleThread*>(ptr);
		if (process_exit == 0) { // no process exit
			if (main_loop_obj == thread->m_loop) {
				main_loop_obj = nullptr;
			}
			Release(thread->m_loop);
		}
		delete thread;
	}
	
	static void thread_initialize() {
		all_threads = new Map<ThreadID, SimpleThread*>();
		all_threads_mutex = new Mutex;
		all_thread_end_listens = new List<ListenSignal*>();
		int err = pthread_key_create(&specific_key, thread_destructor);
		XX_CHECK(err == 0);
	}
	
	static void set_thread_specific_data(SimpleThread* thread) {
		XX_CHECK(!pthread_getspecific(specific_key));
		pthread_setspecific(specific_key, thread);
	}
	
	static inline SimpleThread* get_thread_specific_data() {
		return reinterpret_cast<SimpleThread*>(pthread_getspecific(specific_key));
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
 
	static void run2(Exec body, SimpleThread* thread) {
#if XX_ANDROID
		JNI::ScopeENV scope;
#endif
		set_thread_specific_data(thread);
		if ( !thread->m_abort ) {
			body(*thread);
		}
		{
			ScopeLock scope(*all_threads_mutex);
			for (auto& i : *all_thread_end_listens) {
				ListenSignal* s = i.value();
				if (s->thread == thread) {
					ScopeLock scope(s->mutex);
					s->cond.notify_one();
				}
			}
			all_threads->del(thread->id());
		}
	}
	
	static ThreadID run(Exec exec, uint gid, cString& name) {
		if ( process_exit ) {
			return ThreadID();
		} else {
			ScopeLock scope(*all_threads_mutex);
			SimpleThread* thread = new SimpleThread();
			std::thread std_thread(run2, exec, thread);
			thread->m_gid = gid;
			thread->m_id = std_thread.get_id();
			thread->m_name = name;
			thread->m_abort = false;
			thread->m_loop = nullptr;
			memset(thread->m_data, 0, sizeof(void*[256]));
			all_threads->set(thread->m_id, thread);
			std_thread.detach();
			return thread->m_id;
		}
	}
	
	static void abort_group(uint gid) {
		ScopeLock scope(*all_threads_mutex);
		for ( auto& i : *all_threads ) {
			auto thread = i.value();
			if ( thread->m_gid == gid ) {
				ScopeLock lock(thread->m_mutex);
				thread->m_abort = true;
				thread->m_cond.notify_one(); // awaken sleep status
			}
		}
	}
	
	static void awaken_group(uint gid) {
		ScopeLock scope(*all_threads_mutex);
		for ( auto& i : *all_threads ) {
			auto thread = i.value();
			if (thread->m_gid == gid) {
				ScopeLock lock(thread->m_mutex);
				thread->m_cond.notify_one(); // awaken sleep status
			}
		}
	}
	
	static void atexit_exec() {
		process_exit++; // exit
		Array<ThreadID> ths;
		{ //
			ScopeLock scope(*all_threads_mutex);
			for ( auto& i : *all_threads ) {
				auto t = i.value();
				ScopeLock scope(t->m_mutex);
				if (t->m_loop) {
					t->m_loop->stop();
				}
				t->m_abort = true;
				t->m_cond.notify_one(); // awaken sleep status
				ths.push(t->id());
			}
		}
		ThreadID id = current_id();
		for ( auto& i: ths ) {
			if (i.value() != id) {
				// 在这里等待这个线程的结束,这个时间默认为1秒钟
				wait_end(i.value(), XX_ATEXIT_WAIT_TIMEOUT); // wait 1s
			}
		}
	}
	
};

void* SimpleThread::get_specific_data(char id) {
	return Inl::get_thread_specific_data()->m_data[byte(id)];
}

void SimpleThread::set_specific_data(char id, void* data) {
	Inl::get_thread_specific_data()->m_data[byte(id)] = data;
}

ThreadID SimpleThread::detach(Exec exec, cString& name) {
	return Inl::run(exec, 0, name);
}

void SimpleThread::abort(ThreadID id, int64 wait_end_timeoutUs) {
	Lock lock(*all_threads_mutex);
	auto i = all_threads->find(id);
	if ( !i.is_null() ) {
		auto t = i.value();
		auto& t2 = *t;
		XX_THREAD_LOCK(t2, {
			t->m_abort = true;
			t->m_cond.notify_one(); // awaken sleep status
		});
		if ( wait_end_timeoutUs ) {
			ListenSignal signal = { t };
			auto it = all_thread_end_listens->push(&signal);
			{ //
				Lock l(signal.mutex);
				lock.unlock();
				if (wait_end_timeoutUs > 0) {
					signal.cond.wait_for(l, std::chrono::microseconds(wait_end_timeoutUs)); // wait
				} else {
					signal.cond.wait(l); // wait
				}
			}
			lock.lock();
			all_thread_end_listens->del(it);
		}
	}
}

void SimpleThread::wait_end(ThreadID id, int64 timeoutUs) {
	Lock lock(*all_threads_mutex);
	auto i = all_threads->find(id);
	if ( !i.is_null() ) {
		ListenSignal signal = { i.value() };
		auto it = all_thread_end_listens->push(&signal);
		{ //
			Lock l(signal.mutex);
			lock.unlock();
			if (timeoutUs > 0) {
				signal.cond.wait_for(l, std::chrono::microseconds(timeoutUs)); // wait
			} else {
				signal.cond.wait(l); // wait
			}
		}
		lock.lock();
		all_thread_end_listens->del(it);
	}
}

/**
 * @func sleep_for
 */
void SimpleThread::sleep_for(uint64 timeUs) {
	auto cur = current();
	if ( cur ) {
		Lock lock(cur->m_mutex);
		if ( !cur->m_abort ) {
			if (timeUs == 0) {
				cur->m_cond.wait(lock); // wait
			} else {
				cur->m_cond.wait_for(lock, std::chrono::microseconds(timeUs));
			}
		}
	} else {
		std::this_thread::sleep_for(std::chrono::microseconds(timeUs));
	}
}

/**
 * @func current_id
 */
ThreadID SimpleThread::current_id() {
	return std::this_thread::get_id();
}

/**
 * @func current
 */
SimpleThread* SimpleThread::current() {
	return Inl::get_thread_specific_data();
}

/**
 * @func awaken
 */
void SimpleThread::awaken(ThreadID id) {
	ScopeLock lock(*all_threads_mutex);
	auto i = all_threads->find(id);
	if ( !i.is_null() ) {
		ScopeLock scope(i.value()->m_mutex);
		i.value()->m_cond.notify_one(); // awaken sleep status
	}
}

XX_INIT_BLOCK(thread_init_once) {
	atexit(SimpleThread::Inl::atexit_exec);
	SimpleThread::Inl::thread_initialize();
}

// --------------------- ThreadRunLoop ---------------------

class RunLoop::Inl: public RunLoop {
 public:
#define _inl(self) static_cast<RunLoop::Inl*>(self)
	
	void run(int64 timeout) {
		if (process_exit) return;
		uv_async_t uv_async;
		uv_timer_t uv_timer;
		{ //
			ScopeLock lock(m_mutex);
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
	}
	
	static void resolve_queue_before(uv_handle_t* handle) {
		((Inl*)handle->data)->resolve_queue();
	}
	
	bool is_alive() {
		// m_uv_async 外是否还有活着的`handle`与请求
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
	
	void resolve_queue_after(int64 timeout_ms) {
		if (m_uv_loop->stop_flag != 0) { // 循环停止标志
			close_uv_async();
		}
		else if (timeout_ms == -1) { //
			if (m_keep_count == 0 && m_work.length() == 0) {
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
						if (timeout > 0) { // 需要等待超时
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
				uv_async_send(m_uv_async);
			}
			m_record_timeout = 0; // 取消超时记录
		}
	}
	
	void resolve_queue(List<Queue>& queue) {
		int64 now = sys::time_monotonic();
		for (auto i = queue.begin(), e = queue.end(); i != e; ) {
			auto t = i++;
			if (now >= t.value().time) { //
				SimpleEvent data = { 0, this, 0 };
				t.value().resolve->call(data);
				queue.del(t);
			}
		}
	}
	
	void resolve_queue() {
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
			resolve_queue_after(duration / 1e3);
		}
	}
	
	/**
	 * @func post
	 */
	uint post(cCb& exec, uint group, uint64 delay_us) {
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
	
	inline void abort_group(uint group) {
		ScopeLock lock(m_mutex);
		abort_group_non_lock(group);
	}

	void abort_group_non_lock(uint group) {
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
		m_work.del(it);
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
	static void uv_work_cb(uv_work_t* req) {
		Work* self = (Work*)req->data;
		Se e = { 0, self->host, 0 };
		self->work->call(e);
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
			Se e = { 0, host, 0 };
			done->call(e);
		}
		_inl(host)->activate_loop();
	}
};

/**
 * @constructor
 */
RunLoop::RunLoop(SimpleThread* t)
: m_keep_count(0)
, m_independent_mutex(nullptr)
, m_thread(t)
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
	XX_CHECK(m_keep_count == 0);
	XX_CHECK(m_work.length() == 0);
	XX_CHECK(m_uv_async == nullptr, "Secure deletion must ensure that the run loop has exited");
	if (m_uv_loop != uv_default_loop()) {
		uv_loop_delete(m_uv_loop);
	}
	_inl_t(m_thread)->del_run_loop(this);
}

/**
 * @func current() 获取当前线程消息队列
 */
RunLoop* RunLoop::current() {
	auto t = SimpleThread::Inl::get_thread_specific_data();
	XX_CHECK(t);
	auto loop = t->loop();
	if (!loop) {
		 loop = new RunLoop(t);
	}
	return loop;
}

/**
 * @func main_loop();
 */
RunLoop* RunLoop::main_loop() {
	if (!main_loop_obj) {
		ScopeLock scope(*all_threads_mutex);
		main_loop_obj = current();
		uv_loop_delete(main_loop_obj->m_uv_loop);
		main_loop_obj->m_uv_loop = uv_default_loop();
	}
	return main_loop_obj;
}

/**
 * @func is_main_loop 当前线程是为主循环
 */
bool RunLoop::is_main_loop() {
	if (main_loop_obj) {
		return main_loop_obj == current();
	}
	return false;
}

/**
 * @func is_process_exit
 */
bool RunLoop::is_process_exit() {
	return process_exit;
}

/**
 * @func runing()
 */
bool RunLoop::runing() const {
	return m_uv_async;
}

/**
 * @func is_alive
 */
bool RunLoop::is_alive() const {
	return uv_loop_alive(m_uv_loop);
}

/**
 * @func sync # 延时
 */
uint RunLoop::post(cCb& cb, uint64 delay_us) {
	return _inl(this)->post(cb, 0, delay_us);
}

/**
 * @func work()
 */
uint RunLoop::work(cCb& cb, cCb& done) {
	Work* work = new Work();
	work->id = iid32();
	work->work = cb;
	work->done = done;
	work->uv_req.data = work;
	work->host = this;
	post(Cb([work, this](Se& ev) {
		int r = uv_queue_work(m_uv_loop, &work->uv_req,
													Work::uv_work_cb, Work::uv_after_work_cb);
		XX_ASSERT(!r);
		work->it = m_work.push(work);
	}));
	return work->id;
}

/**
 * @func cancel_work(id)
 */
void RunLoop::cancel_work(uint id) {
	post(Cb([=](Se& ev) {
		for (auto& i : m_work) {
			if (i.value()->id == id) {
				int r = uv_cancel((uv_req_t*)&i.value()->uv_req);
				XX_ASSERT(!r);
				break;
			}
		}
	}));
}

/**
 * @overwrite
 */
uint RunLoop::post_message(cCb& cb, uint64 delay_us) {
	return _inl(this)->post(cb, 0, delay_us);
}

/**
 * @func abort # 中止同步
 */
void RunLoop::abort(uint id) {
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
 * 保持活动状态,并返回一个代理,只要不删除返回的代理对像,消息队列会一直保持活跃状态
 * @func keep_alive
 */
KeepLoop* RunLoop::keep_alive(bool declear) {
	ScopeLock lock(m_mutex);
	m_keep_count++;  // 增加一个引用计数
	KeepLoop* rv = new KeepLoop(declear);
	rv->m_loop = this;
	return rv;
}

/**
 * @func loop(id) 通过线程获取
 * @ret {RunLoop*}
 */
RunLoop* RunLoop::loop(ThreadID id) {
	ScopeLock scope(*all_threads_mutex);
	auto i = all_threads->find(id);
	if (i.is_null()) {
		return nullptr;
	}
	return i.value()->loop();
}

/**
 * @func keep_alive_current 保持当前循环活跃并返回代理
 */
KeepLoop* RunLoop::keep_alive_current(bool declear) {
	RunLoop* loop = current();
	if ( loop ) {
		return loop->keep_alive(declear);
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

KeepLoop::~KeepLoop() {
	ScopeLock lock(m_loop->m_mutex);
	if ( m_declear ) {
		_inl(m_loop)->abort_group_non_lock(m_group);
	}
	if (m_loop->m_keep_count > 0) { // 减少一个引用计数
		m_loop->m_keep_count--;
	}
	if (m_loop->m_keep_count == 0) { // 可以结束了
		_inl(m_loop)->activate_loop(); // 激活循环状态,不再等待
	}
}

uint KeepLoop::post(cCb& exec, uint64 delay_us) {
	return _inl(m_loop)->post(exec, m_group, delay_us);
}

uint KeepLoop::post_message(cCb& cb, uint64 delay_us) {
	return _inl(m_loop)->post(cb, m_group, delay_us);
}

void KeepLoop::clear() {
	_inl(m_loop)->abort_group(m_group); // abort all
}

/**
 * @constructor
 */
ParallelWorking::ParallelWorking() : ParallelWorking(RunLoop::current()) { }

ParallelWorking::ParallelWorking(RunLoop* loop) : m_proxy(nullptr), m_gid(iid32()) {
	XX_CHECK(loop, "Can not find current thread run loop.");
	m_proxy = loop->keep_alive();
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
ThreadID ParallelWorking::detach_child(Exec exec, cString& name) {
	return SimpleThread::Inl::run(exec, m_gid, name);
}

/**
 * @func abort_child
 */
void ParallelWorking::abort_child(ThreadID id) {
	if ( id == ThreadID() ) {
		SimpleThread::Inl::abort_group(m_gid);
	} else {
		SimpleThread::abort(id);
	}
}

/**
 * @func awaken
 */
void ParallelWorking::awaken_child(ThreadID id) {
	if ( id == ThreadID() ) {
		SimpleThread::Inl::awaken_group(m_gid);
	} else {
		SimpleThread::awaken(id);
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
 * @func abort
 */
void ParallelWorking::abort(uint id) {
	if ( id ) {
		m_proxy->abort(id);
	} else {
		m_proxy->clear();
	}
}

XX_END

