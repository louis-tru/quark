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

#include "../../util/fs.h"
#include "../../util/http.h"
#include "../js.h"
#include "../str.h"
#include "./_cb.h"
#include "./_fs.h"

/**
 * @ns qk::js
 */

JS_BEGIN

/**
 * @class WrapFileStat
 */
class WrapFileStat: public WrapObject {
	public:

	/**
	 * @constructor([path])
	 * @arg [path] {String}
	 */
	static void constructor(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || ! args[0]->IsString(worker)) {
			New<WrapFileStat>(args, new FileStat());
		} else {
			New<WrapFileStat>(args, new FileStat(args[0]->ToStringValue(worker)));
		}
	}

	/**
	 * @func is_valid()
	 * @ret {bool}
	 */
	static void is_valid(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->is_valid() );
	}

	/**
	 * @func is_file()
	 * @ret {bool}
	 */
	static void is_file(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->is_file() );
	}

	/**
	 * @func is_dir()
	 * @ret {bool}
	 */
	static void is_dir(FunctionCall args)  {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->is_dir() );
	}

	/**
	 * @func is_link()
	 * @ret {bool}
	 */
	static void is_link(FunctionCall args)  {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->is_link() );
	}

	/**
	 * @func is_sock()
	 * @ret {bool}
	 */
	static void is_sock(FunctionCall args)  {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->is_sock() );
	}

	/**
	 * @func mode()
	 * @ret {uint64}
	 */
	static void mode(FunctionCall args)  {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->mode() );
	}

	/**
	 * @func type()
	 * @ret {DirentType}
	 */
	static void type(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->type() );
	}
	
	/**
	 * @func group()
	 * @ret {uint64}
	 */
	static void group(FunctionCall args)  {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->group() );
	}

	/**
	 * @func owner()
	 * @ret {uint64}
	 */
	static void owner(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->owner() );
	}

	/**
	 * @func size()
	 * @ret {uint64}
	 */
	static void size(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->size() );
	}

	/**
	 * @func nlink()
	 * @ret {uint64}
	 */
	static void nlink(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->nlink() );
	}

	/**
	 * @func ino()
	 * @ret {uint64}
	 */
	static void ino(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->ino() );
	}

	/**
	 * @func blksize()
	 * @ret {uint64}
	 */
	static void blksize(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->blksize() );
	}

	/**
	 * @func blocks()
	 * @ret {uint64}
	 */
	static void blocks(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->blocks() );
	}

	/**
	 * @func flags()
	 * @ret {uint64}
	 */
	static void flags(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->flags() );
	}

	/**
	 * @func gen()
	 * @ret {uint64}
	 */
	static void gen(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->gen() );
	}

	/**
	 * @func dev()
	 * @ret {uint64}
	 */
	static void dev(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->dev() );
	}

	/**
	 * @func rdev()
	 * @ret {uint64}
	 */
	static void rdev(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->rdev() );
	}

	/**
	 * @func atime()
	 * @ret {uint64}
	 */
	static void atime(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->atime() / 1000 );
	}

	/**
	 * @func mtime()
	 * @ret {uint64}
	 */
	static void mtime(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->mtime() / 1000 );
	}

	/**
	 * @func ctime()
	 * @ret {uint64}
	 */
	static void ctime(FunctionCall args)  {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->ctime() / 1000 );
	}

	/**
	 * @func birthtime()
	 * @ret {uint64}
	 */
	static void birthtime(FunctionCall args)  {
		JS_WORKER(args);
		JS_SELF(FileStat);
		JS_RETURN( self->birthtime() / 1000 );
	}

	static void binding(Local<JSObject> exports, Worker* worker) {
		
		JS_DEFINE_CLASS(FileStat, constructor, {
			JS_SET_CLASS_METHOD(isValid, is_valid);
			JS_SET_CLASS_METHOD(isFile, is_file);
			JS_SET_CLASS_METHOD(isDir, is_dir);
			JS_SET_CLASS_METHOD(isDirectory, is_dir);
			JS_SET_CLASS_METHOD(isLink, is_link);
			JS_SET_CLASS_METHOD(isSock, is_sock);
			JS_SET_CLASS_METHOD(mode, mode);
			JS_SET_CLASS_METHOD(type, type);
			JS_SET_CLASS_METHOD(group, group);
			JS_SET_CLASS_METHOD(owner, owner);
			JS_SET_CLASS_METHOD(size, size);
			JS_SET_CLASS_METHOD(nlink, nlink);
			JS_SET_CLASS_METHOD(ino, ino);
			JS_SET_CLASS_METHOD(blksize, blksize);
			JS_SET_CLASS_METHOD(blocks, blocks);
			JS_SET_CLASS_METHOD(flags, flags);
			JS_SET_CLASS_METHOD(gen, gen);
			JS_SET_CLASS_METHOD(dev, dev);
			JS_SET_CLASS_METHOD(rdev, rdev);
			JS_SET_CLASS_METHOD(atime, atime);
			JS_SET_CLASS_METHOD(mtime, mtime);
			JS_SET_CLASS_METHOD(ctime, ctime);
			JS_SET_CLASS_METHOD(birthtime, birthtime);
		}, NULL);
	}
};

bool parse_encoding(FunctionCall args, const Local<JSValue>& arg, Encoding& en) {
	JS_WORKER(args);
	String s = arg->ToStringValue(worker);
	en = Coder::parse_encoding( s );
	if ( en == Encoding::unknown ) {
		worker->throwError( 
			"Unknown encoding \"%s\", the optional value is "
			"[binary|ascii|base64|hex|utf8|ucs2|utf16|utf32]", *s );
		return false;
	}
	return true;
}

