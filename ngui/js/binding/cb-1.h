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

#ifndef __ngui__js__cb__
#define __ngui__js__cb__

#include "ngui/js/js.h"
#include "nxutils/codec.h"

/**
 * @ns ngui::js
 */

JS_BEGIN

Local<JSValue> convert_buffer(Worker* worker, 
	Buffer& buffer, Encoding encoding = Encoding::unknown);
Callback get_callback_for_buffer(Worker* worker, 
	Local<JSValue> cb, Encoding encoding = Encoding::unknown);
Callback get_callback_for_buffer_http_error(Worker* worker, 
	Local<JSValue> cb, Encoding encoding = Encoding::unknown);
Callback get_callback_for_response_data_http_error(Worker* worker, Local<JSValue> cb);
Callback get_callback_for_io_stream(Worker* worker, Local<JSValue> cb);
Callback get_callback_for_io_stream_http_error(Worker* worker, Local<JSValue> cb);
Callback get_callback_for_none(Worker* worker, Local<JSValue> cb);
Callback get_callback_for_array_dirent(Worker* worker, Local<JSValue> cb);
Callback get_callback_for_bool(Worker* worker, Local<JSValue> cb);
Callback get_callback_for_int(Worker* worker, Local<JSValue> cb);
Callback get_callback_for_file_stat(Worker* worker, Local<JSValue> cb);

JS_END
#endif
