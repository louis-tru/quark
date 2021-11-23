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

#include "./error.h"
#include "./codec.h"
#include "./dict.h"

namespace flare {

	namespace codec {
		// ============ Utf-8 编码的范围 ==============
		//  1 | 0000 0000 - 0000 007F |                                              0xxxxxxx
		//  2 | 0000 0080 - 0000 07FF |                                     110xxxxx 10xxxxxx
		//  3 | 0000 0800 - 0000 FFFF |                            1110xxxx 10xxxxxx 10xxxxxx
		//  4 | 0001 0000 - 0010 FFFF |                   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		//  5 | 0020 0000 - 03FF FFFF |          111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		//  6 | 0400 0000 - 7FFF FFFF | 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		// ===========================================
		// 单个unicode转换成utf8的长度
		static uint32_t encode_utf8_char_length(uint32_t unicode) {
			if (unicode < 0x7F + 1) {               // 单字节编码
				return 1;
			}
			else {
				if (unicode < 0x7FF + 1) {            // 两字节编码
					return 2;
				}
				else if (unicode < 0xFFFF + 1) {      // 三字节编码
					return 3;
				}
				else if (unicode < 0x10FFFF + 1) {    // 四字节编码
					return 4;
				}
				else if (unicode < 0x3FFFFFF + 1) {   // 五字节编码
					if (unicode > 0x200000 - 1) {
						return 5;
					}
					else { // 这个区间没有编码
						return 0;
					}
				}
				else {                                //六字节编码
					return 6;
				}
				return 0;
			}
		}

		template <class Char>
		static uint32_t encode_utf8_str_length(const Char* source, uint32_t len) {
			uint32_t rev = 0;
			const Char* end = source + len;
			while (source < end) {
				rev += encode_utf8_char_length(*source);
				source++;
			}
			return rev;
		}

		// 单个unicode转换到utf-8编码
		static uint32_t encode_unicode_to_utf8_char(uint32_t unicode, char* s) {
			uint32_t rev = 1;
			
			if (unicode < 0x7F + 1) {             // 单字节编码
				*s = unicode;
			}
			else {
				// int length;
				if (unicode < 0x7FF + 1) {            // 两字节编码
					rev = 2;
					*s = 0b11000000;
				}
				else if (unicode < 0xFFFF + 1) {      // 三字节编码
					rev = 3;
					*s = 0b11100000;
				}
				else if (unicode < 0x10FFFF + 1) {    // 四字节编码
					rev = 4;
					*s = 0b11110000;
				}
				else if (unicode < 0x3FFFFFF + 1) {   // 五字节编码
					if (unicode > 0x200000 - 1) {
						rev = 5;
						*s = 0b11111000;
					}
					else { // 这个区间没有编码
						return 0;
					}
				}
				else {                               //六字节编码
					rev = 6;
					*s = 0b11111100;
				}
				for (int i = rev - 1; i > 0; i--) {
					s[i] = 0b10000000 | (unicode & 0b00111111);
					unicode >>= 6;
				}
				s[0] |= unicode;
			}
			return rev;
		}

		/**
		 * @func encode_to_binary
		 * @private
		 */
		template <class Char>
		static Buffer encode_to_binary(const Char* source, uint32_t len) {
			const Char* end = source + len;
			//uint32_t size = sizeof(Char);
			auto rev = Buffer::alloc(len, len + 1);
			char* data = *rev;
			while (source < end) {
				*data = *source;
				data++; source++;
			}
			(*rev)[len] = '\0';
			return rev;
		}

		/**
		 * @func encode_to_ascii
		 * @private
		 */
		template <class Char>
		static Buffer encode_to_ascii(const Char* source, uint32_t len) {
			// ucs2/ucs4到ascii会丢失所有ascii以外的编码
			const Char* end = source + len;
			auto rev = Buffer::alloc(len, len + 1);
			char* data = *rev;
			while (source < end) {
				*data = *(uint8_t*)source % 128;
				data++; source++;
			}
			*data = '\0';
			return rev;
		}

