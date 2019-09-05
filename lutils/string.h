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

#ifndef __langou__utils__string__
#define __langou__utils__string__

#include <stdarg.h>
#include "lutils/array.h"
#include "lutils/util.h"
#include "lutils/handle.h"
#include "lutils/container.h"

XX_NS(langou)

/**
 * @class BasicString 字符串模板
 * @bases Object
 * @template <class Char = char, class Container = Container<char>>
 */
template <typename _Char, class _Container>
class XX_EXPORT BasicString: public Object {
 public:
	typedef _Char Char;
	typedef _Container Container;
	BasicString();
	BasicString(char i);
	BasicString(int i);
	BasicString(uint i);
	BasicString(int64 i);
	BasicString(uint64 i);
	BasicString(float f);
	BasicString(double f);
	BasicString(const Char* s1, uint s1_len, const Char* s2, uint s2_len);
	BasicString(const Char* s, uint len);
	BasicString(const BasicString& s);
	BasicString(BasicString&& s);
	BasicString(ArrayBuffer<Char>&& data);
	BasicString(ArrayBuffer<Char>& data) = delete;
	BasicString(const Object& o);
	template<typename Char2>
	BasicString(const Char2* s);
	virtual ~BasicString();
	static String format(cchar* format, ...);
	inline bool is_empty() const;
	bool is_blank() const;
	inline const Char* c() const;
	inline const Char* operator*() const;
	inline Char operator[](uint index) const;
	inline uint capacity() const;
	inline uint length() const;
	BasicString full_copy() const;
	Array<BasicString> split(const BasicString& sp) const;
	BasicString trim() const;
	BasicString trim_left() const;
	BasicString trim_right() const;
	BasicString& upper_case();
	BasicString& lower_case();
	BasicString to_upper_case() const;
	BasicString to_lower_case() const;
	int index_of(const BasicString& s, uint start = 0) const;
	int last_index_of(const BasicString& s, int start) const;
	int last_index_of(const BasicString& s) const;
	BasicString replace(const BasicString& s, const BasicString& rep) const;
	BasicString replace_all(const BasicString& s, const BasicString& rep) const;
	inline BasicString substr(uint start, uint length) const;
	inline BasicString substr(uint start) const;
	inline BasicString substring(uint start, uint end) const;
	inline BasicString substring(uint start) const;
	BasicString& push(const Char* s, uint len);
	BasicString& push(const BasicString& s);
	BasicString& push(Char s);
	inline BasicString& operator+=(const BasicString& s);
	inline BasicString operator+(const BasicString& s) const;
	BasicString& operator=(const BasicString& s);
	BasicString& operator=(BasicString&& s);
	bool operator==(const BasicString& s) const;
	bool operator!=(const BasicString& s) const;
	bool operator>(const BasicString& s) const;
	bool operator<(const BasicString& s) const;
	bool operator>=(const BasicString& s) const;
	bool operator<=(const BasicString& s) const;
	int index_of(const Char* s, uint start = 0) const;
	int last_index_of(const Char* s, int start) const;
	int last_index_of(const Char* s) const;
	BasicString replace(const Char* s, const Char* rep) const;
	BasicString replace_all(const Char* s, const Char* rep) const;
	BasicString& push(const Char* s);
	BasicString& operator+=(const Char* s);
	BasicString operator+(const Char* s) const;
	BasicString& assign(const Char* s, uint len);
	BasicString& operator=(const Char* s);
	bool operator==(const Char* s) const;
	bool operator!=(const Char* s) const;
	bool operator>(const Char* s) const;
	bool operator<(const Char* s) const;
	bool operator>=(const Char* s) const;
	bool operator<=(const Char* s) const;
	uint hash_code() const;
	virtual String to_string() const;
	Char* collapse();
	ArrayBuffer<Char> collapse_buffer();
	ArrayBuffer<Char> copy_buffer() const;
	int to_int() const;
	uint to_uint() const;
	int64 to_int64() const;
	uint64 to_uint64() const;
	float to_float() const;
	double to_double() const;
	bool to_bool() const;
	bool to_int(int* out) const;
	bool to_uint(uint* out) const;
	bool to_int64(int64* out) const;
	bool to_uint64(uint64* out) const;
	bool to_float(float* out) const;
	bool to_double(double* out) const;
	bool to_bool(bool* out) const;

 private:
	class StringCore;
	friend class StringCore;
	StringCore* m_core; // string core
	BasicString(StringCore* core);
};

XX_END

#include "string.inl"

#endif
