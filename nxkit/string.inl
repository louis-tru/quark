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

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "nxkit/buffer.h"

NX_NS(ngui)

template <typename Char, typename Char2>
static void _memcpy(Char* s, const Char2* s2, uint len) {
	Char* s_end = s + len;
	while (s < s_end) {
		*s = *s2;
		s++; s2++;
	}
}

template<typename Char>
static uint _strlen(const Char* s) {
	uint rev = 0;
	while (*s != 0) {
		rev++; s++;
	}
	return rev;
}

template<typename Char, typename Char2>
static int _memcmp(const Char* s1, const Char2* s2, uint len) {
	const Char* s1_end = s1 + len;
	Char rev = 0;
	while (s1 < s1_end) {
		rev = *s1 - *s2;
		if (rev != 0) {
			break;
		}
		s1++; s2++;
	}
	return rev;
}

template<> NX_INLINE void _memcpy<char, char>(char* s, cchar* s2, uint len) {
	memcpy(s, s2, len);
}

template<> NX_INLINE void _memcpy(uint16* s, const uint16* s2, uint len) {
	memcpy(s, s2, len * sizeof(uint16));
}

template<> NX_INLINE void _memcpy(uint* s, const uint* s2, uint len) {
	memcpy(s, s2, len * sizeof(uint));
}

template<> NX_INLINE uint _strlen<char>(cchar* s) {
	return (uint)strlen(s);
}

template<> NX_INLINE int _memcmp<char, char>(cchar* s1, cchar* s2, uint len) {
	return memcmp(s1, s2, len);
}

template <class BasicString>
static int _index_of(const BasicString* self,
										 const typename BasicString::Char* s, uint len, uint start)
{
	typedef typename BasicString::Char Char;
	
	uint self_len = self->length();
	
	if (self_len < len) {
		return -1;
	}
	if (start + len > self_len) {
		return -1;
	}
	
	const Char* p = self->c() + start;
	uint end = self_len - len + 1;
	
	for ( ; start < end; start++, p++) {
		if (_memcmp(p, s, len) == 0) {
			return start;
		}
	}
	return -1;
}

template <class BasicString>
static int _last_index_of(const BasicString* self,
													const typename BasicString::Char* s, int len, int start)
{
	// typedef typename BasicString::Char Char;
	int slen = self->length();
	if ( start + len > slen )
		start = slen - len;
	for ( ; start > -1; start--) {
		if (_memcmp(self->c() + start, s, len) == 0) {
			return start;
		}
	}
	return -1;
}

template <class BasicString>
BasicString replace_(const BasicString* self,
										 const typename BasicString::Char* s, uint s_len,
										 const typename BasicString::Char* rep, uint rep_len)
{
	int index = _index_of(self, s, s_len, 0);
	if (index != -1) {
		String rev(self->c(), index, rep, rep_len);
		index += s_len;
		rev.push(self->c() + index, self->length() - index);
		return rev;
	}
	return *self;
}

template <class BasicString>
BasicString replace_all_(const BasicString* self,
												 const typename BasicString::Char* s, uint s_len,
												 const typename BasicString::Char* rep, uint rep_len)
{
	const typename BasicString::Char* self_s = self->c();
	int prev = 0;
	int index;
	String rev;
	
	while ((index = _index_of(self, s, s_len, prev)) != -1) {
		rev.push(self_s + prev, index - prev);
		rev.push(rep, rep_len);
		prev = index + s_len;
	}
	rev.push(self_s + prev, self->length() - prev);
	return rev;
}

// 1 > , -1 <, 0 ==
template <class BasicString>
NX_INLINE static int _compare(const BasicString* self, const typename BasicString::Char* s) {
	return _memcmp(self->c(), s, self->length() + 1);
}

/**
 * @class BasicString::StringCore
 * @private
 */
template <typename Char, class Container>
class BasicString<Char, Container>::StringCore {
 public:

