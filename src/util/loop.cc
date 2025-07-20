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

#include "./thread.h"

namespace qk {
	RunLoop        *__first_loop = nullptr;
	Array<RunLoop*> __loops;

	Qk_DEFINE_INLINE_MEMBERS(RunLoop, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<RunLoop::Inl*>(self)

		void timer_start(timer_t *timer) {
			_timer.set(timer->id, timer);
			auto timeout = timer->timeout;
			auto repeatTimeout = timer->repeatCount ? (timeout ? timeout: 1): 0;
			Qk_ASSERT_EQ(0, uv_timer_init(_uv_loop, timer));
			Qk_ASSERT_EQ(0, uv_timer_start(timer, [](uv_timer_t *h) {
				auto timer = static_cast<timer_t*>(h);
				auto rc = timer->cb->resolve();
				if (rc != 0 || timer->repeatCount == 0) {
					_inl(timer->data)->timer_stop(timer);
				} else if (timer->repeatCount > 0) {
					timer->repeatCount--;
				}
			}, timeout, repeatTimeout));
		}

		void timer_stop(timer_t *timer) {
			uv_close((uv_handle_t*)timer, [](uv_handle_t* h){
				delete (timer_t*)(h);
			});
			_timer.erase(timer->id);
		}

		void async_send() {
			if (_uv_async)
				Qk_ASSERT_EQ(0, uv_async_send(_uv_async));
		}

		void msgs_call() {
			Lock lock(_mutex);
			if (_msg.length()) {
				auto msg = std::move(_msg);
				lock.unlock();
				for (auto &m: msg) {
					if (m.timer)
						timer_start((timer_t*)m.timer);
					else
						m.cb->resolve(this);
				}
				lock.lock();
			}
			if (_msg.length()) {
				Qk_ASSERT_EQ(0, uv_async_send(_uv_async));
			}
			else if (is_alive()) {
				Qk_ASSERT_EQ(0, uv_timer_start(_uv_timer, [](auto h) {
					_inl(h->data)->msgs_call();
				}, 5e3, 0)); // 5 second check
			} else {
				uv_stop(_uv_loop); // stop run
			}
		}

		void post(Cb &cb) {
			_mutex.lock();
			_msg.pushBack({ 0, cb });
			_this->async_send();
			_mutex.unlock();
		}

		void death() {
			ScopeLock lock(*__threads_mutex);
			Qk_ASSERT_RAW(_uv_async == nullptr, "Secure deletion must ensure that the run loop has exited");
			// clear(); // clear all
			// Qk_ASSERT_EQ(nullptr, _uv_loop->closing_handles);

			if (__first_loop == this) {
				__first_loop = nullptr;
			}
			_thread = nullptr;
			_tid = ThreadID();
		}

		static RunLoop* current_from(RunLoop **inOut) {
			auto t = thread_self_inl();
			if (!t)
				return nullptr;

			auto loop = t->loop;
			if (!loop) {
				ScopeLock scope(*__threads_mutex);

				if (inOut) { // use inOut
					loop = *inOut;
					if (!loop) {
						*inOut = loop = new RunLoop(uv_loop_new());
					}
				} else {
					for (auto i: __loops) {
						if (!i->_thread)
							loop = i; break;
					}
					if (!loop) {
						__loops.push((
							loop = new RunLoop(__first_loop ? uv_loop_new(): uv_default_loop())
						));
					}
				}

				Qk_ASSERT_RAW(loop->_uv_async == nullptr);
				Qk_ASSERT_RAW(loop->_thread == nullptr);
				loop->_thread = t;
				loop->_tid = t->id;
				t->loop = loop;

				if (!__first_loop) {
					__first_loop = loop;
				}
			}
			return loop;
		}
	};

	void runloop_death(RunLoop *loop) {
		if (loop)
			_inl(loop)->death();
	}

	RunLoop* current_from(RunLoop **inOut) {
		return RunLoop::Inl::current_from(inOut);
	}

