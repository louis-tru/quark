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

#include <ftr/util/array.h>

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

}
#endif