		/**
		 * @func encode_to_utf8
		 * @private
		 */
		template <class Char>
		static Buffer encode_to_utf8(const Char* source, uint32_t len) {
			uint32_t utf8_len = encode_utf8_str_length(source, len);
			auto rev = Buffer::alloc(utf8_len, utf8_len + 1);
			char* data = *rev;
			
			const Char* end = source + len;
			while (source < end) {
				data += encode_unicode_to_utf8_char(*source, data);
				source++;
			}
			*data = '\0';
			return rev;
		}

		/**
		 * @func encode_to_base64
		 * @private
		 */
		template <class Char>
		static Buffer encode_to_base64(const Char* source, uint32_t len) {
			cChar* src = (cChar*)source;
			uint32_t slen = len * sizeof(Char);
			uint32_t dlen = (slen + 2 - ((slen + 2) % 3)) / 3 * 4;
			auto rev = Buffer::alloc(dlen, dlen + 1);
			char* dst = *rev;
			dst[dlen] = 0;
			
			uint32_t a;
			uint32_t b;
			uint32_t c;
			uint32_t i;
			uint32_t k;
			uint32_t n;
			
			static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789+/";
			
			i = 0;
			k = 0;
			n = slen / 3 * 3;
			
			while (i < n) {
				a = src[i + 0] & 0xff;
				b = src[i + 1] & 0xff;
				c = src[i + 2] & 0xff;
				
				dst[k + 0] = table[a >> 2];
				dst[k + 1] = table[((a & 3) << 4) | (b >> 4)];
				dst[k + 2] = table[((b & 0x0f) << 2) | (c >> 6)];
				dst[k + 3] = table[c & 0x3f];
				
				i += 3;
				k += 4;
			}
			
			if (n != slen) {
				switch (slen - n) {
					case 1:
						a = src[i + 0] & 0xff;
						dst[k + 0] = table[a >> 2];
						dst[k + 1] = table[(a & 3) << 4];
						dst[k + 2] = '=';
						dst[k + 3] = '=';
						break;
						
					case 2:
						a = src[i + 0] & 0xff;
						b = src[i + 1] & 0xff;
						dst[k + 0] = table[a >> 2];
						dst[k + 1] = table[((a & 3) << 4) | (b >> 4)];
						dst[k + 2] = table[(b & 0x0f) << 2];
						dst[k + 3] = '=';
						break;
				}
			}
			return rev;
		}

		/**
		 * @func encode_to_hex
		 * @private
		 */
		template <class Char>
		static Buffer encode_to_hex(const Char* source, uint32_t len) {
			cChar* hex = "0123456789abcdef";
			cChar* start = (cChar*)source;
			cChar* end = start + len * sizeof(Char);
			uint32_t byte_len = len * sizeof(Char) * 2;
			auto rev = Buffer::alloc(byte_len, byte_len + 1);
			char* data = *rev;
			while (start < end) {
				uint8_t ch = *start;
				data[0] = hex[ch >> 4];
				data[1] = hex[ch & 15];
				data+=2; start++;
			}
			(*rev)[byte_len] = '\0';
			return rev;
		}

		/**
		 * @func encode_to_ucs2
		 * @private
		 */
		template <class Char>
		static Buffer encode_to_ucs2(const Char* source, uint32_t len) {
			// ucs4到ucs2会丢失所有ucs2以外的编码
			const Char* end = source + len;
			auto rev = Buffer::alloc(len * 2, len * 2 + 1);
			char* data = *rev;
			while (source < end) {
				*reinterpret_cast<uint16_t*>(data) = *source;
				data += 2; source++;
			}
			*data = '\0';
			return rev;
		}

