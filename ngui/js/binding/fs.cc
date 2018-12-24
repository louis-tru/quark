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

#include "ngui/utils/fs.h"
#include "ngui/utils/http.h"
#include "ngui/js/js.h"
#include "ngui/js/str.h"
#include "cb-1.h"

/**
 * @ns avocado::ajs
 */

JS_BEGIN

template<class T, class Err = Error, int ErrStyle = 1>
Callback get_callback_for_t(Worker* worker, Local<JSValue> cb) {
	if ( !cb.IsEmpty() && cb->IsFunction(worker) ) {
		CopyablePersistentFunc func(worker, cb.To<JSFunction>());
		
		return Cb([worker, func](Se& d) {
			XX_ASSERT(!func.IsEmpty());
			HandleScope scope(worker);
			Local<JSFunction> f = func.strong();
			
			if ( d.error ) {
				Local<JSValue> arg = worker->New(*static_cast<const Err*>(d.error));
				if (ErrStyle == 0) {
					f->Call(worker, 1, &arg);
				} else {
					Local<JSFunction> e = f->Get(worker, worker->strs()->Throw()).To<JSFunction>();
					e->Call(worker, 1, &arg, f);
				}
			} else {
				T* data = static_cast<T*>(d.data);
				Local<JSValue> arg = worker->New(*data);
				if (ErrStyle == 0) {
					Local<JSValue> args[2] = { worker->NewNull(), arg };
					f->Call(worker, 2, args);
				} else {
					f->Call(worker, 1, &arg);
				}
			}
		});
	} else {
		return 0;
	}
}

template<class Err = Error, bool ErrStyle = 1>
Callback get_callback_for_buffer_2(Worker* worker, Local<JSValue> cb, Encoding encoding) {
	if ( !cb.IsEmpty() && cb->IsFunction(worker) ) {
		CopyablePersistentFunc func(worker, cb.To<JSFunction>());

		return Cb([worker, func, encoding](Se& d) {
			XX_ASSERT(!func.IsEmpty());
			HandleScope scope(worker);
			Local<JSFunction> f = func.strong();
			
			if ( d.error ) {
				Local<JSValue> arg = worker->New(*static_cast<const Err*>(d.error));
				if (ErrStyle == 0) {
					f->Call(worker, 1, &arg);
				} else {
					Local<JSFunction> e = f->Get(worker, worker->strs()->Throw()).To<JSFunction>();
					e->Call(worker, 1, &arg, f);
				}
			} else {
				Buffer* data = static_cast<Buffer*>(d.data);
				Local<JSValue> arg;
				switch (encoding) {
					case Encoding::hex: // 编码
					case Encoding::base64: {
						Buffer buff = Coder::encoding(encoding, *data);
						arg = worker->NewString(buff);
						break;
					}
					case Encoding::unknown:
						arg = worker->New(*data);
						break;
					default: {// 解码 to ucs2
						Ucs2String str(Coder::decoding_to_uint16(encoding, *data));
						arg = worker->New(str);
						break;
					}
				}
				if (ErrStyle == 0) {
					Local<JSValue> args[2] = { worker->NewNull(), arg };
					f->Call(worker, 2, args);
				} else {
					f->Call(worker, 1, &arg);
				}
			}
		});
	} else {
		return 0;
	}
}

