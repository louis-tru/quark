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

#ifndef __qgr__js__node_1__
#define __qgr__js__node_1__

#include "qgr/utils/loop.h"

namespace qgr {
	namespace js {
		class Worker;
	}
}

namespace node {
	class Environment;
	struct QgrApi {
		qgr::js::Worker* (*create_qgr_js_worker)(Environment* env, bool is_inspector,
																							 int argc, const char* const* arg);
		void (*delete_qgr_js_worker)(qgr::js::Worker* worker);
		qgr::RunLoop* (*qgr_main_loop)();
		void (*run_qgr_loop)(qgr::RunLoop* loop);
		char* (*encoding_to_utf8)(const uint16_t* src, int length, int* out_len);
		uint16_t* (*decoding_utf8_to_uint16)(const char* src, int length, int* out_len);
		void (*print)(const char* msg, ...);
		bool (*is_process_exit)();
	};

	extern struct QgrApi* qgr_api;
	
	extern void set_qgr_api(struct QgrApi api);
}

#endif
