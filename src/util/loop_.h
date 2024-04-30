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

#ifndef __quark__util__loop___
#define __quark__util__loop___

#include "./loop.h"
#include <uv.h>

namespace qk {
	extern Mutex          *__threads_mutex;
	extern std::atomic_int __is_process_exit_safe;

	struct Thread {
		int               abort; // abort signal of run loop
		ThreadID          id;
		String            tag;
	};

	struct Thread_INL: Thread, CondMutex {
		RunLoop*          loop;
		List<CondMutex*>  waitSelfEnd; // external wait thread end
		void             (*exec)(void *arg);
		void              *arg;
	};

	struct Keep: KeepLoop {
		Keep(RunLoop *loop, void (*check)(void *ctx), void* ctx);
		~Keep() override;
		void post_message(Cb cb) override;
		RunLoop* _loop;
		List<KeepLoop*>::Iterator _id;
		uv_check_t *_uv_check;
		void (*_check)(void *ctx);
		void *_check_ctx;
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
		void done_work(int status);
	};

	Thread_INL* thread_current_inl();
}
#endif
