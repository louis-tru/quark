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

#ifndef __ftr__util__dict__
#define __ftr__util__dict__

#include "./util.h"
#include "./iterator.h"
#include "./array.h"
#include <initializer_list>

namespace ftr {

	template<typename T> struct Compare {
		static uint64_t hash_code(const T& key) {
			return key.hash_code();
		}
	};

	template<typename T> struct Compare<T*> {
		typedef T* Type;
		static uint64_t hash_code(const Type& key) {
			return (uint64_t)key;
		}
	};
	
	template<> FX_EXPORT uint64_t Compare<char>::hash_code(const char& key);
	template<> FX_EXPORT uint64_t Compare<uint8_t>::hash_code(const uint8_t& key);
	template<> FX_EXPORT uint64_t Compare<int16_t>::hash_code(const int16_t& key);
	template<> FX_EXPORT uint64_t Compare<uint16_t>::hash_code(const uint16_t& key);
	template<> FX_EXPORT uint64_t Compare<int>::hash_code(const int& key);
	template<> FX_EXPORT uint64_t Compare<uint32_t>::hash_code(const uint32_t& key);
	template<> FX_EXPORT uint64_t Compare<int64_t>::hash_code(const int64_t& key);
	template<> FX_EXPORT uint64_t Compare<uint64_t>::hash_code(const uint64_t& key);
	template<> FX_EXPORT uint64_t Compare<float>::hash_code(const float& key);
	template<> FX_EXPORT uint64_t Compare<double>::hash_code(const double& key);
	template<> FX_EXPORT uint64_t Compare<bool>::hash_code(const bool& key);

	/**
	 * @class Dict hash table
	 */
	template<
		typename Key, typename Value, 
		typename Compare = Compare<Key>, typename A = MemoryAllocator
	>
	class FX_EXPORT Dict: public Object {
		public:
		struct Data {
			Key   first;
			Value second;
		};
		struct Node {
			typedef Dict::Data Data;
			Node* prev() const { return _prev; }
			Node* next() const { return _next; }
			Data&       data() { return *reinterpret_cast<Data*>((&_conflict) + 1); }
			const Data& data() const { return *reinterpret_cast<const Data*>((&_conflict) + 1); }
			private:
			friend class Dict;
			uint64_t hash_code;
			Node *_prev, *_next, *_conflict;
		};
		typedef ComplexIterator<const Node, Node> IteratorConst;
		typedef ComplexIterator<      Node, Node> Iterator;

		Dict();
		Dict(Dict&& dict);
		Dict(const Dict& dict);
		Dict(std::initializer_list<Data>&& list);

		virtual ~Dict();

		Dict& operator=(const Dict& value);
		Dict& operator=(Dict&& value);

		const Value& operator[](const Key& key) const;
		Value&       operator[](const Key& key);
		Value&       operator[](Key&& key);

		IteratorConst find(const Key& key) const;
		Iterator find(const Key& key);

		uint32_t count(const Key& key) const;

		Array<Key>   keys() const;
		Array<Value> values() const;

		const Value& get(const Key& key) const;
		Value&       get(const Key& key);
		Value&       get(Key&& key);

		Value& set(const Key& key, const Value& value);
		Value& set(const Key& key, Value&& value);
		Value& set(Key&& key, const Value& value);
		Value& set(Key&& key, Value&& value);

		Iterator erase(const Key& key);
		Iterator erase(IteratorConst it);
		void erase(IteratorConst first, IteratorConst end);
		void clear();

		IteratorConst begin() const;
		IteratorConst end() const;
		Iterator begin();
		Iterator end();

		uint32_t length() const;

		private:

		void init_();
		void set_(Node** indexed, Node* first, Node* last, uint32_t len, uint32_t capacity);
		bool get_(const Key& key, Data** data);
		void erase_(Node* node);
		void optimize_();
		Node* link_(Node* prev, Node* next);
		Node* node_(IteratorConst it);

		Node** _indexed;
		Node   _end; // { _prev = last, _next = first }
		uint32_t  _length;
		uint32_t  _capacity;
	};

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
	Dict<K, V, C, A>::Dict(std::initializer_list<Data>&& list) {
		init_();
		if (list.size()) {
			for (auto i: list)
				set(std::move(i.first), std::move(i.second));
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
			for (auto i: dict)
				set(i.first, i.second);
		}
		return *this;
	}

	template<typename K, typename V, typename C, typename A>
	Dict<K, V, C, A>& Dict<K, V, C, A>::operator=(Dict&& dict) {
		clear();
		if (dict._length) {
			set_(dict._indexed, dict._end._next, dict._end._prev, dict._length, dict._capacity);
			dict.init_();
		}
		return *this;
	}

