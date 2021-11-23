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

#ifndef __flare__util__codec__
#define __flare__util__codec__

#include "./util.h"
#include "./array.h"

namespace flare {

	/**
	* @enum Encoding
	*/
	enum class Encoding {
		binary = 0, /**binary*/
		ascii,      /**ascii*/
		base64,     /**base64*/
		hex,        /**hex*/
		utf8,       /**utf8*/
		ucs2,       /**ucs2*/
		ucs4,       /**ucs4*/
		utf16,      /**utf16*/
		utf32,      /**utf32*/
		unknown,
		BINARY  = binary,
		ASCII   = ascii,
		BASE64  = base64,
		HEX     = hex,
		UTF8    = utf8,
		UCS2    = ucs2,
		UCS4    = ucs4,
		UTF16   = utf16,
		UTF32   = utf32,
		UNKNOWN = unknown,
		//@end enum
	};

	/**
	* @class Codec
	*/
	class F_EXPORT Codec {
	 public:
		
		/**
		* @func parse_encoding
		*/
		static Encoding parse_encoding(cString& en);
		
		/**
		* @func encoding_string
		*/
		static String encoding_string(Encoding en);
		
		// encoding
		static Buffer encode(Encoding target_en, const ArrayBuffer<char>& source);
		static Buffer encode(Encoding target_en, const ArrayString<char>& source);
		static Buffer encode(Encoding target_en, const ArrayBuffer<uint16_t>& source);
		static Buffer encode(Encoding target_en, const ArrayString<uint16_t>& source);
		static Buffer encode(Encoding target_en, const ArrayBuffer<uint32_t>& source);
		static Buffer encode(Encoding target_en, const ArrayString<uint32_t>& source);
		// decoding
		static ArrayBuffer<char>     decode_to_buffer(Encoding source_en, const ArrayBuffer<char>& source);
		static ArrayBuffer<char>     decode_to_buffer(Encoding source_en, const ArrayString<char>& source);
		static ArrayBuffer<uint16_t> decode_to_uint16(Encoding source_en, const ArrayBuffer<char>& source);
		static ArrayBuffer<uint16_t> decode_to_uint16(Encoding source_en, const ArrayString<char>& source);
		static ArrayBuffer<uint32_t> decode_to_uint32(Encoding source_en, const ArrayBuffer<char>& source);
		static ArrayBuffer<uint32_t> decode_to_uint32(Encoding source_en, const ArrayString<char>& source);
	};

	typedef Codec Coder;

}
#endif
