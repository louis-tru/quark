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

#include "ftr-js/wrap.h"
#include "ftr/util/buffer.h"
#include "ftr-js/str.h"

/**
 * @ns ftr::js
 */

JS_BEGIN

/**
 * @class NativeBuffer
 */
class NativeBuffer {
 public:
	
	/**
	 * @func parseEncoding()
	 */
	static bool parseEncoding(FunctionCall args, const Local<JSValue>& arg, Encoding& en) {
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

	/**
	 * @func fromString(str[,encoding])
	 * @arg arg  {String}
	 * @arg [encoding=utf8] {binary|ascii|base64|hex|utf8|ucs2|utf16|utf32}
	 */
	static void fromString(FunctionCall args) {
		JS_WORKER(args);

		if ( args.Length() < 1 || !args[0]->IsString(worker) ) { // 参数错误
			JS_THROW_ERR(
										"* @func fromString(str[,encoding])\n"
										"* @arg arg {String}\n"
										"* @arg [encoding=utf8] {binary|ascii|base64|hex|utf8|ucs2|utf16|utf32}\n"
			);
		}

		Encoding en = Encoding::utf8;
		Local<JSValue> r;

		if ( args.Length() > 1 ) {
			if ( ! parseEncoding(args, args[1], en) ) return;
		}

		JS_RETURN( worker->NewUint8Array(args[0].To<JSString>(), en) );
	}

	/**
	 * @func convertString(uint8array,[encoding[,start[,end]]])
	 * @arg uint8array {Uint8Array}
	 * @arg [encoding=utf8] {binary|ascii|base64|hex|utf8|ucs2|utf16|utf32}
	 * @arg [start=0] {uint}
	 * @arg [end] {uint}
	 */
	static void convertString(FunctionCall args) {
		JS_WORKER(args);

		int args_index = 0;
		if (args.Length() < 1 || !args[0]->IsUint8Array()) {
			JS_THROW_ERR(
				"* @func convertString(uint8array,[encoding[,start[,end]]])\n"
				"* @arg uint8array {Uint8Array}\n"
				"* @arg [encoding=utf8] {binary|ascii|base64|hex|utf8|ucs2|utf16|utf32}\n"
				"* @arg [start=0] {uint}\n"
				"* @arg [end] {uint}\n"
			);
		}

		Local<JSUint8Array> self = args[args_index++].To<JSUint8Array>();
		
		Encoding encoding = Encoding::utf8;
		int len = self->ByteLength(worker);
		char* data = self->weakBuffer(worker).value();
		uint start = 0;
		uint end = len;

		if (args.Length() > args_index && args[args_index]->IsString(worker)) {
			if ( ! parseEncoding(args, args[args_index], encoding) ) return;
			args_index++;
		}
		if (args.Length() > args_index && args[args_index]->ToUint32Maybe(worker).To(start)) {
			start = FX_MIN(len, start);
			args_index++;
		}
		if (args.Length() > args_index && args[args_index]->ToUint32Maybe(worker).To(end)) {
			end = FX_MIN(len, end);
			args_index++;
		}

		if ( end <= start ) {
			JS_RETURN( JSString::Empty(worker) );
		}

		switch (encoding) {
			case Encoding::hex: // 编码
			case Encoding::base64: {
				Buffer buff = Coder::encoding(encoding, data + start, end - start);
				JS_RETURN( worker->New(buff.collapse_string(), true) );
				break;
			} default: { // 解码to ucs2
				Ucs2String str( Coder::decoding_to_uint16(encoding, data + start, end - start) );
				JS_RETURN( worker->New(str) );
				break;
			}
		}
	}

	/**
	 * @func binding
	 */
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_SET_METHOD(fromString, fromString);
		JS_SET_METHOD(convertString, convertString);
	}
};

JS_REG_MODULE(_buffer, NativeBuffer);
JS_END
