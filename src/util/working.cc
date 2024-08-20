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

 #include "./working.h"
 #include "./loop_.h"
 #include <uv.h>

namespace qk {

	ParallelWorking::ParallelWorking(RunLoop* loop) : _loop(loop) {
		Qk_Assert(loop, "Can not find current thread run loop.");
	}

	ParallelWorking::~ParallelWorking() {
		abort_child();
	}

	ThreadID ParallelWorking::spawn_child(Func func, cString& name) {
		ScopeLock scope(_mutex2);
		struct Tmp {
			typedef NonObjectTraits Traits;
			ParallelWorking* self;
			Func func;
		};
		auto id = thread_new([](void* arg) {
			Handle<Tmp> tmp = (Tmp*)arg;
			auto id = thread_self_id();
			tmp->func();
			ScopeLock scope(tmp->self->_mutex2);
			tmp->self->_childs.erase(id);
		}, new Tmp, name);
		_childs[id] = 1;
		return id;
	}

	void ParallelWorking::abort_child(ThreadID id) {
		if ( id == ThreadID() ) {
			Dict<ThreadID, int> childs;
			{
				ScopeLock scope(_mutex2);
				childs = _childs;
			}
			for (auto& i : childs) {
				thread_try_abort(i.key);
			}
			for (auto& i : childs) {
				thread_join_for(i.key);
			}
			Qk_DEBUG("ParallelWorking::abort_child() ok, count: %d", childs.length());
		} else {
			{
				ScopeLock scope(_mutex2);
				Qk_ASSERT(_childs.find(id) != _childs.end(),
					"Only subthreads belonging to \"ParallelWorking\" can be aborted");
			}
			thread_try_abort(id);
			thread_join_for(id);
			Qk_DEBUG("ParallelWorking::abort_child(id) ok");
		}
	}

	void ParallelWorking::awaken_child(ThreadID id) {
		ScopeLock scope(_mutex2);
		if ( id == ThreadID() ) {
			for (auto& i : _childs) {
				thread_resume(i.key);
			}
		} else {
			Qk_ASSERT(_childs.find(id) != _childs.end(),
				"Only subthreads belonging to \"ParallelWorking\" can be resume");
			thread_resume(id);
		}
	}

	void ParallelWorking::post(Cb cb) {
		return _loop->post(cb);
	}

	struct BackendLoop {
		RunLoop* loop;
		ThreadID id;
		Mutex mutex;
		Condition cond;

		static void run(void* arg) {
			auto self = (BackendLoop*)arg;
			auto t = thread_current_();
			auto loop = current_from(&self->loop);
			self->mutex.lock();
			self->id = t->id;
			self->cond.notify_all(); // call wait ok
			self->mutex.unlock();
		run:
			loop->run();
			int wait = 100; // wait 10s
			while (wait-- && t->abort == 0) {
				thread_sleep(1e5); // Not using within 100 milliseconds will release threads
				if (loop->is_alive())
					goto run;
			}
			ScopeLock lock(self->mutex);
			self->id = ThreadID();
			Qk_DEBUG("backend_loop() thread end");
		}

		RunLoop* get_loop() {
			Lock lock(mutex);
			if (id == ThreadID()) {
				thread_new(run, this, "work_loop");
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
