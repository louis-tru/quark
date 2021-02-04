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
#include "./handle.h"

namespace ftr {

	cChar _Str::ws[8] = {
		0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x20, /*0xA0,*/ 0x0
	};

	cChar test_big_Char[] = { 1, 0, 0, 0 };
	const int* test_big_int = (const int*)test_big_Char;
	const bool is_big_data = *test_big_int != 1;

	void _Str::strcpy(void* o_, int sizeof_o, const void* i_, int sizeof_i, uint32_t len) {
		char* o = (char*)o_;
		char* i = (char*)i_;
		if (len && i) {
			if (sizeof_o == sizeof_i) {
				::memcpy(o, i, len * sizeof_o);
				::memset(((char*)o) + len * sizeof_o, 0, sizeof_o);
			} else {
				int min = FX_MIN(sizeof_o, sizeof_i);
				int max = FX_MIN(sizeof_o, sizeof_i);
				if (is_big_data) { // big data layout
					for (int j = 0; j < len; j++) {
						::memcpy(o, i + max - min, min); // Copy low order only
						o+=sizeof_o; i+=sizeof_i;
					}
				} else {
					for (int j = 0; j < len; j++) {
						::memcpy(o, i, min); // Copy low order only
						o+=sizeof_o; i+=sizeof_i;
					}
				}
				::memset(o, 0, sizeof_o);
			}
		}
	}
	
	bool _to_number(const void* i, int sizeof_i, int len_i, const char* f, void* o) {
		if (sizeof_i == 1) {
			return sscanf( (const char*)i, f, o);
		} else {
			Char i_[65];
			_Str::strcpy(i_, 1, i, sizeof_i, FX_MIN(len_i, 64));
			return sscanf( i_, f, o );
		}
	}

	bool _Str::to_number(const void* i, int sizeof_i, int len, int32_t* o) {
		return _to_number(i, sizeof_i, len, "%d", o);
	}
	
	bool _Str::to_number(const void* i, int size_of, int len, uint32_t* o) {
		return _to_number(i, size_of, len, "%u", o);
	}

	bool _Str::to_number(const void* i, int sizeof_i, int len, int64_t* o) {
		#if FX_ARCH_64BIT
			return _to_number(i, sizeof_i, len, "%ld", o);
		#else
			return _to_number(i, sizeof_i, len, "%lld", o);
		#endif
	}

	bool _Str::to_number(const void* i, int size_of, int len, uint64_t* o) {
		#if FX_ARCH_64BIT
			return _to_number(i, size_of, len, "%lu", o);
		#else
			return _to_number(i, size_of, len, "%llu", o);
		#endif
	}

	bool _Str::to_number(const void* i, int size_of, int len, float* o) {
		return _to_number(i, size_of, len, "%fd", o);
	}

