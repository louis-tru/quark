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

#ifndef __ftr__util__string__
#define __ftr__util__string__

#include "./buffer.h"

namespace ftr {

	template<
		typename T = char,
		HolderMode M = HolderMode::kStrong,
		typename A = AllocatorDefault
	> class BasicString;

	typedef BasicString<char,     HolderMode::kWeak>   String;
	typedef BasicString<uint16_t, HolderMode::kWeak>   String16;
	typedef BasicString<uint32_t, HolderMode::kWeak>   String32;
	typedef BasicString<char,     HolderMode::kStrong> SString;
	typedef BasicString<uint16_t, HolderMode::kStrong> SString16;
	typedef BasicString<uint32_t, HolderMode::kStrong> SString32;
	// alias
	typedef const  String   cString;
	typedef const  String16 cString16;
	typedef const  String32 cString32;
	typedef const  char     cchar;

	template<typename T, HolderMode M, typename A>
	class FX_EXPORT BasicString: public Object {
		public:
			// constructors
			BasicString(const T* s); // copy constructors
			BasicString(const T* s, uint32_t len); // copy constructors
			BasicString(const T* a, uint32_t a_len, const T* b, uint32_t b_len); // copy constructors
			BasicString(char i); // strong types can call
			BasicString(int32_t i); // strong types can call
			BasicString(int64_t i); // strong types can call
			BasicString(uint32_t i); // strong types can call
			BasicString(uint64_t i); // strong types can call
			BasicString(float f); // strong types can call
			BasicString(double f); // strong types can call

			/**
			 * @func format string
			 */
			static String format(const char* format, ...);

			// operator+
			Strong operator+(const Weak& s) const; // concat new
			template<HolderMode M2, typename A2>
			Strong operator+(const ArrayBuffer<T, M2, A2>& s) const; // concat new
			// operator+=
			ArrayBuffer& operator+=(const Weak& s); // write, Only strong types can be call
			template<HolderMode M2, typename A2>
			ArrayBuffer& operator+=(const ArrayBuffer<T, M2, A2>& s); // write, Only strong types can be call

			// operator compare
			bool operator==(const T* s) const;
			bool operator!=(const T* s) const;
			bool operator> (const T* s) const;
			bool operator< (const T* s) const;
			bool operator>=(const T* s) const;
			bool operator<=(const T* s) const;
			//
			template<HolderMode M2, typename A2> 
			inline bool operator==(const ArrayBuffer<T, M2, A2>& s) const { return operator==(s._val); }
			template<HolderMode M2, typename A2> 
			inline bool operator!=(const ArrayBuffer<T, M2, A2>& s) const { return operator!=(s._val); }
			template<HolderMode M2, typename A2> 
			inline bool operator> (const ArrayBuffer<T, M2, A2>& s) const { return operator>(s._val); }
			template<HolderMode M2, typename A2> 
			inline bool operator< (const ArrayBuffer<T, M2, A2>& s) const { return operator<(s._val); }
			template<HolderMode M2, typename A2> 
			inline bool operator>=(const ArrayBuffer<T, M2, A2>& s) const { return operator>=(s._val); }
			template<HolderMode M2, typename A2> 
			inline bool operator<=(const ArrayBuffer<T, M2, A2>& s) const { return operator<=(s._val); }

			// substr
			inline Weak substr(uint32_t start, uint32_t length) const { return slice(start, start + length); }
			inline Weak substring(uint32_t start, uint32_t end) const { return slice(start, end); }
			inline Weak substr(uint32_t start) const { return slice(start); }
			inline Weak substring(uint32_t start) const { return slice(start); }
			// split
			std::vector<Weak> split(const Weak& sp) const;
			// trim
			Weak trim() const;
			Weak trim_left() const;
			Weak trim_right() const;
			// upper, lower
			ArrayBuffer& upper_case(); // Only strong types can be call
			ArrayBuffer& lower_case(); // Only strong types can be call
			Strong       to_upper_case() const;
			Strong       to_lower_case() const;
			// index_of
			int index_of(const Weak& s, uint32_t start = 0) const;
			int last_index_of(const Weak& s, int start) const;
			int last_index_of(const Weak& s) const;
			// replace
			Strong replace(const Weak& s, const Weak& rep) const;
			Strong replace_all(const Weak& s, const Weak& rep) const;

