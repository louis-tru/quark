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

namespace internal {

	struct str {
		static bool to_number(int32_t* o, const char* i, int len);
		static bool to_number(int64_t* o, const char* i, int len);
		static bool to_number(uint32_t* o, const char* i, int len);
		static bool to_number(uint64_t* o, const char* i, int len);
		static bool to_number(float* o, const char* i, int len);
		static bool to_number(double* o, const char* i, int len);
		static void strcp(void* o, char size_o, const void* i, char size_i, uint32_t len);
		template <typename Output, typename Input>
		static void strcp(Output* o, const Input* i, uint32_t len) {
			internal::str::strcp(o, sizeof(Output), i, sizeof(Input), len);
		}
		static uint32_t strlen(const void* s, int sizeof);
		static int memcmp(const void* s1, const void* s2, uint32_t len, char sizeof_i);
		template <typename T>
		static int memcmp(const T* s1, const T* s2, uint32_t len) {
			return internal::str::memcmp(s1, s2, len, sizeof(T));
		}
		// 1 > , -1 <, 0 ==
		template <class BasicString>
		static int compare(const BasicString* self, const typename BasicString::Type* s) {
			return internal::str::memcmp(self->val(), s, self->length() + 1);
		}
		static int32_t sprintf(char*& o, uint32_t& capacity, const char* f, ...);

		static const char ws[8];

		template <class BasicString>
		static int index_of(const BasicString* self,
												const typename BasicString::T* s, uint len, uint start)
		{
			typedef typename BasicString::T T;
			
			uint self_len = self->length();
			
			if (self_len < len) {
				return -1;
			}
			if (start + len > self_len) {
				return -1;
			}
			
			const T* p = self->c() + start;
			uint end = self_len - len + 1;
			
			for ( ; start < end; start++, p++) {
				if (internal::str::memcmp(p, s, len) == 0) {
					return start;
				}
			}
			return -1;
		}

		template <class BasicString>
		static int last_index_of(const BasicString* self,
															const typename BasicString::T* s, int len, int start)
		{
			// typedef typename BasicString::T T;
			int slen = self->length();
			if ( start + len > slen )
				start = slen - len;
			for ( ; start > -1; start--) {
				if (internal::str::memcmp(self->c() + start, s, len) == 0) {
					return start;
				}
			}
			return -1;
		}

		template <class BasicString>
		BasicString replace(const BasicString* self,
												const typename BasicString::T* s, uint s_len,
												const typename BasicString::T* rep, uint rep_len)
		{
			int index = index_of(self, s, s_len, 0);
			if (index != -1) {
				String rev(self->c(), index, rep, rep_len);
				index += s_len;
				rev.push(self->c() + index, self->length() - index);
				return rev;
			}
			return *self;
		}

		template <class BasicString>
		BasicString replace_all(const BasicString* self,
														const typename BasicString::T* s, uint s_len,
														const typename BasicString::T* rep, uint rep_len)
		{
			const typename BasicString::T* self_s = self->c();
			int prev = 0;
			int index;
			String rev;
			
			while ((index = index_of(self, s, s_len, prev)) != -1) {
				rev.push(self_s + prev, index - prev);
				rev.push(rep, rep_len);
				prev = index + s_len;
			}
			rev.push(self_s + prev, self->length() - prev);
			return rev;
		}

	};
}