	template<typename K, typename V, typename C, typename A>
	const V& Dict<K, V, C, A>::operator[](const K& key) const {
		return get(key);
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
	typename Dict<K, V, C, A>::IteratorConst Dict<K, V, C, A>::find(const K& key) const {
		uint64_t hash = C::hash_code(key);
		uint32_t index = hash % _capacity;
		Node* node = _indexed[index];
		if (node) {
			while (node->_conflict != node) {
				if (node->hash_code == hash)
					break;
				node = node->_conflict;
			}
			return IteratorConst(node);
		} else {
			return IteratorConst(&_end);
		}
	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::Iterator Dict<K, V, C, A>::find(const K& key) {
		return Iterator(const_cast<Node*>(((const Dict*)this)->find(key).ptr()));
	}

	template<typename K, typename V, typename C, typename A>
	uint32_t Dict<K, V, C, A>::count(const K& key) const {
		return _indexed[C::hash_code(key) % _capacity] ? 1: 0;
	}

	template<typename K, typename V, typename C, typename A>
	Array<K> Dict<K, V, C, A>::keys() const {
		Array<K> ls;
		for (auto i: *this)
			ls.push(i.first);
		return std::move(ls);
	}

	template<typename K, typename V, typename C, typename A>
	Array<V> Dict<K, V, C, A>::values() const {
		Array<K> ls;
		for (auto i: *this)
			ls.push(i.second);
		return std::move(ls);
	}
	
	template<typename K, typename V, typename C, typename A>
	const V& Dict<K, V, C, A>::get(const K& key) const {
		auto it = find(key);
		FX_CHECK(it != IteratorConst(&_end), "Could not find dict key");
		return it->second;
	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::get(const K& key) {
		Data* data;
		if (get_(key, &data)) {
			new(&data->first) K(key);
			new(&data->second) V();
		}
		return data->second;
	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::get(K&& key) {
		Data* data;
		if (get_(key, &data)) {
			new(&data->first) K(std::move(key));
			new(&data->second) V();
		}
		return data->second;
	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::set(const K& key, const V& value) {
		return (get(key) = value);
	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::set(const K& key, V&& value) {
		return (get(key) = std::move(value));
	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::set(K&& key, const V& value) {
		return (get(std::move(key)) = value);
	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::set(K&& key, V&& value) {
		return (get(std::move(key)) = std::move(value));
	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::Iterator Dict<K, V, C, A>::erase(const K& key) {
		return erase(find(key));
	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::Iterator Dict<K, V, C, A>::erase(IteratorConst it) {
		ASSERT(_length);
		auto node = node_(it);
		if (node != &_end) {
			auto next = link_(node->_prev, node->_next);
			erase_(node);
			_length--;
			optimize_();
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
		optimize_();
	}

	template<typename K, typename V, typename C, typename A>
	void Dict<K, V, C, A>::clear() {
		erase(IteratorConst(_end._next), IteratorConst(&_end));
		A::free(_indexed);
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
		_end.hash_code = 0;
		_end._conflict = &_end;
		set_(nullptr, &_end, &_end, 0, 0);
	}

	template<typename K, typename V, typename C, typename A>
	void Dict<K, V, C, A>::set_(Node** indexed, Node* first, Node* last, uint32_t len, uint32_t capacity) {
		_indexed = indexed;
		_end._prev = last;
		_end._next = first;
		_length = len;
		_capacity = capacity;
	}
	
	template<typename K, typename V, typename C, typename A>
	bool Dict<K, V, C, A>::get_(const K& key, Data** data) {
		if (!_capacity) {
			_indexed = (Node**)A::aalloc(_indexed, 1, &_capacity, sizeof(Node*));
			::memset(_indexed, 0, sizeof(Node*) * _capacity);
		}
		uint64_t hash = C::hash_code(key);
		uint32_t index = hash % _capacity;
		auto result = false;
		auto node = _indexed[index];
		if (node) {
			while (node->_conflict != node) {
				if (node->hash_code == hash)
					break;
				node = node->_conflict;
			}
			// TODO ...
		} else { // insert new key
			node = (Node*)A::alloc(sizeof(Node) + sizeof(Data));
			node->hash_code = hash;
			node->_conflict = node;
			link_(_end._prev, node);
			link_(node, &end);
			_indexed[index] = node;
			_length++;
			result = true;
		}
		*data = &node->data();
		return result;
	}
	
	template<typename K, typename V, typename C, typename A>
	void Dict<K, V, C, A>::erase_(Node* node) {
		auto data = node->data();
		uint64_t hash = C::hash_code(data.first);
		uint32_t index = hash % _capacity;
		auto next = node->_conflict;
		if (node != next) { // conflict
			do
				next = next->_conflict;
			while(next->_conflict != node);
			next->_conflict = node->_conflict;
			_indexed[index] = next;
		} else {
			_indexed[index] = nullptr;
		}
		data.~Data(); // destructor
		A::free(node);
	}
	
	template<typename K, typename V, typename C, typename A>
	void Dict<K, V, C, A>::optimize_() {
		if (_capacity > FX_MIN_CAPACITY) {
			auto scale = float(_length) / float(_capacity);
			if (scale > 0.7 || scale < 0.2) {
				_indexed = (Node**)A::aalloc(_indexed, uint32_t(_length / 0.7) , &_capacity, sizeof(Node*));
				::memset(_indexed, 0, sizeof(Node*) * _capacity);
				auto node = _end._next, end = _end._prev;
				while (node != end) {
					node = node->_next;
					
				}
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