template<class Err = Error, bool ErrStyle = 1>
Callback get_callback_for_io_stream2(Worker* worker, Local<JSValue> cb) {
	if ( !cb.IsEmpty() && cb->IsFunction(worker) ) {
		CopyablePersistentFunc func(worker, cb.To<JSFunction>());
		
		return Cb([worker, func](Se& d) {
			XX_ASSERT(!func.IsEmpty());
			HandleScope scope(worker);
			
			Local<JSFunction> f = func.strong();
			
			if ( d.error ) {
				Local<JSValue> arg = worker->New(*static_cast<const Err*>(d.error));
				if (ErrStyle == 0) {
					f->Call(worker, 1, &arg);
				} else {
					Local<JSFunction> e = f->Get(worker, worker->strs()->Throw()).To<JSFunction>();
					e->Call(worker, 1, &arg, f);
				}
			} else {
				IOStreamData* data = static_cast<IOStreamData*>(d.data);
				Local<JSObject> arg = worker->NewObject();
				arg->Set(worker, worker->strs()->data(), worker->New(data->buffer()) );
				arg->Set(worker, worker->strs()->complete(), worker->New(data->complete()) );
				arg->Set(worker, worker->strs()->size(), worker->New(data->size()) );
				arg->Set(worker, worker->strs()->total(), worker->New(data->total()) );
				if (ErrStyle == 0) {
					Local<JSValue> args[2] = { worker->NewNull(), arg };
					f->Call(worker, 2, args);
				} else {
					f->Call(worker, 1, reinterpret_cast<Local<JSValue>*>(&arg));
				}
			}
		});
	} else {
		return 0;
	}
}

template<bool ErrStyle = 1>
Callback get_callback_for_none_2(Worker* worker, Local<JSValue> cb) {
	if ( !cb.IsEmpty() && cb->IsFunction(worker) ) {
		CopyablePersistentFunc func(worker, cb.To<JSFunction>());
		
		return Cb([worker, func](Se& d) {
			XX_ASSERT(!func.IsEmpty());
			HandleScope scope(worker);
			Local<JSFunction> f = func.strong();
			if ( d.error ) {
				Local<JSValue> arg = worker->New(*static_cast<const Error*>(d.error));
				if (ErrStyle == 0) {
					f->Call(worker, 1, &arg);
				} else {
					Local<JSFunction> e = f->Get(worker, worker->strs()->Throw()).To<JSFunction>();
					e->Call(worker, 1, &arg, f);
				}
			} else {
				if (ErrStyle == 0) {
					Local<JSValue> arg = worker->NewNull();
					f->Call(worker, 1, &arg);
				} else {
					f->Call(worker);
				}
			}
		});
	} else {
		return 0;
	}
}

Callback get_callback_for_none(Worker* worker, Local<JSValue> cb) {
	return get_callback_for_none_2(worker, cb);
}

Callback get_callback_for_buffer(Worker* worker, Local<JSValue> cb, Encoding encoding) {
	return get_callback_for_buffer_2(worker, cb, encoding);
}

Callback get_callback_for_buffer_http_error(Worker* worker, Local<JSValue> cb, Encoding encoding) {
	return get_callback_for_buffer_2<HttpError>(worker, cb, encoding);
}

Callback get_callback_for_io_stream(Worker* worker, Local<JSValue> cb) {
	return get_callback_for_io_stream2(worker, cb);
}

Callback get_callback_for_io_stream_http_error(Worker* worker, Local<JSValue> cb) {
	return get_callback_for_io_stream2<HttpError>(worker, cb);
}

/**
 * @class NativeFS
 */
class NativeFS {
public:
	
	/**
	 * @func chmod_r_sync(path[,mode])
	 * @arg path {String}
	 * @arg [mode=default_mode] {uint}
	 * @ret {bool}
	 */

