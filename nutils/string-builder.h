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

#ifndef __ngui__utils__string_builder__
#define __ngui__utils__string_builder__

#include "nutils/list.h"
#include "nutils/string.h"
#include "nutils/buffer.h"

XX_NS(ngui)

template <typename Char,
					class Container,
					class ItemAllocator = DefaultAllocator>
class BasicStringBuilder;
typedef BasicStringBuilder<char, Container<char>> StringBuilder;
typedef BasicStringBuilder<uint16, Container<uint16>> Ucs2StringBuilder;

/**
 * @class BasicLongString 字符串链表
 */
template <typename Char, class Container, class ItemAllocator>
class XX_EXPORT BasicStringBuilder: public List<BasicString<Char, Container>, ItemAllocator> {
 public:
	typedef BasicString<Char, Container> Item;
	typedef typename List<Item, ItemAllocator>::Iterator Iterator;
	typedef typename List<Item, ItemAllocator>::IteratorConst IteratorConst;
	
	BasicStringBuilder();
	BasicStringBuilder(const BasicStringBuilder&);
	BasicStringBuilder(BasicStringBuilder&&);
	virtual ~BasicStringBuilder();
	BasicStringBuilder& operator=(const BasicStringBuilder&);
	BasicStringBuilder& operator=(BasicStringBuilder&&);
	void push(const Item& item);
	void push(Item&& item);
	void push(const BasicStringBuilder& ls);
	void push(BasicStringBuilder&& ls);
	void unshift(const Item& item);
	void unshift(Item&& item);
	void unshift(const BasicStringBuilder& ls);
	void unshift(BasicStringBuilder&& ls);
	void pop();
	void shift();
	void insert(IteratorConst it, const Item& item);
	void insert(IteratorConst it, Item&& item);
	void del(IteratorConst it);
	void clear();
	String join(cString& sp) const;
	virtual String to_string() const;
	BasicString<Char, Container> to_basic_string() const;
	ArrayBuffer<Char> to_buffer() const;
	uint string_length() const;
	
 private:
	uint m_string_length;
};

XX_END

#include "string-builder.inl"

#endif
