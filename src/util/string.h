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

#ifndef __quark__util__string__
#define __quark__util__string__

#include "./array.h"
#include "./list.h"
#include <stdarg.h>
#include <string.h>

namespace qk {
	typedef char                 Char;
	typedef StringImpl<uint16_t> String2;
	typedef StringImpl<uint32_t> String4;
	typedef const char           cChar;
	typedef const String2        cString2;
	typedef const String4        cString4;

	class Qk_EXPORT StringBase {
	public:
		typedef NonObjectTraits Traits;
		typedef Allocator::Prt<char> Ptr;
		typedef void (*Realloc)(Ptr *ptr, uint32_t, uint32_t);
		typedef void (*Free)(void* ptr);
		static constexpr char MAX_SHORT_LEN = 32;
		uint32_t size() const;
		struct Long { // shared string
			struct Base { Ptr ptr; uint32_t length; };
			inline Ptr* ptr() { return reinterpret_cast<Ptr*>(this); }
			char     *val;
			uint32_t capacity, length, flag;
			std::atomic_int ref;
		};
		struct Short { char val[36]; char length; };
	protected:
		StringBase();
		StringBase(const StringBase& str);
		StringBase(uint32_t len, Realloc alloc, uint8_t sizeOf);
		StringBase(Long::Base base, uint8_t sizeOf);
		~StringBase();
		void     assign(Long::Base base, uint8_t sizeOf, Free free);
		void     assign(const StringBase& s, Free free);
		char*    ptr();
		cChar*   ptr() const;
		uint32_t capacity() const;
		char*    realloc(uint32_t len, Realloc alloc, Free free, uint8_t sizeOf);
		Buffer   collapse(Realloc alloc, Free free);
		void     clear(Free free);
		// union storage val
		union { struct Long* l; struct Short s; } _val;
		template<typename T, typename A> friend class StringImpl;
	};

	// TODO delete change string api
	template<typename T, typename A>
	class StringImpl: public StringBase {
	public:
		static constexpr int Type = sizeof(T) >> 1; // range 0,1,2
		static_assert(Type <= 2, "string type range only 0,1,2");
		// @constructors
		StringImpl(); // empty string constructors
		StringImpl(const StringImpl& s); // copy constructors
		StringImpl(Array<T, A>&& data); // right value copy constructors
		StringImpl(ArrayBuffer<T, A>&& data); // right value copy constructors
		StringImpl(const T* s); // copy constructors
		StringImpl(const T* s, uint32_t len); // copy constructors
		StringImpl(const T* a, uint32_t aLen, const T* b, uint32_t bLen); // copy constructors
		StringImpl(char i); // char string

		StringImpl(int32_t i); // number to string constructors
		StringImpl(uint32_t i);
		StringImpl(int64_t i);
		StringImpl(uint64_t i);
		StringImpl(float i);
		StringImpl(double i);

		~StringImpl();
		/**
		 * @method format string
		 */
		static StringImpl format(cChar* format, ...);

		// =========================================
		// modify method assign, append assign ...
		// =========================================
		// assign operator, call assign()
		StringImpl& operator=(const T* s);
		StringImpl& operator=(const StringImpl& s);
		// assign value operator
		StringImpl& assign(const T* s, uint32_t len); // operator=
		StringImpl& assign(const T s); // operator=
		// operator+=
		// concat string to current this, call append()
		StringImpl& operator+=(const T* s);
		StringImpl& operator+=(const StringImpl& s);
		StringImpl& operator+=(const T s);
		// append string to current this
		StringImpl& append(const T* s, uint32_t len = 0); // operator+=
		StringImpl& append(const StringImpl& s); // operator+=
		StringImpl& append(const T s); // operator+=
		// collapse to array buffer
		ArrayBuffer<T, A> collapse();
		ArrayWeak<T> array() const;
		// upper, lower
		StringImpl& upperCase(); // change current this string
		StringImpl& lowerCase(); // change current this string

		// =========================================
		// const method
		// =========================================
		// operator+
		// concat string, create new string
		// call StringImpl(const T* a, uint32_t a_len, const T* b, uint32_t b_len)
		StringImpl operator+(const T* s) const;
		StringImpl operator+(const StringImpl& s) const;
		StringImpl operator+(const T s) const;