		/**
		 * @func encode_to_ucs4
		 * @private
		 */
		template <class Char>
		static Buffer encode_to_ucs4(const Char* source, uint32_t len) {
			const Char* end = source + len;
			auto rev = Buffer::alloc(len * 4, len * 4 + 1);
			char* data = *rev;
			while (source < end) {
				*reinterpret_cast<uint32_t*>(data) = *source;
				data += 4; source++;
			}
			*data = '\0';
			return rev;
		}

		static Buffer encode_with_buffer(Encoding target_en, cChar* source, uint32_t len) {
			// cChar* end = source + len;
			
			switch (target_en) {
				case Encoding::binary: {
					return encode_to_binary(source, len);
				}
				case Encoding::ascii: {
					return encode_to_ascii(source, len);
				}
				case Encoding::base64: {
					return encode_to_base64(source, len);
				}
				case Encoding::hex: {
					return encode_to_hex(source, len);
				}
				case Encoding::utf8: {
					return encode_to_utf8((const uint8_t*)source, len);
				}
				case Encoding::utf16: { // 暂时使用ucs2,ucs2不能包括所有字符编码
					//F_WARN("%s", "From ascii to ucs2 would be a waste of memory space.");
					return encode_to_ucs2(source, len);
				}
				case Encoding::ucs2: {  // 最小2字节编码
					//F_WARN("%s", "From ascii to ucs2 would be a waste of memory space.");
					return encode_to_ucs2(source, len);
				}
				case Encoding::utf32:
				case Encoding::ucs4: {  // 最小4字节编码
					//F_WARN("%s", "From ascii to ucs4 would be a waste of memory space.");
					return encode_to_ucs4(source, len);
				}
				default: F_ERR("%s", "Unknown encode."); break;
			}
			return Buffer();
		}

		static Buffer encode_with_uint16(Encoding target_en, const uint16_t* source, uint32_t len) {
			// const uint16* end = source + len;
			
			switch (target_en) {
				case Encoding::binary: {
					//F_WARN("%s", "Conversion from ucs2 to Binary will lose data.");
					return encode_to_binary(source, len);
				}
				case Encoding::ascii: {  // 会丢失编码
					//F_WARN("%s", "Conversion from ucs2 to ASCII will lose data.");
					return encode_to_ascii(source, len);
				}
				case Encoding::base64: {
					return encode_to_base64(source, len);
				}
				case Encoding::hex: {
					return encode_to_hex(source, len);
				}
				case Encoding::utf8: {
					return encode_to_utf8(source, len);
				}
				case Encoding::utf16: { // 暂时使用ucs2,ucs2不能包括所有字符编码
					//F_WARN("%s", "No need to convert form ucs2 to ucs2.");
					return encode_to_ucs2(source, len);
				}
				case Encoding::ucs2: { // 最小2字节编码
					//F_WARN("%s", "No need to convert form ucs2 to ucs2.");
					return encode_to_ucs2(source, len);
				}
				case Encoding::utf32:
				case Encoding::ucs4: { // 最小4字节编码
					//F_WARN("%s", "From ucs2 to ucs4 would be a waste of memory space.");
					return encode_to_ucs4(source, len);
				}
				default: F_ERR("%s", "Unknown encode."); break;
			}
			return Buffer();
		}

		static Buffer encode_with_uint32(Encoding target_en, const uint32_t* source, uint32_t len) {
			// const uint32* end = source + len;
			
			switch (target_en) {
				case Encoding::binary: {
					//F_WARN("%s", "Conversion from ucs4 to Binary will lose data.");
					return encode_to_binary(source, len);
				}
				case Encoding::ascii: {  // 会丢失编码
					//F_WARN("%s", "Conversion from ucs4 to ASCII will lose data.");
					return encode_to_ascii(source, len);
				}
				case Encoding::base64: {
					return encode_to_base64(source, len);
				}
				case Encoding::hex: {
					return encode_to_hex(source, len);
				}
				case Encoding::utf8: {
					return encode_to_utf8(source, len);
				}
				case Encoding::utf16: {    // 暂时使用ucs2,ucs2不能包括所有字符编码
					//F_WARN("%s", "Conversion from ucs4 to ucs2 will lose data.");
					return encode_to_ucs2(source, len);
				}
				case Encoding::ucs2: {
					//F_WARN("%s", "Conversion from ucs4 to ucs2 will lose data.");
					return encode_to_ucs2(source, len);
				}
				case Encoding::utf32:
				case Encoding::ucs4: {
					//F_WARN("%s", "No need to convert form ucs4 to ucs4.");
					return encode_to_ucs4(source, len);
				}
				default: F_ERR("%s", "Unknown encode."); break;
			}
			return Buffer();
		}

