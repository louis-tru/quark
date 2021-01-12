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

#include <ftr/utils/macros.h>
#include <ftr/utils/buffer.h>
#include <vector>

namespace ftr {

	template<
		typename T = char,
		HolderMode M = HolderMode::kStrong,
		typename A = AllocatorDefault
	> class BasicString;
	typedef BasicString<char> String;
	typedef BasicString<uint16_t> String16;
	typedef BasicString<uint32_t> String32;

	/**
	* @class BasicString
	*/
	template<class T, HolderMode M, typename A>
	class FX_EXPORT BasicString: public ArrayBuffer<T, M, A> {
		public:
		BasicString();
		// BasicString(char i);
		// BasicString(int i);
		// BasicString(uint32_t i);
		// BasicString(int64 i);
		// BasicString(uint32_t64 i);
		// BasicString(float f);
		// BasicString(double f);
		BasicString(const T* s1, uint32_t s1_len, const T* s2, uint32_t s2_len);
		BasicString(const T* s, uint32_t len);
		BasicString(const ArrayBuffer& s);
		BasicString(ArrayBuffer&& s);
		// BasicString(ArrayBuffer<T>&& data);
		// BasicString(ArrayBuffer<T>& data) = delete;
		// BasicString(const Object& o);
		// template<typename T2>
		// BasicString(const T2* s);
		virtual ~BasicString();

		static String format(const char* format, ...);

		std::vector<BasicString> split(const BasicString& sp) const;
		BasicString trim() const;
		BasicString trim_left() const;
		BasicString trim_right() const;
		BasicString& upper_case();
		BasicString& lower_case();
		BasicString to_upper_case() const;
		BasicString to_lower_case() const;
		int         index_of(const BasicString& s, uint32_t start = 0) const;
		int         last_index_of(const BasicString& s, int start) const;
		int         last_index_of(const BasicString& s) const;
		// int         index_of(const T* s, uint32_t start = 0) const;
		// int         last_index_of(const T* s, int start) const;
		// int         last_index_of(const T* s) const;

		BasicString replace(const BasicString& s, const BasicString& rep) const;
		BasicString replace_all(const BasicString& s, const BasicString& rep) const;
		// BasicString replace(const T* s, const T* rep) const;
		// BasicString replace_all(const T* s, const T* rep) const;

		BasicString substr(uint32_t start, uint32_t length) const;
		BasicString substr(uint32_t start) const;
		BasicString substring(uint32_t start, uint32_t end) const;
		BasicString substring(uint32_t start) const;

		// BasicString& push(const T* s, uint32_t len);
		// BasicString& push(const BasicString& s);
		// BasicString& push(T s);
		
		// operator overload
		BasicString& operator+=(const BasicString& s);
		BasicString  operator+(const BasicString& s) const;
		BasicString& operator=(const BasicString& s);
		BasicString& operator=(      BasicString&& s); // assign
		bool         operator==(const BasicString& s) const;
		bool         operator!=(const BasicString& s) const;
		bool         operator>(const BasicString& s) const;
		bool         operator<(const BasicString& s) const;
		bool         operator>=(const BasicString& s) const;
		bool         operator<=(const BasicString& s) const;

		// BasicString& operator+=(const T* s);
		// BasicString  operator+(const T* s) const;
		// BasicString& operator=(const T* s);
		// bool         operator==(const T* s) const;
		// bool         operator!=(const T* s) const;
		// bool         operator>(const T* s) const;
		// bool         operator<(const T* s) const;
		// bool         operator>=(const T* s) const;
		// bool         operator<=(const T* s) const;

		// BasicString& assign(const T* s, uint32_t len);

		uint32_t hash_code() const;

		// int32_t      to_int() const;
		// uint32_t to_uint32_t() const;
		// int64_t to_int64() const;
		// uint64_t to_uint32_t64() const;
		// float to_float() const;
		// double to_double() const;
		// bool to_bool() const;
		// bool to_int(int* out) const;
		// bool to_uint32_t(uint32_t* out) const;
		// bool to_int64(int64* out) const;
		// bool to_uint32_t64(uint32_t64* out) const;
		// bool to_float(float* out) const;
		// bool to_double(double* out) const;
		// bool to_bool(bool* out) const;
	};
}

#endif