	StringCore(): length(0), container(1), m_ref(1) {
		*(*container) = '\0';
	}

	StringCore(const StringCore& core): length(core.length), container(core.container), m_ref(1) 
	{}

	StringCore(uint len): length(len), container(len + 1), m_ref(1) {
		(*container)[len] = '\0';
	}

	StringCore(uint len, Char* value): length(len), container(len + 1, value), m_ref(1) 
	{}

	void retain() {
		m_ref++;
	}

	void release() {
		NX_ASSERT(m_ref > 0);
		if ( --m_ref == 0 ) {
			delete this; // 只有当引用记数变成0才会释放
		}
	}

	void modify(BasicString* host) {
		// 修改需要从共享核心中分离出来
		// 多个线程同时使用一个StringCore时候如果考虑线程安全需要加锁
		if ( m_ref > 1 ) { // 大于1表示为共享核心
			host->m_core = new StringCore(*this);
			release();
		}
	}

	inline Char* value() {
		return *container;
	}

	inline uint capacity() {
		return container.capacity();
	}

	inline Char* collapse() {
		length = 0;
		return container.collapse();
	}

	inline int ref() const {
		return m_ref; 
	}

	/**
	 * @func empty() use empty string core
	 */
	static StringCore* empty() {
		static StringCore* core(new StringCore());
		core->m_ref++; // ref +1
		return core;
	}

	uint length;
	Container container;
 protected:
	std::atomic_int m_ref;
};

// ** BasicString::StringCore END **

template <typename Char, class Container>
BasicString<Char, Container>::BasicString(StringCore* core): m_core(core) {
}

template <typename Char, class Container>
BasicString<Char, Container>::BasicString() {
	m_core = StringCore::empty();
}

template <typename Char, class Container>
BasicString<Char, Container>::BasicString(const Char* a, uint a_len, const Char* b, uint b_len)
: m_core(new StringCore(a_len + b_len))
{
	_memcpy(m_core->value(), a, a_len);
	_memcpy(m_core->value() + a_len, b, b_len);
}

template <typename Char, class Container>
BasicString<Char, Container>::BasicString(const Char* s, uint len)
: m_core(len ? new StringCore(len) : StringCore::empty()) {
	_memcpy(m_core->value(), s, len);
}

template <typename Char, class Container>
BasicString<Char, Container>::BasicString(const BasicString& s): m_core(s.m_core) {
	m_core->retain();
}

template <typename Char, class Container>
BasicString<Char, Container>::BasicString(BasicString&& v): m_core(v.m_core) {
	v.m_core = StringCore::empty();
}

template <typename Char, class Container>
BasicString<Char, Container>::BasicString(char c): m_core(new StringCore(1)) {
	*(m_core->value()) = c;
}

template <typename Char, class Container>
BasicString<Char, Container>::BasicString(ArrayBuffer<Char>&& data) {
	
	uint len = data.length();
	Char* s = data.collapse();
	if (s) {
		m_core = new StringCore(len, s);
	} else {
		m_core = StringCore::empty();
	}
}

template <typename Char, class Container>
BasicString<Char, Container>::BasicString (int i) {
	char s[11];
	sprintf(s, "%d", i);
	uint size = _strlen(s);
	m_core = new StringCore(size);
	_memcpy(m_core->value(), s, size);
}

template <typename Char, class Container>
BasicString<Char, Container>::BasicString (uint i) {
	char s[11];
	sprintf(s, "%u", i);
	uint size = _strlen(s);
	m_core = new StringCore(size);
	_memcpy(m_core->value(), s, size);
}

template <typename Char, class Container>
BasicString<Char, Container>::BasicString (int64 i) {
	char s[20];
#if NX_ARCH_64BIT
	sprintf(s, "%ld", i);
#else
	sprintf(s, "%lld", i);
#endif
	uint size = _strlen(s);
	m_core = new StringCore(size);
	_memcpy(m_core->value(), s, size);
}