		// ============== decode ==============

		// 1字节 0xxxxxxx
		// 2字节 110xxxxx 10xxxxxx
		// 3字节 1110xxxx 10xxxxxx 10xxxxxx
		// 4字节 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		// 5字节 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		// 6字节 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		static int decode_utf8_word_length(char c) {

			if ((c & 0x80) == 0) { // 小于 128 (c & 10000000) == 00000000
				// uft8单字节编码 0xxxxxxx
				return 1;
			}
			else if ((c & 0xe0) == 0xc0) { // (c & 11100000) == 11000000
				// uft8双字节编码 110xxxxx 10xxxxxx
				return 2;
			}
			else if ((c & 0xf0) == 0xe0) { //(c & 11110000) == 11100000
				// uft8三字节编码 1110xxxx 10xxxxxx 10xxxxxx
				return 3;
			}
			else if ((c & 0xf8) == 0xf0) { // (c & 11111000) == 11110000
				// uft8四字节编码 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
				return 4;
			}
			else if ((c & 0xfc) == 0xf8) { // (c & 11111100) == 11111000
				// uft8五字节编码 , utf8最多可用6个字节表示31位二进制
				return 5;
			}
			else if ((c & 0xfe) == 0xfc) { // (c & 11111110) == 11111100
				return 6;
			}
			// 未知容错,算成一个长度
			return 1;
		}

		// uincode 长度
		static uint32_t decode_utf8_str_length(cChar* source, uint32_t len) {
			uint32_t rev = 0;
			cChar* end = source + len;
			while (source < end) {
				source += decode_utf8_word_length(*source);
				rev++;
			}
			return rev;
		}

