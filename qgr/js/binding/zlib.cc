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

#include "qgr/util/zlib.h"
#include "fs-1.h"

/**
 * @ns qgr::js
 */

JS_BEGIN

/**
 * @class WrapGZip
 */
class WrapGZip {
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		typedef WrapFileImpl<GZip> WrapGZip;
		JS_DEFINE_CLASS(GZip, WrapGZip::constructor, {
			JS_SET_CLASS_ACCESSOR(path, WrapGZip::path);
			JS_SET_CLASS_METHOD(isOpen, WrapGZip::is_open);
			JS_SET_CLASS_METHOD(open, WrapGZip::open);
			JS_SET_CLASS_METHOD(close, WrapGZip::close);
			JS_SET_CLASS_METHOD(read, WrapGZip::read);
			JS_SET_CLASS_METHOD(write, WrapGZip::write);
		}, nullptr);
	}
};

/**
 * @class WrapZipReader
 */
class WrapZipReader: public WrapObject {
 public:

	/**
	 * @constructor(path[,passwd]) 
	 * @arg path {String}
	 * @arg [passwd] {String}
	 */
	static void constructor(FunctionCall args) {
		JS_WORKER(args);
		if ( args.Length() == 0 || !args[0]->IsString(worker) ) {
			JS_THROW_ERR(
				"* @constructor(path[,passwd])\n"
				"* @arg path {String}\n"
				"* @arg [passwd] {String}\n"
			);
		}
		String path = args[0]->ToStringValue(worker);
		String passwd;
		if ( args.Length() > 1 ) {
			passwd = args[1]->ToStringValue(worker);
		}
		New<WrapZipReader>(args, new ZipReader(path, passwd));
	}
	
	/**
	 * @func open()
	 */
	static void open(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(ZipReader);
		JS_RETURN( self->open() );
	}
	
	/**
	 * @func close()
	 */
	static void close(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(ZipReader);
		JS_RETURN( self->close() );
	}
	
	/**
	 * @get path {String}
	 */
	static void path(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(ZipReader);
		JS_RETURN( self->path() );
	}
	
	/**
	 * @get compatible_path {String}
	 */
	static void compatible_path(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(ZipReader);
		JS_RETURN( self->compatible_path() );
	}
	
	/**
	 * @get passwd {String}
	 */
	static void passwd(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(ZipReader);
		JS_RETURN( self->passwd() );
	}
	
