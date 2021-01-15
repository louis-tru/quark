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

	namespace internal {

		const char str::ws[8] = {
			0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x20, /*0xA0,*/ 0x0
		};

		const char test_big_char[] = { 1, 0, 0, 0 };
		const int* test_big_int = (const int*)test_big_char;
		const bool is_big_data = *test_big_int != 1;

		static void assign(void* l, const void* r, int len) {
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

		void str::strcp(char* o, int size_o, const char* i, int size_i, uint32_t len) {
			if (len && i) {
				if (size_o == size_i) {
					::memcpy(o, i, len * size_o);
					((char*)o)[len] = '\0';
				} else {
					int min = FX_MIN(size_o, size_i);
					int max = FX_MIN(size_o, size_i);
					if (is_big_data) { // big data layout
						for (int j = 0; j < len; j++) {
							assign(o, i + max - min, min);
							o+=size_o; i+=size_i;
						}
					} else {
						for (int j = 0; j < len; j++) {
							assign(o, i, min);
							o+=size_o; i+=size_i;
						}
					}
					::memset(o, 0, size_o);
				}
			}
		}

		static bool str_sscanf(const char* i, const char* f, void* o, int len, int sizeof_i) {
			if (sizeof_i == 1) {
				return sscanf( i, f, o, len );
			} else {
        char o2[65];
        len = FX_MIN(len, 64);
        str::strcp(o2, 1, i, sizeof_i, len);
        return sscanf( o2, f, o, len );
			}
		}

		bool str::to_number(const char* i, int32_t* o, int len) {
			return str_sscanf(i, "%d", o, len, sizeof(int32_t));
		}
		
		bool str::to_number(const char* i, int64_t* o, int len) {
			#if FX_ARCH_64BIT
				return str_sscanf(i, "%ld", o, len, sizeof(int64_t));
			#else
				return str_sscanf(i, "%lld", o, len, sizeof(int64_t));
			#endif
		}

		bool str::to_number(const char* i, uint32_t* o, int len) {
			return str_sscanf(i, "%lld", o, len, sizeof(uint32_t));
		}

		bool str::to_number(const char* i, uint64_t* o, int len) {
			#if FX_ARCH_64BIT
				return str_sscanf(i, "%lu", o, len, sizeof(uint64_t));
			#else
				return str_sscanf(i, "%llu", o, len, sizeof(uint64_t));
			#endif
		}

		bool str::to_number(const char* i, float* o, int len) {
			return str_sscanf(i, "%fd", o, len, sizeof(float));
		}

		bool str::to_number(const char* i, double* o, int len) {
			return str_sscanf(i, "%lf", o, len, sizeof(double));
		}

		uint32_t str::strlen(const char* s, int size_of) {
			if (s) {
				if (size_of == 1) {
					return (uint32_t)::strlen(s);
				} else {
					uint32_t rev = 0;
					while (*s != 0) {
						rev++; s+=size_of;
					}
					return rev;
				}
			} else {
				return 0;
			}
		}

		int str::memcmp(const char* s1, const char* s2, uint32_t len, int size_of) {
			return ::memcmp(s1, s2, len * size_of);
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
  
    MutableString string_format(const char* f, va_list arg) {
      MutableString str;
      char* buf = nullptr;
      int len = internal::vasprintf(&buf, f, arg);
      if (buf) {
//        str = Buffer(buf, len).collapse_string();
      }
      return str;
    }

		int32_t str::sprintf(char** o, uint32_t* capacity, const char* f, ...) {
			va_list arg;
			va_start(arg, f);
			int32_t len = vasprintf(o, f, arg);
			va_end(arg);
			if (o && capacity) {
				*capacity = len + 1;
			}
			return len;
		}

		int str::index_of(const char* s1, uint32_t s1_len, const char* s2, uint32_t s2_len, uint32_t start, int size_of) {
			if (s1_len < s2_len) return -1;
			if (start + s2_len > s1_len) return -1;

			int32_t end = s1_len - s2_len + 1;

			while ( start < end ) {
				if (str::memcmp(s1 + (start * size_of), s2, s2_len, size_of) == 0) {
					return start;
				}
				start++;
			}
			return -1;
		}

		int str::last_index_of(const char* s1, uint32_t s1_len, const char* s2, uint32_t s2_len, uint32_t _start, int size_of) {
			int32_t start = _start;
			if ( start + s2_len > s1_len )
				start = s1_len - s2_len;
			while ( start > -1 ) {
				if (str::memcmp(s1 + (start * size_of), s2, s2_len, size_of) == 0) {
					return start;
				}
				start--;
			}
			return -1;
		}

		struct _Str {
			void realloc(uint32_t capacity) {
				capacity = FX_MAX(FX_MIN_CAPACITY, capacity);
				if ( capacity > _capacity || capacity < _capacity / 4.0 ) {
					capacity = powf(2, ceil(log2(capacity)));
					uint32_t size = sizeof(char) * capacity;
					_capacity = capacity;
					_val = static_cast<char*>(_val ? ::realloc(_val, size) : ::malloc(size));
				}
				ASSERT(_val);
			}

			uint32_t _capacity;
			char*    _val;
		};

		char* str::replace(
			const char* s1, uint32_t s1_len,
			const char* s2, uint32_t s2_len,
			const char* rep, uint32_t rep_len,
			int size_of, uint32_t* out_len, uint32_t* capacity_out, bool all
		) {
			_Str s_tmp;
			uint32_t s_tmp_to = 0;
			uint32_t from = 0;
			int32_t  find, before_len;

			while ((find = index_of(s1, s1_len, s2, s2_len, from, size_of)) != -1) {
				before_len = find - from;
        s_tmp.realloc((s_tmp_to + before_len + rep_len + 1) * size_of); // realloc

				if (before_len) {
					::memcpy(
						s_tmp._val + s_tmp_to * size_of,  // to
						s1         + from     * size_of,  // from
						before_len            * size_of   // size
					);
					s_tmp_to += before_len;
					from += before_len;
				}
				::memcpy(s_tmp._val + s_tmp_to * size_of, rep, rep_len * size_of);
				s_tmp_to += rep_len;
				from += s2_len;

				if (!all) {
					break;
				}
			}

			before_len = s1_len - from;
			s_tmp.realloc((s_tmp_to + before_len + 1) * size_of);

			::memcpy(
				s_tmp._val + s_tmp_to * size_of,  // to
				s1         + from     * size_of,  // from
				before_len            * size_of   // size
			);
			s_tmp_to += before_len;

			::memset(s_tmp._val + s_tmp_to * size_of, 0, size_of);

			*capacity_out = s_tmp._capacity;
			*out_len = s_tmp_to;
			return s_tmp._val;
		}
	}

	template <>
  MutableString BasicString<>::format(const char* f, ...) {
    va_list arg;
    va_start(arg, f);
    MutableString str = internal::string_format(f, arg);
    va_end(arg);
    return str;
	}

	template <>
	BasicString<char>::BasicString(const char* s, uint32_t len)
		: ArrayBuffer<char, HolderMode::kWeak>(const_cast<char*>(s), len) {
	}

	template <>
	BasicString<uint16_t>::BasicString(const uint16_t* s, uint32_t len)
		: ArrayBuffer<uint16_t, HolderMode::kWeak>(const_cast<uint16_t*>(s), len) {
	}

	template <>
	BasicString<uint32_t>::BasicString(const uint32_t* s, uint32_t len)
		: ArrayBuffer<uint32_t, HolderMode::kWeak>(const_cast<uint32_t*>(s), len) {
	}

}
