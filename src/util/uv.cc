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

#include "./handle.h"
#include "./uv.h"
#include "./dict.h"

namespace qk {

	struct Tasks {
		Mutex mutex;
		Dict<uint32_t, AsyncIOTask*> task;
		AsyncIOTask* getTask(uint32_t id) {
			ScopeLock scope(mutex);
			AsyncIOTask *r = nullptr;
			task.get(id, r);
			return r;
		}
	} *tasks = new Tasks;

	AsyncIOTask::AsyncIOTask(RunLoop* loop)
		: _id(getId32()), _loop(loop), _abort(false)
	{
		Qk_Fatal_Assert(_loop, "#AsyncIOTask#AsyncIOTask loop nullptr");
		ScopeLock scope(tasks->mutex);
		tasks->task.set(_id, this);
	}

	AsyncIOTask::~AsyncIOTask() {
		ScopeLock scope(tasks->mutex);
		tasks->task.erase(_id);
	}

	void AsyncIOTask::abort() {
		if ( !_abort ) {
			_abort = true;
			release(); // end
		}
	}

	void AsyncIOTask::safe_abort(uint32_t id) {
		if (id) {
			Sp<AsyncIOTask> sp(tasks->getTask(id));
			if (sp) {
				auto task = sp.collapse();
				task->_loop->post(Cb([id, task](Cb::Data& e) {
					auto sp = Sp<AsyncIOTask>::without(task);
					task->abort();
				}));
			} // if (tasks->values.get(id, out))
		}
	}

}