	/**
	 * @func exists(in_path)
	 * @arg in_path {String}
	 * @ret {bool}
	 */
	static void exists(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || !args[0]->IsString(worker)) {
			JS_THROW_ERR(
										"* @func exists(in_path)\n"
										"* @arg in_path {String}\n"
										"* @ret {bool}\n"
										);
		}
		JS_SELF(ZipReader);
		JS_RETURN( self->exists(args[0]->ToStringValue(worker)) );
	}
	
	/**
	 * @func is_file(in_path)
	 * @arg in_path {String}
	 * @ret {bool}
	 */
	static void is_file(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || !args[0]->IsString(worker)) {
			JS_THROW_ERR(
										"* @func isFile(in_path)\n"
										"* @arg in_path {String}\n"
										"* @ret {bool}\n"
										);
		}
		JS_SELF(ZipReader);
		JS_RETURN( self->is_file(args[0]->ToStringValue(worker)) );
	}
	
	/**
	 * @func is_directory(in_path)
	 * @arg in_path {String}
	 * @ret {bool}
	 */
	static void is_directory(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || !args[0]->IsString(worker)) {
			JS_THROW_ERR(
										"* @func isDirectory(in_path)\n"
										"* @arg in_path {String}\n"
										"* @ret {bool}\n"
										);
		}
		JS_SELF(ZipReader);
		JS_RETURN( self->is_directory(args[0]->ToStringValue(worker)) );
	}
	
	/**
	 * @func readdir(in_path)
	 * @arg in_path {String}
	 * @ret {Array}
	 */
	static void readdir(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || !args[0]->IsString(worker)) {
			JS_THROW_ERR(
										"* @func readdir(in_path)\n"
										"* @arg in_path {String}\n"
										"* @ret {Array}\n"
										);
		}
		JS_SELF(ZipReader);
		JS_RETURN( self->readdir(args[0]->ToStringValue(worker)) );
	}
	
	/**
	 * @func jump(in_path)
	 * @arg in_path {String}
	 * @ret {bool}
	 */
	static void jump(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || !args[0]->IsString(worker)) {
			JS_THROW_ERR(
				"* @func jump(in_path)"
				"* @arg in_path {String}"
				"* @ret {bool}"
			);
		}
		JS_SELF(ZipReader);
		JS_RETURN( self->jump(args[0]->ToStringValue(worker)) );
	}
	
	/**
	 * @func first() jump to first
	 * @ret {bool}
	 */
	static void first(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(ZipReader);
		JS_RETURN( self->first() );
	}
	
	/**
	 * @func next() jump to next file
	 * @ret {bool}
	 */
	static void next(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(ZipReader);
		JS_RETURN( self->next() );
	}
	
	/**
	 * @func read(buffer[,size])
	 * @arg buffer {Buffer}
	 * @arg [size=buffer.length] {uint}
	 * @ret {int} 
	 */
	static void read(FunctionCall args) {

		cchar* argument = 
		"* @func read(buffer[,size])\n"
		"* @arg buffer {Buffer}\n"
		"* @arg [size=buffer.length] {uint}\n"
		"* @ret {int}\n"
		;

		JS_WORKER(args);
		if (args.Length() == 0 || !worker->has_buffer(args[0])) {
			JS_THROW_ERR(argument);
		}
		Buffer* buff = Wrap<Buffer>::unpack(args[0].To<JSObject>())->self();
		uint length = buff->length();
		if (args.Length() > 1) {
			if (args[1]->IsUint32(worker)) {
				uint len = args[1]->ToUint32Value(worker);
				length = av_min(len, length);
			} else {
				JS_THROW_ERR(argument);
			}
		}
		JS_SELF(ZipReader);
		JS_RETURN( self->read(**buff, length) );
	}
	
	/**
	 * @func current()
	 * @ret {String} return current file name
	 */
	static void current(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(ZipReader);
		JS_RETURN( self->current() );
	}
	
	/**
	 * @func compressed_size()
	 * @ret {uint} return current file compressed size
	 */
	static void compressed_size(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(ZipReader);
		JS_RETURN( self->compressed_size() );
	}
	
	/**
	 * @func uncompressed_size()
	 * @ret {uint} return current file uncompressed size
	 */
	static void uncompressed_size(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(ZipReader);
		JS_RETURN( self->uncompressed_size() );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		ajs_define_class(ZipReader, constructor, {
			JS_SET_CLASS_METHOD(open, open);
			JS_SET_CLASS_METHOD(close, close);
			JS_SET_CLASS_ACCESSOR(path, path);
			JS_SET_CLASS_ACCESSOR(compatiblePath, compatible_path);
			JS_SET_CLASS_ACCESSOR(passwd, passwd);
			JS_SET_CLASS_METHOD(exists, exists);
			JS_SET_CLASS_METHOD(isDirectory, is_directory);
			JS_SET_CLASS_METHOD(isFile, is_file);
			JS_SET_CLASS_METHOD(readdir, readdir);
			JS_SET_CLASS_METHOD(jump, jump);
			JS_SET_CLASS_METHOD(first, first);
			JS_SET_CLASS_METHOD(next, next);
			JS_SET_CLASS_METHOD(read, read);
			JS_SET_CLASS_METHOD(current, current);
			JS_SET_CLASS_METHOD(compressedSize, compressed_size);
			JS_SET_CLASS_METHOD(uncompressedSize, uncompressed_size);
		}, nullptr);
	}
};

/**
 * @class WrapZipWriter
 */
class WrapZipWriter: public WrapObject {
 public:
	