		// operator compare
		bool operator==(const T* s) const;
		inline bool operator==(const StringImpl& s) const { return operator==(s.c_str()); }
		bool operator!=(const T* s) const;
		inline bool operator!=(const StringImpl& s) const { return operator!=(s.c_str()); }
		bool operator> (const T* s) const;
		inline bool operator> (const StringImpl& s) const { return operator>(s.c_str()); }
		bool operator< (const T* s) const;
		inline bool operator< (const StringImpl& s) const { return operator<(s.c_str()); }
		bool operator>=(const T* s) const;
		inline bool operator>=(const StringImpl& s) const { return operator>=(s.c_str()); }
		bool operator<=(const T* s) const;
		inline bool operator<=(const StringImpl& s) const { return operator<=(s.c_str()); }

		inline bool  isEmpty() const;
		inline const T* c_str() const;
		inline const T* operator*() const;

		inline T operator[](uint32_t index) const;
		inline uint32_t length() const;
		inline uint32_t capacity() const;

		String toString() const; // to string
		// get string hash code
		uint64_t hashCode() const;
		StringImpl<T, A> copy() const;
		// trim
		StringImpl trim() const;
		StringImpl trimLeft() const;
		StringImpl trimRight() const;
		// substr
		StringImpl substr(uint32_t start, uint32_t length = 0x7FFFFFFF) const;
		StringImpl substring(uint32_t start, uint32_t end = 0x7FFFFFFF) const;
		// upper, lower
		StringImpl toUpperCase() const; // create new string
		StringImpl toLowerCase() const; // create new string
		// index_of
		int indexOf(const StringImpl& s, uint32_t start = 0) const;
		int lastIndexOf(const StringImpl& s, uint32_t start = 0x7FFFFFFF) const;
		// replace
		StringImpl replace(const StringImpl& s, const StringImpl& rep) const;
		StringImpl replaceAll(const StringImpl& s, const StringImpl& rep) const;
		// split
		Array<StringImpl> split(const StringImpl& sp) const;
		// to number
		template<typename T2> T2   toNumber()        const;
		template<typename T2> bool toNumber(T2* out) const;
	private:
		template<typename T2> void number_(T2 i);
		T* val();
	};
}

// ----------------------------------------------------------------------------------

namespace qk {

	class Qk_EXPORT _Str {
	public:
		typedef void* (*Alloc)(uint32_t);
		typedef StringBase::Realloc Realloc;
		typedef StringBase::Long::Base Base;

		static cChar ws[8];
		static bool toNumber(const void* i, int iLen, int sizeOf, int32_t* o);
		static bool toNumber(const void* i, int iLen, int sizeOf, uint32_t* o);
		static bool toNumber(const void* i, int iLen, int sizeOf, int64_t* o);
		static bool toNumber(const void* i, int iLen, int sizeOf, uint64_t* o);
		static bool toNumber(const void* i, int iLen, int sizeOf, float* o);
		static bool toNumber(const void* i, int iLen, int sizeOf, double* o);
		static int  oPrinti(char* o, uint32_t oLen, int32_t i);
		static int  oPrinti(char* o, uint32_t oLen, uint32_t i);
		static int  oPrinti(char* o, uint32_t oLen, int64_t i);
		static int  oPrinti(char* o, uint32_t oLen, uint64_t i);
		static int  oPrinti(char* o, uint32_t oLen, float i);
		static int  oPrinti(char* o, uint32_t oLen, double i);
		static int  oPrintf(char* o, uint32_t oLen, cChar* f, ...);
		static Base sPrinti(int sizeOf, Alloc alloc, int32_t i); // num
		static Base sPrinti(int sizeOf, Alloc alloc, uint32_t i);
		static Base sPrinti(int sizeOf, Alloc alloc, int64_t i);
		static Base sPrinti(int sizeOf, Alloc alloc, uint64_t i);
		static Base sPrinti(int sizeOf, Alloc alloc, float i);
		static Base sPrinti(int sizeOf, Alloc alloc, double i);
		static Base sPrintf(int sizeOf, Alloc alloc, cChar* f, ...); // str
		static Base sPrintfv(int sizeOf, Alloc alloc, cChar* f, va_list arg);
		static String printfv(cChar* f, va_list arg);