			// to number
			template<typename T2> T2   to_number()        const;
			template<typename T2> bool to_number(T2* out) const;
		
		private:
	};


	// -------------------------------------- IMPL --------------------------------------

	class FX_EXPORT _Str {
		static const char ws[8];
		static bool to_number(const char* i, int32_t* o, int len);
		static bool to_number(const char* i, int64_t* o, int len);
		static bool to_number(const char* i, uint32_t* o, int len);
		static bool to_number(const char* i, uint64_t* o, int len);
		static bool to_number(const char* i, float* o, int len);
		static bool to_number(const char* i, double* o, int len);
		static void strcp(char* o, int size_o, const char* i, int size_i, uint32_t len);
		template <typename Output, typename Input>
		static void strcp(Output* o, const Input* i, uint32_t len) {
			internal::str::strcp(o, sizeof(Output), i, sizeof(Input), len);
		}
		static uint32_t strlen(const char* s, int size_of);
		static int memcmp(const char* s1, const char* s2, uint32_t len, int size_of);
		// 1 > , -1 <, 0 ==
		template <typename T>
		static int memcmp(const T* s1, const T* s2, uint32_t len) {
			return internal::str::memcmp(s1, s2, len, sizeof(T));
		}
		static int32_t sprintf(char** o, uint32_t* capacity, const char* f, ...);
		static int32_t index_of(
			const char* s1, uint32_t s1_len,
			const char* s2, uint32_t s2_len, uint32_t start, int size_of
		);
		static int32_t last_index_of(
			const char* s1, uint32_t s1_len,
			const char* s2, uint32_t s2_len, uint32_t start, int size_of
		);
		static char* replace(
			const char* s1, uint32_t s1_len,
			const char* s2, uint32_t s2_len,
			const char* rep, uint32_t rep_len,
			int size_of, uint32_t* out_len, uint32_t* capacity_out, bool all
		);
	};

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, M, A>::ArrayBuffer(const T* s)
		: ArrayBuffer<T, M, A>(s, internal::str::strlen(s, sizeof(T))) {
	}

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, M, A>::ArrayBuffer(const T* s, uint32_t len): _length(len), _capacity(0), _val(nullptr) {
		if (len) {
			realloc_(_length + 1);
			internal::str::strcp(_val, s, _length);
		}
	}

	template <>
	ArrayBuffer<char, HolderMode::kWeak>::ArrayBuffer(const char* s, uint32_t len);

	template <>
	ArrayBuffer<uint16_t, HolderMode::kWeak>::ArrayBuffer(const uint16_t* s, uint32_t len);

	template <>
	ArrayBuffer<uint32_t, HolderMode::kWeak>::ArrayBuffer(const uint32_t* s, uint32_t len);

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, M, A>::ArrayBuffer(const T* a, uint32_t a_len, const T* b, uint32_t b_len) {
		static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
		_length = a_len + b_len;
		realloc_(_length + 1);
		internal::str::strcp(_val, a, a_len);
		internal::str::strcp(_val + a_len, b, b_len);
	}

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, M, A>::ArrayBuffer(char i): ArrayBuffer<T, M, A>(1) {
		_val[0] = i; _val[1] = '\0';
	}

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, M, A>::ArrayBuffer(int i) {
		static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
		_length = internal::str::sprintf(&_val, &_capacity, "%d", i);
	}

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, M, A>::ArrayBuffer(uint32_t i) {
		static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
		_length = internal::str::sprintf(&_val, &_capacity, "%u", i);
	}

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, M, A>::ArrayBuffer(int64_t i) {
		static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
		#if FX_ARCH_64BIT
			_length = internal::str::sprintf(&_val, &_capacity, "%ld", i);
		#else
			_length = internal::str::sprintf(&_val, &_capacity, "%lld", i);
		#endif
	}

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, M, A>::ArrayBuffer(uint64_t i) {
		static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
		#if FX_ARCH_64BIT
			_length = internal::str::sprintf(&_val, &_capacity, "%lu", i);
		#else
			_length = internal::str::sprintf(&_val, &_capacity, "%llu", i);
		#endif
	}

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, M, A>::ArrayBuffer(float i) {
		static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
		_length = internal::str::sprintf(&_val, &_capacity, "%f", i);
	}

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, M, A>::ArrayBuffer(double i) {
		static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
		_length = internal::str::sprintf(&_val, &_capacity, "%g", i);
	}

