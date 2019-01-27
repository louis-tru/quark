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

XX_NS(shark)

template <class Char, class Container, class Allocator>
BasicStringBuilder<Char, Container, Allocator>::BasicStringBuilder()
: List<Item, Allocator>(), m_string_length(0) {
	
}

template <class Char, class Container, class Allocator>
BasicStringBuilder<Char, Container, Allocator>::BasicStringBuilder(const BasicStringBuilder& s)
: List<Item, Allocator>(s), m_string_length(s.m_string_length) {
	
}

template <class Char, class Container, class Allocator>
BasicStringBuilder<Char, Container, Allocator>::BasicStringBuilder(BasicStringBuilder&& s)
: List<Item, Allocator>(move(s)), m_string_length(s.m_string_length) {
	s.m_string_length = 0;
}

template <class Char, class Container, class Allocator>
BasicStringBuilder<Char, Container, Allocator>::~BasicStringBuilder() {
	m_string_length = 0;
}

template <class Char, class Container, class Allocator>
BasicStringBuilder<Char, Container, Allocator>&
BasicStringBuilder<Char, Container, Allocator>::operator=(const BasicStringBuilder& s) {
	m_string_length = s.m_string_length;
	List<Item, Allocator>::operator=(s);
	return *this;
}

template <class Char, class Container, class Allocator>
BasicStringBuilder<Char, Container, Allocator>&
BasicStringBuilder<Char, Container, Allocator>::operator=(BasicStringBuilder&& s) {
	m_string_length = s.m_string_length;
	List<Item, Allocator>::operator=(move(s));
	s.m_string_length = 0;
	return *this;
}

template <class Char, class Container, class Allocator>
void BasicStringBuilder<Char, Container, Allocator>::push(const Item& item) {
	m_string_length += item.length();
	List<Item, Allocator>::push(item);
}

template <class Char, class Container, class Allocator>
void BasicStringBuilder<Char, Container, Allocator>::push(Item&& item) {
	m_string_length += item.length();
	List<Item, Allocator>::push(move(item));
}

template <class Char, class Container, class Allocator>
void BasicStringBuilder<Char, Container, Allocator>::push(const BasicStringBuilder& ls) {
	m_string_length += ls.m_string_length;
	List<Item, Allocator>::push(ls);
}

template <class Char, class Container, class Allocator>
void BasicStringBuilder<Char, Container, Allocator>::push(BasicStringBuilder&& ls) {
	m_string_length += ls.m_string_length;
	List<Item, Allocator>::push(move(ls));
	ls.m_string_length = 0;
}

template <class Char, class Container, class Allocator>
void BasicStringBuilder<Char, Container, Allocator>::unshift(const Item& item) {
	m_string_length += item.length();
	List<Item, Allocator>::unshift(item);
}

template <class Char, class Container, class Allocator>
void BasicStringBuilder<Char, Container, Allocator>::unshift(Item&& item) {
	m_string_length += item.length();
	List<Item, Allocator>::unshift(move(item));
}

template <class Char, class Container, class Allocator>
void BasicStringBuilder<Char, Container, Allocator>::unshift(const BasicStringBuilder& ls) {
	m_string_length += ls.string_length();
	List<Item, Allocator>::unshift(ls);
}

template <class Char, class Container, class Allocator>
void BasicStringBuilder<Char, Container, Allocator>::unshift(BasicStringBuilder&& ls) {
	m_string_length += ls.string_length();
	List<Item, Allocator>::unshift(move(ls));
	ls.m_string_length = 0;
}

template <class Char, class Container, class Allocator>
void BasicStringBuilder<Char, Container, Allocator>::pop() {
	if (this->length()) {
		m_string_length -= (--this->end()).value().length();
		List<Item, Allocator>::pop();
	}
}

template <class Char, class Container, class Allocator>
void BasicStringBuilder<Char, Container, Allocator>::shift() {
	if (this->length()) {
		m_string_length -= this->begin().value().length();
		List<Item, Allocator>::shift();
	}
}

template <class Char, class Container, class Allocator>
void BasicStringBuilder<Char, Container, Allocator>::insert(ConstIterator it, const Item& item) {
	m_string_length += item.length();
	List<Item, Allocator>::insert(it, item);
}

template <class Char, class Container, class Allocator>
void BasicStringBuilder<Char, Container, Allocator>::insert(ConstIterator it, Item&& item) {
	m_string_length += item.length();
	List<Item, Allocator>::insert(it, move(item));
}

template <class Char, class Container, class Allocator>
void BasicStringBuilder<Char, Container, Allocator>::del(ConstIterator it) {
	m_string_length -= it.value().length();
	List<Item, Allocator>::del(it);
}

template <class Char, class Container, class Allocator>
void BasicStringBuilder<Char, Container, Allocator>::clear() {
	m_string_length = 0;
	List<Item, Allocator>::clear();
}

template <class Char, class Container, class Allocator>
String BasicStringBuilder<Char, Container, Allocator>::join(cString& sp) const {
	return List<Item, Allocator>::json(sp);
}

template <class Char, class Container, class Allocator>
String BasicStringBuilder<Char, Container, Allocator>::to_string() const {
	return "[BasicStringBuilder]";
}

template<> String BasicStringBuilder<char, Container<char>>::join(cString& sp) const;
template<> String BasicStringBuilder<char, Container<char>>::to_string() const;

template <class Char, class Container, class Allocator>
BasicString<Char, Container>
BasicStringBuilder<Char, Container, Allocator>::to_basic_string() const {
	if (this->length() == 1) {
		return this->begin().value();
	}
	return to_buffer();
}

template <class Char, class Container, class Allocator>
ArrayBuffer<Char> BasicStringBuilder<Char, Container, Allocator>::to_buffer() const {
	ArrayBuffer<Char> buff(m_string_length, m_string_length + 1);
	// uint index = 0;
	Char* data = *buff;
	for (auto i = this->begin(), e = this->end(); i != e; i++) {
		uint len = i.value().length();
		memcpy(data, *i.value(), len * sizeof(Char));
		data += len;
	}
	*data = 0;
	return buff;
}

template <class Char, class Container, class Allocator>
uint BasicStringBuilder<Char, Container, Allocator>::string_length() const {
	return m_string_length;
}

XX_END
