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

#ifndef __noug__util__string__
#define __noug__util__string__

#include "./array.h"
#include "./list.h"
#include <stdarg.h>
#include <string.h>

namespace noug {

	typedef        char                  Char;
	typedef        ArrayString<uint16_t> String2;
	typedef        ArrayString<uint32_t> String4;
	typedef const  char                  cChar;
	typedef const  ArrayString<uint16_t> cString2;
	typedef const  ArrayString<uint32_t> cString4;
	
	class N_EXPORT ArrayStringBase: public Object {
	public:
		typedef void* (*AAlloc)(void* val, uint32_t, uint32_t*, uint32_t size_of);
		typedef void  (*Free)(void* ptr);
		static constexpr char MAX_SHORT_LEN = 32;
		struct ShortStr { char val[36]; char length; };
		struct LongStr {
			uint32_t length, capacity;
			char*    val;
			std::atomic_int ref;
		};
		union Value {
			struct ShortStr s;
			struct LongStr* l; // share long string core
		};
		uint32_t size() const;
	protected:
		ArrayStringBase();
		ArrayStringBase(const ArrayStringBase& str);
		ArrayStringBase(uint32_t len, AAlloc alloc, uint8_t size_of);
		ArrayStringBase(uint32_t len, uint32_t capacity, char* val, uint8_t size_of);
		void assign(    uint32_t len, uint32_t capacity, char* val, uint8_t size_of, Free free);
		void assign(const ArrayStringBase& s, Free free);
		char*       ptr();
		const char* ptr() const;
		uint32_t capacity() const;
		char* realloc(uint32_t len, AAlloc alloc, Free free, uint8_t size_of);
		Buffer collapse(AAlloc alloc, Free free);
		void   clear(Free free);
		static LongStr* NewLong(uint32_t length, uint32_t capacity, char* val);
		static void Release(LongStr* l, Free free);
		static void Retain (LongStr* l);
		Value  _val;
		template<typename T, typename A> friend class ArrayString;
	};

	template<typename T, typename A>
	class ArrayString: public ArrayStringBase {
	public:
		// constructors
		ArrayString(); // empty string constructors
		ArrayString(const ArrayString& s); // copy constructors
		ArrayString(Array<T, A>&& data); // right value copy constructors
		ArrayString(ArrayBuffer<T, A>&& data); // right value copy constructors
		ArrayString(const T* s); // copy constructors
		ArrayString(const T* s, uint32_t len); // copy constructors
		ArrayString(const T* a, uint32_t a_len, const T* b, uint32_t b_len); // copy constructors
		
		ArrayString(int32_t i); // number to string constructors
		ArrayString(uint32_t i);
		ArrayString(int64_t i);
		ArrayString(uint64_t i);
		ArrayString(float i);
		ArrayString(double i);

		virtual ~ArrayString();
		virtual String to_string() const;
		/**
		 * @func format string
		 */
		static ArrayString format(cChar* format, ...);

		inline bool  is_empty() const;
		inline const T* c_str() const;
		inline const T* operator*() const;

		inline T operator[](uint32_t index) const;
		inline uint32_t length() const;
		inline uint32_t capacity() const;

		// assign operator, call assign()
		ArrayString& operator=(const T* s);
		ArrayString& operator=(const ArrayString& s);
		// assign value operator
		ArrayString& assign(const T* s, uint32_t len); // operator=
		ArrayString& assign(const T s); // operator=
		// operator+
		// concat string, create new string
		// call ArrayString(const T* a, uint32_t a_len, const T* b, uint32_t b_len)
		ArrayString operator+(const T* s) const;
		ArrayString operator+(const ArrayString& s) const;
		ArrayString operator+(const T s) const;
		// operator+=
		// concat string to current this, call append()
		ArrayString& operator+=(const T* s);
		ArrayString& operator+=(const ArrayString& s);
		ArrayString& operator+=(const T s);

		// append string to current this
		ArrayString& append(const T* s, uint32_t len = 0); // operator+=
		ArrayString& append(const ArrayString& s); // operator+=
		ArrayString& append(const T s); // operator+=
		// get string hash code
		uint64_t hash_code() const;
		// collapse to array buffer
		ArrayBuffer<T, A> collapse();
		ArrayString<T, A> copy() const;

