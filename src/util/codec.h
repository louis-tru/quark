/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __quark_util_codec__
#define __quark_util_codec__

#include "./util.h"

namespace qk {

	enum Encoding {
		kInvalid_Encoding,
		kBinary_Encoding,
		kLatin1_Encoding = kBinary_Encoding,
		kAscii_Encoding,
		kHex_Encoding,
		kBase64_Encoding,
		kUTF8_Encoding,
		kUTF16_Encoding,
		kUCS4_Encoding,
		kUNICODE_Encoding = kUCS4_Encoding,
	};

	Qk_EXPORT Encoding codec_parse_encoding(cString& en);
	Qk_EXPORT String   codec_encoding_string(Encoding en);
	Qk_EXPORT uint32_t codec_decode_utf8_to_unichar(const uint8_t *str, uint32_t *out);
	Qk_EXPORT uint32_t codec_decode_utf16_to_unichar(const uint16_t* str, uint32_t* out);
	// encode
	Qk_EXPORT ArrayBuffer<char> codec_encode(Encoding target_en, cArray<char>& unicode);
	Qk_EXPORT ArrayBuffer<char> codec_encode(Encoding target_en, cString& unicode);
	Qk_EXPORT ArrayBuffer<char> codec_encode(Encoding target_en, cArray<uint16_t>& unicode); // from UCS2
	Qk_EXPORT ArrayBuffer<char> codec_encode(Encoding target_en, cString2& unicode);
	Qk_EXPORT ArrayBuffer<char> codec_encode(Encoding target_en, cArray<uint32_t>& unicode);
	// decode
	Qk_EXPORT ArrayBuffer<uint32_t> codec_decode_to_unicode(Encoding source_en, cArray<char>& source);
	Qk_EXPORT ArrayBuffer<uint32_t> codec_decode_to_unicode(Encoding source_en, cString& source);
	// It will lose encoding outside UCS1
	Qk_EXPORT ArrayBuffer<char>     codec_decode_to_ucs1(Encoding source_en, cArray<char>& source);
	// It will lose encoding outside UCS2
	Qk_EXPORT ArrayBuffer<uint16_t> codec_decode_to_ucs2(Encoding source_en, cArray<char>& source);
	// utils
	Qk_EXPORT ArrayBuffer<uint16_t> codec_unicode_to_utf16(cArray<uint32_t>& unicode);
	Qk_EXPORT ArrayBuffer<char>     codec_unicode_to_utf8(cArray<uint32_t>& unicode);
	Qk_EXPORT ArrayBuffer<uint32_t> codec_utf16_to_unicode(cArray<uint16_t>& utf16);
	Qk_EXPORT ArrayBuffer<uint32_t> codec_utf8_to_unicode(cArray<char>& utf8);
	Qk_EXPORT ArrayBuffer<char>     codec_utf16_to_utf8(cArray<uint16_t>& utf16);
	Qk_EXPORT ArrayBuffer<uint16_t> codec_utf8_to_utf16(cArray<char>& utf8);
	Qk_EXPORT uint32_t              codec_utf16_to_utf8_length(cArray<uint16_t>& utf16);
}
#endif