static bool parse_file_write_params(FunctionCall args, bool sync, int& args_index,
																		Buffer& buffer, void*& data, int64& size, bool& afterCollapse)
{
	JS_WORKER(args);

	size = 0;
	
	if ( args[args_index]->IsString(worker) ) { // 写入字符串
		int index_str = args_index;
		args_index++;
		
		Encoding en = Encoding::utf8;
		if ( args.Length() > args_index && args[args_index]->IsString(worker) ) { // 第三个参数为编码格式
			if ( ! parse_encoding(args, args[args_index], en) ) {
				return false;
			}
			args_index++;
		}
		buffer = args[index_str]->ToBuffer(worker, en);
		size = buffer.length();
		data = buffer.value();
	}
	else { // ArrayBuffer or TypedArray

		auto wb = args[args_index]->AsBuffer(worker);

		args_index++;

		if (!sync) {
			// 这是一个危险的操作,一定要确保buffer不能被释放否则会导致致命错误
			buffer = Buffer(*wb, wb.length());
			// buffer = wb.copy(); // copy data
			afterCollapse = true;
		}
		data = *wb;
		size = wb.length();
		
		if ( args.Length() > args_index && args[args_index]->IsInt32(worker) ) { // size
			int num = args[args_index]->ToInt32Value(worker);
			if ( num >= 0 ) {
				size = Qk_MIN( num, size );
				if (!sync) {
					buffer.realloc((uint)size);
				}
			}
			args_index++;
		}
	}
	
	return true;
}

/**
 * @class NativeFileHelper
 */
class NativeFileHelper {
 public:

	/**
	 * @func chmodSync(path[,mode])
	 * @arg path {String}
	 * @arg [mode=default_mode] {uint}
	 * @ret {void}
	 */

