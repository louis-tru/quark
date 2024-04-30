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

#ifndef __quark__js__cb__
#define __quark__js__cb__

#include "../js_.h"
#include "../../util/codec.h"

namespace qk { namespace js {

	JSValue* convert_buffer(Worker* worker, Buffer& buffer, Encoding en = kInvalid_Encoding);
	Callback<Buffer> get_callback_for_buffer(Worker* worker, JSValue* cb, Encoding en = kInvalid_Encoding);
	Callback<Buffer> get_callback_for_buffer_http_error(Worker* worker, JSValue* cb, Encoding en = kInvalid_Encoding);
	Callback<ResponseData> get_callback_for_response_data_http_error(Worker* worker, JSValue* cb);
	Callback<StreamResponse> get_callback_for_io_stream(Worker* worker, JSValue* cb);
	Callback<StreamResponse> get_callback_for_io_stream_http_error(Worker* worker, JSValue* cb);
	Callback<Array<Dirent>> get_callback_for_array_dirent(Worker* worker, JSValue* cb);
	Callback<Bool> get_callback_for_bool(Worker* worker, JSValue* cb);
	Callback<Int32> get_callback_for_int(Worker* worker, JSValue* cb);
	Callback<FileStat> get_callback_for_file_stat(Worker* worker, JSValue* cb);
	Cb get_callback_for_none(Worker* worker, JSValue* cb);

} }
#endif
