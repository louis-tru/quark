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

namespace qk {

	struct Thread {
		int         abort; // abort signal of run loop
		ThreadID    id;
		String      tag;
	};

	const Thread* thread_current();

	/**
	* @constructor
	*/
	ParallelWorking::ParallelWorking(): ParallelWorking(RunLoop::current()) {}

	ParallelWorking::ParallelWorking(RunLoop* loop) : _proxy(nullptr) {
		Qk_ASSERT(loop, "Can not find current thread run loop.");
		_proxy = loop->keep_alive("ParallelWorking()");
	}

	/**
	* @destructor
	*/
	ParallelWorking::~ParallelWorking() {
		abort_child();
		delete _proxy; _proxy = nullptr;
	}

	/**
	* @func run
	*/
	ThreadID ParallelWorking::spawn_child(Func func, cString& name) {
		ScopeLock scope(_mutex2);
		struct Tmp {
			typedef NonObjectTraits Traits;
			ParallelWorking* self;
			Func func;
		};
		auto id = thread_new([](void* arg) {
			Handle<Tmp> tmp = (Tmp*)arg;
			auto id = thread_current_id();
			tmp->func();
			ScopeLock scope(tmp->self->_mutex2);
			tmp->self->_childs.erase(id);
		}, new Tmp, name);
		_childs[id] = 1;
		return id;
	}

	/**
	* @func abort_child
	*/
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

	/**
	* @func awaken
	*/
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

	/**
	* @func post message to main thread
	*/
	uint32_t ParallelWorking::post(Cb exec) {
		_proxy->post_message(exec);
		return 0;
	}

	/**
	* @func post
	*/
	uint32_t ParallelWorking::post(Cb exec, uint64_t delayUs) {
		_proxy->post_message(exec, delayUs);
		return 0;
	}

	/**
	* @func cancel
	*/
	void ParallelWorking::cancel(uint32_t id) {
		/*if ( id ) {
			_proxy->cancel(id);
		} else {
			_proxy->cancel_all();
		}*/
	}


	/**
	 * @class BackendLoop
	 */
	class BackendLoop {
	public:
		inline BackendLoop(): _loop(nullptr) {}
		
		inline bool has_current_thread() {
			return thread_current_id() == _thread_id;
		}

		bool is_continue(const Thread* t) {
			ScopeLock scope(_mutex);
			if (!t->abort) {
				/* 趁着循环运行结束到上面这句lock片刻时间拿到队列对像的线程,这里是最后的200毫秒,
				* 200毫秒后没有向队列发送新消息结束线程
				* * *
				* 这里休眠200毫秒给外部线程足够时间往队列发送消息
				*/
				thread_sleep(2e5);
				if ( _loop->is_alive() && !t->abort ) {
					return true; // 继续运行
				}
			}
			_loop = nullptr;
			_thread_id = ThreadID();
			return false;
		}
		
		RunLoop* loop() {
			Lock lock(_mutex);
			if (_loop)
				return _loop;

			thread_new([](void* arg) {
				auto t = thread_current();
				auto self = (BackendLoop*)arg;
				self->_mutex.lock();
				self->_thread_id = t->id;
				self->_loop = RunLoop::current();
				self->_cond.notify_all(); // call wait ok
				self->_mutex.unlock();
				do {
					self->_loop->run(2e7); // 20秒后没有新消息结束线程
				} while(self->is_continue(t));
			}, this, "work_loop");
			
			_cond.wait(lock); // call wait

			return _loop;
		}
		
	private:
		ThreadID _thread_id;
		RunLoop* _loop;
		Mutex _mutex;
		Condition _cond;
	};

	static BackendLoop* _backend_loop = new BackendLoop();

	RunLoop* backend_loop() {
		return _backend_loop->loop();
	}

	bool has_backend_thread() {
		return _backend_loop->has_current_thread();
	}

}
