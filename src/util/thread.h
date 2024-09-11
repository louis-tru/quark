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

// @private head

#ifndef __quark__util__thread___
#define __quark__util__thread___

#include "./loop.h"
#include <uv.h>

namespace qk {
	extern RunLoop        *__first_loop;
	extern Mutex          *__threads_mutex;
	extern std::atomic_int __is_process_exit_atomic;

	struct Thread_INL: Thread, CondMutex {
		void     (*exec)(cThread *t, void *arg);
		void      *arg;
		RunLoop*  loop;
		List<CondMutex*> waitSelfEnd; // external wait thread end
	};

	struct RunLoop::Work {
		typedef NonObjectTraits Traits;
		RunLoop* host;
		uint32_t id;
		Cb work, done;
		uv_work_t uv_req;
		String name;
		static void uv_work_cb(uv_work_t* req);
		static void uv_after_work_cb(uv_work_t* req, int status);
	};

	struct timer_t: uv_timer_t {
		uint32_t id;
		int64_t repeatCount;
		bool calling;
		Cb cb;
	};

	struct check_t: uv_check_t {
		uint32_t id;
		int64_t repeatCount;
		Cb cb;
	};

	//////
	class Threads: public Object {
		Qk_HIDDEN_ALL_COPY(Threads);
	public:
		Threads();
		~Threads() override;
		ThreadID spawn(std::function<void(cThread *t)> func, cString& name);
		void resume(ThreadID id = ThreadID());  // default resume all child
		void abort(ThreadID id = ThreadID());   // default abort all child
	private:
		Mutex _mutex;
		Set<ThreadID> _childs;
	};

	Thread_INL* thread_self_inl();
	void        runloop_death(RunLoop *loop);
	RunLoop*    current_from(RunLoop **inOut);
	RunLoop*    backend_loop();
	bool        has_backend_thread();
}
#endif
