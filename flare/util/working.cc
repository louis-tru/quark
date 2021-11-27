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

 #include "./working.h"

namespace flare {

	/**
	* @constructor
	*/
	ParallelWorking::ParallelWorking(): ParallelWorking(RunLoop::current()) {}

	ParallelWorking::ParallelWorking(RunLoop* loop) : _proxy(nullptr) {
		F_ASSERT(loop, "Can not find current thread run loop.");
		_proxy = loop->keep_alive("ParallelWorking()");
	}

	/**
	* @destructor
	*/
	ParallelWorking::~ParallelWorking() {
		abort_child();
		Release(_proxy); _proxy = nullptr;
	}

	/**
	* @func run
	*/
	ThreadID ParallelWorking::spawn_child(Exec exec, cString& name) {
		ScopeLock scope(_mutex2);
		auto id = Thread::fork([this, exec](Thread& t) {
			int rc = exec(t);
			ScopeLock scope(_mutex2);
			_childs.erase(t.id());
			return rc;
		}, name);
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
				Thread::abort(i.key);
			}
			for (auto& i : childs) {
				Thread::join(i.key);
			}
			F_DEBUG("ParallelWorking::abort_child() ok, count: %d", childs.length());
		} else {
			{
				ScopeLock scope(_mutex2);
				F_ASSERT(_childs.find(id) != _childs.end(),
					"Only subthreads belonging to \"ParallelWorking\" can be aborted");
			}
			Thread::abort(id);
			Thread::join(id);
			F_DEBUG("ParallelWorking::abort_child(id) ok");
		}
	}

	/**
	* @func awaken
	*/
	void ParallelWorking::awaken_child(ThreadID id) {
		ScopeLock scope(_mutex2);
		if ( id == ThreadID() ) {
			for (auto& i : _childs) {
				Thread::resume(i.key);
			}
		} else {
			F_ASSERT(_childs.find(id) != _childs.end(),
				"Only subthreads belonging to \"ParallelWorking\" can be resume");
			Thread::resume(id);
		}
	}

	/**
	* @func post message to main thread
	*/
	uint32_t ParallelWorking::post(Cb exec) {
		return _proxy->post(exec);
	}

	/**
	* @func post
	*/
	uint32_t ParallelWorking::post(Cb exec, uint64_t delayUs) {
		return _proxy->post(exec, delayUs);
	}

	/**
	* @func cancel
	*/
	void ParallelWorking::cancel(uint32_t id) {
		if ( id ) {
			_proxy->cancel(id);
		} else {
			_proxy->cancel_all();
		}
	}


	/**
	 * @class TempWorkLoop
	 */
	class TempWorkLoop {
	 public:
		inline TempWorkLoop(): _loop(nullptr) {}
		
		inline bool has_current_thread() {
			return Thread::current_id() == _thread_id;
		}

		bool is_continue(Thread& t) {
			ScopeLock scope(_mutex);
			if (!t.is_abort()) {
				/* 趁着循环运行结束到上面这句lock片刻时间拿到队列对像的线程,这里是最后的200毫秒,
				* 200毫秒后没有向队列发送新消息结束线程
				* * *
				* 这里休眠200毫秒给外部线程足够时间往队列发送消息
				*/
				Thread::sleep(2e5);
				if ( _loop->is_alive() && !t.is_abort() ) {
					return true; // 继续运行
				}
			}
			_loop = nullptr;
			_thread_id = ThreadID();
			return false;
		}
		
		RunLoop* loop() {
			Lock lock(_mutex);
			if (is_exited())
				return nullptr;
			if (_loop)
				return _loop;
			
			Thread::fork([this](Thread& t) {
				_mutex.lock();
				_thread_id = t.id();
				_loop = RunLoop::current();
				_cond.notify_all(); // call wait ok
				_mutex.unlock();
				do {
					_loop->run(2e7); // 20秒后没有新消息结束线程
				} while(is_continue(t));
				return 0;
			}, "work_loop");
			
			_cond.wait(lock); // call wait

			return _loop;
		}
		
	 private:
		ThreadID _thread_id;
		RunLoop* _loop;
		Mutex _mutex;
		Condition _cond;
	};

	static TempWorkLoop* _temp_work_loop = new TempWorkLoop();

	RunLoop* temp_work_loop() {
		return _temp_work_loop->loop();
	}

	bool has_temp_work_thread() {
		return _temp_work_loop->has_current_thread();
	}

}