template <typename T, HolderMode M, typename A>
template <typename T2>
BasicString<T, M, A>::BasicString(const T2* s) {
	_length = internal::str::strlen(s, sizeof(T2));
	if (_length) {
		realloc_(_length + 1);
		internal::str::strcp(_val, s, _langth);
	}
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(const T* a, uint a_len, const T* b, uint b_len)
: m_core(new StringCore(a_len + b_len))
{
	_length = a_len + b_len;
	realloc_(_length + 1);
	internal::str::strcp(_val, s, a_len);
	internal::str::strcp(_val + a_len, b, b_len);
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(ArrayBuffer&& v): ArrayBuffer(v) {
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(char i): ArrayBuffer(1) {
	_val[0] = i; _val[1] = '\0';
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(int i) {
	_length = internal::str::sprintf(&_val, &_capacity, "%d", i);
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(uint32_t i) {
	_length = internal::str::sprintf(&_val, &_capacity, "%u", i);
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(int64_t i) {
	#if FX_ARCH_64BIT
		_length = internal::str::sprintf(&_val, &_capacity, "%ld", i);
	#else
		_length = internal::str::sprintf(&_val, &_capacity, "%lld", i);
	#endif
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(uint64_t i) {
	#if FX_ARCH_64BIT
		_length = internal::str::sprintf(&_val, &_capacity, "%lu", i);
	#else
		_length = internal::str::sprintf(&_val, &_capacity, "%llu", i);
	#endif
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(float i) {
	_length = internal::str::sprintf(&_val, &_capacity, "%f", i);
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(double i) {
	_length = internal::str::sprintf(&_val, &_capacity, "%g", i);
}

template <>
static String BasicString<>::format(const char* format, ...);

template <typename T, HolderMode M, typename A>
BasicString<T, M, A> ArratBuffer<T, M, A>::collapse_string() {
	return BasicString(*this);
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kWeak, A> BasicString<T, M, A>::substr(uint32_t start, uint32_t length) const {
	return BasicString<T, HolderMode::kWeak, A>(slice(start, start + length));
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kWeak, A> BasicString<T, M, A>::substr(uint32_t start) const {
	return BasicString<T, HolderMode::kWeak, A>(slice(start));
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kWeak, A> BasicString<T, M, A>::substring(uint32_t start, uint32_t end) const {
	return BasicString<T, HolderMode::kWeak, A>(slice(start, end));
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kWeak, A> BasicString<T, M, A>::substring(uint32_t start) const {
	return BasicString<T, HolderMode::kWeak, A>(slice(start));
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kStrong, A> BasicString<T, M, A>::substr_strong(uint32_t start, uint32_t length) const {
	return BasicString<T, HolderMode::kStrong, A>(slice(start, start + length).copy());
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kStrong, A> BasicString<T, M, A>::substr_strong(uint32_t start) const {
	return BasicString<T, HolderMode::kStrong, A>(slice(start).copy());
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kStrong, A> BasicString<T, M, A>::substring_strong(uint32_t start, uint32_t end) const {
	return BasicString<T, HolderMode::kStrong, A>(slice(start, end).copy());
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kStrong, A> BasicString<T, M, A>::substring_strong(uint32_t start) const {
	return BasicString<T, HolderMode::kStrong, A>(slice(start).copy());
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>& BasicString<T, M, A>::operator+=(const BasicString& s) {
	return push(*s, s.length());
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A> BasicString<T, M, A>::operator+(const BasicString& s) const {
	return BasicString(c(), length(), *s, s.length());
}

template <typename T, HolderMode M, typename A>
Array<BasicString<T, M, A>> BasicString<T, M, A>::split(const BasicString& sp) const { // Not Thread safe
	Array<BasicString> rev;
	int splen = sp.length();
	int prev = 0;
	int index = 0;
	while ((index = index_of(sp, prev)) != -1) {
		rev.push(substring(prev, index));
		prev = index + splen;
	}
	rev.push( substring(prev) );
	return rev;
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A> BasicString<T, M, A>::trim() const { // Not Thread safe
	uint len = length();
	T* value = m_core->value();
	uint start = 0;
	uint end = len;
	for ( ; start < len; start++) {
		if (strchr(internal::str::ws, value[start]) == nullptr) {
			break;
		}
	}
	if (start == len) {
		return BasicString(); // empty
	} else {
		for ( ; end > 0; end--) {
			if (strchr(internal::str::ws, value[end - 1]) == nullptr) {
				break;
			}
		}
	}
	if (start == 0 && end == len) {
		return *this;
	}
	return substring(start, end);
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A> BasicString<T, M, A>::trim_left() const { // Not Thread safe
	uint len = length();
	T* value = m_core->value();
	for (uint start = 0; start < len; start++) {
		if (strchr(internal::str::ws, value[start]) == nullptr) {
			if (start == 0) {
				return *this;
			} else {
				return substring(start);
			}
		}
	}
	return BasicString();
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A> BasicString<T, M, A>::trim_right() const { // Not Thread safe
	uint len = length();
	T* value = m_core->value();
	for (uint end = len; end > 0; end--) {
		if (strchr(internal::str::ws, value[end - 1]) == nullptr) {
			if (end == len) {
				return *this;
			} else {
				return substring(0, end);
			}
		}
	}
	return BasicString();
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>&  BasicString<T, M, A>::upper_case() { // Not Thread safe
	m_core->modify(this);
	uint len = length();
	T* s = m_core->value();
	
	for (uint i = 0; i < len; i++, s++) {
		*s = toupper(*s);
	}
	return *this;
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>&  BasicString<T, M, A>::lower_case() { // Not Thread safe
	m_core->modify(this);
	uint len = length();
	T* s = m_core->value();
	
	for (uint i = 0; i < len; i++, s++) {
		*s = tolower(*s);
	}
	return *this;
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A> BasicString<T, M, A>::to_upper_case() const { // Not Thread safe
	return String(*this).upper_case();
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A> BasicString<T, M, A>::to_lower_case() const { // Not Thread safe
	return String(*this).lower_case();
}

template <typename T, HolderMode M, typename A>
int BasicString<T, M, A>::index_of(const BasicString& s, uint start) const { // Not Thread safe
	return _index_of(this, *s, s.length(), start);
}

template <typename T, HolderMode M, typename A>
int BasicString<T, M, A>::last_index_of(const BasicString& s, 
																								int start) const { // Not Thread safe
	return _last_index_of(this, *s, s.length(), start);
}

template <typename T, HolderMode M, typename A>
int BasicString<T, M, A>::last_index_of(const BasicString& s) const { // Not Thread safe
	return _last_index_of(this, *s, s.length(), length());
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A> BasicString<T, M, A>::replace(
	const BasicString& s, const BasicString& rep
) const { // Not Thread safe
	return replace_(this, s.c(), s.length(), *rep, rep.length());
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A> BasicString<T, M, A>::replace_all(
	const BasicString& s, const BasicString& rep
) const { // Not Thread safe
	return replace_all_(this, s.c(), s.length(), *rep, rep.length());
}

//

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>& BasicString<T, M, A>::operator=(const BasicString& s) { // Not Thread safe
	auto old_co = m_core;
	m_core = s.m_core;
	m_core->retain();
	old_co->release();
	return *this;
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>& BasicString<T, M, A>::operator=(BasicString&& s) { // Not Thread safe
	auto core = s.m_core;
	s.m_core = StringCore::empty();
	auto self = m_core;
	m_core = core;
	self->release();
	return *this;
}

template <typename T, HolderMode M, typename A>
bool BasicString<T, M, A>::operator==(const BasicString& s) const {
	return internal::str::compare(this, s.c()) == 0;
}

template <typename T, HolderMode M, typename A>
bool BasicString<T, M, A>::operator!=(const BasicString& s) const {
	return internal::str::compare(this, s.c()) != 0;
}

template <typename T, HolderMode M, typename A>
bool BasicString<T, M, A>::operator>(const BasicString& s) const {
	return internal::str::compare(this, s.c()) > 0;
}

template <typename T, HolderMode M, typename A>
bool BasicString<T, M, A>::operator<(const BasicString& s) const {
	return internal::str::compare(this, s.c()) < 0;
}

template <typename T, HolderMode M, typename A>
bool BasicString<T, M, A>::operator>=(const BasicString& s) const {
	return internal::str::compare(this, s.c()) >= 0;
}

template <typename T, HolderMode M, typename A>
bool BasicString<T, M, A>::operator<=(const BasicString& s) const {
	return internal::str::compare(this, s.c()) <= 0;
}

template <typename T, HolderMode M, typename A>
template<T2>
T2 BasicString<T, M, A>::to_number<T2>() const {
	T2 o;
	internal::str::to_number(&o, _val, _length);
	return o;
}

template <typename T, HolderMode M, typename A>
template<T2>
bool BasicString<T, M, A>::to_number<T2>(T2* o) const {
	return internal::str::to_number(o, _val, _length);
}