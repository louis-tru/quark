/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __quark__util__dict__
#define __quark__util__dict__

#include "./util.h"
#include "./iterator.h"
#include "./array.h"
#include "./error.h"
#include <initializer_list>

namespace qk {

	template<typename T> struct Compare {
		static uint64_t hashCode(const T& key) {
			return key.hashCode();
		}
	};

	template<typename T> struct Compare<T*> {
		typedef T* Type;
		static uint64_t hashCode(const Type& key) {
			return (uint64_t)key;
		}
	};

	template<> Qk_EXPORT uint64_t Compare<char>::hashCode(const char& key);
	template<> Qk_EXPORT uint64_t Compare<uint8_t>::hashCode(const uint8_t& key);
	template<> Qk_EXPORT uint64_t Compare<int16_t>::hashCode(const int16_t& key);
	template<> Qk_EXPORT uint64_t Compare<uint16_t>::hashCode(const uint16_t& key);
	template<> Qk_EXPORT uint64_t Compare<int32_t>::hashCode(const int32_t& key);
	template<> Qk_EXPORT uint64_t Compare<uint32_t>::hashCode(const uint32_t& key);
	template<> Qk_EXPORT uint64_t Compare<int64_t>::hashCode(const int64_t& key);
	template<> Qk_EXPORT uint64_t Compare<uint64_t>::hashCode(const uint64_t& key);
	template<> Qk_EXPORT uint64_t Compare<float>::hashCode(const float& key);
	template<> Qk_EXPORT uint64_t Compare<double>::hashCode(const double& key);
	template<> Qk_EXPORT uint64_t Compare<bool>::hashCode(const bool& key);

	/**
	 * @class Dict hash table
	 */
	template<
		typename Key, typename Value, 
		typename Compare = Compare<Key>, typename A = Allocator
	>
	class Dict: public Object {
	public:
		struct Pair {
			Key   key;
			Value value;
		};

		struct Node {
			typedef Dict::Pair Data;
			typedef const Data cData;
			inline Node* prev() const { return _prev; }
			inline Node* next() const { return _next; }
			inline Data& data() { return *reinterpret_cast<Data*>((&_conflict) + 1); }
			inline cData& data() const { return *reinterpret_cast<cData*>((&_conflict) + 1); }
			uint64_t hashCode;
			Node *_prev, *_next, *_conflict;
		};
		typedef ComplexIterator<Node, true>  IteratorConst;
		typedef ComplexIterator<Node, false> Iterator;

		Dict();
		Dict(Dict&& dict);
		Dict(const Dict& dict);
		Dict(std::initializer_list<Pair>&& list);

		~Dict();

		Dict&         operator=(const Dict& value);
		Dict&         operator=(Dict&& value);

		Value&        operator[](const Key& key);
		Value&        operator[](Key&& key);

		IteratorConst find(const Key& key) const;
		Iterator      find(const Key& key);

		bool          has(const Key& key) const;
		uint32_t      count(const Key& key) const;

		Array<Key>    keys() const;
		Array<Value>  values() const;

		bool          get(const Key& key, const Value* &out) const;
		bool          get(const Key& key, Value &out) const;
		bool          get(const Key& key, Value* &out);
		Value&        get(const Key& key);
		Value&        get(Key&& key);
		Value&        set(const Key& key, const Value& value);
		Value&        set(const Key& key, Value&& value);
		Value&        set(Key&& key, const Value& value);
		Value&        set(Key&& key, Value&& value);
		void          add(const Key& key) { get(key); }
		void          add(Key&& key) { get(std::move(key)); }

		Iterator      erase(IteratorConst it);
		void          erase(IteratorConst first, IteratorConst end);
		bool          erase(const Key& key);
		void          clear();

		uint32_t      length() const;

		IteratorConst begin() const;
		IteratorConst end() const;
		Iterator      begin();
		Iterator      end();

	private:
		void init_();
		void fill_(Node** indexed, Node* first, Node* last, uint32_t len, uint32_t capacity);
		bool make_(const Key& key, Pair** data);
		void erase_(Node* node);
		void optimize_();
		Node* link_(Node* prev, Node* next);
		Node* node_(IteratorConst it);
		const Node* find_(const Key& key, const Node* end = nullptr) const;

		Node**    _indexed;
		Node      _end; // { _prev = last, _next = first }
		uint32_t  _length, _capacity;
	};

	template<typename K, typename V, typename C = Compare<K>, typename A = Allocator>
	using cDict = const Dict<K, V, C, A>;

	template<typename K, typename C = Compare<K>, typename A = Allocator>
	using Set = Dict<K, bool, C, A>;

	template<typename K, typename C = Compare<K>, typename A = Allocator>
	using cSet = const Set<K, C, A>;

	// -----------------------------------------------------------------

	template<typename K, typename V, typename C, typename A>
	Dict<K, V, C, A>::Dict() {
		init_();
	}