	/**
	 * @constructor(path[,passwd])
	 * @arg path {String}
	 * @arg [passwd] {String}
	 */
	static void constructor(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || !args[0]->IsString(worker)) {
			JS_THROW_ERR(
				"* @constructor(path[,passwd])\n"
				"* @arg path {String}\n"
				"* @arg [passwd] {String}\n"
			);
		}
		String path = args[0]->ToStringValue(worker);
		String passwd;
		if ( args.Length() > 1 ) {
			passwd = args[1]->ToStringValue(worker);
		}
		New<WrapZipWriter>(args, new ZipWriter(path, passwd));
	}
	
	/**
	 * @func open([mode])
	 * @arg [mode=OPEN_MODE_CREATE] {OpenMode}
	 */
	static void open(FunctionCall args) {
		JS_WORKER(args);
		ZipWriter::OpenMode mode = ZipWriter::OPEN_MODE_CREATE;
		if ( args.Length() > 0 && !args[0]->IsUint32(worker) ) {
			uint arg = args[0]->ToUint32Value(worker);
			if ( arg < 3 ) {
				mode = (ZipWriter::OpenMode)arg;
			}
		}
		JS_SELF(ZipWriter);
		JS_RETURN( self->open(mode) );
	}
	
	/**
	 * @func close()
	 */
	static void close(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(ZipWriter);
		JS_RETURN( self->close() );
	}
	
	/**
	 * @get path {String}
	 */
	static void path(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(ZipWriter);
		JS_RETURN( self->path() );
	}
	
	/**
	 * @get passwd {String}
	 */
	static void passwd(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(ZipWriter);
		JS_RETURN( self->passwd() );
	}
	
	/**
	 * 获取压缩等级
	 * 0 - 9 个压缩等级, 数字越大需要更多处理时间
	 * -1自动,0为不压缩,1最佳速度,9最佳压缩
	 * 默认为-1
	 * @get level {int}
	 */
	static void level(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(ZipWriter);
		JS_RETURN( self->level() );
	}
	
	/**
	 * @set level {int}
	 */
	static void set_level(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args);
		if (value->IsInt32(worker)) {
			JS_SELF(ZipWriter);
			self->set_level( value->ToInt32Value(worker) );
		}
	}
	
	/**
	 * @func add_file(in_path)
	 * @arg in_path {String}
	 * @ret {bool}
	 */
	static void add_file(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || !args[0]->IsString(worker)) {
			JS_THROW_ERR(
				"* @func add_file(in_path)\n"
				"* @arg in_path {String}\n"
				"* @ret {bool}\n"
			);
		}
		JS_SELF(ZipWriter);
		JS_RETURN( self->add_file( args[0]->ToStringValue(worker) ));
	}
	
	/**
	 * @func write(buffer[,size])
	 * @arg buffer {Buffer}
	 * @arg [size=buffer.length] {uint}
	 * @ret {bool}
	 */
	static void write(FunctionCall args) {

		cchar* argument =
		"* @func write(buffer[,size])\n"
		"* @arg buffer {Buffer}\n"
		"* @arg [size=buffer.length] {uint}\n"
		"* @ret {bool}\n"
		;

		JS_WORKER(args);
		if ( args.Length() == 0 || !worker->has_buffer(args[0]) ) {
			JS_THROW_ERR(argument);
		}
		Buffer* buff = Wrap<Buffer>::unpack(args[0].To<JSObject>())->self();
		uint size = buff->length();
		
		if ( args.Length() > 1 ) {
			if (args[1]->IsUint32(worker)) {
				uint len = args[1]->ToUint32Value(worker);
				size = av_min(len, size);
			} else {
				JS_THROW_ERR(argument);
			}
		}
		JS_SELF(ZipWriter);
		JS_RETURN( self->write(WeakBuffer(**buff, size)) );
	}
	
	/**
	 * @func name()
	 * @ret {String} return current file name
	 */
	static void name(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(ZipWriter);
		JS_RETURN( self->name() );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		ajs_define_class(ZipWriter, constructor, {
			JS_SET_CLASS_METHOD(open, open);
			JS_SET_CLASS_METHOD(close, close);
			JS_SET_CLASS_ACCESSOR(path, path);
			JS_SET_CLASS_ACCESSOR(passwd, passwd);
			JS_SET_CLASS_ACCESSOR(level, level, set_level);
			JS_SET_CLASS_METHOD(add_file, add_file);
			JS_SET_CLASS_METHOD(write, write);
			JS_SET_CLASS_METHOD(name, name);
		}, nullptr);
	}
};