		// 解码单个unicode
		template<class Char>
		static uint32_t decode_utf8_to_word(const uint8_t* str, Char* out) {

			uint32_t c = *str;
			str++;
			if ((c & 0x80) == 0) { // 小于 128 (c & 10000000) == 00000000
				//uft8单字节编码 0xxxxxxx
				*out = c;
				return 1;
			}
			else if ((c & 0xe0) == 0xc0) { // (c & 11100000) == 11000000
				//uft8双字节编码 110xxxxx 10xxxxxx
				uint32_t r_c = 0;
				uint32_t c2 = *str; str++;
				r_c |= (c2 & ~0xc0);
				r_c |= ((c & ~0xe0) << 6);
				*out = r_c;
				return 2;
			}
			else if ((c & 0xf0) == 0xe0) { //(c & 11110000) == 11100000
				//uft8三字节编码 1110xxxx 10xxxxxx 10xxxxxx
				uint32_t r_c = 0;
				uint32_t c2 = *str; str++;
				uint32_t c3 = *str; str++;
				r_c |= (c3 & ~0xc0);
				r_c |= ((c2 & ~0xc0) << 6);
				r_c |= ((c & ~0xf0) << 12);
				*out = r_c;
				return 3;
			}
			else if ((c & 0xf8) == 0xf0) { // (c & 11111000) == 11110000
				//uft8四字节编码 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
				uint32_t r_c = 0;
				uint32_t c2 = *str; str++;
				uint32_t c3 = *str; str++;
				uint32_t c4 = *str; str++;
				r_c |= (c4 & ~0xc0);
				r_c |= ((c3 & ~0xc0) << 6);
				r_c |= ((c2 & ~0xc0) << 12);
				r_c |= ((c & ~0xf8) << 18);
				*out = r_c;
				return 4;
			}
			else if ((c & 0xfc) == 0xf8) { // (c & 11111100) == 11111000
				//uft8五字节编码 , utf8最多可用6个字节表示31位二进制
				uint32_t r_c = 0;
				uint32_t c2 = *str; str++;
				uint32_t c3 = *str; str++;
				uint32_t c4 = *str; str++;
				uint32_t c5 = *str; str++;
				r_c |= (c5 & ~0xc0);
				r_c |= ((c4 & ~0xc0) << 6);
				r_c |= ((c3 & ~0xc0) << 12);
				r_c |= ((c2 & ~0xc0) << 18);
				r_c |= ((c & ~0xfc) << 24);
				*out = r_c;
				return 5;
			}
			else if ((c & 0xfe) == 0xfc) { // (c & 11111110) == 11111100
				//uft8六字节编码
				uint32_t r_c = 0;
				uint32_t c2 = *str; str++;
				uint32_t c3 = *str; str++;
				uint32_t c4 = *str; str++;
				uint32_t c5 = *str; str++;
				uint32_t c6 = *str; str++;
				r_c |= (c6 & ~0xc0);
				r_c |= ((c5 & ~0xc0) << 6);
				r_c |= ((c4 & ~0xc0) << 12);
				r_c |= ((c3 & ~0xc0) << 18);
				r_c |= ((c2 & ~0xc0) << 24);
				r_c |= ((c & ~0xfe) << 30);
				*out = r_c;
				return 6;
			}
			return 1;
		}

		template <class Char>
		static ArrayBuffer<Char> decode_from_binary(cChar* source, uint32_t len) {
			auto rev = ArrayBuffer<Char>::alloc(len, len + 1);
			Char* data = *rev;
			cChar* end = source + len;
			while (source < end) {
				*data = *source;
				data++; source++;
			}
			*data = '\0';
			return rev;
		}

		template <class Char>
		static ArrayBuffer<Char> decode_from_ascii(cChar* source, uint32_t len) {
			auto rev = ArrayBuffer<Char>::alloc(len, len + 1);
			Char* data = *rev;
			cChar* end = source + len;
			while (source < end) {
				*data = *(const uint8_t*)source % 128;
				data++; source++;
			}
			*data = '\0';
			return rev;
		}

		// Doesn't check for padding at the end.  Can be 1-2 bytes over.
		static inline uint32_t base64_decoded_size_fast(uint32_t size) {
			uint32_t remainder = size % 4;
			
			size = (size / 4) * 3;
			if (remainder) {
				if (size == 0 && remainder == 1) {
					// special case: 1-byte input cannot be decoded
					size = 0;
				} else {
					// non-padded input, add 1 or 2 extra bytes
					size += 1 + (remainder == 3);
				}
			}
			
			return size;
		}

		// supports regular and URL-safe base64
		static int16_t unbase64_table[] = {
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2, -1, -1, -2, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, 62, -1, 63,
			52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
			-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
			15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, 63,
			-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
			41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
		};
		#define unbase64(x) unbase64_table[(uint8_t)(x)]

		static int16_t unhex_table[] = {
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
			-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
		};
		#define unhex(x) unhex_table[(uint8_t)(x)]

