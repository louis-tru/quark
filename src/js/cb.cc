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

#include "./js_.h"

namespace qk { namespace js {

	struct Func {
		typedef NonObjectTraits Traits;
		Func(Worker* worker, JSValue* cb)
			: val(worker, cb->cast<JSFunction>()) {}
		Persistent<JSFunction> val;
	};

	template<class Type, class Err = Error>
	Callback<Type> get_callback_for_type(Worker* worker, JSValue* cb) {
		if ( cb && cb->isFunction() ) {
			auto func = new Func(worker, cb);
			typedef Callback<Type> Cb;

			return Cb([worker, func](Cb::Data& d) {
				Sp<Func> h(func);
				auto f = *func->val;
				Js_Handle_Scope(); // Callback Scope

				if ( d.error ) {
					JSValue* arg = worker->types()->jsvalue(*static_cast<const Err*>(d.error));
					f->call(worker, 1, &arg);
				} else {
					Type* data = d.data;
					JSValue* args[2] = { worker->newNull(), worker->types()->jsvalue(*data) };
					f->call(worker, 2, args);
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
				result = worker->newInstance(buffer);
				break;
			case kHex_Encoding: // encode to string
			case kBase64_Encoding: {
				String str = codec_encode(encoding, buffer).collapseString();
				result = worker->newInstance(str);
				break;
			}
			default: { // decode to uft16 string
				String2 str(codec_decode_to_uint16(encoding, buffer));
				result = worker->newInstance(str);
				break;
			}
		}
		return result;
	}

	template<class Err = Error>
	Callback<Buffer> get_callback_for_buffer2(Worker* worker, JSValue* cb, Encoding encoding) {
		if ( cb && cb->isFunction() ) {
			auto func = new Func(worker, cb);
			typedef Callback<Buffer> Cb;

			return Cb([worker, func, encoding](Cb::Data& d) {
				Sp<Func> h(func);
				auto f = *func->val;
				Js_Handle_Scope(); // Callback Scope

				if ( d.error ) {
					JSValue* arg = worker->types()->jsvalue(*static_cast<const Err*>(d.error));
					f->call(worker, 1, &arg);
				} else {
					Buffer* bf = d.data;
					JSValue* args[2] = { worker->newNull(), convert_buffer(worker, *bf, encoding) };
					f->call(worker, 2, args);
				}
			});
		} else {
			return 0;
		}
	}

	template<class Err = Error>
	Callback<StreamResponse> get_callback_for_io_stream2(Worker* worker, JSValue* cb) {
		if ( cb && cb->isFunction() ) {
			auto func = new Func(worker, cb);
			typedef Callback<StreamResponse> Cb;

			return Cb([worker, func](Cb::Data& d) {
				Sp<Func> h(func);
				auto f = *func->val;
				Js_Handle_Scope(); // Callback Scope
				
				if ( d.error ) {
					JSValue* arg = worker->types()->jsvalue(*static_cast<const Err*>(d.error));
					f->call(worker, 1, &arg);
				} else {
					StreamResponse* data = static_cast<StreamResponse*>(d.data);
					JSObject* arg = worker->newObject();
					arg->set(worker, worker->strs()->data(), worker->newInstance(data->buffer()) );
					arg->set(worker, worker->strs()->complete(), worker->newInstance(data->complete()) );
					arg->set(worker, worker->strs()->size(), worker->newInstance(data->size()) );
					arg->set(worker, worker->strs()->total(), worker->newInstance(data->total()) );
					JSValue* args[2] = { worker->newNull(), arg };
					f->call(worker, 2, args);
				}
			});
		} else {
			return 0;
		}
	}

	template<class Err = Error>
	Callback<ResponseData> get_callback_for_response_data2(Worker* worker, JSValue* cb) {
		if ( cb && cb->isFunction() ) {
			auto func = new Func(worker, cb);
			typedef Callback<ResponseData> Cb;
			
			return Cb([worker, func](Cb::Data& d) {
				Sp<Func> h(func);
				auto f = *func->val;
				Js_Handle_Scope(); // Callback Scope

				if ( d.error ) {
					JSValue* arg = worker->types()->jsvalue(*static_cast<const Err*>(d.error));
					f->call(worker, 1, &arg);
				} else {
					ResponseData* data = d.data;
					JSObject* arg = worker->newObject();
					arg->set(worker, worker->strs()->data(), worker->newInstance(data->data) );
					arg->set(worker, worker->strs()->httpVersion(), worker->newInstance(data->http_version) );
					arg->set(worker, worker->strs()->statusCode(), worker->newInstance(data->status_code) );
					arg->set(worker, worker->strs()->responseHeaders(), worker->newInstance(data->response_headers) );
					JSValue* args[2] = { worker->newNull(), arg };
					f->call(worker, 2, args);
				}
			});
		} else {
			return 0;
		}
	}

	Cb get_callback_for_none(Worker* worker, JSValue* cb) {
		if ( cb && cb->isFunction() ) {
			auto func = new Func(worker, cb);

			return Cb([worker, func](Cb::Data& d) {
				Sp<Func> h(func);
				auto f = *func->val;
				Js_Handle_Scope(); // Callback Scope
				if ( d.error ) {
					JSValue* arg = worker->types()->jsvalue(*static_cast<const Error*>(d.error));
					f->call(worker, 1, &arg);
				} else {
					f->call(worker);
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