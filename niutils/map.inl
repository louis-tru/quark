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

XX_NS(ngui)

// Compare

// hash
template<> XX_EXPORT uint Compare<char>::hash(const char& key);
template<> XX_EXPORT uint Compare<byte>::hash(const byte& key);
template<> XX_EXPORT uint Compare<int16>::hash(const int16& key);
template<> XX_EXPORT uint Compare<uint16>::hash(const uint16& key);
template<> XX_EXPORT uint Compare<int>::hash(const int& key);
template<> XX_EXPORT uint Compare<uint>::hash(const uint& key);
template<> XX_EXPORT uint Compare<int64>::hash(const int64& key);
template<> XX_EXPORT uint Compare<uint64>::hash(const uint64& key);
template<> XX_EXPORT uint Compare<float>::hash(const float& key);
template<> XX_EXPORT uint Compare<double>::hash(const double& key);
template<> XX_EXPORT uint Compare<bool>::hash(const bool& key);
// equals
template<> XX_EXPORT bool Compare<String>::equals(cString& a, cString& b, uint ha, uint hb);
template<> XX_EXPORT bool Compare<Ucs2String>::equals(cUcs2String& a, cUcs2String& b, uint ha, uint hb);
template<> XX_EXPORT bool Compare<char>::equals(const char& a, const char& b, uint ha, uint hb);
template<> XX_EXPORT bool Compare<byte>::equals(const byte& a, const byte& b, uint ha, uint hb);
template<> XX_EXPORT bool Compare<int16>::equals(const int16& a, const int16& b, uint ha, uint hb);
template<> XX_EXPORT bool Compare<uint16>::equals(const uint16& a, const uint16& b, uint ha, uint hb);
template<> XX_EXPORT bool Compare<int>::equals(const int& a, const int& b, uint ha, uint hb);
template<> XX_EXPORT bool Compare<uint>::equals(const uint& a, const uint& b, uint ha, uint hb);
template<> XX_EXPORT bool Compare<int64>::equals(const int64& a, const int64& b, uint ha, uint hb);
template<> XX_EXPORT bool Compare<uint64>::equals(const uint64& a, const uint64& b, uint ha, uint hb);
template<> XX_EXPORT bool Compare<float>::equals(const float& a, const float& b, uint ha, uint hb);
template<> XX_EXPORT bool Compare<double>::equals(const double& a, const double& b, uint ha, uint hb);
template<> XX_EXPORT bool Compare<bool>::equals(const bool& a, const bool& b, uint ha, uint hb);

// IteratorConst

template<class Key, class Value, class Compare>
void Map<Key, Value, Compare>::NodeList::realloc(uint capacity) {
	
	if (capacity == 0) {
		::free(this->m_value);
		this->m_capacity = 0;
		this->m_value = nullptr;
		return;
	}
	capacity = XX_MAX(XX_MIN_CAPACITY, capacity);
	if ( !(capacity > this->m_capacity || capacity < this->m_capacity / 4.0) ) {
		return;
	}
	capacity = powf(2, ceil(log2(capacity)));
	
	uint size = sizeof(Node) * capacity;
	Node* value = static_cast<Node*>(::malloc(size));
	memset(value, 0, size);
	
	if (_host->m_length) { // 调整容量
		Node* i = this->m_value;
		Node* end = this->m_value + this->capacity();
		
		while (i < end) {
			if (i->first) { // 非空
				
				Item* item = i->first;
				while (item) { // 移动item
					Item* next = item->next;
					uint index = item->hash % capacity;
					Node* buk = value + index;
					
					if (buk->first) { // 冲突
						buk->last->next = item;
						item->prev = buk->last;
						item->next = NULL;
						buk->last = item;
					} else {
						buk->first = item;
						buk->last = item;
						item->prev = NULL;
						item->next = NULL;
					}
					item = next;
				}
			}
			i++;
		}
		
		::free(this->m_value);
	} else {
		XX_ASSERT(!this->m_capacity);
	}
	
	this->m_capacity = capacity;
	this->m_value = value;
}

template<class Key, class Value, class Compare>
void Map<Key, Value, Compare>::NodeList::auto_realloc() {
	// 使用超过70%需要增加容量
	realloc( ceilf(_host->m_length / 0.7f) );
}

template<class Key, class Value, class Compare>
Map<Key, Value, Compare>::IteratorData::IteratorData(): _host(NULL), _item(NULL) {
}

template<class Key, class Value, class Compare>
Map<Key, Value, Compare>::IteratorData::IteratorData(Map* host, Item* item) : _host(host), _item(item) {
}