		template <class Char>
		static ArrayBuffer<Char> decode_from_base64(cChar* src, uint32_t length) {
			char a, b, c, d;
			
			uint32_t len = base64_decoded_size_fast(length);
			Char* buf = (Char*)::malloc(len + 1);
			Char* dst = buf;
			Char* dstEnd = dst + len;
			cChar* srcEnd = src + length;
			
			while (dst < dstEnd) {
				long remaining = srcEnd - src;
				
				while (unbase64(*src) < 0 && src < srcEnd)
					src++, remaining--;
				if (remaining == 0 || *src == '=')
					break;
				a = unbase64(*src++);
				
				while (unbase64(*src) < 0 && src < srcEnd)
					src++, remaining--;
				if (remaining <= 1 || *src == '=')
					break;
				b = unbase64(*src++);
				
				*dst++ = (a << 2) | ((b & 0x30) >> 4);
				if (dst == dstEnd)
					break;
				
				while (unbase64(*src) < 0 && src < srcEnd)
					src++, remaining--;
				if (remaining <= 2 || *src == '=')
					break;
				c = unbase64(*src++);
				
				*dst++ = ((b & 0x0F) << 4) | ((c & 0x3C) >> 2);
				if (dst == dstEnd)
					break;
				
				while (unbase64(*src) < 0 && src < srcEnd)
					src++, remaining--;
				if (remaining <= 3 || *src == '=')
					break;
				d = unbase64(*src++);
				
				*dst++ = ((c & 0x03) << 6) | (d & 0x3F);
			}
			
			len -= (dstEnd - dst);
			*dst = '\0';
			
			return ArrayBuffer<Char>::from(buf, len);
		}

		template <class Char>
		static ArrayBuffer<Char> decode_from_hex(cChar* source, uint32_t len) {
			if (len % 2 != 0) { // hex 编码数据错误
				return ArrayBuffer<Char>();
			}
			auto rev = ArrayBuffer<Char>::alloc(len / 2, len / 2 + 1);
			cChar* end = source + len;
			Char* data = *rev;
			while (source < end) {
				uint8_t ch0 = (uint8_t)unhex(source[0]);
				uint8_t ch1 = (uint8_t)unhex(source[1]);
				*data = (ch0 * 16 + ch1);
				data++; source+=2;
			}
			*data = '\0';
			return rev;
		};

		template <class Char>
		static ArrayBuffer<Char> decode_from_utf8(cChar* source, uint32_t len) {
			uint32_t rev_len = decode_utf8_str_length(source, len);
			auto rev = ArrayBuffer<Char>::alloc(rev_len, rev_len + 1);
			Char* data = *rev;
			
			cChar* end = source + len;
			while (source < end) {
				source += decode_utf8_to_word(reinterpret_cast<const uint8_t*>(source), data);
				data++;
			}
			*data = '\0';
			return rev;
		}

		template <class Char>
		static ArrayBuffer<Char> decode_from_ucs2(cChar* source, uint32_t len) {
			auto rev = ArrayBuffer<Char>::alloc(len, len + 1);
			Char* data = *rev;
			cChar* end = source + len;
			while (source < end) {
				*data = *reinterpret_cast<const Char*>(source);
				data++; source += 2;
			}
			*data = '\0';
			return rev;
		}

		template <class Char>
		static ArrayBuffer<Char> decode_from_ucs4(cChar* source, uint32_t len) {
			auto rev = ArrayBuffer<Char>::alloc(len, len + 1);
			Char* data = *rev;
			cChar* end = source + len;
			while (source < end) {
				*data = *reinterpret_cast<const Char*>(source);
				data++; source += 4;
			}
			*data = '\0';
			return rev;
		}

		static Buffer decode_to_buffer(Encoding source_en, cChar* source, uint32_t len) {
			switch (source_en) {
				case Encoding::binary: {
					return decode_from_binary<char>(source, len);
				}
				case Encoding::ascii: {
					return decode_from_ascii<char>(source, len);
				}
				case Encoding::base64: {
					return decode_from_base64<char>(source, len);
				}
				case Encoding::hex: {
					return decode_from_hex<char>(source, len);
				}
				case Encoding::utf8: { // 会丢失ascii外的编码
					//F_WARN("%s", "Conversion from utf8 to ascii will lose data.");
					return decode_from_utf8<char>(source, len);
				}
				case Encoding::utf16: { // 暂时使用ucs2
					//F_WARN("%s", "Conversion from utf16 to ascii will lose data.");
					return decode_from_ucs2<char>(source, len);
				}
				case Encoding::ucs2: { // 会丢失ascii外的编码
					//F_WARN("%s", "Conversion from ucs2 to ascii will lose data.");
					return decode_from_ucs2<char>(source, len);
				}
				case Encoding::utf32:
				case Encoding::ucs4: { // 会丢失ascii外的编码
					//F_WARN("%s", "Conversion from ucs4 to ascii will lose data.");
					return decode_from_ucs4<char>(source, len);
				}
				default: F_ERR("%s", "Unknown encode."); break;
			}
			return Buffer();
		}

