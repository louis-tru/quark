/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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

	// std::tuple / std::pair
	// template<typename... T> struct Tuple {};
	template<typename F, typename S> struct Pair {
		F first;
		S second;
	};

	/**
	 * @class Dict hash table
	 */
	template<
		typename Key,
		typename Value,
		typename Compare = Compare<Key>,
		typename B = NonObject
	>
	class Dict: public B {
	public:
		typedef qk::Pair<Key, Value> Pair;
		struct Node;
		struct BaseNode {
			uint64_t hashCode;
			Node *_prev, *_next, *_conflict;
		};
		struct Node: BaseNode {
			typedef Pair Data;
			typedef const Pair cPair;
			inline Node* prev() const { return this->_prev; }
			inline Node* next() const { return this->_next; }
			inline Pair& data() { return pair; }
			inline cPair& data() const { return pair; }
			Pair pair;
		};
		typedef ComplexIterator<Node, true>  IteratorConst;
		typedef ComplexIterator<Node, false> Iterator;

		Dict();
		Dict(Dict&& dict);
		Dict(const Dict& dict);
		Dict(std::initializer_list<Pair>&& list);
		Dict(Allocator* allocator);

		~Dict();

		Dict&         operator=(const Dict& value);
		Dict&         operator=(Dict&& value);

		Value&        operator[](const Key& key);
		Value&        operator[](Key&& key);

		Iterator      find(const Key& key);
		IteratorConst find(const Key& key) const;
		Iterator      findFor(uint64_t hashCode);
		IteratorConst findFor(uint64_t hashCode) const;

		bool          hasFor(uint64_t hashCode) const;
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
		bool          add(const Key& key);
		bool          add(Key&& key);

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
		void reset_();
		void fill_(Node** indexed, Node* first, Node* last, uint32_t len, uint32_t capacity);
		bool make_(const Key& key, Pair** data);
		void erase_(Node* node);
		void optimize_();
		inline Node* end_() { return static_cast<Node*>(&_end); }
		inline const Node* end_() const { return static_cast<const Node*>(&_end); }
		Node* link_(Node* prev, Node* next);
		Node* node_(IteratorConst it);
		const Node* findFor_(uint64_t hashCode, const Node* end) const;
		const Node* find_(const Key& key, const Node* end = nullptr) const;

		Allocator *_allocator;
		Node**    _indexed;
		uint32_t  _length, _capacity;
		BaseNode  _end; // { _prev = last, _next = first }
		// char _nullptr[sizeof(Pair)]; // End empty space
	};

	template<typename K, typename V, typename C = Compare<K>, typename B = NonObject>
	using cDict = const Dict<K, V, C, B>;

	template<typename K, typename C = Compare<K>, typename B = NonObject>
	using Set = Dict<K, bool, C, B>;

	template<typename K, typename C = Compare<K>, typename B = NonObject>
	using cSet = const Set<K, C, B>;

	// -----------------------------------------------------------------

	template<typename K, typename V, typename C, typename B>
	Dict<K, V, C, B>::Dict() {
		reset_();
	}

	template<typename K, typename V, typename C, typename B>
	Dict<K, V, C, B>::Dict(Dict&& dict) {
		reset_();
		operator=(std::move(dict));
	}

	template<typename K, typename V, typename C, typename B>
	Dict<K, V, C, B>::Dict(const Dict& dict) {
		reset_();
		operator=(dict);
	}

	template<typename K, typename V, typename C, typename B>
	Dict<K, V, C, B>::Dict(std::initializer_list<Pair>&& list) {
		reset_();
		if (list.size()) {
			for (auto& i: list)
				set(std::move(i.first), std::move(i.second));
		}
	}

	template<typename K, typename V, typename C, typename B>
	Dict<K, V, C, B>::Dict(Allocator* allocator) {
		reset_();
		_allocator = allocator;
	}

	template<typename K, typename V, typename C, typename B>
	Dict<K, V, C, B>::~Dict() {
		clear();
	}

	template<typename K, typename V, typename C, typename B>
	Dict<K, V, C, B>& Dict<K, V, C, B>::operator=(const Dict& dict) {
		clear();
		if (dict._length) {
			for (auto& i: dict)
				set(i.first, i.second);
		}
		return *this;
	}

	template<typename K, typename V, typename C, typename B>
	Dict<K, V, C, B>& Dict<K, V, C, B>::operator=(Dict&& dict) {
		clear();
		_allocator = dict._allocator; // take ownership of allocator
		if (dict._length) {
			fill_(dict._indexed, dict._end._next, dict._end._prev, dict._length, dict._capacity);
			dict.reset_();
			dict._allocator = _allocator;
		}
		return *this;
	}

	template<typename K, typename V, typename C, typename B>
	V& Dict<K, V, C, B>::operator[](const K& key) {
		return get(key);
	}

	template<typename K, typename V, typename C, typename B>
	V& Dict<K, V, C, B>::operator[](K&& key) {
		return get(key);
	}

	template<typename K, typename V, typename C, typename B>
	const typename Dict<K, V, C, B>::Node* Dict<K, V, C, B>::findFor_(uint64_t hash, const Node* end) const {
		auto node = _indexed[hash % _capacity];
		while (node) {
			if (node->hashCode == hash)
				return node;
			node = node->_conflict;
		}
		return end;
	}

	template<typename K, typename V, typename C, typename B>
	const typename Dict<K, V, C, B>::Node* Dict<K, V, C, B>::find_(const K& key, const Node* end) const {
		return _length ? findFor_(C::hashCode(key), end) : end;
	}

	template<typename K, typename V, typename C, typename B>
	bool Dict<K, V, C, B>::hasFor(uint64_t hashCode) const {
		return _length ? findFor_(hashCode, nullptr) != nullptr : false;
	}

	template<typename K, typename V, typename C, typename B>
	typename Dict<K, V, C, B>::Iterator Dict<K, V, C, B>::find(const K& key) {
		return Iterator(const_cast<Node*>(find_(key, end_())));
	}

	template<typename K, typename V, typename C, typename B>
	typename Dict<K, V, C, B>::IteratorConst Dict<K, V, C, B>::find(const K& key) const {
		return IteratorConst(find_(key, end_()));
	}

	template<typename K, typename V, typename C, typename B>
	typename Dict<K, V, C, B>::Iterator Dict<K, V, C, B>::findFor(uint64_t hash) {
		return Iterator(const_cast<Node*>(findFor_(hash, end_())));
	}

	template<typename K, typename V, typename C, typename B>
	typename Dict<K, V, C, B>::IteratorConst Dict<K, V, C, B>::findFor(uint64_t hash) const {
		return IteratorConst(findFor_(hash, end_()));
	}

	template<typename K, typename V, typename C, typename B>
	bool Dict<K, V, C, B>::has(const K& key) const {
		return find_(key);
	}

	template<typename K, typename V, typename C, typename B>
	uint32_t Dict<K, V, C, B>::count(const K& key) const {
		return _length && _indexed[C::hashCode(key) % _capacity] ? 1/*TODO use 1*/: 0;
	}

	template<typename K, typename V, typename C, typename B>
	Array<K> Dict<K, V, C, B>::keys() const {
		Array<K> ls;
		for (auto& i: *this)
			ls.push(i.first);
		Qk_ReturnLocal(ls);
	}

	template<typename K, typename V, typename C, typename B>
	Array<V> Dict<K, V, C, B>::values() const {
		Array<V> ls;
		for (auto& i: *this)
			ls.push(i.second);
		Qk_ReturnLocal(ls);
	}

	template<typename K, typename V, typename C, typename B>
	bool Dict<K, V, C, B>::get(const K& k, const V* &out) const {
		auto node = find_(k);
		return node ? (out = &node->data().second, true): false;
	}

	template<typename K, typename V, typename C, typename B>
	bool Dict<K, V, C, B>::get(const K& k, V &out) const {
		auto node = find_(k);
		return node ? (out = node->data().second, true): false;
	}

	template<typename K, typename V, typename C, typename B>
	bool Dict<K, V, C, B>::get(const K& k, V* &out) {
		auto node = const_cast<Node*>(find_(k));
		return node ? (out = &node->data().second, true): false;
	}

	template<typename K, typename V, typename C, typename B>
	V& Dict<K, V, C, B>::get(const K& key) {
		Pair* pair;
		if (make_(key, &pair)) {
			new(&pair->first) K(key);
			new(&pair->second) V();
		}
		return pair->second;
	}

	template<typename K, typename V, typename C, typename B>
	V& Dict<K, V, C, B>::get(K&& key) {
		Pair* pair;
		if (make_(key, &pair)) {
			new(&pair->first) K(std::move(key));
			new(&pair->second) V();
		}
		return pair->second;
	}

	template<typename K, typename V, typename C, typename B>
	bool Dict<K, V, C, B>::add(const K& key) {
		Pair* pair;
		auto isMake = make_(key, &pair);
		if (isMake) {
			new(&pair->first) K(key);
			new(&pair->second) V();
		}
		return isMake;
	}

	template<typename K, typename V, typename C, typename B>
	bool Dict<K, V, C, B>::add(K&& key) {
		Pair* pair;
		auto isMake = make_(key, &pair);
		if (isMake) {
			new(&pair->first) K(std::move(key));
			new(&pair->second) V();
		}
		return isMake;
	}

	template<typename K, typename V, typename C, typename B>
	V& Dict<K, V, C, B>::set(const K& key, const V& value) {
		Pair* pair;
		if (make_(key, &pair)) {
			new(&pair->first) K(key);
			new(&pair->second) V(value);
		} else {
			pair->second = value;
		}
		return pair->second;
	}

	template<typename K, typename V, typename C, typename B>
	V& Dict<K, V, C, B>::set(const K& key, V&& value) {
		Pair* pair;
		if (make_(key, &pair)) {
			new(&pair->first) K(key);
			new(&pair->second) V(std::move(value));
		} else {
			pair->second = std::move(value);
		}
		return pair->second;
	}

	template<typename K, typename V, typename C, typename B>
	V& Dict<K, V, C, B>::set(K&& key, const V& value) {
		Pair* pair;
		if (make_(key, &pair)) {
			new(&pair->first) K(std::move(key));
			new(&pair->second) V(value);
		} else {
			pair->second = value;
		}
		return pair->second;
	}

	template<typename K, typename V, typename C, typename B>
	V& Dict<K, V, C, B>::set(K&& key, V&& value) {
		Pair* pair;
		if (make_(key, &pair)) {
			new(&pair->first) K(std::move(key));
			new(&pair->second) V(std::move(value));
		} else {
			pair->second = std::move(value);
		}
		return pair->second;
	}

	template<typename K, typename V, typename C, typename B>
	bool Dict<K, V, C, B>::erase(const K& key) {
		auto node = find_(key);
		return node ? (erase(IteratorConst(node)), true): false;
	}

	template<typename K, typename V, typename C, typename B>
	typename Dict<K, V, C, B>::Iterator Dict<K, V, C, B>::erase(IteratorConst it) {
		Qk_ASSERT(_length);
		auto node = node_(it);
		if (node != &_end) {
			auto next = link_(node->_prev, node->_next);
			erase_(node);
			_length--;
			// optimize_();
			return Iterator(next);
		} else {
			return Iterator(end_());
		}
	}

	template<typename K, typename V, typename C, typename B>
	void Dict<K, V, C, B>::erase(IteratorConst f, IteratorConst e) {
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

	template<typename K, typename V, typename C, typename B>
	void Dict<K, V, C, B>::clear() {
		if (_length)
			erase(IteratorConst(_end._next), IteratorConst(end_()));
		if (_indexed)
			_allocator->free(_indexed);
		_indexed = nullptr;
		_length = 0;
		_capacity = 0;
	}

	template<typename K, typename V, typename C, typename B>
	typename Dict<K, V, C, B>::IteratorConst Dict<K, V, C, B>::begin() const {
		return IteratorConst(_end._next);
	}

	template<typename K, typename V, typename C, typename B>
	typename Dict<K, V, C, B>::IteratorConst Dict<K, V, C, B>::end() const {
		return IteratorConst(end_());
	}

	template<typename K, typename V, typename C, typename B>
	typename Dict<K, V, C, B>::Iterator Dict<K, V, C, B>::begin() {
		return Iterator(_end._next);
	}

	template<typename K, typename V, typename C, typename B>
	typename Dict<K, V, C, B>::Iterator Dict<K, V, C, B>::end() {
		return Iterator(end_());
	}

	template<typename K, typename V, typename C, typename B>
	uint32_t Dict<K, V, C, B>::length() const {
		return _length;
	}

	template<typename K, typename V, typename C, typename B>
	void Dict<K, V, C, B>::reset_() {
		_allocator = Allocator::current();
		_end.hashCode = 0;
		_end._conflict = nullptr;
		// memset(_nullptr, 0, sizeof(Pair));
		fill_(nullptr, end_(), end_(), 0, 0);
	}

	template<typename K, typename V, typename C, typename B>
	void Dict<K, V, C, B>::fill_(Node** indexed, Node* first, Node* last, uint32_t len, uint32_t capacity) {
		_indexed = indexed;
		_end._prev = last;
		_end._next = first;
		first->_prev = end_();
		last->_next = end_();
		_length = len;
		_capacity = capacity;
	}

	template<typename K, typename V, typename C, typename B>
	bool Dict<K, V, C, B>::make_(const K& key, Pair** data) {
		if (!_capacity) {
			_capacity = 4; // init 4 length capacity
			_indexed = _allocator->alloc<Node*>(4);
			::memset(_indexed, 0, sizeof(Node*) * 4);
		}
		auto hash = C::hashCode(key);
		auto index = hash % _capacity;
		auto node = _indexed[index];
		while (node) {
			if (node->hashCode == hash) {
				*data = &node->pair;
				return false;
			}
			node = node->_conflict;
		}
		_length++;
		optimize_();
		index = hash % _capacity;
		// insert new key
		node = _allocator->alloc<Node>(1);
		node->hashCode = hash;
		node->_conflict = _indexed[index];
		link_(_end._prev, node);
		link_(node, end_());
		_indexed[index] = node;
		*data = &node->pair;
		return true;
	}

	template<typename K, typename V, typename C, typename B>
	void Dict<K, V, C, B>::erase_(Node* node) {
		auto index = node->hashCode % _capacity;
		auto begin = _indexed[index];
		if (begin == node) {
			_indexed[index] = node->_conflict;
		} else {
			while (begin->_conflict != node)
				begin = begin->_conflict;
			begin->_conflict = node->_conflict;
		}
		node->pair.~Pair(); // destructor
		_allocator->free(node);
	}

	template<typename K, typename V, typename C, typename B>
	void Dict<K, V, C, B>::optimize_() {
		if (_length > (_capacity >> 1)) {
			_allocator->free(_indexed);
			_capacity <<= 1;
			_indexed = _allocator->alloc<Node*>(_capacity);
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

	template<typename K, typename V, typename C, typename B>
	typename Dict<K, V, C, B>::Node* Dict<K, V, C, B>::link_(Node* prev, Node* next) {
		prev->_next = next;
		next->_prev = prev;
		return next;
	}

	template<typename K, typename V, typename C, typename B>
	typename Dict<K, V, C, B>::Node* Dict<K, V, C, B>::node_(IteratorConst it) {
		return const_cast<Node*>(it.ptr());
	}
	
}

#endif
