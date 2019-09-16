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

#include "nxutils/string.h"

XX_NS(ngui)

template<class Item, class Allocator>
List<Item, Allocator>::IteratorData::IteratorData(): _host(nullptr), _item(nullptr) {
}

template<class Item, class Allocator>
List<Item, Allocator>::IteratorData::IteratorData(List* host, ItemWrap* item): _host(host), _item(item) {
}

template<class Item, class Allocator>
bool List<Item, Allocator>::IteratorData::equals(const IteratorData& it) const {
	return _item == it._item;
}

template<class Item, class Allocator>
bool List<Item, Allocator>::IteratorData::is_null() const {
	return _item == nullptr;
}

template<class Item, class Allocator>
const Item& List<Item, Allocator>::IteratorData::value() const {
	XX_ASSERT(_item);
	return _item->_item;
}

template<class Item, class Allocator>
Item& List<Item, Allocator>::IteratorData::value() {
	XX_ASSERT(_item);
	return _item->_item;
}

template<class Item, class Allocator>
void List<Item, Allocator>::IteratorData::prev() {
	if (_item) {
		if (_item != _host->_first) {
			_item = _item->_prev;
		}
	} else {
		if (_host) {
			_item = _host->_last;
		}
	}
}

template<class Item, class Allocator>
void List<Item, Allocator>::IteratorData::next() {
	if (_item) {
		_item = _item->_next;
	}
}

// List

template<class Item, class Allocator>
List<Item, Allocator>::List() : _first(nullptr), _last(nullptr), _length(0)
{ }

template<class Item, class Allocator>
List<Item, Allocator>::List(const List& list): _first(nullptr), _last(nullptr), _length(0) {
	auto i = list.begin();
	auto end = list.end();
	while (i != end) {
		push(i.value());
		i++;
	}
}

template<class Item, class Allocator>
List<Item, Allocator>::List(List&& list)
: _first(list._first), _last(list._last), _length(list._length)
{
	list._first = nullptr;
	list._last = nullptr;
	list._length = 0;
}

template<class Item, class Allocator>
List<Item, Allocator>::List(const std::initializer_list<Item>& list)
: _first(nullptr), _last(nullptr), _length(0) {
	for ( auto& i : list ) {
		push(move(i));
	}
}

template<class Item, class Allocator>
List<Item, Allocator>::~List() {
	clear();
}

template<class Item, class Allocator>
List<Item, Allocator>& List<Item, Allocator>::operator=(const List& list) {
	clear();
	auto i = list.begin();
	auto end = list.end();
	while (i != end) {
		push(i.value());
		i++;
	}
	return *this;
}

template<class Item, class Allocator>
List<Item, Allocator>& List<Item, Allocator>::operator=(List&& list) {
	clear();
	_first = list._first;
	_last = list._last;
	_length = list._length;
	list._first = nullptr;
	list._last = nullptr;
	list._length = 0;
	return *this;
}

#define list_unshift_item_(item) \
ItemWrap* w = item; \
if (_first) { \
	_first->_prev = w; \
	_first = w; \
} else { \
	_first = w; \
	_last = w; \
} \
_length++; \
return Iterator(IteratorData(this, w))

template<class Item, class Allocator>
typename List<Item, Allocator>::Iterator List<Item, Allocator>::unshift(const Item& item) {
	list_unshift_item_( new ItemWrap({ item, nullptr, _first }) );
}

template<class Item, class Allocator>
typename List<Item, Allocator>::Iterator List<Item, Allocator>::unshift(Item&& item) {
	list_unshift_item_( new ItemWrap({ move(item), nullptr, _first }) );
}

template<class Item, class Allocator>
void List<Item, Allocator>::unshift(const List& ls) {
	for ( auto& i : ls ) unshift( i.value() );
}

template<class Item, class Allocator>
void List<Item, Allocator>::unshift(List&& ls) {
	if (ls._first) {
		if ( _first ) {
			_first->_prev = ls._last;
			ls._last->_next = _first;
			_first = ls._first;
			_length += ls._length;
		} else {
			_first = ls._first;
			_last = ls._last;
			_length = ls._length;
		}
		ls._first = nullptr;
		ls._last = nullptr;
		ls._length = 0;
	}
}