		static ArrayBuffer<uint16_t> decode_to_uint16(Encoding source_en, cChar* source, uint32_t len) {
			switch (source_en) {
				case Encoding::binary: {
					return decode_from_binary<uint16_t>(source, len);
				}
				case Encoding::ascii: {
					return decode_from_ascii<uint16_t>(source, len);
				}
				case Encoding::base64: {
					return decode_from_base64<uint16_t>(source, len);
				}
				case Encoding::hex: {
					return decode_from_hex<uint16_t>(source, len);
				}
				case Encoding::utf8: { // 会丢失ucs2外的编码
					// F_WARN("%s", "Conversion from utf8 to ucs2 will lose data.");
					return decode_from_utf8<uint16_t>(source, len);
				}
				case Encoding::utf16: { // 暂时使用ucs2
					//F_WARN("%s", "Conversion from utf16 to ucs2 will lose data.");
					return decode_from_ucs2<uint16_t>(source, len);
				}
				case Encoding::ucs2: { //
					//F_WARN("%s", "No need to convert form ucs2 to ucs2.");
					return decode_from_ucs2<uint16_t>(source, len);
				}
				case Encoding::utf32:
				case Encoding::ucs4: { // 会丢失ucs2外的编码
					//F_WARN("%s", "Conversion from ucs4 to ucs2 will lose data.");
					return decode_from_ucs4<uint16_t>(source, len);
				}
				default: F_ERR("%s", "Unknown encode."); break;
			}
			return ArrayBuffer<uint16_t>();
		}