	template <>
	SString String::format(const char* format, ...);


	// --------------------------------------------------------------------------------

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, HolderMode::kStrong, A> ArrayBuffer<T, M, A>::operator+(const ArrayBuffer<T, HolderMode::kWeak, A>& s) const { // concat new
		ArrayBuffer<T, HolderMode::kStrong, A> s1(copy());
		s1.write(s);
		return std::move(s1);
	}

	template <typename T, HolderMode M, typename A>
	template<HolderMode M2, typename A2>
	ArrayBuffer<T, HolderMode::kStrong, A> ArrayBuffer<T, M, A>::operator+(const ArrayBuffer<T, M2, A2>& s) const { // concat new
		ArrayBuffer<T, HolderMode::kStrong, A> s1(copy());
		s1.write(s);
		return std::move(s1);
	}

	// --------------------------------------------------------------------------------

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::operator+=(const ArrayBuffer<T, HolderMode::kWeak, A>& s) { // write, Only strong types can be call
		write(s);
		return *this;
	}

	template <typename T, HolderMode M, typename A>
	template<HolderMode M2, typename A2>
	ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::operator+=(const ArrayBuffer<T, M2, A2>& s) { // write, Only strong types can be call
		write(s);
		return *this;
	}

	// --------------------------------------------------------------------------------

	template <typename T, HolderMode M, typename A>
	bool ArrayBuffer<T, M, A>::operator==(const T* s) const {
		return internal::str::memcmp(_val, s, _length) == 0;
	}

	template <typename T, HolderMode M, typename A>
	bool ArrayBuffer<T, M, A>::operator!=(const T* s) const {
		return internal::str::memcmp(_val, s, _length) != 0;
	}

	template <typename T, HolderMode M, typename A>
	bool ArrayBuffer<T, M, A>::operator>(const T* s) const {
		return internal::str::memcmp(_val, s, _length) > 0;
	}

	template <typename T, HolderMode M, typename A>
	bool ArrayBuffer<T, M, A>::operator<(const T* s) const {
		return internal::str::memcmp(_val, s, _length) < 0;
	}

	template <typename T, HolderMode M, typename A>
	bool ArrayBuffer<T, M, A>::operator>=(const T* s) const {
		return internal::str::memcmp(_val, s, _length) >= 0;
	}

	template <typename T, HolderMode M, typename A>
	bool ArrayBuffer<T, M, A>::operator<=(const T* s) const {
		return internal::str::memcmp(_val, s, _length) <= 0;
	}

	// --------------------------------------------------------------------------------

	template <typename T, HolderMode M, typename A>
	std::vector<ArrayBuffer<T, HolderMode::kWeak, A>>
	ArrayBuffer<T, M, A>::split(const ArrayBuffer<T, HolderMode::kWeak, A>& sp) const {
		std::vector<ArrayBuffer<T, HolderMode::kWeak, A>> r;
		int splen = sp.length();
		int prev = 0;
		int index = 0;
		while ((index = index_of(sp, prev)) != -1) {
			r.push_back(substring(prev, index));
			prev = index + splen;
		}
		r.push_back( substring(prev) );
		return r;
	}

