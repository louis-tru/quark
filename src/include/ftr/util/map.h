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

#ifndef __ftr__util__map__
#define __ftr__util__map__

#include "ftr/util/util.h"
#include "ftr/util/container.h"
#include "ftr/util/iterator.h"
#include "ftr/util/array.h"
#include <initializer_list>

FX_NS(ftr)

/**
 * @ns
 */

/**
 * @class Compare
 */
template<class T> class FX_EXPORT Compare {
 public:
	static uint hash(const T& key) {
		return key.hash_code();
	}
	static bool equals(const T& a, const T& b, uint ha, uint hb) {
		return a.equals(b);
	}
};

template<class T> class FX_EXPORT PrtKey {
 public:
	inline PrtKey(T* p): _ptr(p) { }
	inline bool equals(const PrtKey& o) const { return o._ptr == _ptr; }
	inline uint hash_code() const { return size_t(_ptr) % Uint::max; }
 private:
	T* _ptr;
};

/**
 * @class Map Hash map
 * @template<class Key, class Value>
 */
template<class _Key, class _Value,
				class Compare = Compare<_Key>>
class FX_EXPORT Map: public Object {
 public:
	typedef _Key Key;
	typedef _Value Value;
 private:

	struct Item {
		Key   key;
		Value value;
		Item* prev, *next;
		uint  hash;
		bool  mark;
	};

	struct Node {
		Item* first, *last;
	};

	class NodeList: public Container<Node> {
	 public:
		void realloc(uint capacity);
		void auto_realloc();
		Map* _host;
	};

	struct IteratorData {
	 public:
		typedef _Value Value;
		const Key& key() const;
		const Value& value() const;
		Key& key();
		Value& value();
	 private:
		IteratorData();
		IteratorData(Map* host, Item* item);
		bool equals(const IteratorData& it) const;
		bool is_null() const;
		void begen();
		void prev();
		void next();
		Map* _host;
		Item* _item;
		friend class Map;
		friend class IteratorTemplateConst<IteratorData>;
		friend class IteratorTemplate<IteratorData>;
	};

	typedef Array<Item*> Marks;

	friend class NodeList;
	friend class iterator;

 public:
	struct Initializer {
		Key key;
		Value value;
	};
	typedef std::initializer_list<Initializer> InitializerList;
	typedef IteratorTemplateConst<IteratorData> IteratorConst;
	typedef IteratorTemplate<IteratorData> Iterator;

	Map();
	Map(const Map& map);
	Map(Map&& map);
	Map(const InitializerList& list);
	virtual ~Map();
	Map& operator=(const Map& value);
	Map& operator=(Map&& value);
	const Value& operator[](const Key& key) const;
	Value& operator[](const Key& key);
	Value& operator[](Key&& key);
	IteratorConst find(const Key& key) const;
	Iterator find(const Key& key);
	bool has(const Key& key) const;
	Array<Key> keys() const;
	Array<Value> values() const;
	inline const Value& get(const Key& key) const { return operator[](key); }
	inline Value& get(const Key& key) { return operator[](key); }
	inline Value& get(Key&& key) { return operator[](move(key)); }
	Value& set(const Key& key, const Value& value);
	Value& set(Key&& key, Value&& value);
	Value& set(Key&& key, const Value& value);
	Value& set(const Key& key, Value&& value);
	bool del(const Key& key);
	bool del(IteratorConst it);
	void clear();
	void mark(const Key& key);
	void mark(IteratorConst it);
	void del_mark();
	IteratorConst begin() const;
	IteratorConst end() const;
	Iterator begin();
	Iterator end();
	uint length() const;
	
 private:
	Item* find2(const Key& key);
	Item* find_set(const Key& key, bool* is_new);
	void  del2(Item* item);
	uint  m_length;
	NodeList m_nodes;
	Marks m_marks;
};

FX_END

#include "map.inl"

#endif