		class Iterator { public: virtual bool next(String* out) = 0; };
		static String join(cString& sp, void* data, uint32_t len, bool (*iterator)(void* data, String* out));
		static String toString(const void* ptr, uint32_t len, int sizeOf);
		template<typename T>
		static String toString(const T& t) {
			return _To<T, has_object_type<T>::isObj>::call(t);
		}
		template<typename T, bool isObj> struct _To {
			static String call(const T& t) { return String("[unknown]"); }
		};
		template<typename T, typename A> struct _To<StringImpl<T, A>, false> {
			static String call(const StringImpl<T, A>& t) { return t.toString(); }
		};
		template<typename T> struct _To<T, true> {
			static String call(const T& t) { return t.toString(); }
		};
		template<typename T> struct _To<T*, false> {
			typedef T* Type;
			static String call(const Type& t) { return String::format("[%p]", t); }
		};
		static void strcpy(void* o, int sizeof_o, const void* i, int sizeof_i, uint32_t len);
		template <typename Output, typename Input>
		static void strcpy(Output* o, const Input* i, uint32_t len) {
			strcpy(o, sizeof(Output), i, sizeof(Input), len);
		}
		static uint32_t strlen(const void* s, int sizeOf);
		template <typename T>
		static uint32_t strlen(const T* s) {
			return strlen(s, sizeof(T));
		}
		static int memcmp(const void* s1, const void* s2, uint32_t len, int sizeOf);
		// 1 > , -1 <, 0 ==
		template <typename T>
		static int strcmp(const T* s1, const T* s2, uint32_t len) {
			return _Str::memcmp(s1, s2, len + 1, sizeof(T));
		}
		static int32_t index_of(
			const void* s1, uint32_t s1_len,
			const void* s2, uint32_t s2_len, uint32_t start, int sizeOf
		);
		static int32_t last_index_of(
			const void* s1, uint32_t s1_len,
			const void* s2, uint32_t s2_len, uint32_t start, int sizeOf
		);
		static void* replace(
			const void* s1, uint32_t s1_len,
			const void* s2, uint32_t s2_len,
			const void* rep, uint32_t rep_len,
			int sizeOf, uint32_t* out_len, uint32_t* capacity_out, bool all, Realloc realloc
		);
		static int tolower(int c);
		static int toupper(int c);
	};

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	StringImpl<T, A>::StringImpl() {
	}

	template <typename T, typename A>
	StringImpl<T, A>::StringImpl(const StringImpl& s): StringBase(s) {
	}

	template <typename T, typename A>
	StringImpl<T, A>::StringImpl(Array<T, A>&& data)
		: StringBase({(char*)*data, data.capacity(), data.length()}, sizeof(T)) {
		data.collapse();
	}

	template <typename T, typename A>
	StringImpl<T, A>::StringImpl(ArrayBuffer<T, A>&& data)
		: StringBase({(char*)*data, data.capacity(), data.length()}, sizeof(T)) {
		data.collapse();
	}

	template <typename T, typename A>
	StringImpl<T, A>::StringImpl(const T* s)
		: StringImpl<T, A>(s, _Str::strlen(s)) {
	}

	template <typename T, typename A>
	StringImpl<T, A>::StringImpl(const T* s, uint32_t len)
		: StringBase(len, (Realloc)&A::realloc, sizeof(T))
	{
		Qk_Assert(len < 268435456); // 256 MB
		_Str::strcpy(val(), s, len);
	}

	template <typename T, typename A>
	StringImpl<T, A>::StringImpl(const T* a, uint32_t aLen, const T* b, uint32_t bLen)
		: StringBase(aLen + bLen, (Realloc)&A::realloc, sizeof(T))
	{
		Qk_Assert(aLen < 268435456); // 256 MB
		Qk_Assert(bLen < 268435456); // 256 MB
		_Str::strcpy(val(),        a, aLen);
		_Str::strcpy(val() + aLen, b, bLen);
	}
	
	template <typename T, typename A>
	StringImpl<T, A>::StringImpl(char i): StringImpl(&i, 1) {
	}

	template <typename T, typename A>
	StringImpl<T, A>::StringImpl(int32_t i) {
		number_(i);
	}
	template <typename T, typename A>
	StringImpl<T, A>::StringImpl(uint32_t i) {
		number_(i);
	}
	template <typename T, typename A>
	StringImpl<T, A>::StringImpl(int64_t i) {
		number_(i);
	}
	template <typename T, typename A>
	StringImpl<T, A>::StringImpl(uint64_t i) {
		number_(i);
	}
	template <typename T, typename A>
	StringImpl<T, A>::StringImpl(float i) {
		number_(i);
	}
	template <typename T, typename A>
	StringImpl<T, A>::StringImpl(double i) {
		number_(i);
	}