		// operator compare
		bool operator==(const T* s) const;
		inline bool operator==(const ArrayString& s) const { return operator==(s.c_str()); }
		bool operator!=(const T* s) const;
		inline bool operator!=(const ArrayString& s) const { return operator!=(s.c_str()); }
		bool operator> (const T* s) const;
		inline bool operator> (const ArrayString& s) const { return operator>(s.c_str()); }
		bool operator< (const T* s) const;
		inline bool operator< (const ArrayString& s) const { return operator<(s.c_str()); }
		bool operator>=(const T* s) const;
		inline bool operator>=(const ArrayString& s) const { return operator>=(s.c_str()); }
		bool operator<=(const T* s) const;
		inline bool operator<=(const ArrayString& s) const { return operator<=(s.c_str()); }

		// trim
		ArrayString trim() const;
		ArrayString trim_left() const;
		ArrayString trim_right() const;
		// substr
		ArrayString substr(uint32_t start, uint32_t length = 0x7FFFFFFF) const;
		ArrayString substring(uint32_t start, uint32_t end = 0x7FFFFFFF) const;
		// upper, lower
		ArrayString& upper_case(); // change current this string
		ArrayString& lower_case(); // change current this string
		ArrayString  to_upper_case() const; // create new string
		ArrayString  to_lower_case() const; // create new string
		// index_of
		int index_of(const ArrayString& s, uint32_t start = 0) const;
		int last_index_of(const ArrayString& s, uint32_t start = 0x7FFFFFFF) const;
		// replace
		ArrayString replace(const ArrayString& s, const ArrayString& rep) const;
		ArrayString replace_all(const ArrayString& s, const ArrayString& rep) const;
		// split
		Array<ArrayString> split(const ArrayString& sp) const;
		// to number
		template<typename T2> T2   to_number()        const;
		template<typename T2> bool to_number(T2* out) const;
	private:
		template<typename T2> void number_(T2 i);
		T* val();
	};

}

// -------------------------------------- IMPL --------------------------------------

namespace noug {

	class N_EXPORT _Str {
	public:
		// static methods
		typedef char T;
		typedef void* (*Alloc)(uint32_t);
		typedef void* (*AAlloc)(void*, uint32_t, uint32_t*, uint32_t);

		struct Size { uint32_t len; uint32_t capacity; };
		static cChar ws[8];
		static bool to_number(const void* i, int sizeof_i, int len_i, int32_t* o);
		static bool to_number(const void* i, int sizeof_i, int len_i, uint32_t* o);
		static bool to_number(const void* i, int sizeof_i, int len_i, int64_t* o);
		static bool to_number(const void* i, int sizeof_i, int len_i, uint64_t* o);
		static bool to_number(const void* i, int sizeof_i, int len_i, float* o);
		static bool to_number(const void* i, int sizeof_i, int len_i, double* o);
		static int32_t format_n(char* o, uint32_t len_o, cChar* f, ...);
		static int32_t format_n(char* o, uint32_t len_o, int32_t i);
		static int32_t format_n(char* o, uint32_t len_o, uint32_t i);
		static int32_t format_n(char* o, uint32_t len_o, int64_t i);
		static int32_t format_n(char* o, uint32_t len_o, uint64_t i);
		static int32_t format_n(char* o, uint32_t len_o, float i);
		static int32_t format_n(char* o, uint32_t len_o, double i);
		static void* format(Size* size, int size_of, Alloc alloc, cChar* f, ...);
		static void* format(Size* size, int size_of, Alloc alloc, cChar* f, va_list arg);
		static void* format(Size* size, int size_of, Alloc alloc, int32_t i);
		static void* format(Size* size, int size_of, Alloc alloc, uint32_t i);
		static void* format(Size* size, int size_of, Alloc alloc, int64_t i);
		static void* format(Size* size, int size_of, Alloc alloc, uint64_t i);
		static void* format(Size* size, int size_of, Alloc alloc, float i);
		static void* format(Size* size, int size_of, Alloc alloc, double i);
		class Iterator { public: virtual bool next(String* out) = 0; };
		static String join(bool (*iterator)(void* data, String* out), cString& sp, void* data);
		static String to_string(const void* ptr, uint32_t len, int size_of);
		template<typename T>
		static String to_string(const T& t) {
			return _To<T, has_object_type<T>::isObj>::call(t);
		}
		template<typename T, bool isObj> struct _To {
			static String call(const T& t) { return String("[unknown]"); }
		};
		template<typename T> struct _To<T, true> {
			static String call(const T& t) { return t.to_string(); }
		};
		template<typename T> struct _To<T*, false> {
			typedef T* Type;
			static String call(const Type& t) { return String::format("[%p]", t); }
		};
		static void  strcpy(void* o, int sizeof_o, const void* i, int sizeof_i, uint32_t len);
		template <typename Output, typename Input>
		static void strcpy(Output* o, const Input* i, uint32_t len) {
			strcpy(o, sizeof(Output), i, sizeof(Input), len);
		}
		static uint32_t strlen(const void* s, int size_of);
		template <typename T>
		static uint32_t strlen(const T* s) {
			return strlen(s, sizeof(T));
		}
		static int memcmp(const void* s1, const void* s2, uint32_t len, int size_of);
		// 1 > , -1 <, 0 ==
		template <typename T>
		static int memcmp(const T* s1, const T* s2, uint32_t len) {
			return _Str::memcmp(s1, s2, len, sizeof(T));
		}
		static int32_t index_of(
			const void* s1, uint32_t s1_len,
			const void* s2, uint32_t s2_len, uint32_t start, int size_of
		);
		static int32_t last_index_of(
			const void* s1, uint32_t s1_len,
			const void* s2, uint32_t s2_len, uint32_t start, int size_of
		);
		static void* replace(
			const void* s1, uint32_t s1_len,
			const void* s2, uint32_t s2_len,
			const void* rep, uint32_t rep_len,
			int size_of, uint32_t* out_len, uint32_t* capacity_out, bool all, AAlloc realloc
		);
		static int tolower(int c);
		static int toupper(int c);
	};

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	ArrayString<T, A>::ArrayString() {
	}