template <typename Char, class Container>
BasicString<Char, Container>::BasicString (uint64 i) {
	char s[20];
#if NX_ARCH_64BIT
	sprintf(s, "%lu", i);
#else
	sprintf(s, "%llu", i);
#endif
	uint size = _strlen(s);
	m_core = new StringCore(size);
	_memcpy(m_core->value(), s, size);
}

template <typename Char, class Container>
BasicString<Char, Container>::BasicString (float f) {
	char s[20];
	sprintf(s, "%f", f);
	uint size = _strlen(s);
	m_core = new StringCore(size);
	_memcpy(m_core->value(), s, size);
}

template <typename Char, class Container>
BasicString<Char, Container>::BasicString (double f) {
	char s[20];
	sprintf(s, "%g", f);
	uint size = _strlen(s);
	m_core = new StringCore(size);
	_memcpy(m_core->value(), s, size);
}

template <typename Char, class Container>
BasicString<Char, Container>::BasicString(const Object& data) {
	NX_UNIMPLEMENTED();
}

template <typename Char, class Container>
template <typename Char2>
BasicString<Char, Container>::BasicString(const Char2* s) {
	if (s) {
		uint len = _strlen(s);
		if (len) {
			m_core = new StringCore(len);
			_memcpy(m_core->value(), s, len);
			return;
		}
	}
	m_core = StringCore::empty();
}

template <typename Char, class Container>
BasicString<Char, Container>::~BasicString() {
	m_core->release();
	m_core = nullptr;
}

template <typename Char, class Container>
bool BasicString<Char, Container>::is_empty() const { 
	return m_core->length == 0; 
}

template <typename Char, class Container>
const Char* BasicString<Char, Container>::c() const {
	return m_core->value();
}
	
template <typename Char, class Container>
const Char* BasicString<Char, Container>::operator*() const {
	return m_core->value();
}

template <typename Char, class Container>
Char BasicString<Char, Container>::operator[](uint index) const {
	return m_core->value()[index];
}

template <typename Char, class Container>
uint BasicString<Char, Container>::capacity() const {
	return m_core->capacity();
}

template <typename Char, class Container>
uint BasicString<Char, Container>::length() const {
	return m_core->length;
}

template <typename Char, class Container>
BasicString<Char, Container> 
BasicString<Char, Container>::substr(uint start, uint length) const {
	return BasicString(c() + start, length);
}

template <typename Char, class Container>
BasicString<Char, Container> 
BasicString<Char, Container>::substr(uint start) const {
	return substr(start, length() - start);
}

template <typename Char, class Container>
BasicString<Char, Container> 
BasicString<Char, Container>::substring(uint start, uint end) const {
	return BasicString(c() + start, end - start);
}

template <typename Char, class Container>
BasicString<Char, Container> 
BasicString<Char, Container>::substring(uint start) const {
	return substring(start, length());
}

template <typename Char, class Container>
BasicString<Char, Container> 
BasicString<Char, Container>::full_copy() const {
	return BasicString(new StringCore(*m_core));
}

template <typename Char, class Container>
BasicString<Char, Container>& 
BasicString<Char, Container>::operator+=(const BasicString& s) {
	return push(*s, s.length());
}

template <typename Char, class Container>
BasicString<Char, Container> 
BasicString<Char, Container>::operator+(const BasicString& s) const {
	return BasicString(c(), length(), *s, s.length());
}

template<> BasicString<char, Container<char>>::BasicString(const Object& o);
template<> BasicString<uint16, Container<uint16>>::BasicString(const Object& o);
template<> BasicString<uint32, Container<uint32>>::BasicString(const Object& o);

