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

#include "./loop_.h"

namespace qk {
	static RunLoop *__first_loop = nullptr;

	Qk_DEFINE_INLINE_MEMBERS(RunLoop, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<RunLoop::Inl*>(self)

		struct timer_t: public uv_timer_t {
			uint32_t id;
			Cb cb;
		};

		void stop_after_print_message() {
			ScopeLock lock(_mutex);
			for (auto& i: _keep) {
				Qk_DEBUG("Print: RunLoop keep not release \"%s\"", i);
			}
			for (auto& i: _work) {
				Qk_DEBUG("Print: RunLoop work not complete: \"%s\"", i.value);
			}
		}

		void keep_clear(Keep *keep) {
			ScopeLock lock(_mutex);
			Qk_ASSERT(_keep.length());

			if (keep->_uv_check) {
				uv_check_stop(keep->_uv_check);
				::free(keep->_uv_check);
			}
			_keep.erase(keep->_id); // delete keep object for runloop

			if (_keep.length() == 0 && !_uv_loop->stop_flag) { // 可以结束了
				async_send(); // 激活循环状态,不再等待
			}
		}

		static void timer_handle(uv_timer_t* handle) {
			auto timer = (timer_t*)handle;
			auto self = _inl(timer->data);
			if (timer->cb->resolve() || timer->repeat == 0) {
				uv_timer_stop(timer);
				self->_timer.erase(timer->id);
				delete timer;
			}
		}

		void timer_start(uv_timer_t *handle) {
			auto timer = (timer_t*)handle;
			_timer.set(timer->id, timer);
			uv_timer_init(_uv_loop, timer);
			uv_timer_start(timer, timer_handle, timer->timeout, timer->repeat);
		}

		void timer_stop(uint32_t id) {
			auto it = _timer.find(id);
			if (it != _timer.end()) {
				auto timer = (timer_t*)it->value;
				Qk_ASSERT(id == timer->id);
				uv_timer_stop(timer);
				_timer.erase(id);
				delete timer;
			}
		}

		uint32_t post(Cb cb, uint64_t delayUs, uint64_t repeat) {
			if (_thread->abort) {
				Qk_WARN("RunLoop::Inl::post, _thread->abort == true"); return;
			}
			auto isSelf = thread_self_id() == _tid;
			if (delayUs) {
				timer_t *timer = new timer_t;
				*static_cast<uv_timer_t*>(timer) = uv_timer_t{
					.timer_cb=0, .timeout=delayUs/1000, .repeat=repeat, .data=this
				};
				timer->id = getId32();
				timer->cb = cb;
				if (isSelf) {
					timer_start(timer);
				} else {
					ScopeLock lock(_mutex);
					_msg.pushBack({ cb, timer });
					_this->async_send();
				}
				return timer->id;
			} else {
				if (isSelf) {
					cb->resolve();
				} else {
					ScopeLock lock(_mutex);
					_msg.pushBack({ cb, 0 });
					_this->async_send();
				}
				return 0;
			}
		}

		void handles_stop() {
			if (!uv_is_closing((uv_handle_t*)_uv_async))
				uv_close((uv_handle_t*)_uv_async, nullptr); // close handle
			uv_timer_stop(_uv_timer);
		}

		void async_send() {
			if (_uv_async)
				uv_async_send(_uv_async);
		}

		void async_handle() {
			Lock lock(_mutex);
			if (_msg.length()) {
				auto msg = std::move(_msg);
				lock.unlock();
				auto b = msg.begin(), e = msg.end();
				// process timer first, it is possible to delete the timer later on
				while (b != e) {
					auto i = b++;
					auto timer = (timer_t*)i->timer;
					if (timer) {
						Qk_ASSERT(timer->id);
						timer_start(timer);
						_msg.erase(i);
					}
				}
				for (auto &m: msg) {
					Qk_ASSERT(!m.timer);
					m.cb->resolve(this);
				}
				lock.lock();
			}

			if (_msg.length()) {
				uv_async_send(_uv_async);
			} else if (is_alive()) {
				uv_timer_start(_uv_timer, [](auto h) {
					_inl(h->data)->async_handle();
				}, 1e3, 0); // 1 second check
			} else {
				handles_stop();
			}
		}
	};

