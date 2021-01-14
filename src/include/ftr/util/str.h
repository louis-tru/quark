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

#ifndef __ftr__util__str__
#define __ftr__util__str__

#include <ftr/util/buffer.h>
#include <vector>

namespace ftr {

	/**
	* @class BasicString
	*/
	template<class T, HolderMode M, typename A>
	class FX_EXPORT BasicString: public ArrayBuffer<T, M, A> {
		public:

			BasicString() {}
			template<HolderMode M2, typename A2>
			BasicString(const ArrayBuffer<T, M2, A2>& s): ArrayBuffer<T, M, A>(s) {} // Only weak types can be copied
			BasicString(      ArrayBuffer<T, M, A>&  s) : ArrayBuffer<T, M, A>(std::move(s)) {}
			BasicString(      ArrayBuffer<T, M, A>&& s) : ArrayBuffer<T, M, A>(s) {}
			BasicString(const T* s);
			BasicString(const T* s, uint32_t len);
			BasicString(const T* a, uint32_t a_len, const T* b, uint32_t b_len); // Only weak types can be copied
			BasicString(char i);
			BasicString(int32_t i);
			BasicString(int64_t i);
			BasicString(uint32_t i);
			BasicString(uint64_t i);
			BasicString(float f);
			BasicString(double f);

			static BasicString<char, HolderMode::kStrong> format(const char* format, ...);

			BasicString<T, HolderMode::kWeak, A> substr(uint32_t start, uint32_t length) const;
			BasicString<T, HolderMode::kWeak, A> substring(uint32_t start, uint32_t end) const;
			BasicString<T, HolderMode::kWeak, A> substr(uint32_t start) const;
			BasicString<T, HolderMode::kWeak, A> substring(uint32_t start) const;

			template<HolderMode M2, typename A2>
			std::vector<BasicString<T, HolderMode::kWeak, A>> split(const BasicString<T, M2, A2>& sp) const;

			BasicString<T, HolderMode::kWeak, A> trim() const;
			BasicString<T, HolderMode::kWeak, A> trim_left() const;
			BasicString<T, HolderMode::kWeak, A> trim_right() const;

			BasicString<T, M,                  A>& upper_case(); // Only strong types can be call
			BasicString<T, M,                  A>& lower_case(); // Only strong types can be call
			BasicString<T, HolderMode::kStrong, A> to_upper_case() const;
			BasicString<T, HolderMode::kStrong, A> to_lower_case() const;

			template<HolderMode M2, typename A2>
			int index_of     (const BasicString<T, M2, A2>& s, uint32_t start = 0) const;
			template<HolderMode M2, typename A2>
			int last_index_of(const BasicString<T, M2, A2>& s, int start) const;
			template<HolderMode M2, typename A2>
			int last_index_of(const BasicString<T, M2, A2>& s) const;
			
			template<HolderMode M2, typename A2, HolderMode M3, typename A3>
			BasicString<T, HolderMode::kStrong, A> replace(const BasicString<T, M2, A2>& s, const BasicString<T, M3, A3>& rep) const;
			template<HolderMode M2, typename A2, HolderMode M3, typename A3>
			BasicString<T, HolderMode::kStrong, A> replace_all(const BasicString<T, M2, A2>& s, const BasicString<T, M3, A3>& rep) const;

			// assign
			template<HolderMode M2, typename A2>
			BasicString<T, M,                   A>& operator=(const BasicString<T, M2, A2>& s); // Only weak types can be copied assign value
			BasicString<T, M,                   A>& operator=(      BasicString&  s); // assign ref
			BasicString<T, M,                   A>& operator=(      BasicString&& s); // assign right ref
			template<HolderMode M2, typename A2>
			BasicString<T, M,                   A>& operator+=(const BasicString<T, M2, A2>& s); // write, Only strong types can be call
			template<HolderMode M2, typename A2>
			BasicString<T, HolderMode::kStrong, A>  operator+ (const BasicString<T, M2, A2>& s) const; // concat new

			// compare
			template<HolderMode M2, typename A2> bool operator==(const BasicString<T, M2, A2>& s) const;
			template<HolderMode M2, typename A2> bool operator!=(const BasicString<T, M2, A2>& s) const;
			template<HolderMode M2, typename A2> bool operator>(const BasicString<T, M2, A2>& s) const;
			template<HolderMode M2, typename A2> bool operator<(const BasicString<T, M2, A2>& s) const;
			template<HolderMode M2, typename A2> bool operator>=(const BasicString<T, M2, A2>& s) const;
			template<HolderMode M2, typename A2> bool operator<=(const BasicString<T, M2, A2>& s) const;

			template<typename T2> T2   to_number()        const;
			template<typename T2> bool to_number(T2* out) const;
	};

	#include "str.inl"

	typedef BasicString<char,     HolderMode::kWeak> String;
	typedef BasicString<uint16_t, HolderMode::kWeak> String16;
	typedef BasicString<uint32_t, HolderMode::kWeak> String32;
	typedef BasicString<char,     HolderMode::kStrong> MutableString;
	typedef BasicString<uint16_t, HolderMode::kStrong> MutableString16;
	typedef BasicString<uint32_t, HolderMode::kStrong> MutableString32;
}
#endif