	template <typename T, typename A>
	template <typename T2>
	void StringImpl<T, A>::number_(T2 i)
	{
		(void)(1 + i); // test number math operation
		if (sizeof(T) == 1) {
			_val.s.length = _Str::oPrinti(ptr(), MAX_SHORT_LEN, i);
		} else {
			auto base = _Str::sPrinti((int)sizeof(T), &A::alloc, i);
			if (base.ptr.val) {
				StringBase::assign(base, sizeof(T), &A::free );
			}
		}
	}

	template <typename T, typename A>
	StringImpl<T, A>::~StringImpl() {
		clear(&A::free);
	}

	template <typename T, typename A>
	StringImpl<T, A> StringImpl<T, A>::format(cChar* f, ...) {
		va_list arg;
		va_start(arg, f);
		auto base = _Str::sPrintfv(sizeof(T), &A::alloc, f, arg);
		va_end(arg);
		StringImpl s;
		if (base.ptr.val)
			s.StringBase::assign(base, sizeof(T), &A::free);
		Qk_ReturnLocal(s);
	}

	template <> Qk_EXPORT
	String StringImpl<>::format(cChar* f, ...);

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	bool StringImpl<T, A>::isEmpty() const { return size() == 0; }
	
	template <typename T, typename A>
	T StringImpl<T, A>::operator[](uint32_t index) const { return c_str()[index]; }

	template <typename T, typename A>
	const T* StringImpl<T, A>::c_str() const { return (const T*)ptr(); }

	template <typename T, typename A>
	const T* StringImpl<T, A>::operator*() const { return (const T*)ptr(); }

	template <typename T, typename A>
	uint32_t StringImpl<T, A>::length() const { return size() >> Type; }

	template <typename T, typename A>
	uint32_t StringImpl<T, A>::capacity() const { return StringBase::capacity() >> Type; }

	template <typename T, typename A>
	T* StringImpl<T, A>::val() { return (T*)ptr(); }

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	StringImpl<T, A>& StringImpl<T, A>::operator=(const T* s) {
		return assign(s, _Str::strlen(s));
	}

	template <typename T, typename A>
	StringImpl<T, A>& StringImpl<T, A>::operator=(const StringImpl& s) {
		StringBase::assign(s, &A::free);
		return *this;
	}

	template <typename T, typename A>
	StringImpl<T, A>& StringImpl<T, A>::assign(const T* s, uint32_t len) {
		Qk_Assert(len < 268435456); // 256 MB
		_Str::strcpy((T*)realloc(len, (Realloc)&A::realloc, &A::free, sizeof(T)), s, len); // copy str
		return *this;
	}

	template <typename T, typename A>
	StringImpl<T, A>& StringImpl<T, A>::assign(const T s) {
		return assign(&s, 1);
	}

	template <typename T, typename A>
	StringImpl<T, A> StringImpl<T, A>::operator+(const T* s) const {
		return StringImpl(c_str(), length(), s, _Str::strlen(s));
	}

	template <typename T, typename A>
	StringImpl<T, A> StringImpl<T, A>::operator+(const StringImpl& s) const {
		return StringImpl(c_str(), length(), s.c_str(), s.length());
	}
	
	template <typename T, typename A>
	StringImpl<T, A> StringImpl<T, A>::operator+(const T s) const {
		return StringImpl(c_str(), length(), &s, 1);
	}

	template <typename T, typename A>
	StringImpl<T, A>& StringImpl<T, A>::operator+=(const T* s) {
		return append(s, _Str::strlen(s));
	}

	template <typename T, typename A>
	StringImpl<T, A>& StringImpl<T, A>::operator+=(const StringImpl& s) {
		return length() ? append(s.c_str(), s.length()): operator=(s);
	}
	
	template <typename T, typename A>
	StringImpl<T, A>& StringImpl<T, A>::operator+=(const T s) {
		return append(&s, 1);
	}

	template <typename T, typename A>
	StringImpl<T, A>& StringImpl<T, A>::append(const T* s, uint32_t len) {
		if (len > 0) {
			uint32_t len_raw = length();
			auto str = (T*)realloc(len_raw + len, (Realloc)&A::realloc, &A::free, sizeof(T));
			_Str::strcpy(str + len_raw, s, len);
		}
		return *this;
	}

	template <typename T, typename A>
	StringImpl<T, A>& StringImpl<T, A>::append(const StringImpl& s) {
		return operator+=(s);
	}
	
	template <typename T, typename A>
	StringImpl<T, A>& StringImpl<T, A>::append(const T s) {
		return append(&s, 1);
	}

