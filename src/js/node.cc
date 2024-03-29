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

#include "./_js.h"
#include "../../util/loop.h"
#include "../../util/codec.h"
#include <deps/node/src/quark.h>

namespace node {

	QuarkApi* quark_api = nullptr;
	NodeAPI* node_api = nullptr;

	QuarkApi::~QuarkApi() {
		qk::Release(_worker);
		_worker = nullptr;
		quark_api = nullptr;
	}

	void QuarkApi::run_main_loop() {
		qk::RunLoop::main_loop()->run();
	}

	bool QuarkApi::is_exited() {
		return qk::is_exited();
	}

	Char* QuarkApi::encoding_to_utf8(const uint16_t * src, int length, int* out_len) {
		auto buff = qk::codec_encoding(qk::Encoding::UTF8, src, length);
		*out_len = buff.length();
		return buff.collapse();
	}

	uint16_t * QuarkApi::decoding_utf8_to_uint16(cChar* src, int length, int* out_len) {
		auto buff = qk::codec_decoding_to_uint16(qk::Encoding::UTF8, src, length);
		*out_len = buff.length();
		return buff.collapse();
	}

}
