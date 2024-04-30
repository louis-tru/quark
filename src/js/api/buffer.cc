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

#include "../js_.h"
#include "../types.h"

namespace qk { namespace js {

	bool parseEncoding(FunctionArgs args, JSValue* arg, Encoding& en) {
		Js_Worker(args);
		String s = arg->toStringValue(worker);
		en = codec_parse_encoding( s );
		if (en == kInvalid_Encoding) {
			Js_Throw(
				"Unknown encoding \"%s\", the optional value is "
				"[binary|ascii|base64|hex|utf8|utf16|utf32]", *s ), false;
		}
		return true;
	}

	class WrapBuffer {
	public:

		static void fromString(FunctionArgs args) {
			Js_Worker(args);

			if ( args.length() < 1 || !args[0]->isString() ) { // 参数错误
				Js_Throw(
					"* @method fromString(str[,encoding])\n"
					"* @param arg {String}\n"
					"* @param [encoding=utf8] {binary|ascii|base64|hex|utf8|ucs2|utf16|utf32}\n"
				);
			}

			Encoding en = kUTF8_Encoding;
			JSValue* r;

			if ( args.length() > 1 ) {
				if ( ! parseEncoding(args, args[1], en) ) return;
			}

			Js_Return( worker->newUint8Array(args[0]->cast<JSString>(), en) );
		}

		static void toString(FunctionArgs args) {
			Js_Worker(args);

			int args_index = 0;
			if (args.length() < 1 || !args[0]->isUint8Array()) {
				Js_Throw(
					"* @method convertString(uint8array,[encoding[,start[,end]]])\n"
					"* @param uint8array {Uint8Array}\n"
					"* @param [encoding=utf8] {binary|ascii|base64|hex|utf8|ucs2|utf16|utf32}\n"
					"* @param [start=0] {uint}\n"
					"* @param [end] {uint}\n"
				);
			}

			JSUint8Array* self = args[args_index++]->cast<JSUint8Array>();

			Encoding encoding = kUTF8_Encoding;
			int len = self->byteLength(worker);
			cChar* data = self->weakBuffer(worker).val();
			uint32_t start = 0;
			uint32_t end = len;

			if (args.length() > args_index && args[args_index]->isString()) {
				if ( ! parseEncoding(args, args[args_index], encoding) ) return;
				args_index++;
			}
			if (args.length() > args_index) {
				start = args[args_index]->toUint32Value(worker);
				start = Qk_MIN(len, start);
				args_index++;
			}
			if (args.length() > args_index) {
				end = args[args_index]->toUint32Value(worker);
				end = Qk_MIN(len, end);
				args_index++;
			}

			if ( end <= start ) {
				Js_Return( JSString::Empty(worker) );
			}

			switch (encoding) {
				case kHex_Encoding: // 编码
				case kBase64_Encoding: {
					Buffer buff = codec_encode(encoding, WeakBuffer(data + start, end - start).buffer());
					Js_Return( worker->newStringOneByte(buff.collapseString()) );
					break;
				} default: { // 解码to ucs2
					String2 str( codec_decode_to_uint16(encoding, WeakBuffer(data+start, end - start).buffer()));
					Js_Return( worker->newInstance(str) );
					break;
				}
			}
		}

		static void binding(JSObject* exports, Worker* worker) {
			Js_Set_Method(fromString, fromString);
			Js_Set_Method(toString, toString);
		}
	};

	Js_Set_Module(_buffer, WrapBuffer);
} }