	template<typename K, typename V, typename C, typename A>
	Dict<K, V, C, A>::Dict(Dict&& dict) {
		init_();
		operator=(std::move(dict));
	}

	template<typename K, typename V, typename C, typename A>
	Dict<K, V, C, A>::Dict(const Dict& dict) {
		init_();
		operator=(dict);
	}

	template<typename K, typename V, typename C, typename A>
	Dict<K, V, C, A>::Dict(std::initializer_list<Pair>&& list) {
		init_();
		if (list.size()) {
			for (auto& i: list)
				set(std::move(i.key), std::move(i.value));
		}
	}

	template<typename K, typename V, typename C, typename A>
	Dict<K, V, C, A>::~Dict() {
		clear();
	}

	template<typename K, typename V, typename C, typename A>
	Dict<K, V, C, A>& Dict<K, V, C, A>::operator=(const Dict& dict) {
		clear();
		if (dict._length) {
			for (auto& i: dict)
				set(i.key, i.value);
		}
		return *this;
	}

	template<typename K, typename V, typename C, typename A>
	Dict<K, V, C, A>& Dict<K, V, C, A>::operator=(Dict&& dict) {
		clear();
		if (dict._length) {
			fill_(dict._indexed, dict._end._next, dict._end._prev, dict._length, dict._capacity);
			dict.init_();
		}
		return *this;
	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::operator[](const K& key) {
		return get(key);
	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::operator[](K&& key) {
		return get(key);
	}

	template<typename K, typename V, typename C, typename A>
	const typename Dict<K, V, C, A>::Node* Dict<K, V, C, A>::find_(const K& key, const Node* end) const {
		if (_length) {
			auto hash = C::hashCode(key);
			auto node = _indexed[hash % _capacity];
			while (node) {
				if (node->hashCode == hash)
					return node;
				node = node->_conflict;
			}
		}
		return end;
	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::IteratorConst Dict<K, V, C, A>::find(const K& key) const {
		return IteratorConst(find_(key, &_end));
	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::Iterator Dict<K, V, C, A>::find(const K& key) {
		return Iterator(const_cast<Node*>(find_(key, &_end)));
	}

	template<typename K, typename V, typename C, typename A>
	bool Dict<K, V, C, A>::has(const K& key) const {
		return find_(key);
	}

	template<typename K, typename V, typename C, typename A>
	uint32_t Dict<K, V, C, A>::count(const K& key) const {
		return _length && _indexed[C::hashCode(key) % _capacity] ? 1/*TODO use 1*/: 0;
	}

	template<typename K, typename V, typename C, typename A>
	Array<K> Dict<K, V, C, A>::keys() const {
		Array<K> ls;
		for (auto& i: *this)
			ls.push(i.key);
		Qk_ReturnLocal(ls);
	}

	template<typename K, typename V, typename C, typename A>
	Array<V> Dict<K, V, C, A>::values() const {
		Array<V> ls;
		for (auto& i: *this)
			ls.push(i.value);
		Qk_ReturnLocal(ls);
	}

	template<typename K, typename V, typename C, typename A>
	bool Dict<K, V, C, A>::get(const K& k, const V* &out) const {
		auto node = find_(k);
		return node ? (out = &node->data().value, true): false;
	}

	template<typename K, typename V, typename C, typename A>
	bool Dict<K, V, C, A>::get(const K& k, V &out) const {
		auto node = find_(k);
		return node ? (out = node->data().value, true): false;
	}

	template<typename K, typename V, typename C, typename A>
	bool Dict<K, V, C, A>::get(const K& k, V* &out) {
		auto node = const_cast<Node*>(find_(k));
		return node ? (out = &node->data().value, true): false;
	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::get(const K& key) {
		Pair* pair;
		if (make_(key, &pair)) {
			new(&pair->key) K(key);
			new(&pair->value) V();
		}
		return pair->value;
	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::get(K&& key) {
		Pair* pair;
		if (make_(key, &pair)) {
			new(&pair->key) K(std::move(key));
			new(&pair->value) V();
		}
		return pair->value;
	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::set(const K& key, const V& value) {
		Pair* pair;
		if (make_(key, &pair)) {
			new(&pair->key) K(key); new(&pair->value) V(value);
		} else {
			pair->value = value;
		}
		return pair->value;
	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::set(const K& key, V&& value) {
		Pair* pair;
		if (make_(key, &pair)) {
			new(&pair->key) K(key);
			new(&pair->value) V(std::move(value));
		} else {
			pair->value = std::move(value);
		}
		return pair->value;
	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::set(K&& key, const V& value) {
		Pair* pair;
		if (make_(key, &pair)) {
			new(&pair->key) K(std::move(key)); new(&pair->value) V(value);
		} else {
			pair->value = value;
		}
		return pair->value;
	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::set(K&& key, V&& value) {
		Pair* pair;
		if (make_(key, &pair)) {
			new(&pair->key) K(std::move(key)); new(&pair->value) V(std::move(value));
		} else {
			pair->value = std::move(value);
		}
		return pair->value;
	}

	template<typename K, typename V, typename C, typename A>
	bool Dict<K, V, C, A>::erase(const K& key) {
		auto node = find_(key);
		return node ? (erase(IteratorConst(node)), true): false;
	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::Iterator Dict<K, V, C, A>::erase(IteratorConst it) {
		Qk_ASSERT(_length);
		auto node = node_(it);
		if (node != &_end) {
			auto next = link_(node->_prev, node->_next);
			erase_(node);
			_length--;
			// optimize_();
			return Iterator(next);
		} else {
			return Iterator(&_end);
		}
	}

	template<typename K, typename V, typename C, typename A>
	void Dict<K, V, C, A>::erase(IteratorConst f, IteratorConst e) {
		auto node = node_(f);
		auto end = node_(e);
		auto prev = node->_prev;
		while (node != end) {
			auto n = node->_next;
			erase_(node);
			node = n;
			_length--;
		}
		link_(prev, end);
		// optimize_();
	}

	template<typename K, typename V, typename C, typename A>
	void Dict<K, V, C, A>::clear() {
		erase(IteratorConst(_end._next), IteratorConst(&_end));
		A::free(_indexed);
		_indexed = nullptr;
		_length = 0;
		_capacity = 0;
	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::IteratorConst Dict<K, V, C, A>::begin() const {
		return IteratorConst(_end._next);
	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::IteratorConst Dict<K, V, C, A>::end() const {
		return IteratorConst(&_end);
	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::Iterator Dict<K, V, C, A>::begin() {
		return Iterator(_end._next);
	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::Iterator Dict<K, V, C, A>::end() {
		return Iterator(&_end);
	}

	template<typename K, typename V, typename C, typename A>
	uint32_t Dict<K, V, C, A>::length() const {
		return _length;
	}
	
	template<typename K, typename V, typename C, typename A>
	void Dict<K, V, C, A>::init_() {
		_end.hashCode = 0;
		_end._conflict = nullptr;
		fill_(nullptr, &_end, &_end, 0, 0);
	}

	template<typename K, typename V, typename C, typename A>
	void Dict<K, V, C, A>::fill_(Node** indexed, Node* first, Node* last, uint32_t len, uint32_t capacity) {
		_indexed = indexed;
		_end._prev = last;
		_end._next = first;
		first->_prev = &_end;
		last->_next = &_end;
		_length = len;
		_capacity = capacity;
	}

	template<typename K, typename V, typename C, typename A>
	bool Dict<K, V, C, A>::make_(const K& key, Pair** data) {
		if (!_capacity) {
			_capacity = 4; // init 4 length capacity
			_indexed = (Node**)A::alloc(sizeof(Node) * 4);
			::memset(_indexed, 0, sizeof(Node*) * 4);
		}
		auto hash = C::hashCode(key);
		auto index = hash % _capacity;
		auto node = _indexed[index];
		while (node) {
			if (node->hashCode == hash) {
				*data = &node->data();
				return false;
			}
			node = node->_conflict;
		}
		_length++;
		optimize_();
		index = hash % _capacity;
		// insert new key
		node = (Node*)A::alloc(sizeof(Node) + sizeof(Pair));
		node->hashCode = hash;
		node->_conflict = _indexed[index];
		link_(_end._prev, node);
		link_(node, &_end);
		_indexed[index] = node;
		*data = &node->data();
		return true;
	}

	template<typename K, typename V, typename C, typename A>
	void Dict<K, V, C, A>::erase_(Node* node) {
		auto index = node->hashCode % _capacity;
		auto begin = _indexed[index];
		if (begin == node) {
			_indexed[index] = node->_conflict;
		} else {
			while (begin->_conflict != node)
				begin = begin->_conflict;
			begin->_conflict = node->_conflict;
		}
		node->data().~Pair(); // destructor
		A::free(node);
	}

	template<typename K, typename V, typename C, typename A>
	void Dict<K, V, C, A>::optimize_() {
		if (_length > (_capacity >> 1)) {
			A::free(_indexed);
			_capacity <<= 1;
			_indexed = (Node**)A::alloc(sizeof(Node) * _capacity);
			::memset(_indexed, 0, sizeof(Node*) * _capacity);
			auto node = _end._next;
			while (node != &_end) {
				auto index = node->hashCode % _capacity;
				node->_conflict = _indexed[index];
				_indexed[index] = node;
				node = node->_next;
			}
		}
	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::Node* Dict<K, V, C, A>::link_(Node* prev, Node* next) {
		prev->_next = next;
		next->_prev = prev;
		return next;
	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::Node* Dict<K, V, C, A>::node_(IteratorConst it) {
		return const_cast<Node*>(it.ptr());
	}
	
}

#endif