		static ArrayBuffer<uint32_t> decode_to_uint32(Encoding source_en, cChar* source, uint32_t len) {
			switch (source_en) {
				case Encoding::binary: {
					return decode_from_binary<uint32_t>(source, len);
				}
				case Encoding::ascii: {
					return decode_from_ascii<uint32_t>(source, len);
				}
				case Encoding::base64:{
					return decode_from_base64<uint32_t>(source, len);
				}
				case Encoding::hex: {
					return decode_from_hex<uint32_t>(source, len);
				}
				case Encoding::utf8: {
					return decode_from_utf8<uint32_t>(source, len);
				}
				case Encoding::utf16:
				case Encoding::ucs2: {
					return decode_from_ucs2<uint32_t>(source, len);
				}
				case Encoding::utf32:
				case Encoding::ucs4: {
					//F_WARN("%s", "No need to convert form ucs4 to ucs4.");
					return decode_from_ucs4<uint32_t>(source, len);
					break;
				}
				default: F_ERR("%s", "Unknown encode."); break;
			}
			return ArrayBuffer<uint32_t>();
		}

	}

	static Dict<String, Encoding> init_parse_encoding() {
		Dict<String, Encoding> encodes;
		encodes["binary"] = Encoding::binary;
		encodes["ascii"] = Encoding::ascii;
		encodes["base64"] = Encoding::base64;
		encodes["hex"] = Encoding::hex;
		encodes["utf8"] = Encoding::utf8;
		encodes["utf-8"] = Encoding::utf8;
		encodes["ucs2"] = Encoding::ucs2;
		encodes["ucs4"] = Encoding::ucs4;
		encodes["utf16"] = Encoding::utf16;
		encodes["utf-16"] = Encoding::utf16;
		encodes["utf32"] = Encoding::utf32;
		encodes["utf-32"] = Encoding::utf32;
		encodes["unknown"] = Encoding::unknown;
		return encodes;
	}

	static Dict<uint32_t, String> init_encoding_string() {
		Dict<uint32_t, String> strs;
		strs[(uint32_t)Encoding::binary] = "binary";
		strs[(uint32_t)Encoding::ascii] = "ascii";
		strs[(uint32_t)Encoding::base64] = "base64";
		strs[(uint32_t)Encoding::hex] = "hex";
		strs[(uint32_t)Encoding::utf8] = "utf8";
		strs[(uint32_t)Encoding::ucs2] = "ucs2";
		strs[(uint32_t)Encoding::ucs4] = "ucs4";
		strs[(uint32_t)Encoding::utf16] = "utf16";
		strs[(uint32_t)Encoding::utf32] = "utf32";
		strs[(uint32_t)Encoding::unknown] = "unknown";
		return strs;
	}

	Encoding Codec::parse_encoding(cString& en) {
		static const Dict<String, Encoding> encodes(init_parse_encoding());
		String encode = en.to_lower_case();
		
		auto i = encodes.find(en.to_lower_case());
		if (i != encodes.end()) {
			return i->value;
		}
		return Encoding::unknown;
	}

	String Codec::encoding_string(Encoding en) {
		static const Dict<uint32_t, String> strs(init_encoding_string());
		static cString unknown = "unknown";
		auto i = strs.find((uint32_t)en);
		if (i != strs.end()) {
			return i->value;
		}
		return unknown;
	}

	ArrayBuffer<char> Codec::encode(Encoding target_en, const ArrayBuffer<char>& source) {
		return codec::encode_with_buffer(target_en, *source, source.length());
	}

	ArrayBuffer<char> Codec::encode(Encoding target_en, const ArrayBuffer<uint16_t>& source) {
		return codec::encode_with_uint16(target_en, *source, source.length());
	}

	ArrayBuffer<char> Codec::encode(Encoding target_en, const ArrayBuffer<uint32_t>& source) {
		return codec::encode_with_uint32(target_en, *source, source.length());
	}

	ArrayBuffer<char> Codec::encode(Encoding target_en, const ArrayString<char>& source) {
		return codec::encode_with_buffer(target_en, source.c_str(), source.length());
	}

	ArrayBuffer<char> Codec::encode(Encoding target_en, const ArrayString<uint16_t>& source) {
		return codec::encode_with_uint16(target_en, source.c_str(), source.length());
	}

	ArrayBuffer<char> Codec::encode(Encoding target_en, const ArrayString<uint32_t>& source) {
		return codec::encode_with_uint32(target_en, source.c_str(), source.length());
	}

	ArrayBuffer<char> Codec::decode_to_buffer(Encoding source_en, const ArrayString<char>& source) {
		return codec::decode_to_buffer(source_en, source.c_str(), source.length());
	}

	ArrayBuffer<uint16_t> Codec::decode_to_uint16(Encoding source_en, const ArrayString<char>& source) {
		return codec::decode_to_uint16(source_en, source.c_str(), source.length());
	}

	ArrayBuffer<uint32_t> Codec::decode_to_uint32(Encoding source_en, const ArrayString<char>& source) {
		return codec::decode_to_uint32(source_en, source.c_str(), source.length());
	}

	ArrayBuffer<char> Codec::decode_to_buffer(Encoding source_en, const ArrayBuffer<char>& source) {
		return codec::decode_to_buffer(source_en, *source, source.length());
	}

	ArrayBuffer<uint16_t> Codec::decode_to_uint16(Encoding source_en, const ArrayBuffer<char>& source) {
		return codec::decode_to_uint16(source_en, *source, source.length());
	}

	ArrayBuffer<uint32_t> Codec::decode_to_uint32(Encoding source_en, const ArrayBuffer<char>& source) {
		return codec::decode_to_uint32(source_en, *source, source.length());
	}

}
