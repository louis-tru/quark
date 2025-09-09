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

#include "./types.h"

namespace qk { namespace js {

	struct MixFileStat: MixObject {
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(FileStat, 0, {
				if (args.length() == 0 || !args[0]->isString()) {
					New<MixFileStat>(args, new FileStat());
				} else {
					New<MixFileStat>(args, new FileStat(args[0]->toString(worker)->value(worker)));
				}
			});
			Js_Class_Method(isValid, {
				Js_Self(FileStat);
				Js_ReturnBool( self->is_valid() );
			});
			Js_Class_Method(isFile, {
				Js_Self(FileStat);
				Js_ReturnBool( self->is_file() );
			});
			Js_Class_Method(isDir, {
				Js_Self(FileStat);
				Js_ReturnBool( self->is_dir() );
			});
			Js_Class_Method(isLink, {
				Js_Self(FileStat);
				Js_ReturnBool( self->is_link() );
			});
			Js_Class_Method(isSock, {
				Js_Self(FileStat);
				Js_ReturnBool( self->is_sock() );
			});
			Js_Class_Method(mode, {
				Js_Self(FileStat);
				Js_Return( self->mode() );
			});
			Js_Class_Method(type, {
				Js_Self(FileStat);
				Js_Return( self->type() );
			});
			Js_Class_Method(group, {
				Js_Self(FileStat);
				Js_Return( self->group() );
			});
			Js_Class_Method(owner, {
				Js_Self(FileStat);
				Js_Return( self->owner() );
			});
			Js_Class_Method(size, {
				Js_Self(FileStat);
				Js_Return( self->size() );
			});
			Js_Class_Method(nlink, {
				Js_Self(FileStat);
				Js_Return( self->nlink() );
			});
			Js_Class_Method(ino, {
				Js_Self(FileStat);
				Js_Return( self->ino() );
			});
			Js_Class_Method(blksize, {
				Js_Self(FileStat);
				Js_Return( self->blksize() );
			});
			Js_Class_Method(blocks, {
				Js_Self(FileStat);
				Js_Return( self->blocks() );
			});
			Js_Class_Method(flags, {
				Js_Self(FileStat);
				Js_Return( self->flags() );
			});
			Js_Class_Method(gen, {
				Js_Self(FileStat);
				Js_Return( self->gen() );
			});
			Js_Class_Method(dev, {
				Js_Self(FileStat);
				Js_Return( self->dev() );
			});
			Js_Class_Method(rdev, {
				Js_Self(FileStat);
				Js_Return( self->rdev() );
			});
			Js_Class_Method(atime, {
				Js_Self(FileStat);
				Js_Return( self->atime() / 1000 );
			});
			Js_Class_Method(mtime, {
				Js_Self(FileStat);
				Js_Return( self->mtime() / 1000 );
			});
			Js_Class_Method(ctime, {
				Js_Self(FileStat);
				Js_Return( self->ctime() / 1000 );
			});
			Js_Class_Method(birthtime, {
				Js_Self(FileStat);
				Js_Return( self->birthtime() / 1000 );
			});
			cls->exports("FileStat", exports);
		}
	};

	static char* get_file_write_params(
		FunctionArgs args, bool sync, int& args_index,
		Buffer& holder, int64_t& size, Callback<Buffer>& cb
	) {
		Js_Worker(args);

		auto afterCollapse = false;
		char* data = nullptr;

		if ( args[args_index]->isString() ) { // 写入字符串
			int index_str = args_index++;
			Encoding en = kUTF8_Encoding;

			if ( args.length() > args_index && args[args_index]->isString() ) { // 第三个参数为编码格式
				if ( ! parseEncoding(args, args[args_index++], en) ) return nullptr;
			}
			holder = args[index_str]->toString(worker)->toBuffer(worker, en);
			data = holder.val();
			size = holder.length();
		}
		else { // ArrayBuffer or TypedArray
			WeakBuffer weakBuffer;
			Qk_ASSERT_EQ(true, args[args_index++]->asBuffer(worker).to(weakBuffer));

			data = const_cast<char*>(*weakBuffer);
			size = weakBuffer.length();

			if (!sync) { // async
				// 这是一个危险的操作,确保buffer不能被释放否则会导致致命错误
				holder = Buffer(data, size);
				afterCollapse = true;
			}

			if ( args.length() > args_index && args[args_index]->isInt32() ) { // size
				int num = args[args_index++]->toInt32(worker)->value();
				if ( num >= 0 && num < size ) {
					size = num;
					if (!sync) { // async
						holder.collapse();
						holder = Buffer(data, num);
					}
				}
			}
		}

		if (!sync) {
			// keep war buffer Persistent javascript object
			struct PersistentValue {
				PersistentValue(FunctionArgs args): source(args.worker(), args[2]) {}
				Persistent<JSValue> source;
			} *pv = new PersistentValue(args);

			auto cbInt = get_callback_for_int(worker, args[0]);

			cb = Callback<Buffer>([pv, cbInt, afterCollapse](auto &e) {
				Qk_ASSERT( e.data );
				Sp<PersistentValue> h(pv);
				auto len = e.data->length();
				if (afterCollapse) {
					// collapse这个buffer因为这是ArrayBuffer所持有的内存空间,不能在这里被释放
					e.data->collapse();
				}
				if (e.error) {
					cbInt->reject(e.error);
				} else {
					Int32 i(len);
					cbInt->resolve(&i);
				}
			});
		}
		return data;
	}