	template <typename T, typename A>
	ArrayString<T, A>::ArrayString(const ArrayString& s): ArrayStringBase(s) {
	}

	template <typename T, typename A>
	ArrayString<T, A>::ArrayString(Array<T, A>&& data)
		: ArrayStringBase(data.length(), data.capacity(), (char*)data.collapse(), sizeof(T)) {
	}

	template <typename T, typename A>
	ArrayString<T, A>::ArrayString(ArrayBuffer<T, A>&& data)
		: ArrayStringBase(data.length(), data.capacity(), (char*)data.collapse(), sizeof(T)) {
	}

	template <typename T, typename A>
	ArrayString<T, A>::ArrayString(const T* s)
		: ArrayString<T, A>(s, _Str::strlen(s)) {
	}

	template <typename T, typename A>
	ArrayString<T, A>::ArrayString(const T* s, uint32_t len)
		: ArrayStringBase(len, &A::aalloc, sizeof(T))
	{
		_Str::strcpy(val(), s, len);
	}

	template <typename T, typename A>
	ArrayString<T, A>::ArrayString(const T* a, uint32_t a_len, const T* b, uint32_t b_len)
		: ArrayStringBase(a_len + b_len, &A::aalloc, sizeof(T))
	{
		_Str::strcpy(val(),         a, a_len);
		_Str::strcpy(val() + a_len, b, b_len);
	}

	template <typename T, typename A>
	ArrayString<T, A>::ArrayString(int32_t i) {
		number_(i);
	}
	template <typename T, typename A>
	ArrayString<T, A>::ArrayString(uint32_t i) {
		number_(i);
	}
	template <typename T, typename A>
	ArrayString<T, A>::ArrayString(int64_t i) {
		number_(i);
	}
	template <typename T, typename A>
	ArrayString<T, A>::ArrayString(uint64_t i) {
		number_(i);
	}
	template <typename T, typename A>
	ArrayString<T, A>::ArrayString(float i) {
		number_(i);
	}
	template <typename T, typename A>
	ArrayString<T, A>::ArrayString(double i) {
		number_(i);
	}