/**
 * @class NativeZLIB
 */
class NativeZLIB {
 public:

	/**
	 * @func compress_sync(data)
	 * @arg data {String|ArrayBuffer|Buffer}
	 * @ret {Buffer}
	 */
	static void compress_sync(FunctionCall args) {
		JS_WORKER(args);
		if (  args.Length() < 1 ||
				!(args[0]->IsString(worker) ||
					args[0]->IsArrayBuffer(worker) || worker->has_buffer(args[0]))
		) {
			JS_THROW_ERR(
				"* @func compress_sync(data)"
				"* @arg data {String|ArrayBuffer|Buffer}"
				"* @ret {Buffer}"
			);
		}
		int level = -1;
		if ( args.Length() > 1 && args[1]->IsUint32(worker) ) {
			level = args[1]->ToUint32Value(worker);
		}
		
		if (args[0]->IsString(worker)) {
			JS_RETURN( ZLib::compress( args[0]->ToStringValue(worker), level) );
		} else if (args[0]->IsArrayBuffer(worker)) {
			Local<JSArrayBuffer> ab = args[0].To<JSArrayBuffer>();
			WeakBuffer buff(ab->Data(worker), ab->ByteLength(worker));
			JS_RETURN( ZLib::compress(buff, level) );
		} else {
			JS_RETURN( ZLib::compress( *Wrap<Buffer>::unpack(args[0].To<JSObject>())->self(), level) );
		}
	}

	/**
	 * @func uncompress_sync(data)
	 * @arg data {String|ArrayBuffer|Buffer}
	 * @ret {Buffer}
	 */
	static void uncompress_sync(FunctionCall args) {
		JS_WORKER(args);
		if (  args.Length() < 1 ||
				!(args[0]->IsString(worker) ||
					args[0]->IsArrayBuffer(worker) || worker->has_buffer(args[0]))
		) {
			JS_THROW_ERR(
				"* @func uncompress_sync(data)"
				"* @arg data {String|ArrayBuffer|Buffer}"
				"* @ret {Buffer}"
			);
		}
		if (args[0]->IsString(worker)) {
			JS_RETURN( ZLib::uncompress( args[0]->ToStringValue(worker)) );
		} else if (args[0]->IsArrayBuffer(worker)) {
			Local<JSArrayBuffer> ab = args[0].To<JSArrayBuffer>();
			WeakBuffer buff(ab->Data(worker), ab->ByteLength(worker));
			JS_RETURN( ZLib::uncompress(buff) );
		} else {
			JS_RETURN( ZLib::uncompress( *Wrap<Buffer>::unpack(args[0].To<JSObject>())->self()) );
		}
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		WrapGZip::binding(exports, worker);
		WrapZipReader::binding(exports, worker);
		WrapZipWriter::binding(exports, worker);
		ajs_set_property(OPEN_MODE_CREATE, ZipWriter::OPEN_MODE_CREATE);
		ajs_set_property(OPEN_MODE_CREATE_AFTER, ZipWriter::OPEN_MODE_CREATE_AFTER);
		ajs_set_property(OPEN_MODE_ADD_IN_ZIP, ZipWriter::OPEN_MODE_ADD_IN_ZIP);
		ajs_set_method(compressSync, compress_sync);
		ajs_set_method(uncompressSync, uncompress_sync);
	}
};

JS_REG_MODULE(qgr_zlib, NativeZLIB);
JS_END