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

 #include "./_working.h"

namespace ftr {

	/**
	* @constructor
	*/
	ParallelWorking::ParallelWorking(): ParallelWorking(RunLoop::current()) {}

	ParallelWorking::ParallelWorking(RunLoop* loop) : _proxy(nullptr) {
		ASSERT(loop, "Can not find current thread run loop.");
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
		auto id = Thread::spawn([this, exec](Thread& t) {
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
			std::map<ThreadID, int> childs;
			{
				ScopeLock scope(_mutex2);
				childs = _childs;
			}
			for (auto& i : childs) {
				Thread::abort(i.first);
			}
			for (auto& i : childs) {
				Thread::join(i.first);
			}
			DLOG("ParallelWorking::abort_child() ok, count: %d", childs.size());
		} else {
			{
				ScopeLock scope(_mutex2);
				ASSERT(_childs.find(id) != _childs.end(),
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
		ScopeLock scope(_mutex2);
		if ( id == ThreadID() ) {
			for (auto& i : _childs) {
				Thread::awaken(i.first);
			}
		} else {
			ASSERT(_childs.find(id) != _childs.end(),
				"Only subthreads belonging to \"ParallelWorking\" can be awaken");
			Thread::awaken(id);
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

}