template<class Key, class Value, class Compare>
const Key& Map<Key, Value, Compare>::IteratorData::key() const {
	XX_ASSERT(_item);
	return _item->key;
}

template<class Key, class Value, class Compare>
Key& Map<Key, Value, Compare>::IteratorData::key() {
	XX_ASSERT(_item);
	return _item->key;
}

template<class Key, class Value, class Compare>
const Value& Map<Key, Value, Compare>::IteratorData::value() const {
	XX_ASSERT(_item);
	return _item->value;
}

template<class Key, class Value, class Compare>
Value& Map<Key, Value, Compare>::IteratorData::value() {
	XX_ASSERT(_item);
	return _item->value;
}

template<class Key, class Value, class Compare>
bool Map<Key, Value, Compare>::IteratorData::equals(const IteratorData& it) const {
	return _item == it._item;
}

template<class Key, class Value, class Compare>
bool Map<Key, Value, Compare>::IteratorData::is_null() const {
	return _item == nullptr;
}

template<class Key, class Value, class Compare>
void Map<Key, Value, Compare>::IteratorData::begen() {
	Node* i = *_host->m_nodes;
	Node* end = i + _host->m_nodes.capacity();
	while (i < end) {
		if (i->first) {
			_item = i->first; return;
		}
		i++;
	}
	_item = NULL;
}

template<class Key, class Value, class Compare>
void Map<Key, Value, Compare>::IteratorData::prev() {
	Node* buk = NULL;
	
	if (_item) {
		if (_item->prev) {
			_item = _item->prev; return;
		}
		else {
			buk = *_host->m_nodes + ((_item->hash % _host->m_nodes.capacity()) - 1);
		}
	} else { // 查找最后一个
		buk = *_host->m_nodes + (_host->m_nodes.capacity() - 1);
	}
	
	// 向上查找
	Node* begin = *_host->m_nodes;
	while (buk >= begin) {
		if (buk->last) {
			_item = buk->last; return;
		}
		buk--;
	}
}

template<class Key, class Value, class Compare>
void Map<Key, Value, Compare>::IteratorData::next() {
	if (_item) {
		if (_item->next) {
			_item = _item->next; return;
		} else {
			uint capacity = _host->m_nodes.capacity();
			Node* buk = *_host->m_nodes;
			Node* i = buk + ((_item->hash % capacity) + 1);
			buk = buk + capacity;
			
			while (i < buk) {
				if (i->first) {
					_item = i->first; return;
				}
				i++;
			}
		}
		_item = NULL;
	}
}

// ====================== Map ======================

template<class Key, class Value, class Compare>
Map<Key, Value, Compare>::Map()
: m_length(0), m_nodes(), m_marks() {
	m_nodes._host = this;
}

template<class Key, class Value, class Compare>
Map<Key, Value, Compare>::Map(const Map& value)
: m_length(0), m_nodes(), m_marks() {
	m_nodes._host = this;
	operator=(value);
}

template<class Key, class Value, class Compare>
Map<Key, Value, Compare>::Map(Map&& value)
: m_length(value.m_length)
, m_nodes(move(value.m_nodes))
, m_marks(move(value.m_marks))
{
	value.m_length = 0;
	m_nodes._host = this;
}

template<class Key, class Value, class Compare>
Map<Key, Value, Compare>::Map(const InitializerList& list)
: m_length(0), m_nodes(), m_marks() {
	m_nodes._host = this;
	for ( auto& i : list ) {
		set(move(i.key), move(i.value));
	}
}

template<class Key, class Value, class Compare>
Map<Key, Value, Compare>::~Map() {
	clear();
}

template<class Key, class Value, class Compare>
Map<Key, Value, Compare>& Map<Key, Value, Compare>::operator=(const Map& value) {
	clear();

	if (value.m_length) {
		m_nodes.realloc(value.m_nodes.capacity());
		m_length = value.m_length;

		const Node* node = *value.m_nodes;
		const Node* end = node + value.m_nodes.capacity();
		Node* node_1 = *m_nodes;

		while ( node < end ) {
			if ( node->first ) {
				Item* item = node->first;
				Item* item_1 = new Item(*item); // 复制

				item_1->mark = false;
				item_1->prev = item_1->next = NULL;
				node_1->first = node_1->last = item_1;
				item = item->next;

				while (item) {
					Item* item_2 = new Item(*item); // 复制
					item_2->mark = false;
					item_1->next = item_2;
					item_2->prev = item_1;
					item_2->next = NULL;
					node_1->last = item_2;
					//
					item_1 = item_2;
					item = item->next;
				}
			}
			node++; node_1++;
		}
	}

	return *this;
}

