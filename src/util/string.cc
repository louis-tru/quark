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

#include "./string.h"
#include "./codec.h"
#include "./handle.h"

namespace qk {

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
				int min = Qk_MIN(sizeof_o, sizeof_i);
				int max = Qk_MIN(sizeof_o, sizeof_i);
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
			_Str::strcpy(i_, 1, i, sizeof_i, Qk_MIN(len_i, 64));
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
		#if Qk_ARCH_64BIT
			return _to_number(i, sizeof_i, len, "%ld", o);
		#else
			return _to_number(i, sizeof_i, len, "%lld", o);
		#endif
	}

	bool _Str::to_number(const void* i, int size_of, int len, uint64_t* o) {
		#if Qk_ARCH_64BIT
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
	
	typedef ArrayStringBase::Realloc Realloc;
	typedef _Str::Alloc Alloc;
	typedef _Str::Size Size;
	typedef MemoryAllocator::Prt<char> Ptr;
	
	struct _StrTmp {
		void realloc(uint32_t capacity) {
			_realloc(&_ptr, capacity, sizeof(Char));
		}
		Ptr _ptr;
		Realloc  _realloc;
	};

	void* _Str::replace(
		const void* s1_, uint32_t s1_len,
		const void* s2_, uint32_t s2_len,
		const void* rep_, uint32_t rep_len,
		int sizeOf, uint32_t* outLen, uint32_t* capacity_out, bool all, Realloc realloc
	) {
		_StrTmp s_tmp = {0, nullptr, realloc};
		uint32_t s_tmp_to = 0;
		uint32_t from = 0;
		int32_t  find, before_len;
		
		const char* s1 = (const char*)s1_;
		const char* s2 = (const char*)s2_;
		const char* rep = (const char*)rep_;

		while ((find = index_of(s1, s1_len, s2, s2_len, from, sizeOf)) != -1) {
			before_len = find - from;
			s_tmp.realloc((s_tmp_to + before_len + rep_len + 1) * sizeOf); // realloc

			if (before_len) {
				::memcpy(
					s_tmp._ptr.val + s_tmp_to * sizeOf,  // to
					s1         + from     * sizeOf,  // from
					before_len            * sizeOf   // size
				);
				s_tmp_to += before_len;
				from += before_len;
			}
			::memcpy(s_tmp._ptr.val + s_tmp_to * sizeOf, rep, rep_len * sizeOf);
			s_tmp_to += rep_len;
			from += s2_len;

			if (!all) {
				break;
			}
		}

		before_len = s1_len - from;
		s_tmp.realloc((s_tmp_to + before_len + 1) * sizeOf);

		::memcpy(
			s_tmp._ptr.val + s_tmp_to * sizeOf,  // to
			s1         + from     * sizeOf,  // from
			before_len            * sizeOf   // size
		);
		s_tmp_to += before_len;

		::memset(s_tmp._ptr.val + s_tmp_to * sizeOf, 0, sizeOf);

		*capacity_out = s_tmp._ptr.capacity;
		*outLen = s_tmp_to;
		return s_tmp._ptr.val;
	}
	
	int _Str::tolower(int c) {
		if ((c >= 'A') && (c <= 'Z'))
			return c + ('a' - 'A');
		return c;
	}
	
	int _Str::toupper(int c) {
		if ((c >= 'a') && (c <= 'z'))
			return c + ('A' - 'a');
		return c;
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
		return Qk_MIN(r, len);
	}
	
	int32_t _Str::format_n(char* o, uint32_t len_o, int32_t i) {
		return _Str::format_n(o, len_o, "%d", i);
	}
	int32_t _Str::format_n(char* o, uint32_t len_o, uint32_t i) {
		return _Str::format_n(o, len_o, "%u", i);
	}
	int32_t _Str::format_n(char* o, uint32_t len_o, int64_t i) {
#if Qk_ARCH_64BIT
			return _Str::format_n(o, len_o, "%ld", i);
#else
			return _Str::format_n(o, len_o, "%lld", i);
#endif
	}
	int32_t _Str::format_n(char* o, uint32_t len_o, uint64_t i) {
		#if Qk_ARCH_64BIT
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
		int len_ = qk::vasprintf(&str, f, arg);

		if (size_of == 1) {
			size->len = len_;
			size->capacity = len_ + 1;
		}
		else if (size_of == 2) {
			auto b = codec_decode_to_unicode(kUTF8_Encoding, Buffer::from(str, len_));
			size->len = b.length();
			size->capacity = b.length() + 1;
			str = (char*)b.collapse();
		}
		else if (size_of == 4) {
			auto b = codec_decode_to_unicode(kUTF8_Encoding, Buffer::from(str, len_));
			size->len = b.length();
			size->capacity = b.length() + 1;
			str = (char*)b.collapse();
		}	else {
			Qk_FATAL("I won't support it, format");
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
		#if Qk_ARCH_64BIT
			return _Str::format(size, size_of, alloc, "%ld", i);
		#else
			return _Str::format(size, size_of, alloc, "%lld", i);
		#endif
	}
	void* _Str::format(Size* size, int size_of, Alloc alloc, uint64_t i) {
		#if Qk_ARCH_64BIT
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

	String _Str::string_format(cChar* f, va_list arg) {
		Size size;
		char* buf = (char*)_Str::format(&size, 1, &MemoryAllocator::alloc, f, arg);
		return buf ? Buffer::from(buf, size.len, size.capacity).collapseString(): String();
	}
	
	String _Str::join(cString& sp, void* data, uint32_t len, bool (*it)(void* data, String* out)) {
		if (len == 0)
			return String();

		String tmp;
		auto spLen = sp.isEmpty() ? 0: Int32::max(len - 1, 0);
		Array<String> strs(len + spLen);
		int total = spLen * sp.length(), i = 0;

		if (sp.isEmpty()) {
			while (it(data, &tmp)) {
				total += tmp.length();
				strs[i++] = tmp;
			}
		} else {
			if (it(data, &tmp)) {
				total += tmp.length();
				strs[i++] = tmp;
			}
			while (it(data, &tmp)) {
				total += tmp.length();
				strs[i++] = sp;
				strs[i++] = tmp;
			}
		}

		Buffer buff(total);
		auto offset = 0;

		for (auto &i: strs) {
			buff.write(i.c_str(), i.length(), offset);
			offset += i.length();
		}

		return String(std::move(buff));
	}

	String Object::toString() const {
		static String str("[object]");
		return str;
	}

	String _Str::toString(const void* ptr, uint32_t len, int size_of) {
		if (size_of == 1) { // char
			return String((const char*)ptr, len);
		} else if (size_of == 2) { // uint16_t
			return codec_encode(kUTF8_Encoding, ArrayWeak<uint16_t>((const uint16_t*)ptr, len).buffer());
		} else if (size_of == 4) { // uint32_t
			return codec_encode(kUTF8_Encoding, ArrayWeak<uint32_t>((const uint32_t*)ptr, len).buffer());
		} else {
			Qk_FATAL("I won't support it, to_string");
			return String();
		}
	}

	template <>
	String ArrayString<>::toString() const {
		return *this;
	}

	// --------------- ArrayStringBase ---------------

	typedef ArrayStringBase::LongStr LongStr;
	typedef ArrayStringBase::ShortStr ShortStr;

	LongStr* ArrayStringBase::NewLong(uint32_t length, uint32_t capacity, char* val) {
		auto l = (LongStr*)::malloc(sizeof(LongStr));
		l->length = length; l->ptr.capacity = capacity;
		l->ptr.val = val; l->ref = 1;
		l->flag = 127645561;
		return l;
	}

	void ArrayStringBase::Release(LongStr* l, Free free) {
		Qk_ASSERT(l->ref > 0);
		if ( --l->ref == 0) { // After entering, do not reverse
			l->flag = 0; // destroy flag
			free(l->ptr.val);
			// l->ptr.val = nullptr;
			delete l;
		}
	}

	bool ArrayStringBase::Retain(LongStr* l) { // retain long string
		if (l->flag == 127645561 && l->ref++ > 0)
			return l->flag == 127645561; // safe check once
		return false;
	}

	void ArrayStringBase::clear(Free free) {
		if (_val.s.length < 0) {
			Release(_val.l, free);
		}
		_val.s.length = 0;
	}

	ArrayStringBase::ArrayStringBase(): _val({.s={{0},0}}) {
		//Qk_DEBUG("Empty str");
	} // empty

	ArrayStringBase::ArrayStringBase(const ArrayStringBase& str): _val(str._val)
	{
		if (_val.s.length < 0) {
			if (!Retain(_val.l)) { // Retain long fail
				_val = {.s={{0},0}}; // clear
			}
		}
	}

	ArrayStringBase::ArrayStringBase(uint32_t len, Realloc aalloc, uint8_t sizeOf)
		: _val({.s={{0},0}})
	{
		realloc(len, aalloc, nullptr, sizeOf); // alloc
	}

	ArrayStringBase::ArrayStringBase(uint32_t l, uint32_t c, char* v, uint8_t sizeOf)
		: _val({.s={{0},0}})
	{ // use long string
		if (v) {
			_val.s.length = -1; // mark long string
			_val.l = NewLong(l * sizeOf, c * sizeOf, v);
		}
	}
	void ArrayStringBase::assign(uint32_t len, uint32_t capacity, char* val, uint8_t sizeOf, Free free) {
		assign( ArrayStringBase(len, capacity, val, sizeOf), free);
	}

	void ArrayStringBase::assign(const ArrayStringBase& s, Free free) {
		if (s._val.s.length < 0) { // src long
			if (_val.s.length < 0) { // long
				if (_val.l == s._val.l) {
					return;
				}
				Release(_val.l, free);
			}
			if (Retain(s._val.l)) { // Retain long string
				_val = s._val;
			} else { // fail
				_val = {.s={{0},0}};
			}
		} else { // src short
			if (_val.s.length < 0) { // long
				Release(_val.l, free);
			}
			_val = {.s=s._val.s};
		}
	}

	uint32_t ArrayStringBase::size() const {
		return _val.s.length < 0 ? _val.l->length: _val.s.length;
	}

	uint32_t ArrayStringBase::capacity() const {
		return _val.s.length < 0 ? _val.l->ptr.capacity: MAX_SHORT_LEN;
	}

	char* ArrayStringBase::ptr() {
		return _val.s.length < 0 ? _val.l->ptr.val: (char*)_val.s.val;
	}

	const char* ArrayStringBase::ptr() const {
		return _val.s.length < 0 ? _val.l->ptr.val: (const char*)_val.s.val;
	}
	
	char* ArrayStringBase::realloc(uint32_t len, Realloc aalloc, Free free, uint8_t sizeOf) {
		len *= sizeOf;
		
		if (_val.s.length < 0) { // long
			if (_val.l->ref > 1) {
				// TODO 需要从共享核心中分离出来, 多个线程同时使用一个LongStr可能的安全问题
				auto leave = _val.l;
				_val.l = NewLong(len, 0, nullptr);
				// _val.l->val = (char*)
				aalloc(&_val.l->ptr, len + sizeOf, 1);
				::memcpy(_val.l->ptr.val, leave->ptr.val, Qk_MIN(len, leave->length));
				Release(leave, free); // release old
			} else {
				_val.l->length = len;
				// _val.l->val = (char*)
				aalloc(&_val.l->ptr, len + sizeOf, 1);
			}
		}
		else if (len > MAX_SHORT_LEN) {
			auto l = NewLong(len, 0, nullptr);
			l->ptr.val = nullptr;
			aalloc(&l->ptr, len + sizeOf, 1);
			::memcpy(l->ptr.val, _val.s.val, _val.s.length); // copy string
			_val.l = l;
			_val.s.length = -1; // mark long string
		}
		else { // use short string
			_val.s.length = len;
			if (sizeOf == 1) {
				_val.s.val[len] = '\0';
			} else {
				::memset(_val.s.val + len, 0, sizeOf);
			}
			return _val.s.val;
		}
		
		if (sizeOf == 1) {
			_val.l->ptr.val[len] = '\0';
		} else {
			::memset(_val.l->ptr.val + len, 0, sizeOf);
		}
		return _val.l->ptr.val;
	}

	Buffer ArrayStringBase::collapse(Realloc aalloc, Free free) {
		auto s_len = _val.s.length;
		if (s_len < 0) {
			// long string
			if (_val.l->length == 0) {
				return Buffer();
			}
			// collapse
			auto len = _val.l->length;
			Ptr ptr{0,nullptr};
			if (_val.l->ref > 1) { //
				aalloc(&ptr, _val.l->ptr.capacity, 1);
				::memcpy(ptr.val, _val.l->ptr.val, ptr.capacity);
				Release(_val.l, free); // release ref
			} else {
				ptr = _val.l->ptr;
				delete _val.l; // full delete memory
			}
			_val.s = {{0},0}; // use empty string
			return Buffer(ptr.val, len, ptr.capacity);
		} else {
			// short string
			if (s_len == 0) {
				return Buffer();
			}
			Ptr ptr{0,nullptr};
			aalloc(&ptr, MAX_SHORT_LEN + 4, 1);
			::memcpy(ptr.val, _val.s.val, MAX_SHORT_LEN + 4);
			_val.s = {{0},0}; // use empty string
			return Buffer(ptr.val, s_len, ptr.capacity);
		}
	}

}