	uint64_t hashCode(const void* data, uint32_t len);

	template <typename T, typename A>
	uint64_t StringImpl<T, A>::hashCode() const {
		return qk::hashCode(c_str(), length() * sizeof(T));
	}

	template <typename T, typename A>
	ArrayBuffer<T, A> StringImpl<T, A>::collapse() {
		Buffer b = StringBase::collapse((Realloc)&A::realloc, &A::free);
		uint32_t l = b.length(), c = b.capacity();
		return ArrayBuffer<T, A>::from((T*)b.collapse(), l / sizeof(T), c / sizeof(T));
	}

	template <typename T, typename A>
	ArrayWeak<T> StringImpl<T, A>::array() const {
		return ArrayWeak<T>(c_str(), length());
	}

	template <typename T, typename A>
	StringImpl<T, A> StringImpl<T, A>::copy() const {
		return length() ? StringImpl(c_str(), length()): StringImpl();
	}

	template <typename T, typename A>
	String StringImpl<T, A>::toString() const {
		return _Str::toString(c_str(), length(), sizeof(T));
	}

	template <> Qk_EXPORT
	String StringImpl<>::toString() const;

	template <typename T, typename A>
	StringImpl<T, A> Array<T, A>::collapseString() {
		return StringImpl<T, A>(std::move(*this));
	}

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	String Array<T, A>::join(cString& sp) const {
		IteratorConst it[] = { begin(), end() };
		return _Str::join(sp, it, length(), [](void* data, String* out) -> bool {
			auto it = static_cast<IteratorConst*>(data);
			return it[0] == it[1] ? false: ((*out = _Str::toString(*(it[0]++))), true);
		});
	}

	template<typename T, typename A>
	String List<T, A>::join(cString& sp) const {
		IteratorConst it[] = { begin(), end() };
		return _Str::join(sp, it, length(), [](void* data, String* out) -> bool {
			auto it = static_cast<IteratorConst*>(data);
			return it[0] == it[1] ? false: ((*out = _Str::toString(*(it[0]++))), true);
		});
	}

	template<typename T, typename A>
	String Array<T, A>::toString() const {
		return join(String());
	}

	template<typename T, typename A>
	String List<T, A>::toString() const {
		return join(String());
	}

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	bool StringImpl<T, A>::operator==(const T* s) const {
		return _Str::strcmp(c_str(), s, length()) == 0;
	}

	template <typename T, typename A>
	bool StringImpl<T, A>::operator!=(const T* s) const {
		return _Str::strcmp(c_str(), s, length()) != 0;
	}

	template <typename T, typename A>
	bool StringImpl<T, A>::operator>(const T* s) const {
		return _Str::strcmp(c_str(), s, length()) > 0;
	}

	template <typename T, typename A>
	bool StringImpl<T, A>::operator<(const T* s) const {
		return _Str::strcmp(c_str(), s, length()) < 0;
	}

	template <typename T, typename A>
	bool StringImpl<T, A>::operator>=(const T* s) const {
		return _Str::strcmp(c_str(), s, length()) >= 0;
	}

	template <typename T, typename A>
	bool StringImpl<T, A>::operator<=(const T* s) const {
		return _Str::memcmp(c_str(), s, length()) <= 0;
	}

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	StringImpl<T, A> StringImpl<T, A>::trim() const {
		uint32_t start = 0;
		uint32_t len = length();
		uint32_t end = len;
		const T* _val = c_str();
		for (; start < len; start++) {
			if (strchr(_Str::ws, _val[start]) == nullptr) {
				break;
			}
		}
		if (start == len) {
			return StringImpl(); // empty string
		} else {
			for (; end > 0; end--) {
				if (strchr(_Str::ws, _val[end - 1]) == nullptr) {
					break;
				}
			}
		}
		if (start == 0 && end == len) {
			return StringImpl(*this);
		}
		return substring(start, end);
	}

	template <typename T, typename A>
	StringImpl<T, A> StringImpl<T, A>::trimLeft() const {
		const T* _val = c_str();
		auto len = length();
		for (uint32_t start = 0; start < len; start++) {
			if (strchr(_Str::ws, _val[start]) == nullptr) {
				if (start == 0) {
					return StringImpl(*this);
				} else {
					return substring(start);
				}
			}
		}
		return StringImpl();
	}

