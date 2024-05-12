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

#include "./cb.h"

namespace qk { namespace js {

	class NativeFileReader {
	public:

		static void read(FunctionArgs args, bool isStream) {
			Js_Worker(args);
			uint32_t args_index = 1;
			if ( args.length() < 2 || !args[0]->isFunction() || !args[1]->isString() ) {
				if (isStream) {
					Js_Throw(
						"* @method reader.readStream(cb,path)\n"
						"@param cb {Function}\n"
						"@param path {String}\n"
						"* @return {uint} return read id\n"
					);
				} else {
					Js_Throw(
								"* @method reader.readFile(cb,path[,encoding])\n"
								"@param cb {Function}\n"
								"@param path {String}\n"
								"@param [encoding] {Encoding}\n"
								"* @return {uint} return read id\n"
					);
				}
			}
			String path = args[args_index++]->toStringValue(worker);
			Encoding encoding = kInvalid_Encoding;

			if (args.length() > args_index && args[args_index]->isString()) {
				if ( ! parseEncoding(args, args[args_index++], encoding) ) return;
			}
			if ( isStream ) {
				auto cb = get_callback_for_io_stream(worker, args[0]);
				Js_Return( fs_reader()->read_stream( path, cb ) );
			} else {
				auto cb = get_callback_for_buffer(worker, args[0], encoding);
				Js_Return( fs_reader()->read_file( path, cb ) );
			}
		}

		static void binding(JSObject* exports, Worker* worker) {

			Js_Set_Method(readStream, {
				read(args, true);
			});

			Js_Set_Method(readFile, {
				read(args, false);
			});

			Js_Set_Method(readFileSync, {
				if (args.length() == 0 || !args[0]->isString()) {
					Js_Throw(
						"* @method reader.readFileSync(path[,encoding])\n"
						"@param path {String}\n"
						"@param [encoding] {Encoding}\n"
						"* @return {Buffer} return read Buffer\n"
					);
				}
				Encoding encoding = kInvalid_Encoding;
				if (args.length() > 1 && args[1]->isString()) {
					if ( ! parseEncoding(args, args[1], encoding) ) return;
				}
				Buffer rv;
				try {
					rv = fs_reader()->read_file_sync( args[0]->toStringValue(worker) );
				} catch(cError& err) {
					Js_Throw(err);
				}
				Js_Return( convert_buffer(worker, rv, encoding) );
			});

			Js_Set_Method(existsSync, {
				if ( args.length() == 0 || !args[0]->isString() ) {
					Js_Throw(
						"* @method reader.existsSync(path)\n"
						"@param path {String}\n"
						"* @return {bool}\n"
					);
				}
				Js_Return( fs_reader()->exists_sync( args[0]->toStringValue(worker) ) );
			});

			Js_Set_Method(isFileSync, {
				if ( args.length() == 0 || !args[0]->isString() ) {
					Js_Throw(
						"* @method reader.isFileSync(path)\n"
						"@param path {String}\n"
						"* @return {bool}\n"
					);
				}
				Js_Return( fs_reader()->is_file_sync( args[0]->toStringValue(worker) ) );
			});

			Js_Set_Method(isDirectorySync, {
				if ( args.length() == 0 || !args[0]->isString() ) {
					Js_Throw(
						"* @method reader.isDirectorySyncpath)\n"
						"@param path {String}\n"
						"* @return {bool}\n"
					);
				}
				Js_Return( fs_reader()->is_directory_sync( args[0]->toStringValue(worker) ) );
			});

			Js_Set_Method(readdirSync, {
				if ( args.length() == 0 || !args[0]->isString() ) {
					Js_Throw(
						"* @method reader.readdirSync(path)\n"
						"@param path {String}\n"
						"* @return {Array}\n"
					);
				}
				Js_Return( fs_reader()->readdir_sync( args[0]->toStringValue(worker) ) );
			});

			Js_Set_Method(abort, {
				if ( args.length() == 0 || ! args[0]->isUint32() ) {
					Js_Throw(
						"* @method reader.abort(id)\n"
						"@param id {uint} abort id\n"
					);
				}
				fs_reader()->abort( args[0]->toUint32Value(worker).unsafe() );
			});

			Js_Set_Method(clear, {
				fs_reader()->clear();
			});
		}
	};

	Js_Set_Module(_reader, NativeFileReader);
} }
 
