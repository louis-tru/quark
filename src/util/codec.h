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

#ifndef __quark_util_codec__
#define __quark_util_codec__

#include "./util.h"

namespace qk {

	enum Encoding {
		kInvalid_Encoding,
		kBinary_Encoding,
		kAscii_Encoding,
		kHex_Encoding,
		kBase64_Encoding,
		kUTF8_Encoding,
		kUTF16_Encoding,
		kUCS4_Encoding, // Unicode
	};

	Qk_EXPORT Encoding codec_parse_encoding(cString& en);
	Qk_EXPORT String   codec_encoding_string(Encoding en);
	Qk_EXPORT uint32_t codec_decode_utf8_to_unichar(const uint8_t *str, uint32_t *out);
	Qk_EXPORT uint32_t codec_decode_utf16_to_unichar(const uint16_t* str, uint32_t* out);
	// encode
	Qk_EXPORT ArrayBuffer<char> codec_encode(Encoding target_en, cArray<char>& source);
	Qk_EXPORT ArrayBuffer<char> codec_encode(Encoding target_en, cString& source);
	Qk_EXPORT ArrayBuffer<char> codec_encode(Encoding target_en, cArray<uint16_t>& source);
	Qk_EXPORT ArrayBuffer<char> codec_encode(Encoding target_en, cString2& source);
	Qk_EXPORT ArrayBuffer<char> codec_encode(Encoding target_en, cArray<uint32_t>& source);
	// decode
	Qk_EXPORT ArrayBuffer<char>     codec_decode_to_buffer(Encoding source_en, cArray<char>& source);
	Qk_EXPORT ArrayBuffer<uint16_t> codec_decode_to_uint16(Encoding source_en, cArray<char>& source);
	Qk_EXPORT ArrayBuffer<uint32_t> codec_decode_to_uint32(Encoding source_en, cArray<char>& source);
	Qk_EXPORT ArrayBuffer<uint32_t> codec_decode_to_uint32(Encoding source_en, cString& source);
	// utf16
	Qk_EXPORT ArrayBuffer<uint16_t> codec_encode_to_utf16(cArray<uint32_t>& source);
	Qk_EXPORT ArrayBuffer<uint32_t> codec_decode_form_utf16(cArray<uint16_t>& source);
}
#endif