	/**
	 * @func chmod_r(path[,mode[,cb]][,cb])
	 * @arg path {String}
	 * @arg [mode=default_mode] {uint}
	 * @arg [cb] {Function}
	 * @ret {uint} return id
	 */
	template<bool sync> static void chmod_r(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func chmodSyncR(path[,mode])\n"
					"* @arg path {String}\n"
					"* @arg [mode=default_mode] {uint}\n"
					"* @ret {bool}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func chmodR(path[,mode[,cb]][,cb])\n"
					"* @arg path {String}\n"
					"* @arg [mode=default_mode] {uint}\n"
					"* @arg [cb] {Function}\n"
					"* @ret {uint} return id\n"
				);
			}
		}
		int args_index = 1;
		uint mode = FileHelper::default_mode;
		if (args.Length() > 1 && args[1]->IsUint32(worker)) {
			mode = args[1]->ToUint32Value(worker);
			args_index++;
		}
		if ( sync ) {
			JS_RETURN( FileHelper::chmod_r_sync(args[0]->ToStringValue(worker), mode) );
		} else {
			Callback cb;
			if ( args.Length() > args_index ) {
				cb = get_callback_for_none_2<0>(worker, args[args_index]);
			}
			JS_RETURN( FileHelper::chmod_r(args[0]->ToStringValue(worker), mode, cb) );
		}
	}

	/**
	 * @func chown_r_sync(path, owner, group)
	 * @arg path {String}
	 * @arg owner {uint}
	 * @arg group {uint}
	 * @ret {bool}
	 */

	/**
	 * @func chown_r(path, owner, group[,cb])
	 * @arg path {String}
	 * @arg owner {uint}
	 * @arg group {uint}
	 * @arg [cb] {Function}
	 * @ret {uint} return id
	 */
	template<bool sync> static void chown_r(FunctionCall args) {
		JS_WORKER(args);
		if ( args.Length() < 3 ||
				!args[0]->IsString(worker) ||
				!args[1]->IsUint32(worker) || !args[2]->IsUint32(worker) ) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func chownSyncR(path, owner, group)\n"
					"* @arg path {String}\n"
					"* @arg owner {uint}\n"
					"* @arg group {uint}\n"
					"* @ret {bool}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func chownR(path, owner, group[,cb])\n"
					"* @arg path {String}\n"
					"* @arg owner {uint}\n"
					"* @arg group {uint}\n"
					"* @arg [cb] {Function}\n"
					"* @ret {uint} return id\n"
				);
			}
		}
		
		if ( sync ) {
			JS_RETURN( FileHelper::chown_r_sync(args[0]->ToStringValue(worker),
																					 args[1]->ToUint32Value(worker),
																					 args[2]->ToUint32Value(worker)) );
		} else {
			Callback cb; 
			if ( args.Length() > 3 ) {
				cb = get_callback_for_none_2<0>(worker, args[3]);
			}
			JS_RETURN( FileHelper::chown_r(args[0]->ToStringValue(worker),
																			args[1]->ToUint32Value(worker),
																			args[2]->ToUint32Value(worker), cb) );
		}
	}
	
	/**
	 * @func mkdir_p_sync(path[,mode])
	 * @arg path {String}
	 * @arg [mode=default_mode] {uint}
	 * @ret {bool}
	 */
	
	/**
	 * @func mkdir_p(path[,mode[,cb]][,cb])
	 * @arg path {String}
	 * @arg [mode=default_mode] {uint}
	 * @arg [cb] {Function}
	 * @ret {uint} return id
	 */
	template<bool sync> static void mkdir_p(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			if ( sync ){
				JS_THROW_ERR(
					"* @func mkdirSyncP(path[,mode])\n"
					"* @arg path {String}\n"
					"* @arg [mode=default_mode] {uint}\n"
					"* @ret {bool}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func mkdirP(path[,mode[,cb]][,cb])\n"
					"* @arg path {String}\n"
					"* @arg [mode=default_mode] {uint}\n"
					"* @arg [cb] {Function}\n"
					"* @ret {uint} return id\n"
				);
			}
		}
		int args_index = 1;
		uint mode = FileHelper::default_mode;
		if (args.Length() > 1 && args[1]->IsUint32(worker)) {
			mode = args[1]->ToUint32Value(worker);
			args_index++;
		}
		if ( sync ) {
			JS_RETURN( FileHelper::mkdir_p_sync(args[0]->ToStringValue(worker), mode) );
		} else {
			Callback cb;
			if ( args.Length() > args_index ) {
				cb = get_callback_for_none_2<0>(worker, args[args_index]);
			}
			FileHelper::mkdir_p(args[0]->ToStringValue(worker), mode, cb);
		}
	}

	/**
	 * @func rm_r_sync(path)
	 * @arg path {String}
	 * @ret {bool}
	 */

	/**
	 * @func rm_r(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 * @ret {uint} return id
	 */
	template<bool sync> static void rm_r(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || !args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func removeSyncR(path)\n"
					"* @arg path {String}\n"
					"* @ret {bool}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func removeR(path)\n"
					"* @arg path {String}\n"
					"* @arg [cb] {Function}\n"
					"* @ret {uint} return id\n"
				);
			}
		}
		if ( sync ) {
			JS_RETURN( FileHelper::rm_r_sync(args[0]->ToStringValue(worker)) );
		} else {
			Callback cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_none_2<0>(worker, args[1]);
			}
			JS_RETURN( FileHelper::rm_r(args[0]->ToStringValue(worker), cb) );
		}
	}
	
	/**
	 * @func cp_sync(path, target)
	 * @arg path {String}
	 * @arg target {String}
	 * @ret {bool}
	 */
	
	/**
	 * @func cp(path, target)
	 * @arg path {String}
	 * @arg target {String}
	 * @arg [cb] {Function}
	 * @ret {uint} return id
	 */
	template<bool sync> static void cp(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func copySync(path, target)\n"
					"* @arg path {String}\n"
					"* @arg target {String}\n"
					"* @ret {bool}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func copy(path, target)\n"
					"* @arg path {String}\n"
					"* @arg target {String}\n"
					"* @arg [cb] {Function}\n"
					"* @ret {uint} return id\n"
				);
			}
		}
		if ( sync ) {
			JS_RETURN( FileHelper::cp_sync(args[0]->ToStringValue(worker),
																			args[1]->ToStringValue(worker)) );
		} else {
			Callback cb;
			if ( args.Length() > 2 ) {
				cb = get_callback_for_none_2<0>(worker, args[2]);
			}
			JS_RETURN( FileHelper::cp(args[0]->ToStringValue(worker),
																 args[1]->ToStringValue(worker), cb) );
		}
	}
	
	/**
	 * @func cp_r_sync(path, target)
	 * @arg path {String}
	 * @arg target {String}
	 * @ret {bool}
	 */
	
	/**
	 * @func cp_r(path, target)
	 * @arg path {String}
	 * @arg target {String}
	 * @arg [cb] {Function}
	 * @ret {uint} return id
	 */
	template<bool sync> static void cp_r(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func copySyncR(path, target)\n"
					"* @arg path {String}\n"
					"* @arg target {String}\n"
					"* @ret {bool}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func copyR(path, target)\n"
					"* @arg path {String}\n"
					"* @arg target {String}\n"
					"* @arg [cb] {Function}\n"
					"* @ret {uint} return id\n"
				);
			}
		}
		if ( sync ) {
			JS_RETURN( FileHelper::cp_r_sync(args[0]->ToStringValue(worker),
																				args[1]->ToStringValue(worker)) );
		} else {
			Callback cb;
			if ( args.Length() > 2 ) {
				cb = get_callback_for_none_2<0>(worker, args[2]);
			}
			JS_RETURN( FileHelper::cp_r(args[0]->ToStringValue(worker),
																	 args[1]->ToStringValue(worker), cb) );
		}
	}
	
	/**
	 * @func readdirSync(path)
	 * @arg path {String}
	 * @ret {Array} return Array<Dirent>
	 */
	/**
	 * @func readdir(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 */
	template<bool sync> static void readdir(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || !args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func readdirSync(path)\n"
					"* @arg path {String}\n"
					"* @ret {Array} return Array<Dirent>\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func readdir(path[,cb])\n"
					"* @arg path {String}\n"
					"* @arg [cb] {Function}\n"
				);
			}
		}
		if ( sync ) {
			JS_RETURN( FileHelper::readdir_sync(args[0]->ToStringValue(worker)) );
		} else {
			Callback cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_t<Array<Dirent>, Error, 0>(worker, args[1]);
			}
			FileHelper::readdir( args[0]->ToStringValue(worker), cb);
		}
	}
	
	/**
	 * @func exists_file_sync(path)
	 * @arg path {String}
	 * @ret {bool}
	 */
	
	/**
	 * @func is_file(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 */
	template<bool sync> static void is_file(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
											"* @func isFileSync(path)\n"
											"* @arg path {String}\n"
											"* @ret {bool}\n"
											);
			} else {
				JS_THROW_ERR(
											"* @func isFile(path[,cb])\n"
											"* @arg path {String}\n"
											"* @arg [cb] {Function}\n"
											);
			}
		}
		if ( sync ) {
			JS_RETURN( FileHelper::is_file_sync(args[0]->ToStringValue(worker)) );
		} else {
			Callback cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_t<Bool, Error, 0>(worker, args[1]);
			}
			FileHelper::is_file(args[0]->ToStringValue(worker), cb);
		}
	}
	
	/**
	 * @func exists_dir_sync(path)
	 * @arg path {String}
	 * @ret {bool}
	 */
	
	/**
	 * @func is_directory(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 */
	template<bool sync> static void is_directory(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
											"* @func isDirectorySync(path)\n"
											"* @arg path {String}\n"
											"* @ret {bool}\n"
											);
			} else {
				JS_THROW_ERR(
											"* @func isDirectory(path[,cb])\n"
											"* @arg path {String}\n"
											"* @arg [cb] {Function}\n"
											);
			}
		}
		if ( sync ) {
			JS_RETURN( FileHelper::is_directory_sync(args[0]->ToStringValue(worker)) );
		} else {
			Callback cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_t<Bool, Error, 0>(worker, args[1]);
			}
			FileHelper::is_directory(args[0]->ToStringValue(worker), cb);
		}
	}
	
	/**
	 * @func abort(id) abort async io
	 * @arg id {uint}
	 */
	static void abort(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsUint32(worker)) {
			JS_THROW_ERR(
				"* @func abort(id) abort async io\n"
				"* @arg id {uint}\n"
			);
		}
		FileHelper::abort( args[0]->ToUint32Value(worker) );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		//
		JS_SET_PROPERTY(FILE_UNKNOWN, FILE_UNKNOWN);
		JS_SET_PROPERTY(FILE_FILE, FILE_FILE);
		JS_SET_PROPERTY(FILE_DIR, FILE_DIR);
		JS_SET_PROPERTY(FILE_LINK, FILE_LINK);
		JS_SET_PROPERTY(FILE_FIFO, FILE_FIFO);
		JS_SET_PROPERTY(FILE_SOCKET, FILE_SOCKET);
		JS_SET_PROPERTY(FILE_CHAR, FILE_CHAR);
		JS_SET_PROPERTY(FILE_BLOCK, FILE_BLOCK);
		//
		JS_SET_PROPERTY(DEFAULT_MODE, FileHelper::default_mode);
		// api
		JS_SET_METHOD(chmodSyncR, chmod_r<true>);
		JS_SET_METHOD(chownSyncR, chown_r<true>);
		JS_SET_METHOD(mkdirSyncP, mkdir_p<true>);
		JS_SET_METHOD(removeSyncR, rm_r<true>);
		JS_SET_METHOD(copySync, cp<true>);
		JS_SET_METHOD(copySyncR, cp_r<true>);
		JS_SET_METHOD(readdirSync, readdir<true>);
		JS_SET_METHOD(isFileSync, is_file<true>);
		JS_SET_METHOD(isDirectorySync, is_directory<true>);
		JS_SET_METHOD(abort, abort);
		JS_SET_METHOD(mkdirP, mkdir_p<false>);
		JS_SET_METHOD(readdir, readdir<false>);
		JS_SET_METHOD(isFile, is_file<false>);
		JS_SET_METHOD(isDirectory, is_directory<false>);
		JS_SET_METHOD(chmodR, chmod_r<false>);
		JS_SET_METHOD(chownR, chown_r<false>);
		JS_SET_METHOD(removeR, rm_r<false>);
		JS_SET_METHOD(copy, cp<false>);
		JS_SET_METHOD(copyR, cp_r<false>);
	}
};

JS_REG_MODULE(ngui_fs, NativeFS);
JS_END
