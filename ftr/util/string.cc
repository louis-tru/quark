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

#include "./string.h"
#include "./codec.h"
#include <algorithm>

namespace ftr {

	cChar _Str::ws[8] = {
		0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x20, /*0xA0,*/ 0x0
	};

	cChar test_big_Char[] = { 1, 0, 0, 0 };
	const int* test_big_int = (const int*)test_big_Char;
	const bool is_big_data = *test_big_int != 1;

	static void assign(void* l, const void* r, int len) {
		switch (len) {
			case 1:
				*static_cast<Char*>(l) = *static_cast<cChar*>(r);
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

	void _Str::strcp(void* o_, int sizeof_o, const void* i_, int sizeof_i, uint32_t len) {
		char* o = (char*)o_;
		char* i = (char*)i_;
		if (len && i) {
			if (sizeof_o == sizeof_i) {
				::memcpy(o, i, len * sizeof_o);
				((char*)o)[len] = '\0';
			} else {
				int min = FX_MIN(sizeof_o, sizeof_i);
				int max = FX_MIN(sizeof_o, sizeof_i);
				if (is_big_data) { // big data layout
					for (int j = 0; j < len; j++) {
						assign(o, i + max - min, min);
						o+=sizeof_o; i+=sizeof_i;
					}
				} else {
					for (int j = 0; j < len; j++) {
						assign(o, i, min);
						o+=sizeof_o; i+=sizeof_i;
					}
				}
				::memset(o, 0, sizeof_o);
			}
		}
	}
	
	bool _Str::sscanf(const void* i, int sizeof_i, int len_i, const char* f, ...) {
		bool r;
		va_list arg;
		va_start(arg, f);
		if (sizeof_i == 1) {
			r = vsscanf( (const char*)i, f, arg);
		} else {
			Char i2[65];
			len_i = FX_MIN(len_i, 64);
			_Str::strcp(i2, 1, i, sizeof_i, len_i);
			r = vsscanf( i2, f, arg );
		}
		va_end(arg);
		return r;
	}

	bool _Str::to_number(const void* i, int sizeof_i, int len, int32_t* o) {
		return _Str::sscanf(i, sizeof_i, len, "%d", o);
	}
	
	bool _Str::to_number(const void* i, int size_of, int len, uint32_t* o) {
		return _Str::sscanf(i, size_of, len, "%u", o);
	}

	bool _Str::to_number(const void* i, int sizeof_i, int len, int64_t* o) {
		#if FX_ARCH_64BIT
			return _Str::sscanf(i, sizeof_i, len, "%ld", o);
		#else
			return _Str::sscanf(i, sizeof_i, len, "%lld", o);
		#endif
	}

	bool _Str::to_number(const void* i, int size_of, int len, uint64_t* o) {
		#if FX_ARCH_64BIT
			return _Str::sscanf(i, size_of, len, "%lu", o);
		#else
			return _Str::sscanf(i, size_of, len, "%llu", o);
		#endif
	}

	bool _Str::to_number(const void* i, int size_of, int len, float* o) {
		return _Str::sscanf(i, size_of, len, "%fd", o);
	}

	bool _Str::to_number(const void* i, int size_of, int len, double* o) {
		return _Str::sscanf(i, size_of, len, "%lf", o);
		// _str->_length = _Str::sprintf(&_str->_val, &_str->_capacity, "%g", i);
	}

	uint32_t _Str::strlen(const void* s_, int size_of) {
		const char* s = (const char*)s_;
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

	int _Str::memcmp(const void* s1, const void* s2, uint32_t len, int size_of) {
		return ::memcmp(s1, s2, len * size_of);
	}

	int _Str::index_of(
		const void* s1_, uint32_t s1_len, const void* s2_,
		uint32_t s2_len, uint32_t start, int size_of
	) {
		if (s1_len < s2_len) return -1;
		if (start + s2_len > s1_len) return -1;

		const char* s1 = (const char*)s1_;
		const char* s2 = (const char*)s2_;

		int32_t end = s1_len - s2_len + 1;

		while ( start < end ) {
			if (_Str::memcmp(s1 + (start * size_of), s2, s2_len, size_of) == 0) {
				return start;
			}
			start++;
		}
		return -1;
	}

	int _Str::last_index_of(
		const void* s1_, uint32_t s1_len, const void* s2_,
		uint32_t s2_len, uint32_t _start, int size_of
	) {
		const char* s1 = (const char*)s1_;
		const char* s2 = (const char*)s2_;
		int32_t start = _start;
		if ( start + s2_len > s1_len )
			start = s1_len - s2_len;
		while ( start > -1 ) {
			if (_Str::memcmp(s1 + (start * size_of), s2, s2_len, size_of) == 0) {
				return start;
			}
			start--;
		}
		return -1;
	}

	struct _StrTmp {
		void realloc(uint32_t capacity) {
			_val = (char*)AllocatorDefault::realloc(
				_val, capacity, &_capacity, sizeof(Char));
		}
		uint32_t _capacity;
		Char*    _val;
	};

	void* _Str::replace(
		const void* s1_, uint32_t s1_len,
		const void* s2_, uint32_t s2_len,
		const void* rep_, uint32_t rep_len,
		int size_of, uint32_t* out_len, uint32_t* capacity_out, bool all
	) {
		_StrTmp s_tmp;
		uint32_t s_tmp_to = 0;
		uint32_t from = 0;
		int32_t  find, before_len;
		
		const char* s1 = (const char*)s1_;
		const char* s2 = (const char*)s2_;
		const char* rep = (const char*)rep_;

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

	// ---------------------------------------------------------------------

	int32_t vsnprintf(char* o, uint32_t capacity_o, cChar* f, va_list arg) {
		return ::vsnprintf(o, capacity_o, f, arg);
	}
	
	int32_t vasprintf(char** o, cChar* f, va_list arg) {
		return ::vasprintf(o, f, arg);
	}
	
	int32_t snprintf(void* o, int sizeof_o, uint32_t capacity_o, uint32_t* len_o, cChar* f, ...) {
		va_list arg;
		va_start(arg, f);
		int r;
		if (sizeof_o == 1) {
			r = vsnprintf((char*)o, capacity_o, f, arg);
		} else {
			// TODO ...
		}
		va_end(arg);
		
		
		
		return r;
	}

	void* _Str::sprintf(void* o, int sizeof_o, int capacity_o, int* len_o, cChar* f, ...) {
		va_list arg;
		va_start(arg, f);

		void* r;
		
		if (sizeof_o == 1) {
			if (o) {
				*len_o = vasprintf((char*)o, capacity_o, f, arg);
			} else {
				char* o2;
				*len_o = vasprintf(&o2, f, arg);
			}
		} else {
			//			char* str;
			//			int len = vasprintf(&str, f, arg);
			//			if (sizeof_o == 2) {
			//				auto b = Codec::decode_to_uint16(Encoding::UTF8, Buffer::from(str, len));
			//				*len_o = b.length();
			//				r = b.collapse();
			//			} else if (sizeof_o == 4) {
			//				auto b = Codec::decode_to_uint32(Encoding::UTF8, Buffer::from(str, len));
			//				*len_o = b.length();
			//				r = b.collapse();
			//			} else {
			//				FX_FATAL("I won't support it");
			//			}
		}
		va_end(arg);
		
		return r;
	}

	int32_t sprintf(Char** o, uint32_t* capacity, cChar* f, ...) {
		va_list arg;
		va_start(arg, f);
		int32_t len = vasprintf(o, f, arg);
		va_end(arg);
		if (o && capacity) {
			*capacity = len + 1;
		}
		return len;
	}

	String string_format(cChar* f, va_list arg) {
		Char* buf = nullptr;
		int len = ftr::vasprintf(&buf, f, arg);
		if (buf) {
			return Buffer::from(buf, len).collapse_string();
		} else {
			return String();
		}
	}

	template <>
	String String::format(cChar* f, ...) {
		va_list arg;
		va_start(arg, f);
		String str = string_format(f, arg);
		va_end(arg);
		return str;
	}
}