template <typename Char, class Container>
Array<BasicString<Char, Container>>
BasicString<Char, Container>::split(const BasicString& sp) const { // Not Thread safe
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

static const char ws[8] = {
	0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x20, /*0xA0,*/ 0x0
};

/**
 * @func is_blank
 */
template <typename Char, class Container>
bool BasicString<Char, Container>::is_blank() const { // Not Thread safe
	
	uint len = length();
	
	if ( ! len ) return true;
	
	Char* value = m_core->value();
	
	for (uint i = 0 ; i < len; i++) {
		if (strchr(ws, value[i]) == nullptr) {
			return false;
		}
	}
	
	return true;
}

template <typename Char, class Container>
BasicString<Char, Container> 
BasicString<Char, Container>::trim () const { // Not Thread safe
	uint len = length();
	Char* value = m_core->value();
	uint start = 0;
	uint end = len;
	for ( ; start < len; start++) {
		if (strchr(ws, value[start]) == nullptr) {
			break;
		}
	}
	if (start == len) {
		return BasicString(); // empty
	} else {
		for ( ; end > 0; end--) {
			if (strchr(ws, value[end - 1]) == nullptr) {
				break;
			}
		}
	}
	if (start == 0 && end == len) {
		return *this;
	}
	return substring(start, end);
}

template <typename Char, class Container>
BasicString<Char, Container> 
BasicString<Char, Container>::trim_left () const { // Not Thread safe
	uint len = length();
	Char* value = m_core->value();
	for (uint start = 0; start < len; start++) {
		if (strchr(ws, value[start]) == nullptr) {
			if (start == 0) {
				return *this;
			} else {
				return substring(start);
			}
		}
	}
	return BasicString();
}

template <typename Char, class Container>
BasicString<Char, Container> 
BasicString<Char, Container>::trim_right () const { // Not Thread safe
	uint len = length();
	Char* value = m_core->value();
	for (uint end = len; end > 0; end--) {
		if (strchr(ws, value[end - 1]) == nullptr) {
			if (end == len) {
				return *this;
			} else {
				return substring(0, end);
			}
		}
	}
	return BasicString();
}

template <typename Char, class Container>
BasicString<Char, Container>& 
BasicString<Char, Container>::upper_case () { // Not Thread safe
	m_core->modify(this);
	uint len = length();
	Char* s = m_core->value();
	
	for (uint i = 0; i < len; i++, s++) {
		*s = toupper(*s);
	}
	return *this;
}

template <typename Char, class Container>
BasicString<Char, Container>& 
BasicString<Char, Container>::lower_case () { // Not Thread safe
	m_core->modify(this);
	uint len = length();
	Char* s = m_core->value();
	
	for (uint i = 0; i < len; i++, s++) {
		*s = tolower(*s);
	}
	return *this;
}

template <typename Char, class Container>
BasicString<Char, Container> BasicString<Char, Container>::to_upper_case() const { // Not Thread safe
	return String(*this).upper_case();
}

template <typename Char, class Container>
BasicString<Char, Container> BasicString<Char, Container>::to_lower_case() const { // Not Thread safe
	return String(*this).lower_case();
}

template <typename Char, class Container>
Char* BasicString<Char, Container>::collapse() { // Not Thread safe
	Char* rev = nullptr;
	if ( m_core->ref() == 1 ) {
		rev = m_core->collapse();
	} else {
		rev = (Char*)::malloc(m_core->length * sizeof(Char));
		_memcpy(rev, m_core->value(), m_core->length);
		rev[m_core->length] = 0;
	}
	m_core->release();
	m_core = StringCore::empty();
	return rev;
}

template <typename Char, class Container>
ArrayBuffer<Char> BasicString<Char, Container>::collapse_buffer() { // Not Thread safe
	uint len = m_core->length;
	return ArrayBuffer<Char>(collapse(), len);
}

template <typename Char, class Container>
ArrayBuffer<Char> BasicString<Char, Container>::copy_buffer () const { // Not Thread safe
	return WeakArrayBuffer<Char>(*m_core->container, m_core->length).copy();
}

template <typename Char, class Container>
int BasicString<Char, Container>::index_of(const BasicString& s, uint start) const { // Not Thread safe
	return _index_of(this, *s, s.length(), start);
}

template <typename Char, class Container>
int BasicString<Char, Container>::last_index_of(const BasicString& s, 
																								int start) const { // Not Thread safe
	return _last_index_of(this, *s, s.length(), start);
}

template <typename Char, class Container>
int BasicString<Char, Container>::last_index_of(const BasicString& s) const { // Not Thread safe
	return _last_index_of(this, *s, s.length(), length());
}

template <typename Char, class Container>
BasicString<Char, Container>
BasicString<Char, Container>::replace(const BasicString& s,
																			const BasicString& rep) const { // Not Thread safe
	return replace_(this, s.c(), s.length(), *rep, rep.length());
}

template <typename Char, class Container>
BasicString<Char, Container>
BasicString<Char, Container>::replace_all(const BasicString& s, 
																					const BasicString& rep) const { // Not Thread safe
	return replace_all_(this, s.c(), s.length(), *rep, rep.length());
}

template <typename Char, class Container>
BasicString<Char, Container>& 
BasicString<Char, Container>::push(const Char* s, uint len) { // Not Thread safe
	if (len > 0) {
		uint self_length = m_core->length;
		uint full_length = self_length + len;

		if (m_core->ref() > 1) { // 当前不是唯一引用
			auto raw_core = m_core;
			m_core = new StringCore(full_length);
			_memcpy(m_core->value(), raw_core->value(), self_length);
			raw_core->release(); // 并不会真的释放,只是减少一个引用
		}
		else { // 自动调整容量
			m_core->container.realloc(full_length + 1);
			m_core->length = full_length;
		}
		
		Char* value = m_core->value();
		_memcpy(value + self_length, s, len);
		
		value[full_length] = '\0';
	}
	return *this;
}

template <typename Char, class Container>
BasicString<Char, Container>& BasicString<Char, Container>::push(const BasicString& s) { // Not Thread safe
	return push(s.c(), s.length());
}

template <typename Char, class Container>
BasicString<Char, Container>& BasicString<Char, Container>::push(Char s) { // Not Thread safe
	return push(&s, 1);
}

template <typename Char, class Container>
BasicString<Char, Container>& BasicString<Char, Container>::assign(const Char* s, uint len) { // Not Thread safe

	if (m_core->ref() > 1) { // 当前不是唯一引用,抛弃核心创建一个新的核心
		m_core->release();
		m_core = len ? new StringCore(len) : StringCore::empty();
	} else { // 当唯一引用时,调用自动调整容量
		m_core->container.realloc(len + 1);
	}
	m_core->length = len;
	
	Char* value = m_core->value();
	_memcpy(value, s, len);
	value[len] = '\0';
	return *this;
}

template <typename Char, class Container>
BasicString<Char, Container>& BasicString<Char, Container>::operator=(const BasicString& s) { // Not Thread safe
	auto old_co = m_core;
	m_core = s.m_core;
	m_core->retain();
	old_co->release();
	return *this;
}

template <typename Char, class Container>
BasicString<Char, Container>& BasicString<Char, Container>::operator=(BasicString&& s) { // Not Thread safe
	auto core = s.m_core;
	s.m_core = StringCore::empty();
	auto self = m_core;
	m_core = core;
	self->release();
	return *this;
}

template <typename Char, class Container>
bool BasicString<Char, Container>::operator==(const BasicString& s) const {
	return _compare(this, s.c()) == 0;
}

template <typename Char, class Container>
bool BasicString<Char, Container>::operator!=(const BasicString& s) const {
	return _compare(this, s.c()) != 0;
}

template <typename Char, class Container>
bool BasicString<Char, Container>::operator>(const BasicString& s) const {
	return _compare(this, s.c()) > 0;
}

template <typename Char, class Container>
bool BasicString<Char, Container>::operator<(const BasicString& s) const {
	return _compare(this, s.c()) < 0;
}

template <typename Char, class Container>
bool BasicString<Char, Container>::operator>=(const BasicString& s) const {
	return _compare(this, s.c()) >= 0;
}

template <typename Char, class Container>
bool BasicString<Char, Container>::operator<=(const BasicString& s) const {
	return _compare(this, s.c()) <= 0;
}

template <typename Char, class Container>
int BasicString<Char, Container>::index_of (const Char* s, uint start) const {
	return _index_of(this, s, _strlen(s), start);
}

template <typename Char, class Container>
int BasicString<Char, Container>::last_index_of(const Char* s, int start) const {
	return _last_index_of(this, s, _strlen(s), start);
}

template <typename Char, class Container>
int BasicString<Char, Container>::last_index_of(const Char* s) const {
	return _last_index_of(this, s, _strlen(s), length());
}

template <typename Char, class Container>
BasicString<Char, Container>
BasicString<Char, Container>::replace (const Char* s, const Char* rep) const {
	return replace_(this, s, _strlen(s), rep, _strlen(rep));
}

template <typename Char, class Container>
BasicString<Char, Container>
BasicString<Char, Container>::replace_all(const Char* s, const Char* rep) const {
	return replace_all_(this, s, _strlen(s), rep, _strlen(rep));
}

template <typename Char, class Container>
BasicString<Char, Container>& BasicString<Char, Container>::push(const Char* s) {
	return push(s, _strlen(s));
}

template <typename Char, class Container>
BasicString<Char, Container>& BasicString<Char, Container>::operator+=(const Char* s) {
	return push(s, _strlen(s));
}

template <typename Char, class Container>
BasicString<Char, Container> BasicString<Char, Container>::operator+(const Char* s) const {
	return BasicString(c(), length(), s, _strlen(s));
}

template <typename Char, class Container>
BasicString<Char, Container>& BasicString<Char, Container>::operator=(const Char* s) {
	return assign(s, _strlen(s));
}

template <typename Char, class Container>
bool BasicString<Char, Container>::operator==(const Char* s) const {
	return _compare(this, s) == 0;
}

template <typename Char, class Container>
bool BasicString<Char, Container>::operator!=(const Char* s) const {
	return _compare(this, s) != 0;
}

template <typename Char, class Container>
bool BasicString<Char, Container>::operator>(const Char* s) const {
	return _compare(this, s) > 0;
}

template <typename Char, class Container>
bool BasicString<Char, Container>::operator<(const Char* s) const {
	return _compare(this, s) < 0;
}

template <typename Char, class Container>
bool BasicString<Char, Container>::operator>=(const Char* s) const {
	return _compare(this, s) >= 0;
}

template <typename Char, class Container>
bool BasicString<Char, Container>::operator<=(const Char* s) const {
	return _compare(this, s) <= 0;
}

template <typename Char, class Container>
uint BasicString<Char, Container>::hash_code() const {
	return ngui::hash_code(reinterpret_cast<cchar*>(m_core->value()),
														m_core->length * sizeof(Char));
}

template<typename Char, class Container>
String BasicString<Char, Container>::to_string() const {
	static String str("[String]");
	return str;
}

template<> String BasicString<char, Container<char>>::to_string() const;
template<> String BasicString<uint16, Container<uint16>>::to_string() const;
template<> String BasicString<uint32, Container<uint32>>::to_string() const;

template<typename Char>
static int _sscanf(const Char* s, cchar* f, void* out, uint len) {
	char str[21];
	_memcpy(str, s, NX_MIN(len + 1, 21));
	return sscanf( str, f, out);
}

template<> NX_INLINE int _sscanf<char>(cchar* s, cchar* f, void* out, uint len) {
	return sscanf( s, f, out );
}

template <typename Char, class Container>
int BasicString<Char, Container>::to_int() const {
	int i;
	_sscanf( c(), "%d", &i, length() );
	return i;
}

template <typename Char, class Container>
uint BasicString<Char, Container>::to_uint() const {
	uint i;
	_sscanf( c(), "%u", &i, length() );
	return i;
}

template <typename Char, class Container>
int64 BasicString<Char, Container>::to_int64() const {
	int64 i;
#if NX_ARCH_64BIT
	_sscanf( c(), "%ld", &i, length() );
#else
	_sscanf( c(), "%lld", &i, length() );
#endif
	return i;
}

template <typename Char, class Container>
uint64 BasicString<Char, Container>::to_uint64() const {
	uint64 i;
#if NX_ARCH_64BIT
	_sscanf( c(), "%lu", &i, length() );
#else
	_sscanf( c(), "%llu", &i, length() );
#endif
	return i;
}

template <typename Char, class Container>
float BasicString<Char, Container>::to_float() const {
	float i;
	_sscanf( c(), "%fd", &i, length() );
	return i;
}

template <typename Char, class Container>
double BasicString<Char, Container>::to_double() const {
	double i;
	_sscanf( c(), "%lf", &i, length() );
	return i;
}

template <typename Char, class Container>
bool BasicString<Char, Container>::to_bool() const {
	if (_memcmp(c(), "true", 5) == 0) {
		return true;
	}
	return false;
}

template <typename Char, class Container>
bool BasicString<Char, Container>::to_int(int* out) const {
	return _sscanf( c(), "%d", out, length() );
}

template <typename Char, class Container>
bool BasicString<Char, Container>::to_uint(uint* out) const {
	return _sscanf( c(), "%u", out, length() );
}

template <typename Char, class Container>
bool BasicString<Char, Container>::to_int64(int64* out) const {
#if NX_ARCH_64BIT
	return _sscanf( c(), "%ld", out, length() );
#else
	return _sscanf( c(), "%lld", out, length() );
#endif
}

template <typename Char, class Container>
bool BasicString<Char, Container>::to_uint64(uint64* out) const {
#if NX_ARCH_64BIT
	return _sscanf( c(), "%lu", out, length() );
#else
	return _sscanf( c(), "%llu", out, length() );
#endif
}

template <typename Char, class Container>
bool BasicString<Char, Container>::to_float(float* out) const {
	return _sscanf( c(), "%fd", out, length() );
}

template <typename Char, class Container>
bool BasicString<Char, Container>::to_double(double* out) const {
	return _sscanf( c(), "%lf", out, length() );
}

template <typename Char, class Container>
bool BasicString<Char, Container>::to_bool(bool* out) const {
	if (_memcmp(c(), "true", 5) == 0) {
		*out = true;
	} else {
		*out = false;
	}
	return true;
}

#if NX_GNUC
#define NX_STRING_FORMAT(format, str) \
String str; \
va_list __arg;  \
va_start(__arg, format);  \
char* __buf = nullptr;  \
int __len = vasprintf(&__buf, format, __arg); \
if (__buf) {  \
	str = String(Buffer(__buf,__len));  \
} \
va_end(__arg)
#else
#define NX_STRING_FORMAT(format, str) \
String str; \
va_list __arg;  \
va_start(__arg, format);  \
uint __len = _vscprintf(format, __arg); \
if (__len) {  \
	char* buf = (char*)malloc(__len+1); \
	buf[__len] = '\0';  \
	va_start(__arg, format);  \
	_vsnprintf_s(buf, __len + 1, format, __arg);  \
	str = String(Buffer(buf, __len)); \
} \
va_end(__arg)
#endif

template <typename Char, class Container>
String BasicString<Char, Container>::format(cchar* format, ...) {
	NX_STRING_FORMAT(format, str);
	return str;
}

// Array join

template<class Item, class Container>
String Array<Item, Container>::join(cString& sp) const {
	String rev;
	
	const Item* value = *_container;
	const Item* end = value + _length;
	
	if (value < end) {
		rev.push(String(*value));
		value++;
	}
	
	while (value < end) {
		rev.push(sp);
		rev.push(String(*value));
		value++;
	}
	return rev;
}

NX_END