#define list_push_item_(item) \
ItemWrap* w = item; \
	if (_last) { \
	_last->_next = w; \
	_last = w; \
} else { \
	_first = _last = w; \
} \
_length++; \
return Iterator(IteratorData(this, w))

template<class Item, class Allocator>
typename List<Item, Allocator>::Iterator List<Item, Allocator>::push(const Item& item) {
	list_push_item_( new ItemWrap({ item, _last, nullptr }) );
}

template<class Item, class Allocator>
typename List<Item, Allocator>::Iterator List<Item, Allocator>::push(Item&& item) {
	list_push_item_( new ItemWrap({ move(item), _last, nullptr }) );
}

template<class Item, class Allocator>
void List<Item, Allocator>::push(const List& ls) {
	for ( auto& i : ls ) push( i.value() );
}

template<class Item, class Allocator>
void List<Item, Allocator>::push(List&& ls) {
	if (ls._first) {
		if ( _first ) {
			_last->_next = ls._first;
			ls._first->_prev = _last;
			_last = ls._last;
			_length += ls._length;
		} else {
			_first = ls._first;
			_last = ls._last;
			_length = ls._length;
		}
		ls._first = nullptr;
		ls._last = nullptr;
		ls._length = 0;
	}
}

template<class Item, class Allocator>
void List<Item, Allocator>::pop() {
	if (_last) {
		ItemWrap* w = _last;
		if ( _last == _first ) {
			_last = nullptr;
			_first = nullptr;
		} else {
			_last = _last->_prev;
			if (_last) {
				_last->_next = nullptr;
			}
		}
		_length--;
		delete w;
	}
}

template<class Item, class Allocator>
void List<Item, Allocator>::shift() {
	if (_first) {
		ItemWrap* w = _first;
		if ( _first == _last ) {
			_first = nullptr;
			_last = nullptr;
		} else {
			_first = _first->_next;
			if (_first) {
				_first->_prev = nullptr;
			}
		}
		_length--;
		delete w;
	}
}

template<class Item, class Allocator>
void List<Item, Allocator>::pop(uint count) {
	for ( int i = 0; i < count && _first; i++ ) pop();
}

template<class Item, class Allocator>
void List<Item, Allocator>::shift(uint count) {
	for ( int i = 0; i < count && _first; i++ ) shift();
}

#define list_before_item_(item)\
if ((*it)._item->_prev) {\
	ItemWrap* w = item;\
	(*it)._item->_prev = w;\
	w->_prev->_next = w;\
	_length++;\
	return Iterator(IteratorData(this, w));\
} else {\
	list_unshift_item_( item );\
}

template<class Item, class Allocator>
typename List<Item, Allocator>::Iterator
List<Item, Allocator>::before(IteratorConst it, const Item& item) {
	list_before_item_( new ItemWrap({ item, (*it)._item->_prev, (*it)._item }) );
}

template<class Item, class Allocator>
typename List<Item, Allocator>::Iterator
List<Item, Allocator>::before(IteratorConst it, Item&& item) {
	list_before_item_( new ItemWrap({ move(item), (*it)._item->_prev, (*it)._item }) );
}

#define list_after_item_(item)\
if ((*it)._item->_next) {\
	ItemWrap* w = item;\
	(*it)._item->_next = w;\
	w->_next->_prev = w;\
	_length++;\
	return Iterator(IteratorData(this, w));\
} else {\
	list_push_item_( item );\
}

template<class Item, class Allocator>
typename List<Item, Allocator>::Iterator
List<Item, Allocator>::after(IteratorConst it, const Item& item) {
	list_after_item_( new ItemWrap({ item, (*it)._item, (*it)._item->_next }) );
}

template<class Item, class Allocator>
typename List<Item, Allocator>::Iterator
List<Item, Allocator>::after(IteratorConst it, Item&& item) {
	list_after_item_( new ItemWrap({ move(item), (*it)._item, (*it)._item->_next }) );
}

#undef list_push_item_
#undef list_unshift_item_
#undef list_before_item_
#undef list_after_item_