	struct NativeFileReader {
		static void read(FunctionArgs args, bool isStream) {
			Js_Worker(args);
			uint32_t args_index = 1;
			if ( args.length() < 2 || !args[0]->isFunction() || !args[1]->isString() ) {
				if (isStream) {
					Js_Throw(
						"@method reader.readStream(cb,path)\n"
						"@param cb:Function\n"
						"@param path:string\n"
						"@return {Uint} return read id\n"
					);
				} else {
					Js_Throw(
								"@method reader.readFile(cb,path,encoding?)\n"
								"@param cb:Function\n"
								"@param path:string\n"
								"@param encoding?:Encoding\n"
								"@return {Uint} return read id\n"
					);
				}
			}
			String path = args[args_index++]->toString(worker)->value(worker);
			Encoding encoding = kInvalid_Encoding;
			bool isHttp = fs_is_http_file(path);

			if (args.length() > args_index && args[args_index]->isString()) {
				if ( ! parseEncoding(args, args[args_index++], encoding) ) return;
			}
			if ( isStream ) {
				auto cb = isHttp ?
					get_callback_for_io_stream_http_error(worker, args[0]):
					get_callback_for_io_stream(worker, args[0]);
				Js_Return( fs_reader()->read_stream( path, cb ) );
			} else {
				auto cb = isHttp ? 
					get_callback_for_buffer_http_error(worker, args[0], encoding):
					get_callback_for_buffer(worker, args[0], encoding);
				Js_Return( fs_reader()->read_file( path, cb ) );
			}
		}

		static void binding(JSObject* exports, Worker* worker) {

			Js_Method(readFile, {
				read(args, false);
			});

			Js_Method(readStream, {
				read(args, true);
			});

			Js_Method(readFileSync, {
				if (args.length() == 0 || !args[0]->isString()) {
					Js_Throw(
						"@method reader.readFileSync(path,encoding?)\n"
						"@param path:string\n"
						"@param encoding?:Encoding\n"
						"@return {Buffer} return read Buffer\n"
					);
				}
				Encoding encoding = kInvalid_Encoding;
				if (args.length() > 1 && args[1]->isString()) {
					if ( ! parseEncoding(args, args[1], encoding) ) return;
				}
				Buffer rv;
				try {
					rv = fs_reader()->read_file_sync( args[0]->toString(worker)->value(worker) );
				} catch(cError& err) {
					Js_Throw(err);
				}
				Js_Return( convert_buffer(worker, rv, encoding) );
			});

			Js_Method(existsSync, {
				if ( args.length() == 0 || !args[0]->isString() ) {
					Js_Throw(
						"@method reader.existsSync(path)\n"
						"@param path:string\n"
						"@return {boolean}\n"
					);
				}
				Js_ReturnBool( fs_reader()->exists_sync( args[0]->toString(worker)->value(worker) ) );
			});

			Js_Method(isFileSync, {
				if ( args.length() == 0 || !args[0]->isString() ) {
					Js_Throw(
						"@method reader.isFileSync(path)\n"
						"@param path:string\n"
						"@return {boolean}\n"
					);
				}
				Js_ReturnBool( fs_reader()->is_file_sync( args[0]->toString(worker)->value(worker) ) );
			});

			Js_Method(isDirectorySync, {
				if ( args.length() == 0 || !args[0]->isString() ) {
					Js_Throw(
						"@method reader.isDirectorySync(path)\n"
						"@param path:string\n"
						"@return {boolean}\n"
					);
				}
				Js_ReturnBool( fs_reader()->is_directory_sync( args[0]->toString(worker)->value(worker) ) );
			});

			Js_Method(readdirSync, {
				if ( args.length() == 0 || !args[0]->isString() ) {
					Js_Throw(
						"@method reader.readdirSync(path)\n"
						"@param path:string\n"
						"@return {Array}\n"
					);
				}
				auto path = args[0]->toString(worker)->value(worker);
				auto r = fs_reader()->readdir_sync(path);
				Js_Return( worker->types()->jsvalue(r) );
			});

			Js_Method(abort, {
				if ( args.length() == 0 || ! args[0]->isUint32() ) {
					Js_Throw(
						"@method reader.abort(id)\n"
						"@param id:Uint abort id\n"
					);
				}
				fs_reader()->abort( args[0]->toUint32(worker)->value() );
			});

			Js_Method(clear, {
				fs_reader()->clear();
			});
		}
	};