template<class Key, class Value, class Compare>
Map<Key, Value, Compare>& Map<Key, Value, Compare>::operator=(Map&& value) {
	clear();
	m_length = value.m_length;
	m_nodes = ngui::move(value.m_nodes);
	m_nodes._host = this;
	m_marks = move(value.m_marks);
	value.m_length = 0;
	return *this;
}

template<class Key, class Value, class Compare>
const Value& Map<Key, Value, Compare>::operator[](const Key& key) const {
	return find(key).value();
}

template<class Key, class Value, class Compare>
Value& Map<Key, Value, Compare>::operator[](const Key& key) {
	bool is = false;
	Item* item = find_set(key, &is);
	if (is) { // 新的
		new(&item->key) Key(key);
		new(&item->value) Value();
	}
	return item->value;
}

template<class Key, class Value, class Compare>
Value& Map<Key, Value, Compare>::operator[](Key&& key) {
	bool is = false;
	Item* item = find_set(key, &is);
	if (is) { // 新的
		new(&item->key) Key(move(key));
		new(&item->value) Value();
	}
	return item->value;
}

template<class Key, class Value, class Compare>
typename Map<Key, Value, Compare>::Item* Map<Key, Value, Compare>::find2(const Key& key) {
	
	if (m_length) {
		
		uint hash = Compare::hash(key);
		Item* item = ( *m_nodes + (hash % m_nodes.capacity()) )->first;
		
		while (item) {
			// 用equals做比较,冲突越多(链表长度)会越慢.相同才返回
			if ( Compare::equals(item->key, key, item->hash, hash) ) {
				return item;
			} else { // 不相同继续比较
				item = item->next;
			}
		}
	}
	return nullptr;
}

template<class Key, class Value, class Compare>
typename Map<Key, Value, Compare>::Item*
Map<Key, Value, Compare>::find_set(const Key& key, bool* is_new) {
	uint hash = Compare::hash(key); // hash code
	
	if (m_length) {
		uint index = hash % m_nodes.capacity();
		Node* buk = *m_nodes + index;
		Item* item = buk->first;
		
		while (item) {
			// 用equals做比较,冲突越多(链表长度)会越慢.相同才返回
			if (Compare::equals(item->key, key, item->hash, hash)) {
				return item;
			} else { // 不相同继续比较
				item = item->next;
			}
		}
	}
	
	// 添加一个新的Item
	m_length++;
	m_nodes.auto_realloc();  // 自动调整容量
	
	uint index = hash % m_nodes.capacity();
	Node* buk = *m_nodes + index;
	Item* item = static_cast<Item*>(::malloc(sizeof(Item)));
	
	if (buk->first) { // 有冲突,插入到链表最开始
		buk->first->prev = item;
		item->next = buk->first;
		buk->first = item;
	} else {
		item->next = NULL;
		buk->first = item;
		buk->last = item;
	}
	
	item->prev = NULL;
	item->hash = hash;
	item->mark = false;
	*is_new = true;
	
	return item;
}

template<class Key, class Value, class Compare>
typename Map<Key, Value, Compare>::IteratorConst
Map<Key, Value, Compare>::find(const Key& key) const {
	Item* item = const_cast<Map*>(this)->find2(key);
	return IteratorConst(IteratorData(const_cast<Map*>(this), item));
}

template<class Key, class Value, class Compare>
typename Map<Key, Value, Compare>::Iterator Map<Key, Value, Compare>::find(const Key& key) {
	return Iterator(IteratorData(this, find2(key)));
}

template<class Key, class Value, class Compare>
bool Map<Key, Value, Compare>::has(const Key& key) const {
	return const_cast<Map*>(this)->find2(key) != NULL;
}

template<class Key, class Value, class Compare>
Array<Key> Map<Key, Value, Compare>::keys() const {
	Array<Key> rev;
	for (auto i = begin(), e = end(); i != e; i++)
		rev.push(i.data().key());
	return rev;
}

template<class Key, class Value, class Compare>
Array<Value> Map<Key, Value, Compare>::values() const {
	Array<Value> rev;
	for (auto i = begin(), e = end(); i != e; i++)
		rev.push(i.value());
	return rev;
}

template<class Key, class Value, class Compare>
Value& Map<Key, Value, Compare>::set(const Key& key, const Value& value) {
	bool is = false;
	Item* item = find_set(key, &is);
	if (is) { // 新的
		new(&item->key) Key(key);
		new(&item->value) Value(value);
	} else {
		item->value = value;
	}
	return item->value;
}