	void RunLoop::run() {
		if (__is_process_exit_atomic) {
			Qk_Warn("cannot run RunLoop::run(), __is_process_exit_atomic != 0"); return;
		}
		if (!_thread) {
			Qk_Warn("cannot run RunLoop::run(), _thread == nullptr"); return;
		}
		Qk_ASSERT(!_uv_async, "It is running and cannot be called repeatedly");
		Qk_ASSERT(thread_self_id() == _tid, "Must run on the target thread");

		// init run
		uv_timer_t uv_timer = {.data=this};
		uv_async_t uv_async = {.data=this};

		uv_timer_init(_uv_loop, &uv_timer);
		uv_async_init(_uv_loop, &uv_async, [](auto h) {
			_inl(h->data)->msgs_call();
		});
		_mutex.lock();
		_uv_async = &uv_async;
		_uv_timer = &uv_timer;
		_mutex.unlock();

		_this->async_send();

		uv_run(_uv_loop, UV_RUN_DEFAULT); // run uv loop

		uv_close((uv_handle_t*)_uv_async, nullptr); // close handle
		uv_close((uv_handle_t*)_uv_timer, nullptr);

		if (_uv_loop->closing_handles) {
			uv_run(_uv_loop, UV_RUN_NOWAIT); // exec uv close handles
		}
		Qk_ASSERT_EQ(nullptr, _uv_loop->closing_handles);

		// loop end
		_mutex.lock();
		_uv_async = nullptr;
		_uv_timer = nullptr;
		_mutex.unlock();
	}

	void RunLoop::check_t::call(Data &e) {
		host->_check.set(id, this);
		retain(); // retain for _check.set
		Qk_ASSERT_EQ(0, uv_check_init(host->_uv_loop, &uv_check));
		Qk_ASSERT_EQ(0, uv_check_start(&uv_check, [](uv_check_t *h) {
			auto self = (check_t*)(h->data);
			self->cb->resolve();
			if (self->repeatCount == 0) {
				self->stop_check();
			} else if (self->repeatCount > 0) {
				self->repeatCount--;
			}
		}));
	}

	void RunLoop::check_t::stop_check() {
		uv_close((uv_handle_t*)&uv_check, [](uv_handle_t* h) {
			static_cast<check_t*>(h->data)->release(); // release for hold on _check.set
		});
		host->_check.erase(id);
	}

	void RunLoop::work_t::call(Data &e) {
		host->_work.set(id, this);
		retain(); // retain for _work.set
		Qk_ASSERT_EQ(0, uv_queue_work(host->_uv_loop, &uv_req, [](uv_work_t* req) {
			auto self = static_cast<work_t*>(req->data);
			self->work->resolve(self->host);
		}, [](uv_work_t* req, int status) {
			auto self = static_cast<work_t*>(req->data);
			auto host = _inl(self->host);
			Qk_ASSERT_EQ(host->_tid, thread_self_id());
			host->_work.erase(self->id);
			if (UV_ECANCELED != status) // no cancel
				self->done->resolve(host);
			host->async_send();
			self->release(); // release hold for _work.set
		}));
	}

	// ----------------------------- R u n . L o o p -----------------------------

	RunLoop::RunLoop(uv_loop_t* uv)
		: _thread(nullptr)
		, _uv_loop(uv)
		, _uv_async(nullptr)
	{}

	RunLoop::~RunLoop() {
		Qk_Fatal("Cannot call destructor");
	}

	void RunLoop::clear() {
		if (thread_self_id() != _tid)
			return;

		_mutex.lock();
		List<Msg>             msgs(std::move(_msg));
		Dict<uint32_t, uv_timer_t*> timers(std::move(_timer));
		Dict<uint32_t, check_t*> checks(std::move(_check));
		Dict<uint32_t, work_t*> works(std::move(_work));
		_mutex.unlock();

		for (auto &i: msgs) {
			if (i.timer) {
				auto timer = (timer_t*)(i.timer);
				Qk_DLog("RunLoop::clear(), discard timer %p", timer);
				delete timer;
			} else {
				Qk_DLog("RunLoop::clear(), discard msg cb");
			}
		}

		for (auto &i: timers) {
			auto timer = (timer_t*)(i.value);
			_this->timer_stop(timer);
			Qk_DLog("RunLoop::clear(), discard timer %p", timer);
		}

		for (auto &i: checks) {
			auto check = (check_t*)(i.value);
			check->stop_check();
			Qk_DLog("RunLoop::clear(), discard check %p", check);
		}

		for (auto& i: works) {
			Qk_DLog("RunLoop::clear(), discard work %p", i.value);
			Qk_ASSERT_EQ(0, uv_cancel((uv_req_t*)&i.value->uv_req));
		}
	}