	struct NativeFs {
		static void chmod(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || ! args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"@method chmodSync(path[,mode])\n"
						"@param path:string\n"
						"@param mode?:Uint\n"
					);
				} else {
					Js_Throw(
						"@method chmod(path[,mode[,cb]][,cb])\n"
						"@param path:string\n"
						"@param mode?:Uint\n"
						"@param cb?:Function\n"
					);
				}
			}
			int args_index = 1;
			uint32_t mode = fs_default_mode;
			if (args.length() > 1 && args[1]->isUint32()) {
				mode = args[1]->toUint32(worker)->value();
				args_index++;
			}
			if ( sync ) {
				try {
					fs_chmod_sync(args[0]->toString(worker)->value(worker), mode);
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > args_index ) {
					cb = get_callback_for_none(worker, args[args_index]);
				}
				fs_chmod(args[0]->toString(worker)->value(worker), mode, cb);
			}
		}

		static void chmod_recursion(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || ! args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"@method chmodRecursionSync(path,mode?)\n"
						"@param path:string\n"
						"@param mode?:Uint\n"
						"@return {void}\n"
					);
				} else {
					Js_Throw(
						"@method chmodRecursion(path,mode?,cb?)\n"
						"@param path:string\n"
						"@param mode?:Uint\n"
						"@param cb?:Function\n"
						"@return {Uint} return id\n"
					);
				}
			}
			int args_index = 1;
			uint32_t mode = fs_default_mode;
			if (args.length() > 1 && args[1]->isUint32()) {
				mode = args[1]->toUint32(worker)->value();
				args_index++;
			}
			if ( sync ) {
				try {
					fs_chmod_recursion_sync(args[0]->toString(worker)->value(worker), mode);
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > args_index ) {
					cb = get_callback_for_none(worker, args[args_index]);
				}
				Js_Return( fs_chmod_recursion(args[0]->toString(worker)->value(worker), mode, cb) );
			}
		}

		static void chown(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 3 ||
					!args[0]->isString() ||
					!args[1]->isUint32() || !args[2]->isUint32() ) {
				if ( sync ) {
					Js_Throw(
						"@method chownSync(path,owner,group)\n"
						"@param path:string\n"
						"@param owner:Uint\n"
						"@param group:Uint\n"
						"@return {void}\n"
					);
				} else {
					Js_Throw(
						"@method chown(path,owner,group,cb?)\n"
						"@param path:string\n"
						"@param owner:Uint\n"
						"@param group:Uint\n"
						"@param cb?:Function\n"
					);
				}
			}
			if ( sync ) {
				try {
					fs_chown_sync(args[0]->toString(worker)->value(worker),
												args[1]->toUint32(worker)->value(),
												args[2]->toUint32(worker)->value());
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 3 ) {
					cb = get_callback_for_none(worker, args[3]);
				}
				fs_chown(args[0]->toString(worker)->value(worker),
								args[1]->toUint32(worker)->value(),
								args[2]->toUint32(worker)->value(), cb);
			}
		}

		static void chown_recursion(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if ( args.length() < 3 ||
					!args[0]->isString() ||
					!args[1]->isUint32() || !args[2]->isUint32() ) {
				if ( sync ) {
					Js_Throw(
						"@method chownRecursionSync(path,owner,group)\n"
						"@param path:string\n"
						"@param owner:Uint\n"
						"@param group:Uint\n"
						"@return {void}\n"
					);
				} else {
					Js_Throw(
						"@method chownRecursion(path,owner,group,cb?)\n"
						"@param path:string\n"
						"@param owner:Uint\n"
						"@param group:Uint\n"
						"@param cb?:Function\n"
						"@return {Uint} return id\n"
					);
				}
			}
			
			if ( sync ) {
				try {
					fs_chown_recursion_sync(args[0]->toString(worker)->value(worker),
													args[1]->toUint32(worker)->value(),
													args[2]->toUint32(worker)->value());
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 3 ) {
					cb = get_callback_for_none(worker, args[3]);
				}
				Js_Return( fs_chown_recursion(args[0]->toString(worker)->value(worker),
															args[1]->toUint32(worker)->value(),
															args[2]->toUint32(worker)->value(), cb) );
			}
		}

		static void mkdir(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || !args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"@method mkdirSync(path,mode?)\n"
						"@param path:string\n"
						"@param mode?:Uint\n"
						"@return {void}\n"
					);
				} else {
					Js_Throw(
						"@method mkdir(path,mode?,cb?)\n"
						"@param path:string\n"
						"@param mode?:Uint\n"
						"@param cb?:Function\n"
					);
				}
			}
			int args_index = 1;
			uint32_t mode = fs_default_mode;
			if (args.length() > 1 && args[1]->isUint32()) {
				mode = args[1]->toUint32(worker)->value();
				args_index++;
			}
			if ( sync ) {
				try {
					fs_mkdir_sync(args[0]->toString(worker)->value(worker), mode);
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > args_index ) {
					cb = get_callback_for_none(worker, args[args_index]);
				}
				fs_mkdir(args[0]->toString(worker)->value(worker), mode, cb);
			}
		}

		static void mkdirs(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || ! args[0]->isString()) {
				if ( sync ){
					Js_Throw(
						"@method mkdirsSync(path,mode?)\n"
						"@param path:string\n"
						"@param mode?:Uint\n"
						"@return {void}\n"
					);
				} else {
					Js_Throw(
						"@method mkdirs(path,mode?,cb?)\n"
						"@param path:string\n"
						"@param mode?:Uint\n"
						"@param cb?:Function\n"
						"@return {Uint} return id\n"
					);
				}
			}
			int args_index = 1;
			uint32_t mode = fs_default_mode;
			if (args.length() > 1 && args[1]->isUint32()) {
				mode = args[1]->toUint32(worker)->value();
				args_index++;
			}
			if ( sync ) {
				try {
					fs_mkdirs_sync(args[0]->toString(worker)->value(worker), mode);
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > args_index ) {
					cb = get_callback_for_none(worker, args[args_index]);
				}
				fs_mkdirs(args[0]->toString(worker)->value(worker), mode, cb);
			}
		}

		static void rename(FunctionArgs args, bool sync) {
			Js_Worker(args);

			if (args.length() < 2 || !args[0]->isString() || !args[1]->isString()) {
				if ( sync ) {
					Js_Throw(
						"@method renameSync(name,new_name)\n"
						"@param name:string\n"
						"@param new_name:string\n"
						"@return {void}\n"
					);
				} else {
					Js_Throw(
						"@method rename(name,new_name,cb?)\n"
						"@param name:string\n"
						"@param new_name:string\n"
						"@param cb?:Function\n"
					);
				}
			}

			if ( sync ) {
				try {
					fs_rename_sync(args[0]->toString(worker)->value(worker),
																	args[1]->toString(worker)->value(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 2 ) {
					cb = get_callback_for_none(worker, args[2]);
				}
				fs_rename(args[0]->toString(worker)->value(worker), args[1]->toString(worker)->value(worker), cb);
			}
		}

		static void link(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 2 || !args[0]->isString() || !args[1]->isString()) {
				if ( sync ) {
					Js_Throw(
						"@method linkSync(path,newPath)\n"
						"@param path:string\n"
						"@return {void}\n"
					);
				} else {
					Js_Throw(
						"@method link(path,newPath,cb?)\n"
						"@param path:string\n"
						"@param cb?:Function\n"
					);
				}
			}
			if ( sync ) {
				try {
					fs_link_sync(
						args[0]->toString(worker)->value(worker),
						args[1]->toString(worker)->value(worker)
					);
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 2 ) {
					cb = get_callback_for_none(worker, args[2]);
				}
				fs_link(args[0]->toString(worker)->value(worker), args[1]->toString(worker)->value(worker), cb);
			}
		}

		static void unlink(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || !args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"@method unlinkSync(path)\n"
						"@param path:string\n"
						"@return {void}\n"
					);
				} else {
					Js_Throw(
						"@method unlink(path,cb?)\n"
						"@param path:string\n"
						"@param cb?:Function\n"
					);
				}
			}
			if ( sync ) {
				try {
					fs_unlink_sync(args[0]->toString(worker)->value(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 1 ) {
					cb = get_callback_for_none(worker, args[1]);
				}
				fs_unlink(args[0]->toString(worker)->value(worker), cb);
			}
		}

		static void rmdir(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || !args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"@method rmdirSync(path)\n"
						"@param path:string\n"
						"@return {void}\n"
					);
				} else {
					Js_Throw(
						"@method rmdir(path)\n"
						"@param path:string\n"
						"@param cb?:Function\n"
					);
				}
			}
			if ( sync ) {
				try {
					fs_rmdir_sync(args[0]->toString(worker)->value(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 1 ) {
					cb = get_callback_for_none(worker, args[1]);
				}
				fs_rmdir(args[0]->toString(worker)->value(worker), cb);
			}
		}

		static void remove_recursion(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || !args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"@method removeRecursionSync(path)\n"
						"@param path:string\n"
						"@return {void}\n"
					);
				} else {
					Js_Throw(
						"@method removeRecursion(path,cb?)\n"
						"@param path:string\n"
						"@param cb?:Function\n"
						"@return {Uint} return id\n"
					);
				}
			}
			if ( sync ) {
				try {
					fs_remove_recursion_sync(args[0]->toString(worker)->value(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 1 ) {
					cb = get_callback_for_none(worker, args[1]);
				}
				Js_Return( fs_remove_recursion(args[0]->toString(worker)->value(worker), cb) );
			}
		}

		static void copy(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 2 || !args[0]->isString() || !args[1]->isString()) {
				if ( sync ) {
					Js_Throw(
						"@method copySync(path,target)\n"
						"@param path:string\n"
						"@param target:string\n"
						"@return {void}\n"
					);
				} else {
					Js_Throw(
						"@method copy(path,target,cb?)\n"
						"@param path:string\n"
						"@param target:string\n"
						"@param cb?:Function\n"
						"@return {Uint} return id\n"
					);
				}
			}
			if ( sync ) {
				try {
					fs_copy_sync(args[0]->toString(worker)->value(worker),
																args[1]->toString(worker)->value(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 2 ) {
					cb = get_callback_for_none(worker, args[2]);
				}
				Js_Return( fs_copy(args[0]->toString(worker)->value(worker),
																		args[1]->toString(worker)->value(worker), cb) );
			}
		}

		static void copy_recursion(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 2 || !args[0]->isString() || !args[1]->isString()) {
				if ( sync ) {
					Js_Throw(
						"@method copyRecursionSync(path,target)\n"
						"@param path:string\n"
						"@param target:string\n"
						"@return {void}\n"
					);
				} else {
					Js_Throw(
						"@method copyRecursion(path,target,cb?)\n"
						"@param path:string\n"
						"@param target:string\n"
						"@param cb?:Function\n"
						"@return {Uint} return id\n"
					);
				}
			}
			if ( sync ) {
				try {
					fs_copy_recursion_sync(args[0]->toString(worker)->value(worker),
																	args[1]->toString(worker)->value(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 2 ) {
					cb = get_callback_for_none(worker, args[2]);
				}
				Js_Return( fs_copy_recursion(args[0]->toString(worker)->value(worker),
																			args[1]->toString(worker)->value(worker), cb) );
			}
		}

		static void readdir(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || !args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"@method readdirSync(path)\n"
						"@param path:string\n"
						"@return {Array} return Array<Dirent>\n"
					);
				} else {
					Js_Throw(
						"@method readdir(path,cb?)\n"
						"@param path:string\n"
						"@param cb?:Function\n"
					);
				}
			}
			if ( sync ) {
				Array<Dirent> r;
				try {
					r = fs_readdir_sync(args[0]->toString(worker)->value(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
				Js_Return( worker->types()->jsvalue(r) );
			} else {
				Callback<qk::Array<qk::Dirent>> cb;
				if ( args.length() > 1 ) {
					cb = get_callback_for_array_dirent(worker, args[1]);
				}
				fs_readdir( args[0]->toString(worker)->value(worker), cb);
			}
		}

		static void stat(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || ! args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"@method statSync(path)\n"
						"@param path:string\n"
						"@return {FileStat}\n"
					);
				} else {
					Js_Throw(
						"@method stat(path,cb?)\n"
						"@param path:?string\n"
						"@param cb?:Function\n"
					);
				}
			}
			if ( sync ) {
				FileStat r;
				try {
					r = fs_stat_sync( args[0]->toString(worker)->value(worker) );
				} catch(cError& err) {
					Js_Throw(err);
				}
				Js_Return( worker->types()->jsvalue(r) );
			} else {
				Callback<FileStat> cb;
				if ( args.length() > 1 ) {
					cb = get_callback_for_file_stat(worker, args[1]);
				}
				fs_stat(args[0]->toString(worker)->value(worker), cb);
			}
		}

		static void exists(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if ( sync ) {
				if (args.length() < 1 || ! args[0]->isString()) {
					Js_Throw(
						"@method existsSync(path)\n"
						"@param path:string\n"
						"@return {boolean}\n"
					);
				}
				Js_ReturnBool( fs_exists_sync(args[0]->toString(worker)->value(worker)) );
			} else {
				if (args.length() < 2 || !args[0]->isString() || !args[1]->isFunction()) {
					Js_Throw(
						"@method exists(path,cb?)\n"
						"@param path:string\n"
						"@param cb?:Function\n"
					);
				}
				fs_exists(args[0]->toString(worker)->value(worker), get_callback_for_bool(worker, args[1]));
			}
		}

		static void is_file(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if ( sync ) {
				if (args.length() < 1 || ! args[0]->isString()) {
					Js_Throw(
						"@method isFileSync(path)\n"
						"@param path:string\n"
						"@return {boolean}\n"
					);
				}
				Js_ReturnBool( fs_is_file_sync(args[0]->toString(worker)->value(worker)) );
			} else {
				if (args.length() < 2 || !args[0]->isString() || !args[1]->isFunction()) {
					Js_Throw(
						"@method isFile(path,cb?)\n"
						"@param path:string\n"
						"@param cb?:Function\n"
					);
				}
				fs_is_file(args[0]->toString(worker)->value(worker), get_callback_for_bool(worker, args[1]));
			}
		}

		static void is_directory(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if ( sync ) {
				if (args.length() < 1 || ! args[0]->isString()) {
					Js_Throw(
						"@method isDirectorySync(path)\n"
						"@param path:string\n"
						"@return {boolean}\n"
					);
				}
				Js_ReturnBool( fs_is_directory_sync(args[0]->toString(worker)->value(worker)) );
			} else {
				if (args.length() < 2 || !args[0]->isString() || !args[1]->isFunction()) {
					Js_Throw(
						"@method isDirectory(path,cb?)\n"
						"@param path:string\n"
						"@param cb?:Function\n"
					);
				}
				fs_is_directory(args[0]->toString(worker)->value(worker), get_callback_for_bool(worker, args[1]));
			}
		}

		static void readable(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if ( sync ) {
				if (args.length() < 1 || ! args[0]->isString()) {
					Js_Throw(
						"@method readableSync(path)\n"
						"@param path:string\n"
						"@return {boolean}\n"
					);
				}
				Js_ReturnBool( fs_readable_sync(args[0]->toString(worker)->value(worker)) );
			} else {
				if (args.length() < 2 || !args[0]->isString() || !args[1]->isFunction()) {
					Js_Throw(
						"@method readable(path,cb?)\n"
						"@param path:string\n"
						"@param cb?:Function\n"
					);
				}
				fs_readable(args[0]->toString(worker)->value(worker), get_callback_for_bool(worker, args[1]));
			}
		}

		static void writable(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if ( sync ) {
				if (args.length() < 1 || ! args[0]->isString()) {
					Js_Throw(
						"@method writableSync(path)\n"
						"@param path:string\n"
						"@return {boolean}\n"
					);
				}
				Js_ReturnBool( fs_writable_sync(args[0]->toString(worker)->value(worker)) );
			} else {
				if (args.length() < 2 || !args[0]->isString() || !args[1]->isFunction()) {
					Js_Throw(
						"@method writable(path,cb?)\n"
						"@param path:string\n"
						"@param cb?:Function\n"
					);
				}
				fs_writable(args[0]->toString(worker)->value(worker), get_callback_for_bool(worker, args[1]));
			}
		}

		static void executable(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if ( sync ) {
				if (args.length() < 1 || ! args[0]->isString()) {
					Js_Throw(
						"@method executableSync(path)\n"
						"@param path:string\n"
						"@return {boolean}\n"
					);
				}
				Js_ReturnBool( fs_executable_sync(args[0]->toString(worker)->value(worker)) );
			} else {
				if (args.length() < 2 || !args[0]->isString() || !args[1]->isFunction()) {
					Js_Throw(
						"@method executable(path,cb?)\n"
						"@param path:string\n"
						"@param cb?:Function\n"
					);
				}
				fs_executable(args[0]->toString(worker)->value(worker), get_callback_for_bool(worker, args[1]));
			}
		}

		static void read_file(FunctionArgs args, bool sync) {
			Js_Worker(args);

			int args_index = 0;
			if (sync) {
				if (args.length() < 1 || !args[0]->isString()) {
					Js_Throw(
						"@method readFileSync(path,encoding?)\n"
						"@param path:string\n"
						"@param encoding?:string\n"
						"@return {Buffer} return file buffer\n"
					);
				}
			} else {
				args_index++;
				if (args.length() < 2 || !args[0]->isFunction() || !args[1]->isString()) {
					Js_Throw(
						"@method readFile(cb,path,encoding?)\n"
						"@param cb:Function\n"
						"@param path:string\n"
						"@param encoding?:string\n"
					);
				}
			}

			String path = args[args_index++]->toString(worker)->value(worker);
			Encoding encoding = kInvalid_Encoding;

			if (args.length() > args_index && args[args_index]->isString()) { //
				if ( !parseEncoding(args, args[args_index++], encoding) ) return;
			}

			if ( sync ) {
				Buffer r;
				try {
					r = fs_read_file_sync(path, -1);
				} catch(cError& err) {
					Js_Throw(err);
				}
				Js_Return( convert_buffer(worker, r, encoding) );
			} else {
				// Qk_Log("read_file,args,%d", args.length());
				fs_read_file(path, get_callback_for_buffer(worker, args[0], encoding));
			}
		}

		static void write_file(FunctionArgs args, bool sync) {
			Js_Worker(args);

			int args_index = 0;
			if ( sync ) {
				if ( args.length() < 2 || !args[0]->isString() ||
						!(args[1]->isString() || args[1]->isBuffer())
				) {
					Js_Throw(
						"@method writeFileSync(path,buffer,size?)\n"
						"@method writeFileSync(path,string,encoding?)\n"
						"@param path:string\n"
						"@param string:string\n"
						"@param buffer:Uint8Array|ArrayBuffer\n"
						"@param size?:Int write data size\n"
						"@param encoding?:Encoding\n"
						"@return {Int}\n"
					);
				}
			} else {
				args_index++;
				if ( args.length() < 3 || !args[0]->isFunction() || !args[1]->isString() ||
						!(args[2]->isString() || args[2]->isBuffer())
					) {
					Js_Throw(
						"@method writeFile(cb,path,buffer,size?)\n"
						"@method writeFile(cb,path,string,encoding?)\n"
						"@param cb:Function\n"
						"@param path:string\n"
						"@param string:string\n"
						"@param buffer:Uint8Array|ArrayBuffer\n"
						"@param size?:Int write data size\n"
						"@param encoding?:Encoding\n"
					);
				}
			}
			
			String path = args[args_index++]->toString(worker)->value(worker);
			Buffer holder;
			int64_t size;
			Callback<Buffer> cb;

			auto data = get_file_write_params(args, sync, args_index, holder, size, cb);
			if (!data)
				return;

			if ( sync ) {
				int r;
				try {
					r = fs_write_file_sync(path, data, size);
				} catch(cError& err) {
					Js_Throw(err);
				}
				Js_Return( r );
			} else {
				fs_write_file(path, holder, cb);
			}
		}

		static void open(FunctionArgs args, bool sync) {
			Js_Worker(args);

			int args_index = 0;
			if (sync) {
				if ( args.length() < 1 || !args[0]->isString() ) {
					Js_Throw(
						"@method openSync(path,flag?)\n"
						"@param path:string\n"
						"@param flag?:FileOpenFlag\n"
						"@return {Int} return file handle `success >= 0`\n"
					);
				} 
			} else {
				args_index++;
				if ( args.length() < 2 || !args[0]->isFunction() || !args[1]->isString() ) {
					Js_Throw(
						"@method open(cb,path,flag?)\n"
						"@param cb:Function\n"
						"@param path:string\n"
						"@param flag?:FileOpenFlag\n"
					);
				}
			}

			String path = args[args_index++]->toString(worker)->value(worker);
			FileOpenFlag flag = FileOpenFlag::FOPEN_R;

			if ( args.length() > args_index && args[args_index]->isUint32() ) {
				uint32_t num = args[args_index++]->toUint32(worker)->value();
				flag = (FileOpenFlag)num;
			}

			if ( sync ) {
				int r;
				try {
					r = fs_open_sync(path, flag);
				} catch(cError& err) {
					Js_Throw(err);
				}
				Js_Return( r );
			} else {
				fs_open(path, flag, get_callback_for_int(worker, args[0]));
			}
		}

		static void close(FunctionArgs args, bool sync) {
			Js_Worker(args);

			int args_index = 0;
			if (sync) {
				if ( args.length() < 1 || !args[0]->isInt32() ) {
					Js_Throw(
						"@method closeSync(fd)\n"
						"@param fd:Int file handle\n"
						"@return {void}\n"
						);
				}
			} else {
				args_index++;
				if ( args.length() < 2 || !args[0]->isFunction() || !args[1]->isInt32() ) {
					Js_Throw(
						"@method close(cb,fd)\n"
						"@param cb:Function\n"
						"@param fd:Int file handle\n"
						);
				}
			}

			int fd = args[args_index]->toInt32(worker)->value();
			if ( sync ) {
				try {
					fs_close_sync(fd);
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				fs_close(fd, get_callback_for_none(worker, args[0]));
			}
		}

		static void read(FunctionArgs args, bool sync) {
			Js_Worker(args);
			
			uint32_t args_index = 0;
			if (sync) {
				if ( args.length() < 2 || !args[0]->isInt32() || !args[1]->isUint8Array() ) {
					Js_Throw(
						"@method readSync(fd,buffer,size?,offsetFd?)\n"
						"@param fd:Int file handle\n"
						"@param buffer:Buffer output buffer\n"
						"@param ssize?:Int **default** is -1\n"
						"@param offsetFd?:Int **default** is -1\n"
						"@return {Int} return read data length\n"
					);
				}
			} else {
				args_index++;
				if ( args.length() < 3 || !args[0]->isFunction() || !args[1]->isInt32() || !args[2]->isUint8Array() ) {
					Js_Throw(
						"@method read(cb,fd,buffer[,size[,offsetFd]])\n"
						"@param cb:Function\n"
						"@param fd:Int file handle\n"
						"@param buffer:Buffer output buffer\n"
						"@param size?:Int **default** is -1\n"
						"@param offsetFd?:Int **default** is -1\n"
					);
				}
			}
			
			int fd = args[args_index++]->toInt32(worker)->value();
			WeakBuffer weakBuffer;
			Qk_ASSERT_EQ(true, args[args_index++]->asBuffer(worker).to(weakBuffer));
			char* data = const_cast<char*>(*weakBuffer);
			uint32_t size = weakBuffer.length();
			int64_t offset = -1;

			// size
			if ( args.length() > args_index && args[args_index]->isInt32() ) {
				int num = args[args_index++]->toInt32(worker)->value();
				if ( num >= 0 ) {
					size = Qk_Min(size, num);
				}
			}

			// offset
			if ( args.length() > args_index && args[args_index]->isInt32() ) {
				offset = args[args_index++]->toInt32(worker)->value();
				if ( offset < 0 ) offset = -1;
			}
			
			if ( sync ) {
				int r;
				try {
					r = fs_read_sync(fd, data, size, offset);
				} catch(cError& err) {
					Js_Throw(err);
				}
				Js_Return( r );
			} else {
				// keep war buffer Persistent javascript object
				struct PersistentValue {
					Persistent<JSValue> args[2];
				} *pv = new PersistentValue;

				auto cbInt = get_callback_for_int(worker, args[0]);

				fs_read(fd, Buffer(data, size), offset, Callback<Buffer>([pv, cbInt](auto& e) {
					Qk_ASSERT(e.data);
					Sp<PersistentValue> h(pv);
					auto len = e.data->length();
					// collapse这个buffer因为这是ArrayBuffer所持有的内存空间,不能在这里被释放
					e.data->collapse();
					if (e.error) {
						cbInt->reject(e.error);
					} else {
						Int32 i(len);
						cbInt->resolve(&i);
					}
				}));
			}
		}

		static void write(FunctionArgs args, bool sync) {
			Js_Worker(args);

			int args_index = 0;
			if (sync) {
				if (args.length() < 2 || !args[0]->isInt32() ||
					!(args[1]->isString() || args[1]->isBuffer())
				) {
					Js_Throw(
						"@method writeSync(fd,buffer,size?,offsetFd?)\n"
						"@method writeSync(fd,string,encoding?,offsetFd?)\n"
						"@param fd:Int file handle\n"
						"@param buffer:Uint8Array|ArrayBuffer write buffer\n"
						"@param string:string write string\n"
						"@param size?:Int **default** is -1\n"
						"@param offsetFd?:Int **default** is -1\n"
						"@param encoding?:string **default** is 'utf8'\n"
						"@return {Int} return write data length\n"
					);
				}
			} else {
				args_index++;
				if (args.length() < 3 || !args[0]->isFunction() || !args[1]->isInt32() ||
					!(args[2]->isString() || args[2]->isBuffer() )
				) {
					Js_Throw(
						"@method write(cb,fd,buffer,size?,offsetFd?)\n"
						"@method write(cb,fd,string,encoding?,offsetFd?)\n"
						"@param cb:Function\n"
						"@param fd:Int file handle\n"
						"@param buffer:Uint8Array|ArrayBuffer write buffer\n"
						"@param string:string write string\n"
						"@param size?:Int} write size, **default** is `-1` use buffer.length\n"
						"@param offsetFd?:Int **default** is -1\n"
						"@param encoding?:string\n **default** is 'utf8'\n"
					);
				}
			}

			int fd = args[args_index++]->toInt32(worker)->value();
			Buffer holder;
			int64_t size;
			int64_t offset = -1;
			Callback<Buffer> cb;

			auto data = get_file_write_params(args, sync, args_index, holder, size, cb);
			if (!data)
				return;

			if (args.length() > args_index && args[args_index]->isInt32()) { // offset
				offset = args[args_index++]->toInt32(worker)->value();
				if ( offset < 0 )
					offset = -1;
			}

			if ( sync ) {
				int r;
				Js_Try_Catch({
					r = fs_write_sync(fd, data, size, offset);
				}, Error);
				Js_Return( r );
			} else {
				fs_write(fd, holder, offset, cb);
			}
		}

		static void binding(JSObject* exports, Worker* worker) {
			MixFileStat::binding(exports, worker);

			Js_Property(defaultMode, fs_default_mode);
			// sync
			Js_Method(chmodSync, { chmod(args, 1); });
			Js_Method(chownSync, { chown(args, 1); });
			Js_Method(mkdirSync, { mkdir(args, 1); });
			Js_Method(renameSync, { rename(args, 1); });
			Js_Method(linkSync, { link(args, 1); });
			Js_Method(unlinkSync, { unlink(args, 1); });
			Js_Method(rmdirSync, { rmdir(args, 1); });
			Js_Method(readdirSync, { readdir(args, 1); });
			Js_Method(statSync, { stat(args, 1); });
			Js_Method(existsSync, { exists(args, 1); });
			Js_Method(isFileSync, { is_file(args, 1); });
			Js_Method(isDirectorySync, { is_directory(args, 1); });
			Js_Method(readableSync, { readable(args, 1); });
			Js_Method(writableSync, { writable(args, 1); });
			Js_Method(executableSync, { executable(args, 1); });
			Js_Method(copySync, { copy(args, 1); });
			Js_Method(chmodRecursionSync, { chmod_recursion(args, 1); });
			Js_Method(chownRecursionSync, { chown_recursion(args, 1); });
			Js_Method(mkdirsSync, { mkdirs(args, 1); });
			Js_Method(removeRecursionSync, { remove_recursion(args, 1); });
			Js_Method(copyRecursionSync, { copy_recursion(args, 1); });
			Js_Method(writeFileSync, { write_file(args, 1); });
			Js_Method(readFileSync, { read_file(args, 1); });
			Js_Method(openSync, { open(args, 1); });
			Js_Method(closeSync, { close(args, 1); });
			Js_Method(readSync, { read(args, 1); });
			Js_Method(writeSync, { write(args, 1); });
			// async
			Js_Method(chmod, { chmod(args, 0); });
			Js_Method(chown, { chown(args, 0); });
			Js_Method(mkdir, { mkdir(args, 0); });
			Js_Method(mkdirs, { mkdirs(args, 0); });
			Js_Method(rename, { rename(args, 0); });
			Js_Method(link, { link(args, 0); });
			Js_Method(unlink, { unlink(args, 0); });
			Js_Method(rmdir, { rmdir(args, 0); });
			Js_Method(readdir, { readdir(args, 0); });
			Js_Method(stat, { stat(args, 0); });
			Js_Method(exists, { exists(args, 0); });
			Js_Method(isFile, { is_file(args, 0); });
			Js_Method(isDirectory, { is_directory(args, 0); });
			Js_Method(readable, { readable(args, 0); });
			Js_Method(writable, { writable(args, 0); });
			Js_Method(executable, { executable(args, 0); });
			Js_Method(copy, { copy(args, 0); });
			Js_Method(chmodRecursion, { chmod_recursion(args, 0); });
			Js_Method(chownRecursion, { chown_recursion(args, 0); });
			Js_Method(removeRecursion, { remove_recursion(args, 0); });
			Js_Method(copyRecursion, { copy_recursion(args, 0); });
			Js_Method(writeFile, { write_file(args, 0); });
			Js_Method(readFile, { read_file(args, 0); });
			Js_Method(open, { open(args, 0); });
			Js_Method(close, { close(args, 0); });
			Js_Method(read, { read(args, 0); });
			Js_Method(write, { write(args, 0); });

			Js_Method(readStream, {
				if (args.length() < 2 || !args[0]->isFunction() || !args[1]->isString()) {
					Js_Throw(
						"@method readStream(cb,path)\n"
						"@param cb:Function\n"
						"@param path:string\n"
						"@return {Uint} return abort id\n"
					);
				}
				auto cb = get_callback_for_io_stream(worker, args[0]);
				String path = args[1]->toString(worker)->value(worker);
				Js_Return( fs_read_stream(path, cb) );
			});

			Js_Method(abort, {
				if (args.length() < 1 || ! args[0]->isUint32()) {
					Js_Throw(
						"@method abort(id) abort async io\n"
						"@param id:Uint\n"
					);
				}
				fs_abort( args[0]->toUint32(worker)->value() );
			});

			// ------------------------------------------------------------------------
			// NativeFileReader
			auto reader = worker->newObject();
			Js_Property(reader, reader);
			NativeFileReader::binding(reader, worker);

			// ------------------------------------------------------------------------
			// fs path
			Js_Method(executable, {
				Js_Return( fs_executable() );
			});
			Js_Method(documents, {
				if (args.length() == 0 || !args[0]->isString()) {
					Js_Return( fs_documents() );
				}
				Js_Return( fs_documents( args[0]->toString(worker)->value(worker)) );
			});
			Js_Method(temp, {
				if (args.length() == 0 || !args[0]->isString()) {
					Js_Return( fs_temp() );
				}
				Js_Return( fs_temp( args[0]->toString(worker)->value(worker)) );
			});
			Js_Method(resources, {
				if (args.length() == 0 || !args[0]->isString()) {
					Js_Return( fs_resources() );
				}
				Js_Return( fs_resources( args[0]->toString(worker)->value(worker)) );
			});
			Js_Method(cwd, {
				Js_Return( fs_cwd() );
			});
			Js_Method(chdir, {
				if (args.length() == 0 || !args[0]->isString()) {
					Js_ReturnBool( false );
				}
				Js_ReturnBool( fs_chdir(args[0]->toString(worker)->value(worker)) );
			});
			Js_Method(extname, {
				if (args.length() == 0 || !args[0]->isString())
					Js_Throw( "Bad argument." );
				Js_Return( fs_extname( args[0]->toString(worker)->value(worker)) );
			});
			Js_Method(dirname, {
				if (args.length() == 0 || !args[0]->isString())
					Js_Throw( "Bad argument." );
				Js_Return( fs_dirname( args[0]->toString(worker)->value(worker)) );
			});
			Js_Method(basename, {
				if (args.length() == 0 || !args[0]->isString())
					Js_Throw( "Bad argument." );
				Js_Return( fs_basename( args[0]->toString(worker)->value(worker)) );
			});
		}
	};

	Js_Module(_fs, NativeFs);
} }