	void RunLoop::run() {
		if (__is_process_exit_safe) {
			Qk_WARN("cannot run RunLoop::run(), __is_process_exit_safe != 0"); return;
		}
		if (_thread->abort) {
			Qk_WARN("cannot run RunLoop::run(), _thread->abort != 0"); return;
		}
		Qk_ASSERT(!_uv_async, "It is running and cannot be called repeatedly");
		Qk_ASSERT(thread_self_id() == _tid, "Must run on the target thread");

		// init run
		uv_async_t uv_async = {.data=this};
		uv_timer_t uv_timer = {.data=this};
		_mutex.lock();
		_uv_async = &uv_async;
		_uv_timer = &uv_timer;
		_mutex.unlock();

		uv_timer_init(_uv_loop, _uv_timer);
		uv_async_init(_uv_loop, _uv_async, [](auto h) {
			_inl(h->data)->async_handle();
		});
		_this->async_send();

		uv_run(_uv_loop, UV_RUN_DEFAULT); // run uv loop
		_this->handles_stop();

		if (_uv_loop->closing_handles) {
			uv_run(_uv_loop, UV_RUN_ONCE); // exec close handles
		}
		Qk_ASSERT(!_uv_loop->closing_handles);

		// loop end
		_mutex.lock();
		_uv_async = nullptr;
		_uv_timer = nullptr;
		_mutex.unlock();

		_this->stop_after_print_message();
	}

	void RunLoop::Work::uv_work_cb(uv_work_t* req) {
		Work* self = (Work*)req->data;
		self->work->resolve(self->host);
	}

	void RunLoop::Work::uv_after_work_cb(uv_work_t* req, int status) {
		Sp<Work> self((Work*)req->data);
		self->done_work(status);
	}

	void RunLoop::Work::done_work(int status) {
		_inl(host)->_work.erase(id);
		if (UV_ECANCELED != status) { // cancel
			done->resolve(host);
		}
		_inl(host)->async_send();
	}

	// ----------------------------- R u n . L o o p -----------------------------

	RunLoop::RunLoop(Thread* t, uv_loop_t* uv)
		: _thread(t)
		, _tid(t->id)
		, _uv_loop(uv)
		, _uv_async(nullptr)
	{
    Qk_ASSERT(_tid != ThreadID());
		Qk_ASSERT(!static_cast<Thread_INL*>(t)->loop);
		// set run loop
		static_cast<Thread_INL*>(t)->loop = this;
	}

	RunLoop::~RunLoop() {
		ScopeLock lock(*__threads_mutex);
		Qk_Fatal_Assert(_uv_async == nullptr, "Secure deletion must ensure that the run loop has exited");

		{
			ScopeLock lock(_mutex);
			for (auto& i: _keep) {
				Qk_WARN("RunLoop keep not release \"%p\"", i);
				static_cast<Keep*>(i)->_loop = nullptr;
			}
			for (auto& i: _work) {
				Qk_WARN("RunLoop work not complete: \"%p\"", i.value);
				delete i.value;
			}
		}

		for (auto &i: _msg) {
			if (!i.timer) {
				i.cb->resolve(this); // resolve last message
			} else {
				auto timer = (Inl::timer_t*)(i.timer);
				delete timer;
			}
		}
		for (auto &i: _timer) {
			auto timer = (Inl::timer_t*)(i.value);
			uv_timer_stop(timer);
			delete timer;
		}

		if (__first_loop == this) {
			__first_loop = nullptr;
		}

		if (_uv_loop != uv_default_loop()) {
			uv_loop_delete(_uv_loop);
		}

		// delete run loop
		auto t = static_cast<Thread_INL*>(_thread);
		Qk_ASSERT(t->loop);
		Qk_ASSERT(t->loop == this);
		t->loop = nullptr;
		_thread = nullptr;
	}

