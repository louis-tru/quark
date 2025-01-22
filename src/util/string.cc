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
	const bool is_bigData = *test_big_int != 1;

	void _Str::strcpy(void* o_, int sizeof_o, cVoid* i_, int sizeof_i, uint32_t len) {
		char* o = (char*)o_;
		char* i = (char*)i_;
		if (len && i) {
			if (sizeof_o == sizeof_i) {
				::memcpy(o, i, len * sizeof_o);
				::memset(o + len * sizeof_o, 0, sizeof_o);
			} else {
				int min = Qk_Min(sizeof_o, sizeof_i);
				int max = Qk_Min(sizeof_o, sizeof_i);
				if (is_bigData) { // big data layout
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
	
	static bool _toNumber(cVoid* i, int len_i, int sizeof_i, const char* f, void* o) {
		if (sizeof_i == 1) {
			return sscanf( (const char*)i, f, o);
		} else {
			Char i_[65];
			_Str::strcpy(i_, 1, i, sizeof_i, Qk_Min(len_i, 64));
			return sscanf( i_, f, o );
		}
	}

	bool _Str::toNumber(cVoid* i, int len, int sizeOf, int32_t* o) {
		return _toNumber(i, len, sizeOf, "%d", o);
	}

	bool _Str::toNumber(cVoid* i, int len, int sizeOf, uint32_t* o) {
		return _toNumber(i, len, sizeOf, "%u", o);
	}

	bool _Str::toNumber(cVoid* i, int len, int sizeOf, int64_t* o) {
		return _toNumber(i, len, sizeOf, Qk_ARCH_64BIT ? "%ld": "%lld", o);
	}

	bool _Str::toNumber(cVoid* i, int len, int sizeOf, uint64_t* o) {
		return _toNumber(i, len, sizeOf, Qk_ARCH_64BIT ? "%lu": "%llu", o);
	}

	bool _Str::toNumber(cVoid* i, int len, int sizeOf, float* o) {
		return _toNumber(i, len, sizeOf, "%f", o);
	}

	bool _Str::toNumber(cVoid* i, int len, int sizeOf, double* o) {
		return _toNumber(i, len, sizeOf, "%lf", o);
	}

	uint32_t _Str::strlen(cVoid* s_, int sizeOf) {
		const char* s = (const char*)s_;
		if (s) {
			if (sizeOf == 1) {
				return (uint32_t)::strlen(s);
			} else {
				uint32_t rev = 0;
				while (*s != 0) {
					rev++; s+=sizeOf;
				}
				return rev;
			}
		} else {
			return 0;
		}
	}

	int _Str::memcmp(cVoid* s1, cVoid* s2, uint32_t len, int sizeOf) {
		return ::memcmp(s1, s2, len * sizeOf);
	}

	int _Str::index_of(
		cVoid* s1_, uint32_t s1_len, cVoid* s2_,
		uint32_t s2_len, uint32_t start, int sizeOf
	) {
		if (s1_len < s2_len) return -1;
		if (start + s2_len > s1_len) return -1;

		const char* s1 = (const char*)s1_;
		const char* s2 = (const char*)s2_;

		int32_t end = s1_len - s2_len + 1;

		while ( start < end ) {
			if (_Str::memcmp(s1 + (start * sizeOf), s2, s2_len, sizeOf) == 0) {
				return start;
			}
			start++;
		}
		return -1;
	}

	int _Str::last_index_of(
		cVoid* s1_, uint32_t s1_len, cVoid* s2_,
		uint32_t s2_len, uint32_t _start, int sizeOf
	) {
		const char* s1 = (const char*)s1_;
		const char* s2 = (const char*)s2_;
		int32_t start = _start;
		if ( start + s2_len > s1_len )
			start = s1_len - s2_len;
		while ( start > -1 ) {
			if (_Str::memcmp(s1 + (start * sizeOf), s2, s2_len, sizeOf) == 0) {
				return start;
			}
			start--;
		}
		return -1;
	}
	
	typedef _Str::Alloc Alloc;
	typedef StringBase::Realloc Realloc;
	typedef StringBase::Free Free;
	typedef StringBase::Long::Base Base;
	typedef Allocator::Prt<char> Ptr;
	typedef StringBase::Long Long;
	typedef StringBase::Short Short;

	struct _StrTmp {
		void realloc(uint32_t capacity) {
			realloc_func(&ptr, capacity, sizeof(Char));
		}
		inline char *val() { return ptr.val; }
		Ptr     ptr;
		Realloc realloc_func;
	};

	void* _Str::replace(
		cVoid* s1_, uint32_t s1_len,
		cVoid* s2_, uint32_t s2_len,
		cVoid* rep_, uint32_t rep_len,
		int sizeOf, uint32_t* outLen, uint32_t* capacity_out, bool all, Realloc realloc
	) {
		_StrTmp tmp = {nullptr,0,realloc};
		uint32_t tmp_to = 0;
		uint32_t from = 0;
		int32_t  find, before_len;

		const char* s1 = (const char*)s1_;
		const char* s2 = (const char*)s2_;
		const char* rep = (const char*)rep_;

		while ((find = index_of(s1, s1_len, s2, s2_len, from, sizeOf)) != -1) {
			before_len = find - from;
			tmp.realloc((tmp_to + before_len + rep_len + 1) * sizeOf); // realloc

			if (before_len) {
				::memcpy(
					tmp.val() + tmp_to * sizeOf,  // to
					s1        + from   * sizeOf,  // from
					before_len         * sizeOf   // size
				);
				tmp_to += before_len;
				from   += before_len;
			}
			::memcpy(tmp.val() + tmp_to * sizeOf, rep, rep_len * sizeOf);
			tmp_to += rep_len;
			from   += s2_len;

			if (!all) {
				break;
			}
		}

		before_len = s1_len - from;
		tmp.realloc((tmp_to + before_len + 1) * sizeOf);

		::memcpy(
			tmp.val() + tmp_to * sizeOf,  // to
			s1        + from   * sizeOf,  // from
			before_len         * sizeOf   // size
		);
		tmp_to += before_len;

		::memset(tmp.val() + tmp_to * sizeOf, 0, sizeOf);

		*capacity_out = tmp.ptr.capacity;
		*outLen = tmp_to;
		return tmp.ptr.val;
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

	int _Str::oPrinti(char* o, uint32_t oLen, int32_t i) {
		return _Str::oPrintf(o, oLen, "%d", i);
	}
	int _Str::oPrinti(char* o, uint32_t oLen, uint32_t i) {
		return _Str::oPrintf(o, oLen, "%u", i);
	}
	int _Str::oPrinti(char* o, uint32_t oLen, int64_t i) {
		return _Str::oPrintf(o, oLen, Qk_ARCH_64BIT ? "%ld": "%lld", i);
	}
	int _Str::oPrinti(char* o, uint32_t oLen, uint64_t i) {
		return _Str::oPrintf(o, oLen, Qk_ARCH_64BIT ? "%lu": "%llu", i);
	}
	int _Str::oPrinti(char* o, uint32_t oLen, float i) {
		return _Str::oPrintf(o, oLen, "%f", i);
	}
	int _Str::oPrinti(char* o, uint32_t oLen, double i) {
		return _Str::oPrintf(o, oLen, "%g", i);
	}
	int _Str::oPrintf(char* o, uint32_t len, cChar* f, ...) {
		va_list arg;
		va_start(arg, f);
		int r = vsnprintf(o, len, f, arg);
		va_end(arg);
		return Qk_Min(r, len);
	}

	Base _Str::sPrinti(int sizeOf, Alloc alloc, int32_t i) {
		return _Str::sPrintf(sizeOf, alloc, "%d", i);
	}
	Base _Str::sPrinti(int sizeOf, Alloc alloc, uint32_t i) {
		return _Str::sPrintf(sizeOf, alloc, "%u", i);
	}
	Base _Str::sPrinti(int sizeOf, Alloc alloc, int64_t i) {
		return _Str::sPrintf(sizeOf, alloc, Qk_ARCH_64BIT ? "%ld": "%lld", i);
	}
	Base _Str::sPrinti(int sizeOf, Alloc alloc, uint64_t i) {
		return _Str::sPrintf(sizeOf, alloc, Qk_ARCH_64BIT ? "%lu": "%llu", i);
	}
	Base _Str::sPrinti(int sizeOf, Alloc alloc, float i) {
		return _Str::sPrintf(sizeOf, alloc, "%f", i);
	}
	Base _Str::sPrinti(int sizeOf, Alloc alloc, double i) {
		return _Str::sPrintf(sizeOf, alloc, "%g", i);
	}

	Base _Str::sPrintf(int sizeOf, Alloc alloc, cChar* f, ...) {
		va_list arg;
		va_start(arg, f);
		auto base = _Str::sPrintfv(sizeOf, alloc, f, arg);
		va_end(arg);
		return base;
	}

	Base _Str::sPrintfv(int sizeOf, Alloc alloc, cChar* f, va_list arg) {
		char *val;
		int len_ = vasprintf(&val, f, arg);
		Qk_ASSERT(len_ >= 0);

		uint32_t len = len_;
		Base base{ val, len + 1, len };

		switch (sizeOf) {
			case 1: break;
			case 2: {
				auto b = codec_decode_to_ucs2(kUTF8_Encoding, Buffer::from(val, len));
				base.length = b.length();
				base.ptr.capacity = b.capacity();
				base.ptr.val = (char*)b.collapse();
				break;
			}
			case 4: {
				auto b = codec_decode_to_unicode(kUTF8_Encoding, Buffer::from(val, len));
				base.length = b.length();
				base.ptr.capacity = b.capacity();
				base.ptr.val = (char*)b.collapse();
				break;
			}
			default: Qk_Fatal("I won't support it, format");
		}

		if (&Allocator::alloc != alloc && alloc != (void*)&::malloc) {
			char* val = (char*)alloc(base.ptr.capacity * sizeOf);
			::memcpy(val, base.ptr.val, base.length * sizeOf);
			::memset(val + base.length * sizeOf, 0, sizeOf);
			::free(base.ptr.val);
			base.ptr.val = val;
		}

		return base;
	}

	String _Str::printfv(cChar* f, va_list arg) {
		auto base = _Str::sPrintfv(1, &Allocator::alloc, f, arg);
		if (base.ptr.val) {
			return Buffer::from((char*)base.ptr.val, base.length, base.ptr.capacity).collapseString();
		}
		return String();
	}

	String _Str::join(cString& sp, void* data, uint32_t len, bool (*it)(void* data, String* out)) {
		if (len == 0)
			return String();

		String tmp;
		if (len == 1)
			return it(data, &tmp), tmp;

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

	String _Str::toString(cVoid* ptr, uint32_t len, int sizeOf) {
		if (sizeOf == 1) { // char
			return String((const char*)ptr, len);
		} else if (sizeOf == 2) { // uint16_t
			return codec_encode(kUTF8_Encoding, ArrayWeak<uint16_t>((const uint16_t*)ptr, len).buffer());
		} else if (sizeOf == 4) { // uint32_t
			return codec_encode(kUTF8_Encoding, ArrayWeak<uint32_t>((const uint32_t*)ptr, len).buffer());
		} else {
			Qk_Fatal("I won't support it, to_string");
			return String();
		}
	}

	template <>
	String StringImpl<>::toString() const {
		return *this;
	}

	template <>
	String StringImpl<>::format(cChar* f, ...) {
		va_list arg;
		va_start(arg, f);
		auto base = _Str::sPrintfv(1, &Allocator::alloc, f, arg);
		va_end(arg);
		StringImpl s;
		if (base.ptr.val)
			s.StringBase::assign(base, 1, &Allocator::free);
		Qk_ReturnLocal(s);
	}

	// --------------- StringBase ---------------

	static Long* NewLong(uint32_t length, uint32_t capacity, char* val) {
		auto l = (Long*)::malloc(sizeof(Long));
		l->length = length;
		l->capacity = capacity;
		l->val = val;
		l->ref = 1;
		l->flag = 127645561;
		return l;
	}

	static void ReleaseLong(Long* l, Free free) {
		Qk_ASSERT(l->ref > 0);
		if ( --l->ref == 0) { // After entering, do not reverse
			l->flag = 0; // destroy flag
			free(l->val);
			::free(l);
		}
	}

	static bool RetainLong(Long* l) { // retain long string
		if (l->flag == 127645561 && l->ref++ > 0)
			return l->flag == 127645561; // safe check once
		return false;
	}

	void StringBase::clear(Free free) {
		if (_val.s.length < 0) {
			ReleaseLong(_val.l, free);
			_val.l = nullptr;
		}
		_val.s.length = 0;
	}

	StringBase::StringBase(): _val{.s={{0},0}} {
	} // empty

	StringBase::StringBase(const StringBase& str): _val(str._val)
	{
		if (_val.s.length < 0) {
			if (!RetainLong(_val.l)) { // Retain long fail
				_val = {.s={{0},0}}; // clear
			}
		}
	}

	StringBase::StringBase(uint32_t len, Realloc aalloc, uint8_t sizeOf)
		: _val{.s={{0},0}}
	{
		realloc(len, aalloc, nullptr, sizeOf); // alloc
	}

	StringBase::StringBase(Long::Base base, uint8_t sizeOf)
		: _val{.s={{0},0}}
	{ // use long string
		if (base.ptr.val) {
			_val.s.length = -1; // mark long string
			_val.l = NewLong(base.length * sizeOf, base.ptr.capacity * sizeOf, base.ptr.val);
		}
	}

	StringBase::~StringBase() {
		Qk_ASSERT(_val.s.length >= 0);
	}

	void StringBase::assign(Long::Base base, uint8_t sizeOf, Free free) {
		StringBase str(base, sizeOf);
		assign( str, free);
		str.clear(free); // release
	}

	void StringBase::assign(const StringBase& s, Free free) {
		if (s._val.s.length < 0) { // src long
			if (_val.s.length < 0) { // long
				if (_val.l == s._val.l) {
					return;
				}
				ReleaseLong(_val.l, free);
			}
			if (RetainLong(s._val.l)) { // Retain long string
				_val = s._val;
			} else { // fail
				_val = {.s={{0},0}};
			}
		} else { // src short
			if (_val.s.length < 0) { // long
				ReleaseLong(_val.l, free);
			}
			_val = {.s=s._val.s};
		}
	}

	uint32_t StringBase::size() const {
		return _val.s.length < 0 ? _val.l->length: _val.s.length;
	}

	uint32_t StringBase::capacity() const {
		return _val.s.length < 0 ? _val.l->capacity: MAX_SHORT_LEN;
	}

	char* StringBase::ptr() {
		return _val.s.length < 0 ? (char*)_val.l->val: (char*)_val.s.val;
	}

	const char* StringBase::ptr() const {
		return _val.s.length < 0 ? (cChar*)_val.l->val: (cChar*)_val.s.val;
	}

	char* StringBase::realloc(uint32_t len, Realloc aalloc, Free free, uint8_t sizeOf) {
		len *= sizeOf;

		if (_val.s.length < 0) { // long
			if (_val.l->ref > 1) {
				// TODO 需要从共享核心中分离出来, 多个线程同时使用一个Long可能的安全问题
				auto old = _val.l;
				_val.l = NewLong(len, 0, nullptr);
				aalloc(_val.l->ptr(), len + sizeOf, 1);
				::memcpy(_val.l->val, old->val, Qk_Min(len, old->length));
				ReleaseLong(old, free); // release old
			} else {
				_val.l->length = len;
				aalloc(_val.l->ptr(), len + sizeOf, 1);
			}
		}
		else if (len > MAX_SHORT_LEN) {
			auto l = NewLong(len, 0, nullptr);
			aalloc(l->ptr(), len + sizeOf, 1);
			::memcpy(l->val, _val.s.val, _val.s.length); // copy string
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
			_val.l->val[len] = '\0';
		} else {
			::memset(_val.l->val + len, 0, sizeOf);
		}
		return _val.l->val;
	}

	Buffer StringBase::collapse(Realloc aalloc, Free free) {
		auto s_len = _val.s.length;
		if (s_len < 0) {
			// long string
			if (_val.l->length == 0) {
				return Buffer();
			}
			// collapse
			auto len = _val.l->length;
			Ptr ptr{nullptr,0};
			if (_val.l->ref > 1) { //
				aalloc(&ptr, _val.l->capacity, 1);
				::memcpy(ptr.val, _val.l->val, ptr.capacity);
				ReleaseLong(_val.l, free); // release ref
			} else {
				ptr = *_val.l->ptr();
				::free(_val.l); // full delete memory
			}
			_val.s = {{0},0}; // use empty string
			return Buffer(ptr.val, len, ptr.capacity);
		} else {
			// short string
			if (s_len == 0) {
				return Buffer();
			}
			Ptr ptr{nullptr,0};
			aalloc(&ptr, MAX_SHORT_LEN + 4, 1);
			::memcpy(ptr.val, _val.s.val, MAX_SHORT_LEN + 4);
			_val.s = {{0},0}; // use empty string
			return Buffer(ptr.val, s_len, ptr.capacity);
		}
	}

}