	template <typename T, typename A>
	StringImpl<T, A> StringImpl<T, A>::trimRight() const {
		const T* _val = c_str();
		auto len = length();
		for (uint32_t end = len; end > 0; end--) {
			if (strchr(_Str::ws, _val[end - 1]) == nullptr) {
				if (end == len) {
					return StringImpl(*this);
				} else {
					return substring(0, end);
				}
			}
		}
		return StringImpl();
	}

	template <typename T, typename A>
	StringImpl<T, A> StringImpl<T, A>::substr(uint32_t start, uint32_t len) const {
		auto s = ArrayWeak<T>(c_str(), length())->slice(start, start + len);
		return StringImpl(*s, s.length());
	}

	template <typename T, typename A>
	StringImpl<T, A> StringImpl<T, A>::substring(uint32_t start, uint32_t end) const {
		auto s = ArrayWeak<T>(c_str(), length())->slice(start, end);
		return StringImpl(*s, s.length());
	}

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	StringImpl<T, A>&  StringImpl<T, A>::upperCase() {
		T* s = (T*)realloc(length(), (Realloc)&A::realloc, &A::free, sizeof(T));
		for (uint32_t i = 0, len = length(); i < len; i++, s++) {
			*s = _Str::toupper(*s);
		}
		return *this;
	}

	template <typename T, typename A>
	StringImpl<T, A>&  StringImpl<T, A>::lowerCase() {
		T* s = (T*)realloc(length(), (Realloc)&A::realloc, &A::free, sizeof(T));
		for (uint32_t i = 0, len = length(); i < len; i++, s++)
			*s = _Str::tolower(*s);
		return *this;
	}

	template <typename T, typename A>
	StringImpl<T, A> StringImpl<T, A>::toUpperCase() const {
		return StringImpl(*this).upperCase();
	}

	template <typename T, typename A>
	StringImpl<T, A> StringImpl<T, A>::toLowerCase() const {
		return StringImpl(*this).lowerCase();
	}

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	int StringImpl<T, A>::indexOf(const StringImpl& s, uint32_t start) const {
		return _Str::index_of(c_str(), length(), s.c_str(), s.length(), start, sizeof(T));
	}

	template <typename T, typename A>
	int StringImpl<T, A>::lastIndexOf(const StringImpl& s, uint32_t start) const {
		return _Str::last_index_of(c_str(), length(), s.c_str(), s.length(), start, sizeof(T));
	}

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	StringImpl<T, A> StringImpl<T, A>::replace(const StringImpl& s, const StringImpl& rep) const {
		StringImpl r;
		uint32_t len, capacity;
		T* val = (T*)_Str::replace(
			c_str(), length(), s.c_str(), s.length(),
			rep.c_str(), rep.length(), sizeof(T), &len, &capacity, false, (Realloc)&Allocator::realloc
		);
		r.StringBase::assign({val,capacity,len}, sizeof(T), &A::free );
		return r;
	}

	template <typename T, typename A>
	StringImpl<T, A> StringImpl<T, A>::replaceAll(const StringImpl& s, const StringImpl& rep) const {
		StringImpl r;
		uint32_t len, capacity;
		T* val = (T*)_Str::replace(
			c_str(), length(), s.c_str(), s.length(),
			rep.c_str(), rep.length(), sizeof(T), &len, &capacity, true, (Realloc)&Allocator::realloc
		);
		r.StringBase::assign({val,capacity,len}, sizeof(T), &A::free );
		return r;
	}

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	Array<StringImpl<T, A>>
	StringImpl<T, A>::split(const StringImpl& sp) const {
		Array<StringImpl<T, A>> r;
		int splen = sp.length();
		int prev = 0, index = 0;
		while ((index = indexOf(sp, prev)) != -1) {
			// printf("A,index=%d,prev=%d\n", index, prev);
			r.push(substring(prev, index));
			prev = index + splen;
		}
		r.push( substring(prev) );
		return r;
	}

	// --------------------------------------------------------------------------------

	template<typename T, typename A>
	template<typename T2>
	T2 StringImpl<T, A>::toNumber() const {
		T2 o;
		_Str::toNumber(c_str(), length(), sizeof(T), &o);
		return o;
	}

	template<typename T, typename A>
	template<typename T2>
	bool StringImpl<T, A>::toNumber(T2* o) const {
		return _Str::toNumber(c_str(), length(), sizeof(T), o);
	}
}

namespace std {
	template<typename T, typename A>
	struct hash<qk::StringImpl<T, A>> {
		size_t operator()(const qk::StringImpl<T, A>& val) const {
			return val.hashCode();
		}
	};
}

#endif