	RunLoop* RunLoop::current() {
		auto t = thread_current_inl();
		if (!t) {
			return nullptr;
		}
		auto loop = t->loop;
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

	bool RunLoop::is_current(RunLoop* loop) {
		return loop && loop->_tid == thread_self_id();
	}

	RunLoop* RunLoop::first() {
		// NOTE: 小心线程安全,最好先确保已调用过`current()`
		if (!__first_loop) {
			current();
			Qk_ASSERT(__first_loop); // asset
		}
		return __first_loop;
	}

	bool RunLoop::runing() const {
		return _uv_async;
	}

	bool RunLoop::is_alive() const {
		return _uv_loop->active_reqs.count != 0 ||
					_uv_loop->active_handles > 1 ||
					_uv_loop->closing_handles != NULL ||
					_keep.length() != 0;
	}

	void RunLoop::post(Cb cb) {
		_this->post(cb, 0, 0);
	}

	uint32_t RunLoop::timer(Cb cb, uint64_t time, uint64_t repeat) {
		return _this->post(cb, time ? time : 1, repeat);
	}

	void RunLoop::timer_stop(uint32_t id) {
		if (id) {
			_this->post(Cb([this,id](auto&e) {
				_this->timer_stop(id);
			}), 0, 0);
		}
	}

	void RunLoop::next_tick(Cb cb) {
		if (_thread->abort) {
			Qk_WARN("RunLoop::next_tick, _thread->abort == true"); return;
		}
		if (thread_self_id() == _tid) {
			cb->retain();
			auto check = new uv_check_t{.data=*cb};
			uv_check_init(_uv_loop, check);
			uv_check_start(check, [](auto handle) {
				auto cb = (Cb::Core*)(handle->data);
				cb->resolve();
				cb->release();
				uv_check_stop(handle);
				delete handle;
			});
		} else {
			ScopeLock lock(_mutex);
			_msg.pushBack({ cb, 0 });
			_this->async_send();
		}
	}

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

		Data* data_p = &data;
		data.ctx = this;
		data.ok = false;

		if (thread_self_id() == _thread->id) { // is current
			lock.unlock();
			cb->resolve(data_p);
			if (data.ok)
				return;
			lock.lock();
		} else {
			_msg.pushBack({
				Cb([cb, data_p, this](auto&e) { cb->resolve(data_p); }), 0
			});
			_this->async_send();
		}

		while (!data.ok) {
			data.cond.wait(lock); // wait
		}
	}

	uint32_t RunLoop::work(Cb cb, Cb done) {
		if (_thread->abort) {
			Qk_WARN("RunLoop::work, _thread->abort != 0"); return 0;
		}
		Work* work = new Work();
		work->id = getId32();
		work->work = cb;
		work->done = done;
		work->uv_req.data = work;
		work->host = this;

		post(Cb([work, this](auto&ev) {
			int r = uv_queue_work(_uv_loop, &work->uv_req,
														Work::uv_work_cb, Work::uv_after_work_cb);
			Qk_ASSERT(!r);
			_work.set(work->id, work);
		}));

		return work->id;
	}

	void RunLoop::work_cancel(uint32_t id) {
		post(Cb([=](auto&e) {
			Work *out;
			if (_work.get(id, out)) {
				int r = uv_cancel((uv_req_t*)&out->uv_req);
				Qk_ASSERT(!r);
			}
		}));
	}

	void RunLoop::post_message(Cb cb) {
		post(cb);
	}

	void RunLoop::stop() {
		if ( runing() ) {
			post(Cb([this](auto& e) {
				uv_stop(_uv_loop);
			}));
		}
	}

	KeepLoop* RunLoop::keep_alive(void (*check)(void *ctx), void* ctx) {
		ScopeLock lock(_mutex);
		auto keep = new Keep(this, check, ctx);
		keep->_id = _keep.pushBack(keep);
		return keep;
	}

	// ----------------------------- K e e p . L o o p -----------------------------

	Keep::Keep(RunLoop *loop, void (*check)(void *ctx), void* ctx)
		:_loop(loop), _uv_check(nullptr), _check(check), _check_ctx(ctx)
	{
		if (check) {
			_uv_check = (uv_check_t*)::malloc(sizeof(uv_check_t));
			uv_check_init(_loop->uv_loop(), _uv_check);
			_uv_check->data = this;
			uv_check_start(_uv_check, [](uv_check_t *idle) {
				auto keep = ((Keep*)idle->data);
				keep->_check(keep->_check_ctx);
			});
		}
	}

	Keep::~Keep() {
		if (_loop) {
			_inl(_loop)->keep_clear(this);
		} else {
			Qk_DEBUG("Keep already invalid \"%p\", RunLoop already stop and release", this);
		}
	}

	void Keep::post_message(Cb cb) {
		// NOTE: Be careful about thread security issues
		if (_loop)
			_inl(_loop)->post(cb, 0, 0);
	}

	RunLoop* KeepLoop::loop() {
		return static_cast<Keep*>(this)->_loop;
	}

}