	RunLoop* RunLoop::current() {
		return Inl::current_from(nullptr);
	}

	RunLoop* RunLoop::first() {
		// NOTE: Be careful of thread safety,
		// it's best to first ensure that 'current ()' has been called`
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
					_uv_loop->closing_handles != NULL || _msg.length() != 0;
	}

	void RunLoop::post_message(Cb cb) {
		post(cb, true);
	}

	uint32_t RunLoop::timer(Cb cb, uint64_t time, int64_t repeat) {
		auto timer = new timer_t;
		timer->timer_cb = nullptr;
		timer->timeout = time / 1000;
		timer->data = this;
		timer->id = getId32();
		timer->repeatCount = repeat;
		timer->cb = cb;

		if (thread_self_id() == _tid) { // is self thread
			_this->timer_start(timer);
		} else {
			ScopeLock lock(_mutex);
			_msg.pushBack({ timer });
			_this->async_send();
		}
		return timer->id;
	}

	void RunLoop::timer_stop(uint32_t id) {
		if (id) {
			post(Cb([this,id](auto e) {
				uv_timer_t* out;
				if (_timer.get(id, out)) {
					_this->timer_stop((timer_t*)out);
				}
			}));
		}
	}

	uint32_t RunLoop::tick(Cb cb, int64_t repeat) {
		auto isSelfThread = thread_self_id() == _tid;
		if (!isSelfThread && repeat == 0) { // Once on an external thread
			_this->post(cb);
			return 0;
		}
		auto check = new check_t;
		check->host = this;
		check->id = getId32();
		check->repeatCount = repeat;
		check->cb = cb;
		check->uv_check.data = check;

		if (isSelfThread) {
			check->resolve();
		} else {
			Cb cb1(check);
			_this->post(cb1);
		}
		return check->id;
	}

	void RunLoop::tick_stop(uint32_t id) {
		if (id) {
			post(Cb([this,id](auto e) {
				check_t* out;
				if (_check.get(id, out))
					out->stop_check();
			}));
		}
	}

	uint32_t RunLoop::work(Cb cb, Cb done) {
		auto work = new work_t();
		work->host = this;
		work->id = getId32();
		work->work = cb;
		work->done = done;
		work->uv_req.data = work;

		if (thread_self_id() == _tid) {
			work->resolve();
		} else {
			ScopeLock lock(_mutex);
			work->resolve();
		}
		return work->id;
	}

	void RunLoop::work_cancel(uint32_t id) {
		if (id) {
			auto isNoSelfThread = thread_self_id() != _tid;
			if (isNoSelfThread) _mutex.lock();
			work_t *out;
			if (_work.get(id, out)) {
				Qk_ASSERT_EQ(0, uv_cancel((uv_req_t*)&out->uv_req));
			}
			if (isNoSelfThread) _mutex.unlock();
		}
	}

	void RunLoop::post(Cb cb, bool alwaysPost) {
		if (!alwaysPost && thread_self_id() == _tid) {
			cb->resolve();
		} else {
			_this->post(cb);
		}
	}

	void RunLoop::post_sync(Callback<PostSyncData> cb) {
		struct Data: public RunLoop::PostSyncData {
			void complete() override {
				ok = true;
				cond.lock_and_notify_all();
			}
			void wait() {
				while (!ok)
					cond.lock_and_wait_for(); // wait
			}
			bool ok = false;
			CondMutex cond;
		} data, *data_ptr = &data;

		if (thread_self_id() == _tid) { // is current
			cb->resolve(data_ptr);
		} else {
			Cb cb2([cb, data_ptr, this](auto&e){
				cb->resolve(data_ptr);
			});
			_this->post(cb2);
		}
		data_ptr->wait();
	}

	void RunLoop::stop() {
		if ( runing() ) {
			post(Cb([this](auto& e) {
				uv_loop_close(_uv_loop);
				uv_stop(_uv_loop);
			}));
		}
	}
}
