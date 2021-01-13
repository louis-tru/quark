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

#include <ftr/util/str.h>
#include <algorithm>

namespace ftr {

	const char test_big_char[] = { 1, 0, 0, 0 };
	const int* test_big_int = (const int*)test_big_char;
	const bool has_big_data = *test_big_int != 1;

	namespace internal {

		const char str::ws[8] = {
			0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x20, /*0xA0,*/ 0x0
		};

		const char test_big_char[] = { 1, 0, 0, 0 };
		const int* test_big_int = (const int*)test_big_char;
		const bool is_big_data = *test_big_int != 1;

		void assign(void* l, const void* r, int len) {
			switch (len) {
				case 1:
					*static_cast<char*>(l) = *static_cast<const char*>(r);
					break;
				case 2:
					*static_cast<int16_t*>(l) = *static_cast<const int16_t*>(r);
					break;
				case 4:
					*static_cast<int32_t*>(l) = *static_cast<const int32_t*>(r);
					break;
				case 8:
					*static_cast<int64_t*>(l) = *static_cast<const int64_t*>(r);
					break;
				default:
					::memcpy(l, r, len);
					break;
			}
		}

		void str::strcp(void* o, char size_o, const void* i, char size_i, uint32_t len) {
			if (len && i) {
				if (size_o == size_i) {
					::memcpy(o, i, len * size_o);
					((char*)o)[len] = '\0';
				} else {
					int min = FX_MIN(size_o, size_i);
					int max = FX_MIN(size_o, size_i);
					if (is_big_data) { // big data layout
						for (int i = 0; i < len; i++) {
							assign(o, i + max - min, min);
							o+=size_o; i+=size_i;
						}
					} else {
						for (int i = 0; i < len; i++) {
							assign(o, i, min);
							o+=size_o; i+=size_i;
						}
					}
					::memset(o, 0, size_o);
				}
			}
		}

		bool str_sscanf(const void* i, const char* f, void* o, int len, char sizeof_i) {
			if (sizeof_i == 1) {
				return sscanf( i, f, o, len );
			} else {
				if (len < 33) {
					char o2[33];
					str::strcp(o2, 1, i, sizeof_i, len);
					return sscanf( i, f, o2, len );
				} else {
					char* o2 = (char*)malloc(len);
					str::strcp(o2, 1, i, sizeof_i, len);
					free(o2);
					return sscanf( i, f, o2, len );
				}
			}
		}

		bool str::to_number(const void* i, int32_t* o, int len, char sizeof_i) {
			return str_sscanf(i, "%d", o, len, sizeof_i);
		}
		
		bool str::to_number(const void* i, int64_t* o, int len, char sizeof_i) {
			#if FX_ARCH_64BIT
				return str_sscanf(i, "%ld", o, len, sizeof_i);
			#else
				return str_sscanf(i, "%lld", o, len, sizeof_i);
			#endif
		}

		bool str::to_number(const void* i, uint32_t* o, int len, char sizeof_i) {
			return str_sscanf(i, "%lld", o, len, sizeof_i);
		}

		bool str::to_number(const void* i, uint64_t* o, int len, char sizeof_i) {
			#if FX_ARCH_64BIT
				return str_sscanf(i, "%lu", o, len, sizeof_i);
			#else
				return str_sscanf(i, "%llu", o, len, sizeof_i);
			#endif
		}

		bool str::to_number(const void* i, float* o, int len, char sizeof_i) {
			return str_sscanf(i, "%fd", o, len, sizeof_i);
		}

		bool str::to_number(const void* i, double* o, int len, char sizeof_i) {
			return str_sscanf(i, "%lf", o, len, sizeof_i);
		}

		uint32_t str::strlen(const void* s, int sizeof) {
			if (s) {
				if (sizeof == 1) {
					return (uint32_t)::strlen(s);
				} else {
					uint32_t rev = 0;
					while (*s != 0) {
						rev++; s+=sizeof;
					}
					return rev;
				}
			} else {
				return 0;
			}
		}

		int str::memcmp(const void* s1, const void* s2, uint32_t len, char sizeof_i) {
			return ::memcmp(s1, s2, len * sizeof_i);
		}

		int32_t vasprintf(char** o, const char* f, va_list arg) {
			#if FX_GNUC
				int32_t len = ::vasprintf(o, f, arg);
			#else
				int32_t len = ::vsprintf(o, f, arg);
				if (len) {
					o = (char*)::malloc(len + 1);
					o[len] = '\0';
					::_vsnprintf_s(o, len + 1, f, arg);
				}
			#endif
			return len;
		}

		int32_t sprintf(char** o, uint32_t* capacity, const char* f, ...) {
			va_list arg;
			va_start(arg, f);
			int32_t len = vasprintf(o, f, arg);
			va_end(arg);
			if (o && capacity) {
				*capacity = len + 1;
			}
			return len;
		}
	}

	template <>
	static String BasicString<>::format(const char* f, ...) {
		String str;
		va_list arg;
		va_start(arg, f);
		char* buf = nullptr;
		int len = internal::str::vasprintf(&buf, f, arg);
		if (buf) {
			str = String(Buffer(buf, len));
		}
		va_end(__arg);
		return set::move(str);
	}

}