	// --------------------------------------------------------------------------------

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, HolderMode::kWeak, A> ArrayBuffer<T, M, A>::trim() const {
		uint32_t start = 0;
		uint32_t end = _length;
		for ( ; start < _length; start++) {
			if (strchr(internal::str::ws, _val[start]) == nullptr) {
				break;
			}
		}
		if (start == _length) {
			return ArrayBuffer<T, HolderMode::kWeak, A>(); // empty string
		} else {
			for ( ; end > 0; end--) {
				if (strchr(internal::str::ws, _val[end - 1]) == nullptr) {
					break;
				}
			}
		}
		if (start == 0 && end == _length) {
			return ArrayBuffer<T, HolderMode::kWeak, A>(*this);
		}
		return substring(start, end);
	}

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, HolderMode::kWeak, A> ArrayBuffer<T, M, A>::trim_left() const {
		for (uint32_t start = 0; start < _length; start++) {
			if (strchr(internal::str::ws, _val[start]) == nullptr) {
				if (start == 0) {
					return ArrayBuffer<T, HolderMode::kWeak, A>(*this);
				} else {
					return substring(start);
				}
			}
		}
		return ArrayBuffer<T, HolderMode::kWeak, A>();
	}

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, HolderMode::kWeak, A> ArrayBuffer<T, M, A>::trim_right() const {
		for (uint32_t end = _length; end > 0; end--) {
			if (strchr(internal::str::ws, _val[end - 1]) == nullptr) {
				if (end == _length) {
					return ArrayBuffer<T, HolderMode::kWeak, A>(*this);
				} else {
					return substring(0, end);
				}
			}
		}
		return ArrayBuffer<T, HolderMode::kWeak, A>();
	}

	// --------------------------------------------------------------------------------

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, M, A>&  ArrayBuffer<T, M, A>::upper_case() {
		static_assert(M == HolderMode::kStrong, "upper_case(), the weak holder cannot be changed");
		T* s = _val;
		for (uint32_t i = 0; i < _length; i++, s++) {
			*s = ::toupper(*s);
		}
		return *this;
	}

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, M, A>&  ArrayBuffer<T, M, A>::lower_case() {
		static_assert(M == HolderMode::kStrong, "lower_case(), the weak holder cannot be changed");
		T* s = _val;
		for (uint32_t i = 0; i < _length; i++, s++) {
			*s = ::tolower(*s);
		}
		return *this;
	}

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, HolderMode::kStrong, A> ArrayBuffer<T, M, A>::to_upper_case() const {
		return std::move(copy().upper_case());
	}

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, HolderMode::kStrong, A> ArrayBuffer<T, M, A>::to_lower_case() const {
		return std::move(copy().lower_case());
	}

	// --------------------------------------------------------------------------------

	template <typename T, HolderMode M, typename A>
	int ArrayBuffer<T, M, A>::index_of(const Weak& s, uint32_t start) const {
		return internal::str::index_of(_val, _length, s.val(), s.length(), start, sizeof(T));
	}

	template <typename T, HolderMode M, typename A>
	int ArrayBuffer<T, M, A>::last_index_of(const Weak& s, int start) const {
		return internal::str::last_index_of(_val, _length, s.val(), s.length(), start, sizeof(T));
	}

	template <typename T, HolderMode M, typename A>
	int ArrayBuffer<T, M, A>::last_index_of(const Weak& s) const {
		return internal::str::last_index_of(_val, _length, s.val(), s.length(), _length, sizeof(T));
	}

	// --------------------------------------------------------------------------------

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, HolderMode::kStrong, A> ArrayBuffer<T, M, A>::replace(const Weak& s, const Weak& rep) const {
		uint32_t len, capacity;
		T* val = internal::str::replace(
			_val, _length, s._val, s._length, 
			rep._val, rep._length, sizeof(T), &len, &capacity, false
		);
		return Strong(len, capacity, val);
	}

	template <typename T, HolderMode M, typename A>
	ArrayBuffer<T, HolderMode::kStrong, A> ArrayBuffer<T, M, A>::replace_all(const Weak& s, const Weak& rep) const {
		uint32_t len, capacity;
		T* val = internal::str::replace(_val, _length, s._val, s._length,
			rep._val, rep._length, sizeof(T), &len, &capacity, true
		);
		return Strong(len, capacity, val);
	}

	// --------------------------------------------------------------------------------

	template<typename T, HolderMode M, typename A>
	template<typename T2>
	T2 ArrayBuffer<T, M, A>::to_number() const {
		T2 o;
		internal::str::to_number(_val, &o, _length);
		return o;
	}

	template<typename T, HolderMode M, typename A>
	template<typename T2>
	bool ArrayBuffer<T, M, A>::to_number(T2* o) const {
		return internal::str::to_number(_val, o, _length);
	}


}
#endif