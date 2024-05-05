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
 #include <uv.h>

namespace qk {

	struct Thread {
		int         abort; // abort signal of run loop
		ThreadID    id;
		String      tag;
	};

	const Thread* thread_current();

	ParallelWorking::ParallelWorking(RunLoop* loop) : _proxy(nullptr) {
		Qk_ASSERT(loop, "Can not find current thread run loop.");
		_proxy = loop->keep_alive();
	}

	ParallelWorking::~ParallelWorking() {
		abort_child();
		delete _proxy; _proxy = nullptr;
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
				thread_abort(i.key);
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
			thread_abort(id);
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
		return _proxy->loop()->post(cb);
	}

	struct BackendLoop {
		RunLoop* loop;
		ThreadID id;
		Mutex mutex;
		Condition cond;
	} *_backend_loop = new BackendLoop{0};

	RunLoop* backend_loop() {
		auto self = _backend_loop;
		Lock lock(self->mutex);
		if (self->loop)
			return self->loop;

		thread_new([](void* arg) {
			auto t = thread_current();
			auto self = (BackendLoop*)arg;
			auto loop = RunLoop::current();
			{
				ScopeLock lock(self->mutex);
				self->loop = loop;
				self->id = t->id;
				self->cond.notify_all(); // call wait ok
			}

			auto time = uv_hrtime() / 1000;
			do {
				loop->timer(Cb([](auto&e){}), 2e6);
				loop->run();
				if (uv_hrtime() / 1000 - time > 2e6) {
					continue;
				}
				{
					ScopeLock lock(self->mutex);
					if (!t->abort) {
						thread_sleep(1e5); // 外部调用取到loop后,100毫秒内不激活loop将被释放
						if (loop->is_alive() && !t->abort)
							continue;
					}
					self->loop = nullptr;
					self->id = ThreadID();
				}
			} while(0);
		}, self, "work_loop");

		self->cond.wait(lock); // call wait
		return self->loop;
	}

	bool has_backend_thread() {
		return thread_self_id() == _backend_loop->id;
	}

}