	bool _Str::to_number(const void* i, int size_of, int len, double* o) {
		return _to_number(i, size_of, len, "%lf", o);
		// return _to_number(i, size_of, len, "%g", o);
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
	
	typedef ArrayStringBase::AAlloc AAlloc;
	typedef _Str::Alloc Alloc;
	typedef _Str::Size Size;
	
	struct _StrTmp {
		void realloc(uint32_t capacity) {
			_val = (char*)_aalloc(_val, capacity, &_capacity, sizeof(Char));
		}
		uint32_t _capacity;
		Char*    _val;
		AAlloc   _aalloc;
	};

	void* _Str::replace(
		const void* s1_, uint32_t s1_len,
		const void* s2_, uint32_t s2_len,
		const void* rep_, uint32_t rep_len,
		int size_of, uint32_t* out_len, uint32_t* capacity_out, bool all, AAlloc realloc
	) {
		_StrTmp s_tmp = {0, nullptr, realloc};
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

	int32_t vasprintf(char** o, cChar* f, va_list arg) {
		return ::vasprintf(o, f, arg);
	}
	
	int32_t vsnprintf(char* o, uint32_t len, cChar* f, va_list arg) {
		return ::vsnprintf(o, len, f, arg);
	}

	int32_t _Str::format_n(char* o, uint32_t len, cChar* f, ...) {
		va_list arg;
		va_start(arg, f);
		int r = vsnprintf((char*)o, len + 1, f, arg);
		va_end(arg);
		return FX_MIN(r, len);
	}
	
	int32_t _Str::format_n(char* o, uint32_t len_o, int32_t i) {
		return _Str::format_n(o, len_o, "%d", i);
	}
	int32_t _Str::format_n(char* o, uint32_t len_o, uint32_t i) {
		return _Str::format_n(o, len_o, "%u", i);
	}
	int32_t _Str::format_n(char* o, uint32_t len_o, int64_t i) {
		#if FX_ARCH_64BIT
			return _Str::format_n(o, len_o, "%ld", i);
		#else
			return _Str::format_n(o, len_o, "%lld", i);
		#endif
	}
	int32_t _Str::format_n(char* o, uint32_t len_o, uint64_t i) {
		#if FX_ARCH_64BIT
			return _Str::format_n(o, len_o, "%lu", i);
		#else
			return _Str::format_n(o, len_o, "%llu", i);
		#endif
	}
	int32_t _Str::format_n(char* o, uint32_t len_o, float i) {
		return _Str::format_n(o, len_o, "%fd", i);
	}
	int32_t _Str::format_n(char* o, uint32_t len_o, double i) {
		return _Str::format_n(o, len_o, "%g", i);
	}

	void* _Str::format(Size* size, int size_of, Alloc alloc, cChar* f, va_list arg) {
		char* str;
		int len_ = ftr::vasprintf(&str, f, arg);

		if (size_of == 1) {
			size->len = len_;
			size->capacity = len_ + 1;
		}
		else if (size_of == 2) {
			auto b = Codec::decode_to_uint16(Encoding::UTF8, Buffer::from(str, len_));
			size->len = b.length();
			size->capacity = b.length() + 1;
			str = (char*)b.collapse();
		}
		else if (size_of == 4) {
			auto b = Codec::decode_to_uint32(Encoding::UTF8, Buffer::from(str, len_));
			size->len = b.length();
			size->capacity = b.length() + 1;
			str = (char*)b.collapse();
		}	else {
			FX_FATAL("I won't support it, format");
		}

		if (&MemoryAllocator::alloc != alloc && alloc != (void*)&::malloc) {
			char* str_2 = (char*)alloc(size->capacity * size_of);
			memcpy(str_2, str, size->len * size_of);
			memset(str_2 + size->len * size_of, 0, size_of);
			free(str);
			str = str_2;
		}

		return str;
	}

	void* _Str::format(Size* size, int size_of, Alloc alloc, cChar* f, ...) {
		va_list arg;
		va_start(arg, f);
		void* str = _Str::format(size, size_of, alloc, f, arg);
		va_end(arg);
		return str;
	}
	
	// number to string 

	void* _Str::format(Size* size, int size_of, Alloc alloc, int32_t i) {
		return _Str::format(size, size_of, alloc, "%d", i);
	}
	void* _Str::format(Size* size, int size_of, Alloc alloc, uint32_t i) {
		return _Str::format(size, size_of, alloc, "%u", i);
	}
	void* _Str::format(Size* size, int size_of, Alloc alloc, int64_t i) {
		#if FX_ARCH_64BIT
			return _Str::format(size, size_of, alloc, "%ld", i);
		#else
			return _Str::format(size, size_of, alloc, "%lld", i);
		#endif
	}
	void* _Str::format(Size* size, int size_of, Alloc alloc, uint64_t i) {
		#if FX_ARCH_64BIT
			return _Str::format(size, size_of, alloc, "%lu", i);
		#else
			return _Str::format(size, size_of, alloc, "%llu", i);
		#endif
	}
	void* _Str::format(Size* size, int size_of, Alloc alloc, float i) {
		return _Str::format(size, size_of, alloc, "%fd", i);
	}
	void* _Str::format(Size* size, int size_of, Alloc alloc, double i) {
		return _Str::format(size, size_of, alloc, "%g", i);
	}

	String string_format(cChar* f, va_list arg) {
		Size size;
		char* buf = (char*)_Str::format(&size, 1, &MemoryAllocator::alloc, f, arg);
		return buf ? Buffer::from(buf, size.len, size.capacity).collapse_string(): String();
	}
			
	String Object::to_string() const {
		static String str("[object]");
		return str;
	}
	
	String _Str::to_string(const void* ptr, uint32_t len, int size_of) {
		if (size_of == 1) { // char
			return String((const char*)ptr, len);
		} else if (size_of == 2) { // uint16_t
			return Codec::encode(Encoding::UTF8, WeakArrayBuffer<uint16_t>((const uint16_t*)ptr, len));
		} else if (size_of == 4) { // uint32_t
			return Codec::encode(Encoding::UTF8, WeakArrayBuffer<uint32_t>((const uint32_t*)ptr, len));
		} else {
			FX_FATAL("I won't support it, to_string");
		}
	}

	template <>
	String ArrayString<>::to_string() const {
		return *this;
	}
	
	// --------------- ArrayStringBase ---------------
	
	typedef ArrayStringBase::LongStr LongStr;
	typedef ArrayStringBase::ShortStr ShortStr;
	
	LongStr* ArrayStringBase::NewLong(uint32_t length, uint32_t capacity, char* val) {
		auto l = (LongStr*)::malloc(sizeof(LongStr));
		l->length = length; l->capacity = capacity;
		l->val = val; l->ref = 1;
		return l;
	}

	void ArrayStringBase::Release(LongStr* l, Free free) {
		FX_ASSERT(l->ref > 0);
		if ( --l->ref == 0 ) {
			free(l->val);
			l->val = nullptr;
			delete l; // 只有当引用记数变成0才会释放
		}
	}
	
	void ArrayStringBase::clear(Free free) {
		if (_val.s.length < 0) {
			Release(_val.l, free);
		}
		_val.s.length = 0;
	}
	
	void ArrayStringBase::Retain(LongStr* l) { // retain long string
		l->ref++;
	}

	ArrayStringBase::ArrayStringBase(): _val({.s={{0},0}}) {
	} // empty
	
	ArrayStringBase::ArrayStringBase(const ArrayStringBase& str): _val(str._val)
	{
		if (_val.s.length < 0)
			Retain(_val.l);
	} // copy
	
	ArrayStringBase::ArrayStringBase(uint32_t len, AAlloc aalloc, uint8_t size_of)
		: _val({.s={{0},0}})
	{
		realloc(len, aalloc, nullptr, size_of); // alloc
	}
	
	ArrayStringBase::ArrayStringBase(uint32_t l, uint32_t c, char* v, uint8_t size_of)
		: _val({.s={{0},0}})
	{ // use long string
		if (v) {
			_val.s.length = -1; // mark long string
			_val.l = NewLong(l * size_of, c * size_of, v);
		}
	}
	void ArrayStringBase::assign(uint32_t len, uint32_t capacity, char* val, uint8_t size_of, Free free) {
		assign( ArrayStringBase(len, capacity, val, size_of), free);
	}
	
	void ArrayStringBase::assign(const ArrayStringBase& s, Free free) {
		if (s._val.s.length < 0) { // long
			if (_val.s.length < 0) { // long
				if (_val.l == s._val.l) {
					return;
				}
				Release(_val.l, free);
			}
			Retain(s._val.l);
			_val = s._val;
		} else { // short
			if (_val.s.length < 0) { // long
				Release(_val.l, free);
			}
			_val = s._val;
		}
	}

	uint32_t ArrayStringBase::size() const {
		return _val.s.length < 0 ? _val.l->length: _val.s.length;
	}
	
	uint32_t ArrayStringBase::capacity() const {
		return _val.s.length < 0 ? _val.l->capacity: MAX_SHORT_LEN;
	}
	
	char* ArrayStringBase::data() {
		return _val.s.length < 0 ? _val.l->val: (char*)_val.s.val;
	}

	const char* ArrayStringBase::data() const {
		return _val.s.length < 0 ? _val.l->val: (const char*)_val.s.val;
	}
	
	char* ArrayStringBase::realloc(uint32_t len, AAlloc aalloc, Free free, uint8_t size_of) {
		len *= size_of;
		if (_val.s.length < 0) { // long
			if (_val.l->ref > 1) {
				// TODO 需要从共享核心中分离出来, 多个线程同时使用一个LongStr可能的安全问题
				auto leave = _val.l;
				_val.l = NewLong(len, 0, nullptr);
				_val.l->val = (char*)aalloc(_val.l->val, len + size_of, &_val.l->capacity, 1);
				::memcpy(_val.l->val, leave->val, FX_MIN(len, leave->length));
				Release(leave, free); // release old
			} else {
				_val.l->length = len;
				_val.l->val = (char*)aalloc(_val.l->val, len + size_of, &_val.l->capacity, 1);
			}
		} else if (len > MAX_SHORT_LEN) {
			_val.s.length = -1; // mark long string
			_val.l = NewLong(len, 0, nullptr);
			_val.l->val = (char*)aalloc(_val.l->val, len + size_of, &_val.l->capacity, 1);
		} else { // use short string
			_val.s.length = len;
			::memset(_val.s.val + len, 0, size_of);
			return _val.s.val;
		}
		::memset(_val.l->val + len, 0, size_of);
		return _val.l->val;
	}

	Buffer ArrayStringBase::collapse(AAlloc aalloc, Free free) {
		uint32_t s_len = _val.s.length;
		if (s_len < 0) {
			// long string
			if (_val.l->length == 0) {
				return Buffer();
			} else { // collapse
				uint32_t len = _val.l->length;
				uint32_t capacity = _val.l->length;
				char* val = _val.l->val;
				if (_val.l->ref > 1) { //
					val = (char*)aalloc(nullptr, capacity, &capacity, 1);
					::memcpy(val, _val.l->val, capacity);
					Release(_val.l, free); // release ref
				} else {
					delete _val.l; // full delete
				}
				_val.s = {{0},0}; // use empty string
				return Buffer::from(val, len, capacity);
			}
		} else {
			// short string
			if (s_len == 0) {
				return Buffer();
			} else {
				uint32_t capacity;
				char* val = (char*)aalloc(nullptr, MAX_SHORT_LEN + 4, &capacity, 1);
				::memcpy(val, _val.s.val, MAX_SHORT_LEN + 4);
				_val.s = {{0},0}; // use empty string
				return Buffer::from(val, s_len, capacity);
			}
		}
	}

}
