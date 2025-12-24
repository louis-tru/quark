/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./inl.h"

namespace qk {

	Threads::Threads() {
	}

	Threads::~Threads() {
		abort();
	}

	ThreadID Threads::spawn(std::function<void(cThread *t)> func, cString& name) {
		ScopeLock scope(_mutex);
		struct Arg {
			std::function<void(cThread *t)> func;
			Threads *self;
		};
		auto id = thread_new([](auto t, void* arg) {
			Sp<Arg> arg_(static_cast<Arg*>(arg));
			arg_->func(t);
			ScopeLock scope(arg_->self->_mutex);
			arg_->self->_childs.erase(t->id);
		}, new Arg, name);
		_childs.add(id);
		return id;
	}

	void Threads::abort(ThreadID id) {
		if ( id == ThreadID() ) {
			Set<ThreadID> childs;
			{
				ScopeLock scope(_mutex);
				childs = std::move(_childs);
			}
			for (auto& i : childs) {
				thread_try_abort(i.first);
			}
			for (auto& i : childs) {
				thread_join_for(i.first);
			}
			Qk_DLog("Threads::abort() ok, count: %d", childs.length());
		} else {
			{
#if DEBUG
				ScopeLock scope(_mutex);
				Qk_ASSERT(_childs.has(id),
					"Only subthreads belonging to \"Threads\" can be aborted");
#endif
			}
			thread_try_abort(id);
			thread_join_for(id);
			Qk_DLog("Threads::abort(id) ok");
		}
	}

	void Threads::resume(ThreadID id) {
		ScopeLock scope(_mutex);
		if ( id == ThreadID() ) {
			for (auto& i : _childs) {
				thread_resume(i.first);
			}
		} else {
			Qk_ASSERT(_childs.has(id),
				"Only subthreads belonging to \"Threads\" can be resume");
			thread_resume(id);
		}
	}

	struct BackendLoop {
		RunLoop* loop;
		ThreadID id;
		Mutex mutex;
		Condition cond;

		static void run(cThread *t, void* arg) {
			auto self = (BackendLoop*)arg;
			auto loop = current_from(&self->loop);
			{
				ScopeLock lock(self->mutex);
				self->id = t->id;
				self->cond.notify_all(); // call wait ok
			}
		 run:
			loop->timer(Cb([](auto&e){}), 5e3); // 5s
			loop->run();
			int wait = 100; // wait 10s
			while (wait-- && t->abort == 0) {
				thread_sleep(1e5); // Not using within 100 milliseconds will release threads
				if (loop->is_alive())
					goto run;
			}
			ScopeLock lock(self->mutex);
			self->id = ThreadID();
			Qk_DLog("backend_loop() thread end");
		}

		RunLoop* get_loop() {
			Lock lock(mutex);
			if (id == ThreadID()) {
				thread_new(run, this, "first_loop");
				cond.wait(lock); // wait
			}
			return loop;
		}
	} static *_backend_loop = new BackendLoop{0};

	RunLoop* backend_loop() {
		return _backend_loop->get_loop();
	}

	bool has_backend_thread() {
		return thread_self_id() == _backend_loop->id;
	}
}
