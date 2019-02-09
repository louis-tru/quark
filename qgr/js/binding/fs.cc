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

#include "qgr/utils/fs.h"
#include "qgr/utils/http.h"
#include "qgr/js/js.h"
#include "qgr/js/str.h"
#include "cb-1.h"

/**
 * @ns qgr::js
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

/**
 * @class NativeFileHelper
 */
class NativeFileHelper {
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
				cb = get_callback_for_none(worker, args[args_index]);
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
					"* @func chownrSync(path, owner, group)\n"
					"* @arg path {String}\n"
					"* @arg owner {uint}\n"
					"* @arg group {uint}\n"
					"* @ret {bool}\n"
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
			JS_RETURN( FileHelper::chown_r_sync(args[0]->ToStringValue(worker),
																					 args[1]->ToUint32Value(worker),
																					 args[2]->ToUint32Value(worker)) );
		} else {
			Callback cb; 
			if ( args.Length() > 3 ) {
				cb = get_callback_for_none(worker, args[3]);
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
					"* @func mkdirpSync(path[,mode])\n"
					"* @arg path {String}\n"
					"* @arg [mode=default_mode] {uint}\n"
					"* @ret {bool}\n"
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
				cb = get_callback_for_none(worker, args[args_index]);
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
					"* @func removerSync(path)\n"
					"* @arg path {String}\n"
					"* @ret {bool}\n"
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
			JS_RETURN( FileHelper::rm_r_sync(args[0]->ToStringValue(worker)) );
		} else {
			Callback cb;
			if ( args.Length() > 1 ) {
				cb = get_callback_for_none(worker, args[1]);
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
				cb = get_callback_for_none(worker, args[2]);
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
				cb = get_callback_for_none(worker, args[2]);
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
				cb = get_callback_for_type_array_dirent(worker, args[1]);
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
				cb = get_callback_for_type_bool(worker, args[1]);
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
				cb = get_callback_for_type_bool(worker, args[1]);
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
		WrapFileStat::binding(exports, worker);
		//
		JS_SET_PROPERTY(FOPEN_ACCMODE, FOPEN_ACCMODE);
		JS_SET_PROPERTY(FOPEN_RDONLY, FOPEN_RDONLY);
		JS_SET_PROPERTY(FOPEN_WRONLY, FOPEN_WRONLY);
		JS_SET_PROPERTY(FOPEN_RDWR, FOPEN_RDWR);
		JS_SET_PROPERTY(FOPEN_CREAT, FOPEN_CREAT);
		JS_SET_PROPERTY(FOPEN_EXCL, FOPEN_EXCL);
		JS_SET_PROPERTY(FOPEN_NOCTTY, FOPEN_NOCTTY);
		JS_SET_PROPERTY(FOPEN_TRUNC, FOPEN_TRUNC);
		JS_SET_PROPERTY(FOPEN_APPEND, FOPEN_APPEND);
		JS_SET_PROPERTY(FOPEN_NONBLOCK, FOPEN_NONBLOCK);
		JS_SET_PROPERTY(FOPEN_R, FOPEN_R);
		JS_SET_PROPERTY(FOPEN_W, FOPEN_W);
		JS_SET_PROPERTY(FOPEN_A, FOPEN_A);
		JS_SET_PROPERTY(FOPEN_RP, FOPEN_RP);
		JS_SET_PROPERTY(FOPEN_WP, FOPEN_WP);
		JS_SET_PROPERTY(FOPEN_AP, FOPEN_AP);
		JS_SET_PROPERTY(FTYPE_UNKNOWN, FTYPE_UNKNOWN);
		JS_SET_PROPERTY(FTYPE_FILE, FTYPE_FILE);
		JS_SET_PROPERTY(FTYPE_DIR, FTYPE_DIR);
		JS_SET_PROPERTY(FTYPE_LINK, FTYPE_LINK);
		JS_SET_PROPERTY(FTYPE_FIFO, FTYPE_FIFO);
		JS_SET_PROPERTY(FTYPE_SOCKET, FTYPE_SOCKET);
		JS_SET_PROPERTY(FTYPE_CHAR, FTYPE_CHAR);
		JS_SET_PROPERTY(FTYPE_BLOCK, FTYPE_BLOCK);
		JS_SET_PROPERTY(DEFAULT_MODE, FileHelper::default_mode);
		// api
		JS_SET_METHOD(chmodrSync, chmod_r<true>);
		JS_SET_METHOD(chownrSync, chown_r<true>);
		JS_SET_METHOD(mkdirpSync, mkdir_p<true>);
		JS_SET_METHOD(removerSync, rm_r<true>);
		JS_SET_METHOD(copySync, cp<true>);
		JS_SET_METHOD(copyrSync, cp_r<true>);
		JS_SET_METHOD(readdirSync, readdir<true>);
		JS_SET_METHOD(isFileSync, is_file<true>);
		JS_SET_METHOD(isDirectorySync, is_directory<true>);
		JS_SET_METHOD(abort, abort);
		JS_SET_METHOD(mkdirp, mkdir_p<false>);
		JS_SET_METHOD(readdir, readdir<false>);
		JS_SET_METHOD(isFile, is_file<false>);
		JS_SET_METHOD(isDirectory, is_directory<false>);
		JS_SET_METHOD(chmodr, chmod_r<false>);
		JS_SET_METHOD(chownr, chown_r<false>);
		JS_SET_METHOD(remover, rm_r<false>);
		JS_SET_METHOD(copy, cp<false>);
		JS_SET_METHOD(copyr, cp_r<false>);
	}
};

JS_REG_MODULE(qgr_fs, NativeFileHelper);
JS_END