	template <typename T, typename A>
	template <typename T2>
	void ArrayString<T, A>::number_(T2 i)
	{
		(void)(1 + i); // test number math operation
		if (sizeof(T) == 1) {
			_val.s.length = _Str::format_n(ptr(), MAX_SHORT_LEN, i);
		} else {
			_Str::Size size;
			T* v = (T*)_Str::format(&size, (int)sizeof(T), &A::alloc, i);
			if (v) {
				ArrayStringBase::assign( size.len, size.capacity, v, sizeof(T), &A::free );
			}
		}
	}
	
	template <typename T, typename A>
	ArrayString<T, A>::~ArrayString() {
		clear(&A::free);
	}

	template <typename T, typename A>
	ArrayString<T, A> ArrayString<T, A>::format(cChar* f, ...) {
		va_list arg;
		va_start(arg, f);
		_Str::Size size;
		T* v = (T*)_Str::format(&size, sizeof(T), &A::alloc, f, arg);
		va_end(arg);

		ArrayString s;
		if (v)
			s.ArrayStringBase::assign(size.len, size.capacity, (char*)v, sizeof(T), &A::free);
		return s;
	}

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	bool ArrayString<T, A>::is_empty() const { return size() == 0; }
	
	template <typename T, typename A>
	T ArrayString<T, A>::operator[](uint32_t index) const { return c_str()[index]; }

	template <typename T, typename A>
	const T* ArrayString<T, A>::c_str() const { return (const T*)ptr(); }

	template <typename T, typename A>
	const T* ArrayString<T, A>::operator*() const { return (const T*)ptr(); }

	template <typename T, typename A>
	uint32_t ArrayString<T, A>::length() const { return size() / sizeof(T); }

	template <typename T, typename A>
	uint32_t ArrayString<T, A>::capacity() const { return ArrayStringBase::capacity() / sizeof(T); }
	
	template <typename T, typename A>
	T* ArrayString<T, A>::val() { return (T*)ptr(); }

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	ArrayString<T, A>& ArrayString<T, A>::operator=(const T* s) {
		return assign(s, _Str::strlen(s));
	}

	template <typename T, typename A>
	ArrayString<T, A>& ArrayString<T, A>::operator=(const ArrayString& s) {
		ArrayStringBase::assign(s, &A::free);
		return *this;
	}

	template <typename T, typename A>
	ArrayString<T, A>& ArrayString<T, A>::assign(const T* s, uint32_t len) {
		_Str::strcpy((T*)realloc(len, &A::aalloc, &A::free, sizeof(T)), s, len); // copy str
		return *this;
	}

	template <typename T, typename A>
	ArrayString<T, A>& ArrayString<T, A>::assign(const T s) {
		return assign(&s, 1);
	}

	template <typename T, typename A>
	ArrayString<T, A> ArrayString<T, A>::operator+(const T* s) const {
		return ArrayString(c_str(), length(), s, _Str::strlen(s));
	}

	template <typename T, typename A>
	ArrayString<T, A> ArrayString<T, A>::operator+(const ArrayString& s) const {
		return ArrayString(c_str(), length(), s.c_str(), s.length());
	}
	
	template <typename T, typename A>
	ArrayString<T, A> ArrayString<T, A>::operator+(const T s) const {
		return ArrayString(c_str(), length(), &s, 1);
	}

	template <typename T, typename A>
	ArrayString<T, A>& ArrayString<T, A>::operator+=(const T* s) {
		return append(s, _Str::strlen(s));
	}

	template <typename T, typename A>
	ArrayString<T, A>& ArrayString<T, A>::operator+=(const ArrayString& s) {
		return length() ? append(s.c_str(), s.length()): operator=(s);
	}
	
	template <typename T, typename A>
	ArrayString<T, A>& ArrayString<T, A>::operator+=(const T s) {
		return append(&s, 1);
	}

	template <typename T, typename A>
	ArrayString<T, A>& ArrayString<T, A>::append(const T* s, uint32_t len) {
		if (len > 0) {
			uint32_t len_raw = length();
			auto str = (T*)realloc(len_raw + len, &A::aalloc, &A::free, sizeof(T));
			_Str::strcpy(str + len_raw, s, len);
		}
		return *this;
	}

	template <typename T, typename A>
	ArrayString<T, A>& ArrayString<T, A>::append(const ArrayString& s) {
		return operator+=(s);
	}
	