	/**
	 * @func chmod(path[,mode[,cb]][,cb])
	 * @arg path {String}
	 * @arg [mode=default_mode] {uint}
	 * @arg [cb] {Function}
	 */
	static void chmod(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func chmodSync(path[,mode])\n"
					"* @arg path {String}\n"
					"* @arg [mode=default_mode] {uint}\n"
					"* @ret {void}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func chmod(path[,mode[,cb]][,cb])\n"
					"* @arg path {String}\n"
					"* @arg [mode=default_mode] {uint}\n"
					"* @arg [cb] {Function}\n"
				);
			}
		}
		int args_index = 1;
		uint32_t mode = FileHelper::default_mode;
		if (args.Length() > 1 && args[1]->IsUint32(worker)) {
			mode = args[1]->ToUint32Value(worker);
			args_index++;
		}
		if ( sync ) {
			try {
				FileHelper::chmod_sync(args[0]->ToStringValue(worker), mode);
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
		} else {
			Cb cb;
			if ( args.Length() > args_index ) {
				cb = get_callback_for_none(worker, args[args_index]);
			}
			FileHelper::chmod(args[0]->ToStringValue(worker), mode, cb);
		}
	}

	
	/**
	 * @func chmod_r_sync(path[,mode])
	 * @arg path {String}
	 * @arg [mode=default_mode] {uint}
	 * @ret {void}
	 */

	/**
	 * @func chmod_r(path[,mode[,cb]][,cb])
	 * @arg path {String}
	 * @arg [mode=default_mode] {uint}
	 * @arg [cb] {Function}
	 * @ret {uint} return id
	 */
	static void chmod_r(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func chmodrSync(path[,mode])\n"
					"* @arg path {String}\n"
					"* @arg [mode=default_mode] {uint}\n"
					"* @ret {void}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func chmodr(path[,mode[,cb]][,cb])\n"
					"* @arg path {String}\n"
					"* @arg [mode=default_mode] {uint}\n"
					"* @arg [cb] {Function}\n"
					"* @ret {uint} return id\n"
				);
			}
		}
		int args_index = 1;
		uint32_t mode = FileHelper::default_mode;
		if (args.Length() > 1 && args[1]->IsUint32(worker)) {
			mode = args[1]->ToUint32Value(worker);
			args_index++;
		}
		if ( sync ) {
			try {
				FileHelper::chmod_r_sync(args[0]->ToStringValue(worker), mode);
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
		} else {
			Cb cb;
			if ( args.Length() > args_index ) {
				cb = get_callback_for_none(worker, args[args_index]);
			}
			JS_RETURN( FileHelper::chmod_r(args[0]->ToStringValue(worker), mode, cb) );
		}
	}

	/**
	 * @func chownSync(path, owner, group)
	 * @arg path {String}
	 * @arg owner {uint}
	 * @arg group {uint}
	 * @ret {void}
	 */

	/**
	 * @func chown(path, owner, group[,cb])
	 * @arg path {String}
	 * @arg owner {uint}
	 * @arg group {uint}
	 * @arg [cb] {Function}
	 */
	static void chown(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 3 ||
				!args[0]->IsString(worker) ||
				!args[1]->IsUint32(worker) || !args[2]->IsUint32(worker) ) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func chownSync(path, owner, group)\n"
					"* @arg path {String}\n"
					"* @arg owner {uint}\n"
					"* @arg group {uint}\n"
					"* @ret {void}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func chown(path, owner, group[,cb])\n"
					"* @arg path {String}\n"
					"* @arg owner {uint}\n"
					"* @arg group {uint}\n"
					"* @arg [cb] {Function}\n"
				);
			}
		}
		
		if ( sync ) {
			try {
				FileHelper::chown_sync(args[0]->ToStringValue(worker),
															args[1]->ToUint32Value(worker),
															args[2]->ToUint32Value(worker));
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
		} else {
			Cb cb;
			if ( args.Length() > 3 ) {
				cb = get_callback_for_none(worker, args[3]);
			}
			FileHelper::chown(args[0]->ToStringValue(worker),
												args[1]->ToUint32Value(worker),
												args[2]->ToUint32Value(worker), cb);
		}
	}
	
	/**
	 * @func chown_r_sync(path, owner, group)
	 * @arg path {String}
	 * @arg owner {uint}
	 * @arg group {uint}
	 * @ret {void}
	 */

	/**
	 * @func chown_r(path, owner, group[,cb])
	 * @arg path {String}
	 * @arg owner {uint}
	 * @arg group {uint}
	 * @arg [cb] {Function}
	 * @ret {uint} return id
	 */
	static void chown_r(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if ( args.Length() < 3 ||
				!args[0]->IsString(worker) ||
				!args[1]->IsUint32(worker) || !args[2]->IsUint32(worker) ) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func chownrSync(path, owner, group)\n"
					"* @arg path {String}\n"
					"* @arg owner {uint}\n"
					"* @arg group {uint}\n"
					"* @ret {void}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func chownr(path, owner, group[,cb])\n"
					"* @arg path {String}\n"
					"* @arg owner {uint}\n"
					"* @arg group {uint}\n"
					"* @arg [cb] {Function}\n"
					"* @ret {uint} return id\n"
				);
			}
		}
		
		if ( sync ) {
			try {
				FileHelper::chown_r_sync(args[0]->ToStringValue(worker),
																args[1]->ToUint32Value(worker),
																args[2]->ToUint32Value(worker));
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
		} else {
			Cb cb;
			if ( args.Length() > 3 ) {
				cb = get_callback_for_none(worker, args[3]);
			}
			JS_RETURN( FileHelper::chown_r(args[0]->ToStringValue(worker),
																			args[1]->ToUint32Value(worker),
																			args[2]->ToUint32Value(worker), cb) );
		}
	}
	
	/**
	 * @func mkdir_sync(path[,mode])
	 * @arg path {String}
	 * @arg [mode=default_mode] {uint}
	 * @ret {void}
	 */

	/**
	 * @func mkdir(path[,mode[,cb]][,cb])
	 * @arg path {String}
	 * @arg [mode=default_mode] {uint}
	 * @arg [cb] {Function}
	 */
	static void mkdir(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 1 || !args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func mkdirSync(path[,mode])\n"
					"* @arg path {String}\n"
					"* @arg [mode=default_mode] {uint}\n"
					"* @ret {void}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func mkdir(path[,mode[,cb]][,cb])\n"
					"* @arg path {String}\n"
					"* @arg [mode=default_mode] {uint}\n"
					"* @arg [cb] {Function}\n"
				);
			}
		}
		int args_index = 1;
		uint32_t mode = FileHelper::default_mode;
		if (args.Length() > 1 && args[1]->IsUint32(worker)) {
			mode = args[1]->ToUint32Value(worker);
			args_index++;
		}
		if ( sync ) {
			try {
				FileHelper::mkdir_sync(args[0]->ToStringValue(worker), mode);
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
		} else {
			Cb cb;
			if ( args.Length() > args_index ) {
				cb = get_callback_for_none(worker, args[args_index]);
			}
			FileHelper::mkdir(args[0]->ToStringValue(worker), mode, cb);
		}
	}

	/**
	 * @func mkdir_p_sync(path[,mode])
	 * @arg path {String}
	 * @arg [mode=default_mode] {uint}
	 * @ret {void}
	 */
	
	/**
	 * @func mkdir_p(path[,mode[,cb]][,cb])
	 * @arg path {String}
	 * @arg [mode=default_mode] {uint}
	 * @arg [cb] {Function}
	 * @ret {uint} return id
	 */
	static void mkdir_p(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			if ( sync ){
				JS_THROW_ERR(
					"* @func mkdirpSync(path[,mode])\n"
					"* @arg path {String}\n"
					"* @arg [mode=default_mode] {uint}\n"
					"* @ret {void}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func mkdirp(path[,mode[,cb]][,cb])\n"
					"* @arg path {String}\n"
					"* @arg [mode=default_mode] {uint}\n"
					"* @arg [cb] {Function}\n"
					"* @ret {uint} return id\n"
				);
			}
		}
		int args_index = 1;
		uint32_t mode = FileHelper::default_mode;
		if (args.Length() > 1 && args[1]->IsUint32(worker)) {
			mode = args[1]->ToUint32Value(worker);
			args_index++;
		}
		if ( sync ) {
			try {
				FileHelper::mkdir_p_sync(args[0]->ToStringValue(worker), mode);
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
		} else {
			Cb cb;
			if ( args.Length() > args_index ) {
				cb = get_callback_for_none(worker, args[args_index]);
			}
			FileHelper::mkdir_p(args[0]->ToStringValue(worker), mode, cb);
		}
	}

	/**
	 * @func rename_sync(name,new_name)
	 * @arg name {String}
	 * @arg new_name {String}
	 * @ret {void}
	 */

	/**
	 * @func rename(name,new_name[,cb])
	 * @arg name {String}
	 * @arg new_name {String}
	 * @arg [cb] {Function}
	 */  
	static void rename(FunctionCall args, bool sync) {
		JS_WORKER(args);

		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func renameSync(name,new_name)\n"
					"* @arg name {String}\n"
					"* @arg new_name {String}\n"
					"* @ret {void}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func rename(name,new_name[,cb])\n"
					"* @arg name {String}\n"
					"* @arg new_name {String}\n"
					"* @arg [cb] {Function}\n"
				);
			}
		}

		if ( sync ) {
			try {
				FileHelper::rename_sync(args[0]->ToStringValue(worker),
																args[1]->ToStringValue(worker));
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
		} else {
			Cb cb;
			if ( args.Length() > 2 ) {
				cb = get_callback_for_none(worker, args[2]);
			}
			FileHelper::rename(args[0]->ToStringValue(worker), args[1]->ToStringValue(worker), cb);
		}
	}

	/**
	 * @func link_sync(path)
	 * @arg path {String}
	 * @arg newPath {String}
	 * @ret {void}
	 */

	/**
	 * @func link(path[,cb])
	 * @arg path {String}
	 * @arg newPath {String}
	 * @arg [cb] {Function}
	 */
	static void link(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func linkSync(path,newPath)\n"
					"* @arg path {String}\n"
					"* @ret {void}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func link(path,newPath[,cb])\n"
					"* @arg path {String}\n"
					"* @arg [cb] {Function}\n"
				);
			}
		}
		if ( sync ) {
			try {
				FileHelper::link_sync(
					args[0]->ToStringValue(worker),
					args[1]->ToStringValue(worker)
				);
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
		} else {
			Cb cb;
			if ( args.Length() > 2 ) {
				cb = get_callback_for_none(worker, args[2]);
			}
			FileHelper::link(args[0]->ToStringValue(worker), args[1]->ToStringValue(worker), cb);
		}
	}

	/**
	 * @func unlink_sync(path)
	 * @arg path {String}
	 * @ret {void}
	 */

	/**
	 * @func unlink(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 */
	static void unlink(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 1 || !args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func unlinkSync(path)\n"
					"* @arg path {String}\n"
					"* @ret {void}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func unlink(path[,cb])\n"
					"* @arg path {String}\n"
					"* @arg [cb] {Function}\n"
				);
			}
		}
		if ( sync ) {
			try {
				FileHelper::unlink_sync(args[0]->ToStringValue(worker));
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
		} else {
			Cb cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_none(worker, args[1]);
			}
			FileHelper::unlink(args[0]->ToStringValue(worker), cb);
		}
	}

	/**
	 * @func rmdir_sync(path)
	 * @arg path {String}
	 * @ret {void}
	 */

	/**
	 * @func rmdir(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 */
	static void rmdir(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 1 || !args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func rmdirSync(path)\n"
					"* @arg path {String}\n"
					"* @ret {void}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func rmdir(path)\n"
					"* @arg path {String}\n"
					"* @arg [cb] {Function}\n"
				);
			}
		}
		if ( sync ) {
			try {
				FileHelper::rmdir_sync(args[0]->ToStringValue(worker));
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
		} else {
			Cb cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_none(worker, args[1]);
			}
			FileHelper::rmdir(args[0]->ToStringValue(worker), cb);
		}
	}
	
	/**
	 * @func rm_r_sync(path)
	 * @arg path {String}
	 * @ret {void}
	 */

	/**
	 * @func rm_r(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 * @ret {uint} return id
	 */
	static void remove_r(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 1 || !args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func removerSync(path)\n"
					"* @arg path {String}\n"
					"* @ret {void}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func remover(path)\n"
					"* @arg path {String}\n"
					"* @arg [cb] {Function}\n"
					"* @ret {uint} return id\n"
				);
			}
		}
		if ( sync ) {
			try {
				FileHelper::remove_r_sync(args[0]->ToStringValue(worker));
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
		} else {
			Cb cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_none(worker, args[1]);
			}
			JS_RETURN( FileHelper::remove_r(args[0]->ToStringValue(worker), cb) );
		}
	}
	
	/**
	 * @func cp_sync(path, target)
	 * @arg path {String}
	 * @arg target {String}
	 * @ret {void}
	 */
	
	/**
	 * @func cp(path, target)
	 * @arg path {String}
	 * @arg target {String}
	 * @arg [cb] {Function}
	 * @ret {uint} return id
	 */
	static void copy(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func copySync(path, target)\n"
					"* @arg path {String}\n"
					"* @arg target {String}\n"
					"* @ret {void}\n"
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
			try {
				FileHelper::copy_sync(args[0]->ToStringValue(worker),
															args[1]->ToStringValue(worker));
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
		} else {
			Cb cb;
			if ( args.Length() > 2 ) {
				cb = get_callback_for_none(worker, args[2]);
			}
			JS_RETURN( FileHelper::copy(args[0]->ToStringValue(worker),
																  args[1]->ToStringValue(worker), cb) );
		}
	}
	
	/**
	 * @func cp_r_sync(path, target)
	 * @arg path {String}
	 * @arg target {String}
	 * @ret {void}
	 */
	
	/**
	 * @func cp_r(path, target)
	 * @arg path {String}
	 * @arg target {String}
	 * @arg [cb] {Function}
	 * @ret {uint} return id
	 */
	static void copy_r(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 2 || !args[0]->IsString(worker) || !args[1]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func copyrSync(path, target)\n"
					"* @arg path {String}\n"
					"* @arg target {String}\n"
					"* @ret {void}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func copyr(path, target)\n"
					"* @arg path {String}\n"
					"* @arg target {String}\n"
					"* @arg [cb] {Function}\n"
					"* @ret {uint} return id\n"
				);
			}
		}
		if ( sync ) {
			try {
				FileHelper::copy_r_sync(args[0]->ToStringValue(worker),
																args[1]->ToStringValue(worker));
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
		} else {
			Cb cb;
			if ( args.Length() > 2 ) {
				cb = get_callback_for_none(worker, args[2]);
			}
			JS_RETURN( FileHelper::copy_r(args[0]->ToStringValue(worker),
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
	static void readdir(FunctionCall args, bool sync) {
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
			Array<Dirent> r;
			try {
				r = FileHelper::readdir_sync(args[0]->ToStringValue(worker));
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
			JS_RETURN( r );
		} else {
			Cb cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_array_dirent(worker, args[1]);
			}
			FileHelper::readdir( args[0]->ToStringValue(worker), cb);
		}
	}
	
	/**
	 * @func stat_sync(path)
	 * @arg path {String}
	 * @ret {FileStat}
	 */
	
	/**
	 * @func stat(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 */
	static void stat(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func statSync(path)\n"
					"* @arg path {String}\n"
					"* @ret {FileStat}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func stat(path[,cb])\n"
					"* @arg path {String}\n"
					"* @arg [cb] {Function}\n"
				);
			}
		}
		if ( sync ) {
			FileStat r;
			try {
				r = FileHelper::stat_sync( args[0]->ToStringValue(worker) );
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
			JS_RETURN( r );
		} else {
			Cb cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_file_stat(worker, args[1]);
			}
			FileHelper::stat(args[0]->ToStringValue(worker), cb);
		}
	}
	
	/**
	 * @func exists_sync(path)
	 * @arg path {String}
	 * @ret {bool}
	 */

	/**
	 * @func exists(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 */
	static void exists(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func existsSync(path)\n"
					"* @arg path {String}\n"
					"* @ret {bool}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func exists(path[,cb])\n"
					"* @arg path {String}\n"
					"* @arg [cb] {Function}\n"
				);
			}
		}
		if ( sync ) {
			JS_RETURN( FileHelper::exists_sync(args[0]->ToStringValue(worker)) );
		} else {
			Cb cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_bool(worker, args[1]);
			}
			FileHelper::exists(args[0]->ToStringValue(worker), cb);
		}
	}

	/**
	 * @func isFileSync(path)
	 * @arg path {String}
	 * @ret {bool}
	 */
	
	/**
	 * @func isFile(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 */
	static void is_file(FunctionCall args, bool sync) {
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
			Cb cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_bool(worker, args[1]);
			}
			FileHelper::is_file(args[0]->ToStringValue(worker), cb);
		}
	}
	
	/**
	 * @func isDirectorySync(path)
	 * @arg path {String}
	 * @ret {bool}
	 */
	
	/**
	 * @func isDirectory(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 */
	static void is_directory(FunctionCall args, bool sync) {
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
			Cb cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_bool(worker, args[1]);
			}
			FileHelper::is_directory(args[0]->ToStringValue(worker), cb);
		}
	}
	
	/**
	 * @func readable_sync(path)
	 * @arg path {String}
	 * @ret {bool}
	 */

	/**
	 * @func readable(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 */
	static void readable(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func readableSync(path)\n"
					"* @arg path {String}\n"
					"* @ret {bool}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func readable(path[,cb])\n"
					"* @arg path {String}\n"
					"* @arg [cb] {Function}\n"
				);
			}
		}
		if ( sync ) {
			JS_RETURN( FileHelper::readable_sync(args[0]->ToStringValue(worker)) );
		} else {
			Cb cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_bool(worker, args[1]);
			}
			FileHelper::readable(args[0]->ToStringValue(worker), cb);
		}
	}
	
	/**
	 * @func writable_sync(path)
	 * @arg path {String}
	 * @ret {bool}
	 */

	/**
	 * @func writable(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 */
	static void writable(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func writableSync(path)\n"
					"* @arg path {String}\n"
					"* @ret {bool}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func writable(path[,cb])\n"
					"* @arg path {String}\n"
					"* @arg [cb] {Function}\n"
				);
			}
		}
		if ( sync ) {
			JS_RETURN( FileHelper::writable_sync(args[0]->ToStringValue(worker)) );
		} else {
			Cb cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_bool(worker, args[1]);
			}
			FileHelper::writable(args[0]->ToStringValue(worker), cb);
		}
	}
	
	/**
	 * @func executable_sync(path)
	 * @arg path {String}
	 * @ret {bool}
	 */

	/**
	 * @func executable(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 */
	static void executable(FunctionCall args, bool sync) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			if ( sync ) {
				JS_THROW_ERR(
					"* @func executableSync(path)\n"
					"* @arg path {String}\n"
					"* @ret {bool}\n"
				);
			} else {
				JS_THROW_ERR(
					"* @func executable(path[,cb])\n"
					"* @arg path {String}\n"
					"* @arg [cb] {Function}\n"
				);
			}
		}
		if ( sync ) {
			JS_RETURN( FileHelper::executable_sync(args[0]->ToStringValue(worker)) );
		} else {
			Cb cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_bool(worker, args[1]);
			}
			FileHelper::executable(args[0]->ToStringValue(worker), cb);
		}
	}

	/**
	 * @func read_stream(cb,path)
	 * @arg cb {Function}
	 * @arg path {String}
	 * @ret {uint} return abort id
	 */
	static void read_stream(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 2 || !args[0]->IsFunction(worker) || !args[1]->IsString(worker)) {
			JS_THROW_ERR(
										"* @func readStream(cb,path)\n"
										"* @arg cb {Function}\n"
										"* @arg path {String}\n"
										"* @ret {uint} return abort id\n"
										);
		}
		Cb cb = get_callback_for_io_stream(worker, args[0]);
		String path = args[1]->ToStringValue(worker);
		JS_RETURN( FileHelper::read_stream(path, cb) );
	}

	/**
	 * @func read_file_sync(path[,encoding])
	 * @arg path {String}
	 * @ret {Buffer} return file buffer
	 */
	/**
	 * @func read_file(cb,path[,encoding])
	 * @arg cb {Function}
	 * @arg path {String}
	 */
	static void read_file(FunctionCall args, bool sync) {
		JS_WORKER(args);

		int args_index = 0;
		if (sync) {
			if (args.Length() < 1 || !args[0]->IsString(worker)) {
				JS_THROW_ERR(
											"* @func readFileSync(path[,encoding])\n"
											"* @arg path {String}\n"
											"* @arg [encoding] {String}\n"
											"* @ret {Buffer} return file buffer\n"
											);
			}
		} else {
			args_index++;
			if (args.Length() < 2 || !args[0]->IsFunction(worker) || !args[1]->IsString(worker)) {
				JS_THROW_ERR(
											"* @func readFile(cb,path[,encoding])\n"
											"* @arg cb {Function}\n"
											"* @arg path {String}\n"
											"* @arg [encoding] {String}\n"
											);
			}
		}

		String path = args[args_index++]->ToStringValue(worker);
		Encoding encoding = Encoding::unknown;

		if (args.Length() > args_index && args[args_index]->IsString(worker)) { //
			if ( !parse_encoding(args, args[args_index], encoding) ) return;
			args_index++;
		}

		if ( sync ) {
			Buffer r;
			try {
				r = FileHelper::read_file_sync(path, -1);
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
			JS_RETURN( convert_buffer(worker, r, encoding) );
		} else {
			// Qk_LOG("read_file,args,%d", args.Length());
			Cb cb = get_callback_for_buffer(worker, args[0], encoding);
			FileHelper::read_file(path, cb);
		}
	}
	
	/**
	 * @func write_file_sync(path,buffer[,size])
	 * @func write_file_sync(path,string[,encoding])
	 * @arg path {String}
	 * @arg string {String}
	 * @arg buffer {Uint8Array|ArrayBuffer}
	 * @arg [size] {int}
	 * @arg [encoding='utf8'] {String}
	 * @ret {bool}
	 */
	/**
	 * @func write_file(cb,path,buffer)
	 * @func write_file(cb,path,string[,encoding])
	 * @arg cb {Function}
	 * @arg path {String}
	 * @arg string {String}
	 * @arg buffer {Uint8Array|ArrayBuffer}
	 * @arg [size] {int}
	 * @arg [encoding=utf8] {Encoding}
	 */
	static void write_file(FunctionCall args, bool sync) {
		JS_WORKER(args);

		int args_index = 0;
		if ( sync ) {
			if ( args.Length() < 2 || !args[0]->IsString(worker) ||
					!(args[1]->IsString(worker) || args[1]->IsBuffer())
			) { // 参数错误
				JS_THROW_ERR(
											"* @func writeFileSync(path,buffer[,size])\n"
											"* @func writeFileSync(path,string[,encoding])\n"
											"* @arg path {String}\n"
											"* @arg string {String}\n"
											"* @arg buffer {Uint8Array|ArrayBuffer}\n"
											"* @arg [size] {int}\n"
											"* @arg [encoding=utf8] {Encoding}\n"
											"* @ret {int}\n"
											);
			}
		} else {
			args_index++;
			if ( args.Length() < 3 || !args[0]->IsFunction(worker) || !args[1]->IsString(worker) ||
					!(args[2]->IsString(worker) || args[2]->IsBuffer())
				) {
				JS_THROW_ERR(
											"* @func writeFile(cb,path,buffer[,size])\n"
											"* @func writeFile(cb,path,string[,encoding])\n"
											"* @arg cb {Function}\n"
											"* @arg path {String}\n"
											"* @arg string {String}\n"
											"* @arg buffer {Uint8Array|ArrayBuffer}\n"
											"* @arg [size] {int}\n"
											"* @arg [encoding=utf8] {Encoding}\n"
											);
			}
		}
		
		String path = args[args_index++]->ToStringValue(worker);
		Buffer buffer;
		void* data;
		int64_t size;
		bool afterCollapse = false;

		if (!parse_file_write_params(args, sync, args_index, buffer, data, size, afterCollapse))
			return;
		
		if ( sync ) {
			int r;
			try {
				r = FileHelper::write_file_sync(path, data, size);
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
			JS_RETURN( r );
		} else {
			Cb cb = get_callback_for_int(worker, args[0]);
			
			// keep raw buffer Persistent javascript value
			CopyablePersistentValue persistent(worker, args[2]);
			
			FileHelper::write_file(path, buffer, Cb([persistent, afterCollapse, cb, size](Cb::Data& ev) {
				Qk_ASSERT( ev.data );
				if (afterCollapse) {
					// collapse这个buffer因为这是ArrayBuffer所持有的内存空间,绝不能在这里被释放
					static_cast<Buffer*>(ev.data)->collapse();
				}
				Int i(size);
				Cb::Data r = { nullptr, &i, 0 };
				cb->call(r);
			}));
		}
	}
	
	/**
	 * @func open_sync(path[,mode])
	 * @arg path {String}
	 * @arg [mode=FOPEN_R] {FileOpenMode}
	 * @ret {int} return file handle `success >= 0`
	 */
	/**
	 * @func open(cb,path[,mode])
	 * @arg cb {Function}
	 * @arg path {String}
	 * @arg [mode=FOPEN_R] {FileOpenMode}
	 */
	static void open(FunctionCall args, bool sync) {
		JS_WORKER(args);

		int args_index = 0;
		if (sync) {
			if ( args.Length() < 1 || !args[0]->IsString(worker) ) {
				JS_THROW_ERR(
											"* @func openSync(path[,mode])\n"
											"* @arg path {String}\n"
											"* @arg [mode=FOPEN_R] {FileOpenMode}\n"
											"* @ret {int} return file handle `success >= 0`\n"
											);
			} 
		} else {
			args_index++;
			if ( args.Length() < 2 || !args[0]->IsFunction(worker) || !args[1]->IsString(worker) ) {
				JS_THROW_ERR(
											"* @func open(cb,path[,mode])\n"
											"* @arg cb {Function}\n"
											"* @arg path {String}\n"
											"* @arg [mode=FOPEN_R] {FileOpenMode}\n"
											);
			}
		}

		String path = args[args_index++]->ToStringValue(worker);
		FileOpenFlag flag = FileOpenFlag::FOPEN_R;

		if ( args.Length() > args_index && args[args_index]->IsUint32(worker) ) {
			uint32_t num = args[1]->ToUint32Value(worker);
			flag = (FileOpenFlag)num;
			args_index++;
		}

		if ( sync ) {
			int r;
			try {
				r = FileHelper::open_sync(path, flag);
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
			JS_RETURN( r );
		} else {
			Cb cb = get_callback_for_int(worker, args[0]);
			FileHelper::open(path, flag, cb);
		}
	}
	
	/**
	 * @func close_sync(fd)
	 * @arg path {int} file handle
	 * @ret {void}
	 */
	/**
	 * @func close(cb,fd)
	 * @arg fd {int} file handle
	 * @arg [cb] {Function}
	 */
	static void close(FunctionCall args, bool sync) {
		JS_WORKER(args);
		
		int args_index = 0;
		if (sync) {
			if ( args.Length() < 1 || !args[0]->IsInt32(worker) ) {
				JS_THROW_ERR(
										"* @func closeSync(fd)\n"
										"* @arg path {int} file handle\n"
										"* @ret {void}\n"
										);
			}
		} else {
			args_index++;
			if ( args.Length() < 2 || !args[0]->IsFunction(worker) || !args[1]->IsInt32(worker) ) {
				JS_THROW_ERR(
											"* @func close(cb,fd)\n"
											"* @arg cb {Function}\n"
											"* @arg fd {int} file handle\n"
											);
			}
		}
		
		int fd = args[args_index]->ToInt32Value(worker);
		if ( sync ) {
			try {
				FileHelper::close_sync(fd);
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
		} else {
			Cb cb = get_callback_for_none(worker, args[0]);
			FileHelper::close(fd, cb);
		}
	}

	
	/**
	 * @func read_sync(fd,buffer[,size[,offset]])
	 * @arg fd {int} file handle
	 * @arg buffer {Buffer} output buffer
	 * @arg [size=-1] {int}
	 * @arg [offset=-1] {int}
	 * @ret {int} return read data length
	 */
	/**
	 * @func read(cb,fd,buffer[,size[,offset]])
	 * @arg cb {Function}
	 * @arg fd {int} file handle
	 * @arg buffer {Buffer} output buffer
	 * @arg [size=-1] {int}
	 * @arg [offset=-1] {int}
	 */
	static void read(FunctionCall args, bool sync) {
		JS_WORKER(args);
		
		uint32_t args_index = 0;
		if (sync) {
			if ( args.Length() < 2 || !args[0]->IsInt32(worker) || !args[1]->IsUint8Array() ) {
				JS_THROW_ERR(
											"* @func readSync(fd,buffer[,size[,offset]])\n"
											"* @arg fd {int} file handle\n"
											"* @arg buffer {Buffer} output buffer\n"
											"* @arg [size=-1] {int}\n"
											"* @arg [offset=-1] {int}\n"
											"* @ret {int} return read data length\n"
											);
			}
		} else {
			args_index++;
			if ( args.Length() < 3 || !args[0]->IsFunction(worker) || !args[1]->IsInt32(worker) || !args[2]->IsUint8Array() ) {
				JS_THROW_ERR(
											"* @func read(cb,fd,buffer[,size[,offset]])\n"
											"* @arg cb {Function}\n"
											"* @arg fd {int} file handle\n"
											"* @arg buffer {Buffer} output buffer\n"
											"* @arg [size=-1] {int}\n"
											"* @arg [offset=-1] {int}\n"
											);
			}
		}

		int fd = args[args_index++]->ToInt32Value(worker);
		auto raw_buf = args[args_index++]->AsBuffer(worker);
		uint32_t size = raw_buf.length();
		int64_t offset = -1;

		// size
		if ( args.Length() > args_index && args[args_index]->IsInt32(worker) ) {
			int num = args[args_index]->ToInt32Value(worker);
			if ( num >= 0 ) {
				size = Qk_MIN(size, num);
			}
			args_index++;
		}

		// offset
		if ( args.Length() > args_index && args[args_index]->IsInt32(worker) ) {
			offset = args[args_index]->ToInt32Value(worker);
			if ( offset < 0 ) offset = -1;
			args_index++;
		}
		
		if ( sync ) {
			int r;
			try {
				r = FileHelper::read_sync(fd, *raw_buf, size, offset);
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
			JS_RETURN( r );
		} else {
			Cb cb = get_callback_for_int(worker, args[0]);
			
			// keep war buffer Persistent javascript object
			CopyablePersistentValue persistent(worker, args[2]);
			
			FileHelper::read(fd, WeakBuffer(*raw_buf, size), offset,
											 Cb([persistent, cb](Cb::Data& ev) {
				Qk_ASSERT( ev.data );
				Int read_len(static_cast<Buffer*>(ev.data)->length());
				Cb::Data r = { nullptr, &read_len, 0 };
				cb->call(r);
			}));
		}
	}
	
	/**
	 * @func write_sync(fd,buffer[,size[,offset]])
	 * @func write_sync(fd,string[,encoding[,offset]])
	 * @arg fd {int} file handle
	 * @arg buffer {Uint8Array|ArrayBuffer} write buffer
	 * @arg string {String} write string
	 * @arg [size=-1] {int} read size, `-1` use buffer.length
	 * @arg [offset=-1] {int}
	 * @arg [encoding='utf8'] {String}
	 * @ret {int} return write data length
	 */
	/**
	 * @func write(cb,fd,buffer[,size[,offset]])
	 * @func write(cb,fd,string[,encoding[,offset]])
	 * @arg cb {Function}
	 * @arg fd {int} file handle
	 * @arg buffer {Uint8Array|ArrayBuffer} write buffer
	 * @arg string {String} write string
	 * @arg [size=-1] {int} read size, `-1` use buffer.length
	 * @arg [offset=-1] {int}
	 * @arg [encoding='utf8'] {String}
	 */
	static void write(FunctionCall args, bool sync) {
		JS_WORKER(args);

		int args_index = 0;
		if (sync) {
			if (args.Length() < 2 || !args[0]->IsInt32(worker) ||
				!(args[1]->IsString(worker) || args[1]->IsBuffer(worker))
			) { // 参数错误
				JS_THROW_ERR(
											"* @func writeSync(fd,buffer[,size[,offset]])\n"
											"* @func writeSync(fd,string[,encoding[,offset]])\n"
											"* @arg fd {int} file handle\n"
											"* @arg buffer {Uint8Array|ArrayBuffer} write buffer\n"
											"* @arg string {String} write string\n"
											"* @arg [size=-1] {int} read size, `-1` use buffer.length\n"
											"* @arg [offset=-1] {int}\n"
											"* @arg [encoding='utf8'] {String}\n"
											"* @ret {int} return write data length\n"
											);
			}
		} else {
			args_index++;
			if (args.Length() < 3 || !args[0]->IsFunction(worker) || !args[1]->IsInt32(worker) ||
				!(args[2]->IsString(worker) || args[2]->IsBuffer(worker) )
			) {
				JS_THROW_ERR(
											"* @func write(cb,fd,buffer[,size[,offset]])\n"
											"* @func write(cb,fd,string[,encoding[,offset]])\n"
											"* @arg cb {Function}\n"
											"* @arg fd {int} file handle\n"
											"* @arg buffer {Uint8Array|ArrayBuffer} write buffer\n"
											"* @arg string {String} write string\n"
											"* @arg [size=-1] {int} read size, `-1` use buffer.length\n"
											"* @arg [offset=-1] {int}\n"
											"* @arg [encoding='utf8'] {String}\n"
											);
			}
		}
		
		int fd = args[args_index++]->ToInt32Value(worker);
		Buffer buffer;
		void*  data;
		int64_t size;
		int64_t offset = -1;
		bool afterCollapse = false;

		if (!parse_file_write_params(args, sync, args_index, buffer, data, size, afterCollapse))
			return;

		if (args.Length() > args_index && args[args_index]->IsInt32()) { // offset
			offset = args[args_index]->ToInt32Value(worker);
			if ( offset < 0 )
				offset = -1;
			args_index++;
		}
		
		if ( sync ) {
			int r;
			try {
				r = FileHelper::write_sync(fd, data, size, offset);
			} catch(cError& err) {
				JS_THROW_ERR(err);
			}
			JS_RETURN( r );
		} else {
			Cb cb = get_callback_for_int(worker, args[0]);

			// keep raw buffer Persistent javascript value
			CopyablePersistentValue persistent(worker, args[2]);

			FileHelper::write(fd, buffer, offset, Cb([persistent, afterCollapse, cb, size](Cb::Data& ev) {
				Qk_ASSERT( ev.data );
				if (afterCollapse) { // restore raw buffer
					// collapse这个buffer因为这是ArrayBuffer所持有的内存空间,绝不能在这里被释放
					static_cast<Buffer*>(ev.data)->collapse();
				}
				Int i(size);
				Cb::Data r = { nullptr, &i, 0 };
				cb->call(r);
			}));
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
	
	// sync
	static void chmod_sync(FunctionCall args) { chmod(args, 1); }
	static void chown_sync(FunctionCall args) { chown(args, 1); }
	static void mkdir_sync(FunctionCall args) { mkdir(args, 1); }
	static void rename_sync(FunctionCall args) { rename(args, 1); }
	static void link_sync(FunctionCall args) { link(args, 1); }
	static void unlink_sync(FunctionCall args) { unlink(args, 1); }
	static void rmdir_sync(FunctionCall args) { rmdir(args, 1); }
	static void readdir_sync(FunctionCall args) { readdir(args, 1); }
	static void stat_sync(FunctionCall args) { stat(args, 1); }
	static void exists_sync(FunctionCall args) { exists(args, 1); }
	static void is_file_sync(FunctionCall args) { is_file(args, 1); }
	static void is_directory_sync(FunctionCall args) { is_directory(args, 1); }
	static void readable_sync(FunctionCall args) { readable(args, 1); }
	static void writable_sync(FunctionCall args) { writable(args, 1); }
	static void executable_sync(FunctionCall args) { executable(args, 1); }
	static void copy_sync(FunctionCall args) { copy(args, 1); }
	static void chmod_r_sync(FunctionCall args) { chmod_r(args, 1); }
	static void chown_r_sync(FunctionCall args) { chown_r(args, 1); }
	static void mkdir_p_sync(FunctionCall args) { mkdir_p(args, 1); }
	static void remove_r_sync(FunctionCall args) { remove_r(args, 1); }
	static void copy_r_sync(FunctionCall args) { copy_r(args, 1); }
	static void write_file_sync(FunctionCall args) { write_file(args, 1); }
	static void read_file_sync(FunctionCall args) { read_file(args, 1); }
	static void open_sync(FunctionCall args) { open(args, 1); }
	static void close_sync(FunctionCall args) { close(args, 1); }
	static void read_sync(FunctionCall args) { read(args, 1); }
	static void write_sync(FunctionCall args) { write(args, 1); }
	// async
	static void chmod_async(FunctionCall args) { chmod(args, 0); }
	static void chown_async(FunctionCall args) { chown(args, 0); }
	static void mkdir_async(FunctionCall args) { mkdir(args, 0); }
	static void rename_async(FunctionCall args) { rename(args, 0); }
	static void link_async(FunctionCall args) { link(args, 0); }
	static void unlink_async(FunctionCall args) { unlink(args, 0); }
	static void rmdir_async(FunctionCall args) { rmdir(args, 0); }
	static void readdir_async(FunctionCall args) { readdir(args, 0); }
	static void stat_async(FunctionCall args) { stat(args, 0); }
	static void exists_async(FunctionCall args) { exists(args, 0); }
	static void is_file_async(FunctionCall args) { is_file(args, 0); }
	static void is_directory_async(FunctionCall args) { is_directory(args, 0); }
	static void readable_async(FunctionCall args) { readable(args, 0); }
	static void writable_async(FunctionCall args) { writable(args, 0); }
	static void executable_async(FunctionCall args) { executable(args, 0); }
	static void copy_async(FunctionCall args) { copy(args, 0); }
	static void chmod_r_async(FunctionCall args) { chmod_r(args, 0); }
	static void chown_r_async(FunctionCall args) { chown_r(args, 0); }
	static void mkdir_p_async(FunctionCall args) { mkdir_p(args, 0); }
	static void remove_r_async(FunctionCall args) { remove_r(args, 0); }
	static void copy_r_async(FunctionCall args) { copy_r(args, 0); }
	static void write_file_async(FunctionCall args) { write_file(args, 0); }
	static void read_file_async(FunctionCall args) { read_file(args, 0); }
	static void open_async(FunctionCall args) { open(args, 0); }
	static void close_async(FunctionCall args) { close(args, 0); }
	static void read_async(FunctionCall args) { read(args, 0); }
	static void write_async(FunctionCall args) { write(args, 0); }

	static void binding(Local<JSObject> exports, Worker* worker) {
		WrapFileStat::binding(exports, worker);

		JS_SET_PROPERTY(DEFAULT_MODE, FileHelper::default_mode);
		// api sync
		JS_SET_METHOD(chmodSync, chmod_sync);
		JS_SET_METHOD(chownSync, chown_sync);
		JS_SET_METHOD(mkdirSync, mkdir_sync);
		JS_SET_METHOD(renameSync, rename_sync);
		JS_SET_METHOD(linkSync, link_sync);
		JS_SET_METHOD(unlinkSync, unlink_sync);
		JS_SET_METHOD(rmdirSync, rmdir_sync);
		JS_SET_METHOD(readdirSync, readdir_sync);
		JS_SET_METHOD(statSync, stat_sync);
		JS_SET_METHOD(existsSync, exists_sync);
		JS_SET_METHOD(isFileSync, is_file_sync);
		JS_SET_METHOD(isDirectorySync, is_directory_sync);
		JS_SET_METHOD(readableSync, readable_sync);
		JS_SET_METHOD(writableSync, writable_sync);
		JS_SET_METHOD(executableSync, executable_sync);
		JS_SET_METHOD(copySync, copy_sync);
		JS_SET_METHOD(chmodrSync, chmod_r_sync);
		JS_SET_METHOD(chownrSync, chown_r_sync);
		JS_SET_METHOD(mkdirpSync, mkdir_p_sync);
		JS_SET_METHOD(removerSync, remove_r_sync);
		JS_SET_METHOD(copyrSync, copy_r_sync);
		// async
		JS_SET_METHOD(chmod, chmod_async);
		JS_SET_METHOD(chown, chown_async);
		JS_SET_METHOD(mkdir, mkdir_async);
		JS_SET_METHOD(rename, rename_async);
		JS_SET_METHOD(link, link_async);
		JS_SET_METHOD(unlink, unlink_async);
		JS_SET_METHOD(rmdir, rmdir_async);
		JS_SET_METHOD(readdir, readdir_async);
		JS_SET_METHOD(stat, stat_async);
		JS_SET_METHOD(exists, exists_async);
		JS_SET_METHOD(isFile, is_file_async);
		JS_SET_METHOD(isDirectory, is_directory_async);
		JS_SET_METHOD(readable, readable_async);
		JS_SET_METHOD(writable, writable_async);
		JS_SET_METHOD(executable, executable_async);
		JS_SET_METHOD(copy, copy_async);
		JS_SET_METHOD(chmodr, chmod_r_async);
		JS_SET_METHOD(chownr, chown_r_async);
		JS_SET_METHOD(mkdirp, mkdir_p_async);
		JS_SET_METHOD(remover, remove_r_async);
		JS_SET_METHOD(copyr, copy_r_async);
		JS_SET_METHOD(readStream, read_stream);
		JS_SET_METHOD(abort, abort);
		// read/write file sync
		JS_SET_METHOD(writeFileSync, write_file_sync);
		JS_SET_METHOD(readFileSync, read_file_sync);
		JS_SET_METHOD(openSync, open_sync);
		JS_SET_METHOD(closeSync, close_sync);
		JS_SET_METHOD(readSync, read_sync);
		JS_SET_METHOD(writeSync, write_sync);
		// async
		JS_SET_METHOD(writeFile, write_file_async);
		JS_SET_METHOD(readFile, read_file_async);
		JS_SET_METHOD(open, open_async);
		JS_SET_METHOD(close, close_async);
		JS_SET_METHOD(read, read_async);
		JS_SET_METHOD(write, write_async);
	}
};

JS_REG_MODULE(_fs, NativeFileHelper);
JS_END
