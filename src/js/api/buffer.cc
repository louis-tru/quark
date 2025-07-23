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

	bool parseEncoding(FunctionArgs args, JSValue* arg, Encoding& en) {
		Js_Worker(args);
		String s = arg->toString(worker)->value(worker);
		en = codec_parse_encoding(s);
		if (en == kInvalid_Encoding) {
			Js_Throw(
				"Unknown encoding \"%s\", the optional value is "
				"[binary|ascii|base64|hex|utf8|utf16|utf32]", *s ), false;
		}
		return true;
	}

	struct NativeBuffer {
		static void fromString(FunctionArgs args) {
			Js_Worker(args);
			if ( args.length() < 1 || !args[0]->isString() ) { // 参数错误
				Js_Throw(
					"@method fromString(str[,en])\n"
					"@param arg {String}\n"
					"@param [en=utf8] {binary|ascii|base64|hex|utf-8|utf8|utf-16|utf16|ucs4}\n"
				);
			}
			Encoding en = kUTF8_Encoding;
			JSValue* r;
			if ( args.length() > 1 ) {
				if ( ! parseEncoding(args, args[1], en) )
					return;
			}
			auto buf = worker->newUint8Array(args[0]->template cast<JSString>(), en);
			Js_Return( buf );
		}
	
		static void toString(FunctionArgs args) {
			Js_Worker(args);
			int args_index = 0;
			if (args.length() < 1 || !args[0]->isUint8Array()) {
				Js_Throw(
					"@method toString(uint8array,[encoding[,start[,end]]])\n"
					"@param uint8array {Uint8Array}\n"
					"@param [encoding=utf8] {binary|ascii|base64|hex|utf-8|utf8|utf-16|utf16|ucs4}\n"
					"@param [start=0] {uint}\n"
					"@param [end] {uint}\n"
				);
			}

			auto self = args[args_index++]->template cast<JSUint8Array>();
			Encoding en = kUTF8_Encoding;
			int len = self->byteLength(worker);
			cChar* data = self->value(worker).val();
			uint32_t start = 0;
			uint32_t end = len;

			if (args.length() > args_index && args[args_index]->isString()) {
				if ( ! parseEncoding(args, args[args_index], en) )
					return;
				args_index++;
			}
			if (args.length() > args_index) {
				start = args[args_index]->asUint32(worker).from(0);
				start = Qk_Min(len, start);
				args_index++;
			}
			if (args.length() > args_index) {
				end = args[args_index]->asUint32(worker).from(0);
				end = Qk_Min(len, end);
				args_index++;
			}

			if ( end <= start ) {
				Js_Return( worker->newEmpty() );
			}

			auto chAddr = data + start;
			auto lenPart = end - start;

			switch (en) {
				case kHex_Encoding: // convert to hex or base64 string
				case kBase64_Encoding: {
					Buffer buff = codec_encode(en, WeakBuffer(chAddr, lenPart).buffer());
					Js_Return( worker->newStringOneByte(buff.collapseString()) );
					break;
				}
				case kUTF8_Encoding: {
					auto utf16 = codec_utf8_to_utf16(WeakBuffer(chAddr, lenPart).buffer());
					Js_Return( worker->newValue(utf16.collapseString()) );
					break;
				}
				case kBinary_Encoding: {
					WeakBuffer binaray(chAddr, lenPart);
					Js_Return( worker->newStringOneByte(binaray.copy().collapseString()) );
					break;
				}
				case kAscii_Encoding: {
					// decode to ucs1
					auto ucs1 = codec_decode_to_ucs1(en, WeakBuffer(chAddr, lenPart).buffer());
					// ucs1 to js uft16 string
					Js_Return( worker->newStringOneByte(ucs1.collapseString()) );
					break;
				}
				case kUTF16_Encoding: {
					if (user_addr_t(chAddr) % 2 == 0) { // Already aligned
						ArrayWeak<uint16_t> utf16(reinterpret_cast<const uint16_t*>(chAddr), lenPart>>1);
						Js_Return( worker->newValue(utf16.buffer()) );
						break;
					}
				}
				default: { // kUCS4_Encoding
					// decode to unicode
					auto unicode = codec_decode_to_unicode(en, WeakBuffer(chAddr, lenPart).buffer());
					// unicode to js uft16 string
					Js_Return( worker->newValue(unicode.collapseString()) );
					break;
				}
			}
		}

		static void binding(JSObject* exports, Worker* worker) {

			Js_Method(fromString, { fromString(args); });
			Js_Method(toString, { toString(args); });
		}
	};

	Js_Module(_buffer, NativeBuffer);
} }