	template <typename T, typename A>
	ArrayString<T, A>& ArrayString<T, A>::append(const T s) {
		return append(&s, 1);
	}

	uint64_t hash_code(const void* data, uint32_t len);

	template <typename T, typename A>
	uint64_t ArrayString<T, A>::hash_code() const {
		return noug::hash_code(c_str(), length() * sizeof(T));
	}

	template <typename T, typename A>
	ArrayBuffer<T, A> ArrayString<T, A>::collapse() {
		Buffer b = ArrayStringBase::collapse(&A::aalloc, &A::free);
		uint32_t l = b.length(), c = b.capacity();
		return ArrayBuffer<T, A>::from((T*)b.collapse(), l / sizeof(T), c / sizeof(T));
	}

	template <typename T, typename A>
	ArrayString<T, A> ArrayString<T, A>::copy() const {
		return length() ? ArrayString(c_str(), length()): ArrayString();
	}

	template <typename T, typename A>
	String ArrayString<T, A>::to_string() const {
		return _Str::to_string(c_str(), length(), sizeof(T));
	}
	
	template <> N_EXPORT
	String ArrayString<>::to_string() const;

	template <typename T, typename A>
	ArrayString<T, A> Array<T, A>::collapse_string() {
		return ArrayString<T, A>(std::move(*this));
	}

	// --------------------------------------------------------------------------------
	
	template <typename T, typename A>
	String Array<T, A>::join(cString& sp) const {
		IteratorConst it[] = { begin(), end() };
		return _Str::join([](void* data, String* out) -> bool {
			 auto it = static_cast<IteratorConst*>(data);
			 return it[0] == it[1] ? false: ((*out = _Str::to_string(*(++(it[0])))), true);
		}, sp, it);
	}

	template<typename T, typename A>
	String List<T, A>::join(cString& sp) const {
		IteratorConst it[] = { begin(), end() };
		return _Str::join([](void* data, String* out) -> bool {
			 auto it = static_cast<IteratorConst*>(data);
			 return it[0] == it[1] ? false: ((*out = _Str::to_string(*(++(it[0])))), true);
		}, sp, it);
	}

	template<typename T, typename A>
	String Array<T, A>::to_string() const {
		return join(String());
	}

	template<typename T, typename A>
	String List<T, A>::to_string() const {
		return join(String());
	}

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	bool ArrayString<T, A>::operator==(const T* s) const {
		return _Str::memcmp(c_str(), s, length()) == 0;
	}

	template <typename T, typename A>
	bool ArrayString<T, A>::operator!=(const T* s) const {
		return _Str::memcmp(c_str(), s, length()) != 0;
	}

	template <typename T, typename A>
	bool ArrayString<T, A>::operator>(const T* s) const {
		return _Str::memcmp(c_str(), s, length()) > 0;
	}

	template <typename T, typename A>
	bool ArrayString<T, A>::operator<(const T* s) const {
		return _Str::memcmp(c_str(), s, length()) < 0;
	}

	template <typename T, typename A>
	bool ArrayString<T, A>::operator>=(const T* s) const {
		return _Str::memcmp(c_str(), s, length()) >= 0;
	}

