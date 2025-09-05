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
#include <stdio.h>
#include <stdarg.h>

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
					for (uint32_t j = 0; j < len; j++) {
						::memcpy(o, i + max - min, min); // Copy low order only
						o+=sizeof_o; i+=sizeof_i;
					}
				} else {
					for (uint32_t j = 0; j < len; j++) {
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

	bool _Str::starts_with(cVoid* s1, uint32_t s1_len, cVoid* s2, uint32_t s2_len, int sizeOf) {
		if (s1_len < s2_len) return false;
		return _Str::memcmp(s1, s2, s2_len, sizeOf) == 0;
	}

	bool _Str::ends_with(cVoid* s1, uint32_t s1_len, cVoid* s2, uint32_t s2_len, int sizeOf) {
		if (s1_len < s2_len) return false;
		return _Str::memcmp(((cChar*)s1) + (s1_len - s2_len), s2, s2_len, sizeOf) == 0;
	}

	typedef StringBase::Init Init;
	typedef StringBase::Ptr Ptr;
	typedef StringBase::Ref Ref;
	typedef StringBase::Short Short;

	Init _Str::replace(
		cVoid* s1_, uint32_t s1_len,
		cVoid* s2_, uint32_t s2_len,
		cVoid* rep_, uint32_t rep_len,
		int sizeOf, bool all, cAllocator *allocator
	) {
		Init base = {{allocator,0,0},0};
		uint32_t to = 0;
		uint32_t from = 0;
		int32_t  find, before_len;

		const char* s1 = (const char*)s1_;
		const char* s2 = (const char*)s2_;
		const char* rep = (const char*)rep_;

		while ((find = index_of(s1, s1_len, s2, s2_len, from, sizeOf)) != -1) {
			before_len = find - from;
			base.ptr.resize((to + before_len + rep_len + 1) * sizeOf); // realloc

			if (before_len) {
				::memcpy(
					base.ptr.val + to     * sizeOf,  // to
					s1           + from   * sizeOf,  // from
					before_len            * sizeOf   // size
				);
				to     += before_len;
				from   += before_len;
			}
			::memcpy(base.ptr.val + to * sizeOf, rep, rep_len * sizeOf);
			to     += rep_len;
			from   += s2_len;

			if (!all) {
				break;
			}
		}

		before_len = s1_len - from;
		base.ptr.resize((to + before_len + 1) * sizeOf);

		::memcpy(
			base.ptr.val + to     * sizeOf,  // to
			s1           + from   * sizeOf,  // from
			before_len            * sizeOf   // size
		);
		to += before_len;

		::memset(base.ptr.val + to * sizeOf, 0, sizeOf);
		base.length = to;

		return base;
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

	Init _Str::sPrinti(int sizeOf, int32_t i) {
		return _Str::sPrintf(sizeOf, "%d", i);
	}
	Init _Str::sPrinti(int sizeOf, uint32_t i) {
		return _Str::sPrintf(sizeOf, "%u", i);
	}
	Init _Str::sPrinti(int sizeOf, int64_t i) {
		return _Str::sPrintf(sizeOf, Qk_ARCH_64BIT ? "%ld": "%lld", i);
	}
	Init _Str::sPrinti(int sizeOf, uint64_t i) {
		return _Str::sPrintf(sizeOf, Qk_ARCH_64BIT ? "%lu": "%llu", i);
	}
	Init _Str::sPrinti(int sizeOf, float i) {
		return _Str::sPrintf(sizeOf, "%f", i);
	}
	Init _Str::sPrinti(int sizeOf, double i) {
		return _Str::sPrintf(sizeOf, "%g", i);
	}

	Init _Str::sPrintf(int sizeOf, cChar* f, ...) {
		va_list arg;
		va_start(arg, f);
		auto base = _Str::sPrintfv(sizeOf, f, arg);
		va_end(arg);
		return base;
	}

	Init _Str::sPrintfv(int sizeOf, cChar* f, va_list arg) {
		char *val;
		int len_ = vasprintf(&val, f, arg);
		Qk_ASSERT(len_ >= 0);

		uint32_t len = len_;
		Init base{ {&Allocator::Default, val, len + 1}, len };

		switch (sizeOf) {
			case 1: break;
			case 2: {
				auto b = codec_decode_to_ucs2(kUTF8_Encoding, Buffer::from(val, len));
				base = { *(Ptr*)&b._ptr, b.length() };
				b.collapse();
				break;
			}
			case 4: {
				auto b = codec_decode_to_unicode(kUTF8_Encoding, Buffer::from(val, len));
				base = { *(Ptr*)&b._ptr, b.length() };
				b.collapse();
				break;
			}
			default: Qk_Fatal("I won't support it, format");
		}

		return base;
	}

	String _Str::printfv(cChar* f, va_list arg) {
		auto base = _Str::sPrintfv(1, f, arg);
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
		auto base = _Str::sPrintfv(1, f, arg);
		va_end(arg);
		StringImpl s;
		if (base.ptr.val)
			s.StringBase::assign(base, 1);
		Qk_ReturnLocal(s);
	}

	// --------------- StringBase ---------------

	static Ref* NewRef(Init init, uint8_t sizeOf) {
		auto l = (Ref*)::malloc(sizeof(Ref));
		Qk_ASSERT(init.ptr.allocator);
		l->allocator = init.ptr.allocator;
		l->val = init.ptr.val;
		l->capacity = init.ptr.capacity * sizeOf;
		l->length = init.length * sizeOf;
		l->ref = 1;
		l->flag = 127645561;
		return l;
	}

	static void ReleaseRef(Ref* l) {
		Qk_ASSERT(l->ref > 0);
		if ( --l->ref == 0) { // After entering, do not reverse
			l->flag = 0; // destroy flag
			l->free();
			::free(l);
		}
	}

	static bool RetainRef(Ref* l) { // retain long string
		if (l->flag == 127645561 && l->ref++ > 0)
			return l->flag == 127645561; // safe check once
		return false;
	}

	void StringBase::clear() {
		if (_val.s.length < 0) {
			ReleaseRef(_val.r);
			_val.r = nullptr;
		}
		_val.s.length = 0;
	}

	StringBase::StringBase(): _val{.s={{0},0}} {
	} // empty

	StringBase::StringBase(const StringBase& str): _val(str._val)
	{
		if (_val.s.length < 0) {
			if (!RetainRef(_val.r)) { // Retain long fail
				_val = {.s={{0},0}}; // clear
			}
		}
	}

	StringBase::StringBase(uint32_t len, uint8_t sizeOf)
		: _val{.s={{0},0}}
	{
		resize(len, sizeOf); // alloc
	}

	StringBase::StringBase(Init init, uint8_t sizeOf)
		: _val{.s={{0},0}}
	{ // use long string
		if (init.ptr.val) {
			_val.s.length = -1; // mark long string
			_val.r = NewRef(init, sizeOf);
		}
	}

	StringBase::~StringBase() {
		Qk_ASSERT(_val.s.length >= 0);
	}

	void StringBase::assign(Init init, uint8_t sizeOf) {
		StringBase str(init, sizeOf);
		assign(str);
		str.clear(); // release
	}

	void StringBase::assign(const StringBase& s) {
		if (s._val.s.length < 0) { // src long
			if (_val.s.length < 0) { // long
				if (_val.r == s._val.r) {
					return;
				}
				ReleaseRef(_val.r);
			}
			if (RetainRef(s._val.r)) { // Retain long string
				_val = s._val;
			} else { // fail
				_val = {.s={{0},0}};
			}
		} else { // src short
			if (_val.s.length < 0) { // long
				ReleaseRef(_val.r);
			}
			_val = {.s=s._val.s};
		}
	}

	cAllocator* StringBase::allocator() const {
		return _val.s.length < 0 ? _val.r->allocator: &Allocator::Default;
	}

	uint32_t StringBase::size() const {
		return _val.s.length < 0 ? _val.r->length: _val.s.length;
	}

	uint32_t StringBase::capacity() const {
		return _val.s.length < 0 ? _val.r->capacity: MAX_SHORT_LEN;
	}

	char* StringBase::ptr() {
		return _val.s.length < 0 ? (char*)_val.r->val: (char*)_val.s.val;
	}

	const char* StringBase::ptr() const {
		return _val.s.length < 0 ? (cChar*)_val.r->val: (cChar*)_val.s.val;
	}

	typedef Allocator::Ptr<void> VoidPtr;

	char* StringBase::resize(uint32_t len, uint8_t sizeOf) {
		len *= sizeOf;

		if (_val.s.length < 0) { // long
			if (_val.r->ref > 1) {
				// TODO 需要从共享核心中分离出来, 多个线程同时使用一个Ref可能的安全问题
				auto old = _val.r;
				//_val.r = NewRef(len, 0, nullptr);
				_val.r = NewRef({{_val.r->allocator,0,0},len}, 1);
				// aalloc(_val.r, len + sizeOf, 1);
				_val.r->allocator->resize((VoidPtr*)_val.r, len + sizeOf, 1);
				::memcpy(_val.r->val, old->val, Qk_Min(len, old->length));
				ReleaseRef(old); // release old
			} else {
				_val.r->length = len;
				// aalloc(_val.l, len + sizeOf, 1);
				_val.r->allocator->resize((VoidPtr*)_val.r, len + sizeOf, 1);
			}
		}
		else if (len > MAX_SHORT_LEN) {
			//auto l = NewRef(len, 0, nullptr);
			auto l = NewRef({{&Allocator::Default,0,0},len}, 1);
			//aalloc(l, len + sizeOf, 1);
			l->allocator->resize((VoidPtr*)l, len + sizeOf, 1);
			::memcpy(l->val, _val.s.val, _val.s.length); // copy string
			_val.r = l;
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
			_val.r->val[len] = '\0';
		} else {
			::memset(_val.r->val + len, 0, sizeOf);
		}
		return _val.r->val;
	}

	Buffer StringBase::collapse() {
		auto s_len = _val.s.length;
		if (s_len < 0) {
			// long string
			if (_val.r->length == 0) {
				return Buffer();
			}
			// collapse
			auto len = _val.r->length;
			Ptr ptr{&Allocator::Default,nullptr,0};
			if (_val.r->ref > 1) { //
				// aalloc(&ptr, _val.r->capacity, 1);
				ptr.resize(_val.r->capacity);
				::memcpy(ptr.val, _val.r->val, ptr.capacity);
				ReleaseRef(_val.r); // release ref
			} else {
				ptr = *_val.r;
				::free(_val.r); // full delete memory
			}
			_val.s = {{0},0}; // use empty string
			return Buffer(len, ptr);
		} else {
			// short string
			if (s_len == 0) {
				return Buffer();
			}
			Ptr ptr{&Allocator::Default,nullptr,0};
			// aalloc(&ptr, MAX_SHORT_LEN + 4, 1);
			ptr.resize(MAX_SHORT_LEN + 4);
			::memcpy(ptr.val, _val.s.val, MAX_SHORT_LEN + 4);
			_val.s = {{0},0}; // use empty string
			return Buffer(ptr.val, s_len, ptr.capacity);
		}
	}

}
