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

#include "./api/types.h"

namespace qk { namespace js {

	struct Func {
		Func(Worker* worker, JSValue* cb)
			: val(worker, cb->cast<JSFunction>()) {}
		Func(const Func& func) {
			val.copy(func.val);
		}
		Persistent<JSFunction> val;
	};

	template<class Type, class Err = Error>
	Callback<Type> get_callback_for_type(Worker* worker, JSValue* cb) {
		if ( cb && cb->isFunction() ) {
			Func func(worker, cb);
			typedef Callback<Type> Cb;

			return Cb([worker, func](typename Cb::Data& d) {
				Js_Handle_Scope(); // Callback Scope

				if ( d.error ) {
					JSValue* arg = worker->newValue(*static_cast<const Err*>(d.error));
					func.val->call(worker, 1, &arg);
				} else {
					Type* data = d.data;
					JSValue* args[2] = { worker->newNull(), worker->types()->jsvalue(*data) };
					func.val->call(worker, 2, args);
				}
			});
		} else {
			return 0;
		}
	}

	JSValue* convert_buffer(Worker* worker, Buffer& buffer, Encoding encoding) {
		JSValue* result;
		switch (encoding) { // buffer
			case kInvalid_Encoding: // no convert
				result = worker->newValue(buffer);
				break;
			case kHex_Encoding: // encode to hex or base64 string
			case kBase64_Encoding: {
				Buffer buff = codec_encode(encoding, buffer);
				result = worker->newStringOneByte(buff.collapseString());
				break;
			}
			default: { // encode to js uft16 string
				auto unicode = codec_decode_to_unicode( encoding, buffer); // decode to unicode
				result = worker->newValue(unicode.collapseString());
				break;
			}
		}
		return result;
	}

	template<class Err = Error>
	Callback<Buffer> get_callback_for_buffer2(Worker* worker, JSValue* cb, Encoding encoding) {
		if ( cb && cb->isFunction() ) {
			Func func(worker, cb);
			typedef Callback<Buffer> Cb;

			return Cb([worker, func, encoding](Cb::Data& d) {
				Js_Handle_Scope(); // Callback Scope

				if ( d.error ) {
					JSValue* arg = worker->newValue(*static_cast<const Err*>(d.error));
					func.val->call(worker, 1, &arg);
				} else {
					Buffer* bf = d.data;
					JSValue* args[2] = { worker->newNull(), convert_buffer(worker, *bf, encoding) };
					func.val->call(worker, 2, args);
				}
			});
		} else {
			return 0;
		}
	}

	template<class Err = Error>
	Callback<StreamResponse> get_callback_for_io_stream2(Worker* worker, JSValue* cb) {
		if ( cb && cb->isFunction() ) {
			Func func(worker, cb);
			typedef Callback<StreamResponse> Cb;

			return Cb([worker, func](Cb::Data& d) {
				Js_Handle_Scope(); // Callback Scope
				if ( d.error ) {
					JSValue* arg = worker->newValue(*static_cast<const Err*>(d.error));
					func.val->call(worker, 1, &arg);
				} else {
					auto data = &static_cast<StreamResponse*>(d.data)->value;
					JSObject* arg = worker->newObject();
					arg->set(worker, worker->strs()->data(), worker->newValue(data->data) );
					arg->set(worker, worker->strs()->ended(), worker->newBool(data->ended) );
					arg->set(worker, worker->strs()->size(), worker->newValue(data->size) );
					arg->set(worker, worker->strs()->total(), worker->newValue(data->total) );
					JSValue* args[2] = { worker->newNull(), arg };
					func.val->call(worker, 2, args);
				}
			});
		} else {
			return 0;
		}
	}

	template<class Err = Error>
	Callback<ResponseData> get_callback_for_response_data2(Worker* worker, JSValue* cb) {
		if ( cb && cb->isFunction() ) {
			Func func(worker, cb);
			typedef Callback<ResponseData> Cb;

			return Cb([worker, func](Cb::Data& d) {
				Js_Handle_Scope(); // Callback Scope

				if ( d.error ) {
					JSValue* arg = worker->newValue(*static_cast<const Err*>(d.error));
					func.val->call(worker, 1, &arg);
				} else {
					ResponseData* data = d.data;
					JSObject* arg = worker->newObject();
					arg->set(worker, worker->strs()->data(), worker->newValue(data->data) );
					arg->set(worker, worker->strs()->httpVersion(), worker->newValue(data->http_version) );
					arg->set(worker, worker->strs()->statusCode(), worker->newValue(data->status_code) );
					arg->set(worker, worker->strs()->responseHeaders(), worker->newValue(data->response_headers) );
					JSValue* args[2] = { worker->newNull(), arg };
					func.val->call(worker, 2, args);
				}
			});
		} else {
			return 0;
		}
	}

	Cb get_callback_for_none(Worker* worker, JSValue* cb) {
		if ( cb && cb->isFunction() ) {
			Func func(worker, cb);

			return Cb([worker, func](Cb::Data& d) {
				Js_Handle_Scope(); // Callback Scope
				if ( d.error ) {
					JSValue* arg = worker->newValue(*static_cast<const Error*>(d.error));
					func.val->call(worker, 1, &arg);
				} else {
					func.val->call(worker);
				}
			});
		} else {
			return 0;
		}
	}

	Callback<Buffer> get_callback_for_buffer(Worker* worker, JSValue* cb, Encoding encoding) {
		return get_callback_for_buffer2(worker, cb, encoding);
	}

	Callback<Buffer> get_callback_for_buffer_http_error(Worker* worker, JSValue* cb, Encoding encoding) {
		return get_callback_for_buffer2<HttpError>(worker, cb, encoding);
	}

	Callback<ResponseData> get_callback_for_response_data_http_error(Worker* worker, JSValue* cb) {
		return get_callback_for_response_data2<HttpError>(worker, cb);
	}

	Callback<StreamResponse> get_callback_for_io_stream(Worker* worker, JSValue* cb) {
		return get_callback_for_io_stream2(worker, cb);
	}

	Callback<StreamResponse> get_callback_for_io_stream_http_error(Worker* worker, JSValue* cb) {
		return get_callback_for_io_stream2<HttpError>(worker, cb);
	}

	Callback<Array<Dirent>> get_callback_for_array_dirent(Worker* worker, JSValue* cb) {
		return get_callback_for_type<Array<Dirent>>(worker, cb);
	}

	Callback<Bool> get_callback_for_bool(Worker* worker, JSValue* cb) {
		return get_callback_for_type<Bool>(worker, cb);
	}

	Callback<Int32> get_callback_for_int(Worker* worker, JSValue* cb) {
		return get_callback_for_type<Int32>(worker, cb);
	}

	Callback<FileStat> get_callback_for_file_stat(Worker* worker, JSValue* cb) {
		return get_callback_for_type<FileStat>(worker, cb);
	}

} }
