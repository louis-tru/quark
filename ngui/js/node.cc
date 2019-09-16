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

#include "ngui/js/js-1.h"
#include "nxutils/loop.h"
#include "nxutils/codec.h"
#include "depe/node/src/ngui.h"

namespace node {

	NguiEnvironment* ngui_env = nullptr;
	NodeAPI* ngui_node_api = nullptr;

	NguiEnvironment::~NguiEnvironment() {
		ngui::Release(m_worker);
		m_worker = nullptr;
		ngui_env = nullptr;
	}

	void NguiEnvironment::run_loop() {
		ngui::RunLoop::main_loop()->run();
	}

	char* NguiEnvironment::encoding_to_utf8(const uint16_t* src, int length, int* out_len) {
		auto buff = ngui::Codec::encoding(ngui::Encoding::UTF8, src, length);
		*out_len = buff.length();
		return buff.collapse();
	}

	uint16_t* NguiEnvironment::decoding_utf8_to_uint16(const char* src, int length, int* out_len) {
		auto buff = ngui::Codec::decoding_to_uint16(ngui::Encoding::UTF8, src, length);
		*out_len = buff.length();
		return buff.collapse();
	}

	bool NguiEnvironment::is_exited() {
		return ngui::is_exited();
	}

}