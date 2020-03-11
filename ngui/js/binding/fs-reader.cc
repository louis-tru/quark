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

#include "nxkit/fs.h"
#include "ngui/js/js.h"
#include "ngui/js/str.h"
#include "nxkit/codec.h"
#include "cb-1.h"
#include "fs-1.h"

/**
 * @ns ngui::js
 */

JS_BEGIN

class NativeFileReader {
 public:
	
	template<bool stream> static void readFile(FunctionCall args, cchar* argument) {
		JS_WORKER(args);
		if ( args.Length() == 0 || ! args[0]->IsString(worker) ) {
			JS_THROW_ERR(argument);
		}
		
		String path = args[0]->ToStringValue(worker);
		Cb cb;
		
		Encoding encoding = Encoding::unknown;
		int args_index = 1;
		
		if (args.Length() > args_index && args[args_index]->IsString(worker)) { //
			if ( ! parse_encoding(args, args[args_index], encoding) ) return;
			args_index++;
		}
		if ( args.Length() > args_index ) {
			cb = stream ?
				get_callback_for_io_stream(worker, args[args_index]) :
				get_callback_for_buffer(worker, args[args_index], encoding);
		}
		if ( stream ) {
			JS_RETURN( f_reader()->read_stream( path, cb ) );
		} else {
			JS_RETURN( f_reader()->read_file( path, cb ) );
		}
	}
	
	/**
	 * @func readStream(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 * @ret {uint} return read id
	 */
	static void readStream(FunctionCall args) {
		readFile<true>(args,
							 "* @func reader.readStream(path[,cb])\n"
							 "* @arg path {String}\n"
							 "* @arg [cb] {Function}\n"
							 "* @ret {uint} return read id\n"
							 );
	}
	
	/**
	 * @func readFile(path[,cb])
	 * @arg path {String}
	 * @arg [cb] {Function}
	 * @ret {uint} return read id
	 */
	static void readFile(FunctionCall args) {
		readFile<false>(args,
								"* @func reader.readFile(path[,encoding[,cb]])\n"
								"* @arg path {String}\n"
								"* @arg [encoding] {Encoding}\n"
								"* @arg [cb] {Function}\n"
								"* @ret {uint} return read id\n"
								);
	}
	
	/**
	 * @func readFileSync(path)
	 * @arg path {String}
	 * @ret {Buffer} return read Buffer
	 */
	static void readFileSync(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || !args[0]->IsString(worker)) {
			JS_THROW_ERR(
										"* @func reader.readFileSync(path[,encoding])\n"
										"* @arg path {String}\n"
										"* @arg [encoding] {Encoding}\n"
										"* @ret {Buffer} return read Buffer\n"
										);
		}
		
		Encoding encoding = Encoding::unknown;
		
		if (args.Length() > 1 && args[1]->IsString(worker)) {
			if ( ! parse_encoding(args, args[1], encoding) ) return;
		}
		
		Buffer rv;
		try {
			rv = f_reader()->read_file_sync( args[0]->ToStringValue(worker) );
		} catch(cError& err) {
			JS_THROW_ERR(err);
		}
		
		JS_RETURN( convert_buffer(worker, rv, encoding) );
	}
	
	/**
	 * @func existsSync(path)
	 * @arg path {String}
	 * @ret {bool}
	 */
	static void existsSync(FunctionCall args) {
		JS_WORKER(args);
		if ( args.Length() == 0 || !args[0]->IsString(worker) ) {
			JS_THROW_ERR(
										"* @func reader.existsSync(path)\n"
										"* @arg path {String}\n"
										"* @ret {bool}\n"
										);
		}
		JS_RETURN( f_reader()->exists_sync( args[0]->ToStringValue(worker) ) );
	}
	
	/**
	 * @func isFileSync(path)
	 * @arg path {String}
	 * @ret {bool}
	 */
	static void isFileSync(FunctionCall args) {
		JS_WORKER(args);
		if ( args.Length() == 0 || !args[0]->IsString(worker) ) {
			JS_THROW_ERR(
										"* @func reader.isFileSync(path)\n"
										"* @arg path {String}\n"
										"* @ret {bool}\n"
										);
		}
		JS_RETURN( f_reader()->is_file_sync( args[0]->ToStringValue(worker) ) );
	}
	
	/**
	 * @func isDirectorySync(path)
	 * @arg path {String}
	 * @ret {bool}
	 */
	static void isDirectorySync(FunctionCall args) {
		JS_WORKER(args);
		if ( args.Length() == 0 || !args[0]->IsString(worker) ) {
			JS_THROW_ERR(
										"* @func reader.isDirectorySyncpath)\n"
										"* @arg path {String}\n"
										"* @ret {bool}\n"
										);
		}
		JS_RETURN( f_reader()->is_directory_sync( args[0]->ToStringValue(worker) ) );
	}
	
	/**
	 * @func readdirSync(path)
	 * @arg path {String}
	 * @ret {Array}
	 */
	static void readdirSync(FunctionCall args) {
		JS_WORKER(args);
		if ( args.Length() == 0 || !args[0]->IsString(worker) ) {
			JS_THROW_ERR(
										"* @func reader.readdirSync(path)\n"
										"* @arg path {String}\n"
										"* @ret {Array}\n"
										);
		}
		JS_RETURN( f_reader()->readdir_sync( args[0]->ToStringValue(worker) ) );
	}
	
	/**
	 * @func abort(id)
	 * @arg id {uint} abort id
	 */
	static void abort(FunctionCall args) {
		JS_WORKER(args);
		if ( args.Length() == 0 || ! args[0]->IsUint32(worker) ) {
			JS_THROW_ERR(
										"* @func reader.abort(id)\n"
										"* @arg id {uint} abort id\n"
										);
		}
		f_reader()->abort( args[0]->ToUint32Value(worker) );
	}
	
	/**
	 * @func clear()
	 */
	static void clear(FunctionCall args) {
		f_reader()->clear();
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_SET_METHOD(readFile, readFile);
		JS_SET_METHOD(readStream, readStream);
		JS_SET_METHOD(readFileSync, readFileSync);
		JS_SET_METHOD(existsSync, existsSync);
		JS_SET_METHOD(isFileSync, isFileSync);
		JS_SET_METHOD(isDirectorySync, isDirectorySync);
		JS_SET_METHOD(readdirSync, readdirSync);
		JS_SET_METHOD(abort, abort);
		JS_SET_METHOD(clear, clear);
	}
};

JS_REG_MODULE(_reader, NativeFileReader);
JS_END
 