template<class Item, class Allocator>
void List<Item, Allocator>::del(IteratorConst it) {
	ItemWrap* item = it.data()._item;
	if (item) {
		XX_ASSERT( it.data()._host == this);
		ItemWrap* prev = item->_prev;
		ItemWrap* next = item->_next;
		
		if (prev) {
			prev->_next = next;
		} else {
			_first = next;
		}
		if (next) {
			next->_prev = prev;
		} else {
			_last = prev;
		}
		
		_length--;
		delete item;
	}
}

template<class Item, class Allocator>
void List<Item, Allocator>::clear() {
	ItemWrap* item = _first;
	while (item) {
		ItemWrap* tmp = item;
		item = item->_next;
		delete tmp;
	}
	_length = 0;
	_first = _last = nullptr;
}

#define list_find_iterator0_(offset)  \
if (offset < _length) { \
	if (offset <= _length / 2.0) {  \
		return find(begin(), offset); \
	} else {  \
		return find(end(), offset - _length); \
	} \
} \
return end()

template<class Item, class Allocator>
typename List<Item, Allocator>::IteratorConst
List<Item, Allocator>::find(uint offset) const {
	list_find_iterator0_(offset);
}

template<class Item, class Allocator>
typename List<Item, Allocator>::Iterator List<Item, Allocator>::find(uint offset) {
	list_find_iterator0_(offset);
}

#define list_find_iterator_(it, offset)  \
ItemWrap* item = (*it)._item; \
if (offset > 0) { \
	while (offset) {  \
		if (item) { \
			item = item->_next; \
			offset--; \
		} else {  \
			break;  \
		} \
	} \
} else if (offset < 0) {  \
	if (!item) {  \
		item = _last; \
		offset++; \
	} \
	while (offset) {  \
		if (item) { \
			item = item->_prev; \
			offset++; \
		} else {  \
			break;  \
		} \
	} \
}

template<class Item, class Allocator>
typename List<Item, Allocator>::IteratorConst
List<Item, Allocator>::find(IteratorConst it, int offset) const {
	list_find_iterator_(it, offset);
	return IteratorConst(IteratorData(this, item));
}

template<class Item, class Allocator>
typename List<Item, Allocator>::Iterator
List<Item, Allocator>::find(IteratorConst it, int offset) {
	list_find_iterator_(it, offset);
	return Iterator(IteratorData(this, item));
}

#undef _list_find_iterator0
#undef _list_find_iterator

// List join

template<class Item, class Allocator>
String List<Item, Allocator>::join(cString& sp) const {
	String rev;
	ItemWrap* item = _first;
	
	if (item) {
		rev.push(String(item->_item));
		item = item->_next;
	}
	
	while (item) {
		rev.push(sp);
		rev.push(String(item->_item));
		item = item->_next;
	}
	return rev;
}

template<class Item, class Allocator>
const Item& List<Item, Allocator>::first()const {
	return _first->_item;
}

template<class Item, class Allocator>
const Item& List<Item, Allocator>::last()const {
	return _last->_item;
}

template<class Item, class Allocator>
Item& List<Item, Allocator>::first() {
	return _first->_item;
}

template<class Item, class Allocator>
Item& List<Item, Allocator>::last() {
	return _last->_item;
}

template<class Item, class Allocator>
typename List<Item, Allocator>::IteratorConst
List<Item, Allocator>::begin() const {
	return IteratorConst(IteratorData(const_cast<List*>(this), _first));
}

template<class Item, class Allocator>
typename List<Item, Allocator>::IteratorConst
List<Item, Allocator>::end() const {
	return IteratorConst(IteratorData(const_cast<List*>(this), NULL));
}

template<class Item, class Allocator>
typename List<Item, Allocator>::Iterator
List<Item, Allocator>::begin() {
	return Iterator(IteratorData(this, _first));
}

template<class Item, class Allocator>
typename List<Item, Allocator>::Iterator
List<Item, Allocator>::end() {
	return Iterator(IteratorData(this, NULL));
}

template<class Item, class Allocator>
uint List<Item, Allocator>::length() const {
	return _length;
}

XX_END