	template <typename T, typename A>
	bool ArrayString<T, A>::operator<=(const T* s) const {
		return _Str::memcmp(c_str(), s, length()) <= 0;
	}

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	ArrayString<T, A> ArrayString<T, A>::trim() const {
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
			return ArrayString(); // empty string
		} else {
			for (; end > 0; end--) {
				if (strchr(_Str::ws, _val[end - 1]) == nullptr) {
					break;
				}
			}
		}
		if (start == 0 && end == len) {
			return ArrayString(*this);
		}
		return substring(start, end);
	}

	template <typename T, typename A>
	ArrayString<T, A> ArrayString<T, A>::trim_left() const {
		const T* _val = c_str();
		auto len = length();
		for (uint32_t start = 0; start < len; start++) {
			if (strchr(_Str::ws, _val[start]) == nullptr) {
				if (start == 0) {
					return ArrayString(*this);
				} else {
					return substring(start);
				}
			}
		}
		return ArrayString();
	}

	template <typename T, typename A>
	ArrayString<T, A> ArrayString<T, A>::trim_right() const {
		const T* _val = c_str();
		auto len = length();
		for (uint32_t end = len; end > 0; end--) {
			if (strchr(_Str::ws, _val[end - 1]) == nullptr) {
				if (end == len) {
					return ArrayString(*this);
				} else {
					return substring(0, end);
				}
			}
		}
		return ArrayString();
	}

	template <typename T, typename A>
	ArrayString<T, A> ArrayString<T, A>::substr(uint32_t start, uint32_t len) const {
		auto s = ArrayWeak<T, A>(c_str(), length()).slice(start, start + len);
		return ArrayString(*s, s.length());
	}

	template <typename T, typename A>
	ArrayString<T, A> ArrayString<T, A>::substring(uint32_t start, uint32_t end) const {
		auto s = ArrayWeak<T, A>(c_str(), length()).slice(start, end);
		return ArrayString(*s, s.length());
	}

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	ArrayString<T, A>&  ArrayString<T, A>::upper_case() {
		T* s = (T*)realloc(length(), &A::aalloc, &A::free, sizeof(T));
		for (uint32_t i = 0, len = length(); i < len; i++, s++) {
			*s = _Str::toupper(*s);
		}
		return *this;
	}

	template <typename T, typename A>
	ArrayString<T, A>&  ArrayString<T, A>::lower_case() {
		T* s = (T*)realloc(length(), &A::aalloc, &A::free, sizeof(T));
		for (uint32_t i = 0, len = length(); i < len; i++, s++)
			*s = _Str::tolower(*s);
		return *this;
	}

	template <typename T, typename A>
	ArrayString<T, A> ArrayString<T, A>::to_upper_case() const {
		return ArrayString(*this).upper_case();
	}

	template <typename T, typename A>
	ArrayString<T, A> ArrayString<T, A>::to_lower_case() const {
		return ArrayString(*this).upper_case();
	}

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	int ArrayString<T, A>::index_of(const ArrayString& s, uint32_t start) const {
		return _Str::index_of(c_str(), length(), s.c_str(), s.length(), start, sizeof(T));
	}

	template <typename T, typename A>
	int ArrayString<T, A>::last_index_of(const ArrayString& s, uint32_t start) const {
		return _Str::last_index_of(c_str(), length(), s.c_str(), s.length(), start, sizeof(T));
	}

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	ArrayString<T, A> ArrayString<T, A>::replace(const ArrayString& s, const ArrayString& rep) const {
		ArrayString r;
		uint32_t len, capacity;
		T* val = (T*)_Str::replace(
			c_str(), length(), s.c_str(), s.length(),
			rep.c_str(), rep.length(), sizeof(T), &len, &capacity, false, &MemoryAllocator::aalloc
		);
		r.ArrayStringBase::assign( len, capacity, val, sizeof(T), &A::free );
		return r;
	}

	template <typename T, typename A>
	ArrayString<T, A> ArrayString<T, A>::replace_all(const ArrayString& s, const ArrayString& rep) const {
		ArrayString r;
		uint32_t len, capacity;
		T* val = (T*)_Str::replace(
			c_str(), length(), s.c_str(), s.length(),
			rep.c_str(), rep.length(), sizeof(T), &len, &capacity, true, &MemoryAllocator::aalloc
		);
		r.ArrayStringBase::assign( len, capacity, val, sizeof(T), &A::free );
		return r;
	}

	// --------------------------------------------------------------------------------

	template <typename T, typename A>
	Array<ArrayString<T, A>>
	ArrayString<T, A>::split(const ArrayString& sp) const {
		Array<ArrayString<T, A>> r;
		int splen = sp.length();
		int prev = 0;
		int index = 0;
		while ((index = index_of(sp, prev)) != -1) {
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
	T2 ArrayString<T, A>::to_number() const {
		T2 o;
		_Str::to_number(c_str(), sizeof(T), length(), &o);
		return o;
	}

	template<typename T, typename A>
	template<typename T2>
	bool ArrayString<T, A>::to_number(T2* o) const {
		return _Str::to_number(c_str(), sizeof(T), length(), o);
	}

}

namespace std {
	template<typename T, typename A>
	struct hash<noug::ArrayString<T, A>> {
		size_t operator()(const noug::ArrayString<T, A>& val) const {
			return val.hash_code();
		}
	};
}

#endif