template<class Key, class Value, class Compare>
Value& Map<Key, Value, Compare>::set(Key&& key, Value&& value) {
	bool is = false;
	Item* item = find_set(key, &is);
	if (is) { // 新的
		new(&item->key) Key(move(key));
		new(&item->value) Value(move(value));
	} else {
		item->value = move(value);
	}
	return item->value;
}

template<class Key, class Value, class Compare>
Value& Map<Key, Value, Compare>::set(Key&& key, const Value& value) {
	bool is = false;
	Item* item = find_set(key, &is);
	if (is) { // 新的
		new(&item->key) Key(move(key));
		new(&item->value) Value(value);
	} else {
		item->value = value;
	}
	return item->value;
}

template<class Key, class Value, class Compare>
Value& Map<Key, Value, Compare>::set(const Key& key, Value&& value) {
	bool is = false;
	Item* item = find_set(key, &is);
	if (is) { // 新的
		new(&item->key) Key(key);
		new(&item->value) Value(move(value));
	} else {
		item->value = move(value);
	}
	return item->value;
}

template<class Key, class Value, class Compare>
void Map<Key, Value, Compare>::del2(Item* item) {
	Node* buk = *m_nodes + (item->hash % m_nodes.capacity());
	
	XX_ASSERT(!item->mark); // 有标记的不能删除
	
	if (item->prev || item->next) {
		Item* prev = item->prev;
		Item* next = item->next;
		
		if (prev) {
			prev->next = next;
		} else {
			buk->first = next;
		}
		if (next) {
			next->prev = prev;
		} else {
			buk->last = prev;
		}
	} else { // 清空一个存储桶
		buk->first = NULL;
		buk->last = NULL;
	}
	
	m_length--;
	if (m_length) { // 自动调整存储桶长度
		m_nodes.auto_realloc();
	} else { // 已不需要任何存储桶
		m_nodes.free();
	}
	
	delete item;
}

template<class Key, class Value, class Compare>
bool Map<Key, Value, Compare>::del(const Key& key) {
	Item* item = find2(key);
	if (item) {
		del2(item); return 1;
	}
	 return 0;
}

template<class Key, class Value, class Compare>
bool Map<Key, Value, Compare>::del(IteratorConst it) {
	Item* item = it.data()._item;
	if (item) {
		del2(item); return 1;
	}
	 return 0;
}

template<class Key, class Value, class Compare>
void Map<Key, Value, Compare>::clear() {
	if (m_length) {
		Node* i = *m_nodes;
		Node* end = i + m_nodes.capacity();
		
		while (i < end) {
			Item* item = i->first;
			while (item) {
				Item* tmp = item;
				item = item->next;
				delete tmp;
			}
			i++;
		}
		m_length = 0;
		m_nodes.free(); // 释放全部存储桶
		m_marks.clear();
	}
}

template<class Key, class Value, class Compare>
void Map<Key, Value, Compare>::mark(const Key& key) {
	Item* item = find2(key);
	if (item && !item->mark) {
		item->mark = true;
		m_marks.push(item);
	}
}

template<class Key, class Value, class Compare>
void Map<Key, Value, Compare>::mark(IteratorConst it) {
	const IteratorData& data = it.data();
	Item* item = data._item;
	if (item && !item->mark) {
		XX_ASSERT(data._host == this);
		item->mark = true;
		m_marks.push(item);
	}
}

template<class Key, class Value, class Compare>
void Map<Key, Value, Compare>::del_mark() {
	for (auto& i : m_marks) {
		i.value()->mark = false;
		del2(i.value());
	}
	m_marks.clear();
}

template<class Key, class Value, class Compare>
typename Map<Key, Value, Compare>::IteratorConst Map<Key, Value, Compare>::begin() const {
	IteratorData it(const_cast<Map*>(this), NULL); it.begen();
	return IteratorConst(it);
}

template<class Key, class Value, class Compare>
typename Map<Key, Value, Compare>::IteratorConst Map<Key, Value, Compare>::end() const {
	return IteratorConst(IteratorData(const_cast<Map*>(this), NULL));
}

template<class Key, class Value, class Compare>
typename Map<Key, Value, Compare>::Iterator Map<Key, Value, Compare>::begin() {
	IteratorData it(this, NULL); it.begen();
	return Iterator(it);
}

template<class Key, class Value, class Compare>
typename Map<Key, Value, Compare>::Iterator Map<Key, Value, Compare>::end() {
	return Iterator(IteratorData(this, NULL));
}

template<class Key, class Value, class Compare>
uint Map<Key, Value, Compare>::length() const {
	return m_length;
}

XX_END
