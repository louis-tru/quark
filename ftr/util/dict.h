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
			Node* prev() const { return _next; }
			Node* next() const { return _next; }
			Data&       data() { return *reinterpret_cast<T*>((&_conflict) + 1); }
			const Data& data() const { return *reinterpret_cast<const T*>((&_conflict) + 1); }
			private:
			friend class Dict;
			Node *_next, *_conflict;
		};
		typedef ComplexIterator<const Node, Node> IteratorConst;
		typedef ComplexIterator<Node, Node>       Iterator;

		Dict();
		Dict(Dict&& map);
		Dict(const Dict& map);
		Dict(const std::initializer_list<Data>& list);

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

		void erase_(Node* node);
		Node* node_();

		Node** _indexed;
		Node   _end; // { _prev = last, _next = first }
		uint32_t  _length;
	};

	// -----------------------------------------------------------------

	template<typename K, typename V, typename C, typename A>
	Dict<K, V, C, A>::Dict() {
	}

	template<typename K, typename V, typename C, typename A>
	Dict<K, V, C, A>::Dict(Dict&& map) {
	}

	template<typename K, typename V, typename C, typename A>
	Dict<K, V, C, A>::Dict(const Dict& map) {
	}

	template<typename K, typename V, typename C, typename A>
	Dict<K, V, C, A>::Dict(const std::initializer_list<Data>& list) {
	}

	template<typename K, typename V, typename C, typename A>
	Dict<K, V, C, A>::~Dict() {
		clear();
	}

	template<typename K, typename V, typename C, typename A>
	Dict<K, V, C, A>& Dict<K, V, C, A>::operator=(const Dict& value) {

	}

	template<typename K, typename V, typename C, typename A>
	Dict<K, V, C, A>& Dict<K, V, C, A>::operator=(Dict&& value) {
		
	}

	template<typename K, typename V, typename C, typename A>
	const V& Dict<K, V, C, A>::operator[](const K& key) const {

	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::operator[](const K& key) {

	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::operator[](K&& key) {

	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::IteratorConst Dict<K, V, C, A>::find(const K& key) const {

	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::Iterator Dict<K, V, C, A>::find(const K& key) {

	}

	template<typename K, typename V, typename C, typename A>
	uint32_t Dict<K, V, C, A>::count(const K& key) const {

	}

	template<typename K, typename V, typename C, typename A>
	Array<K> Dict<K, V, C, A>::keys() const {

	}

	template<typename K, typename V, typename C, typename A>
	Array<V> Dict<K, V, C, A>::values() const {

	}
	
	template<typename K, typename V, typename C, typename A>
	const V& Dict<K, V, C, A>::get(const K& key) const {

	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::get(const K& key) {

	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::get(K&& key) {

	}
	
	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::set(const K& key, const V& value) {

	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::set(const K& key, V&& value) {

	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::set(K&& key, const V& value) {

	}

	template<typename K, typename V, typename C, typename A>
	V& Dict<K, V, C, A>::set(K&& key, V&& value) {
		//
	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::Iterator Dict<K, V, C, A>::erase(const K& key) {
		// 
	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::Iterator Dict<K, V, C, A>::erase(IteratorConst it) {
		ASSERT(_length);
		auto node = node_(it);
		auto next = link_(node->_prev, node->_next);
		erase_(node);
		_length--;
		return Iterator(next);
	}

	template<typename K, typename V, typename C, typename A>
	void Dict<K, V, C, A>::erase(IteratorConst f, IteratorConst e) {
		auto node = node_(f);
		auto end = node_(e);
		// auto prev = node->_prev;
		while (node != end) {
			auto n = node->_next;
			erase_(node);
			node = n;
			_length--;
		}
		// link_(prev, end);
	}

	template<typename K, typename V, typename C, typename A>
	void Dict<K, V, C, A>::clear() {
		erase(IteratorConst(_end._next), IteratorConst(&_end));
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
	void Dict<K, V, C, A>::erase_(Node* node) {
		reinterpret_cast<Data*>(node + 1)->~Data(); // destructor
		A::free(node);
	}

	template<typename K, typename V, typename C, typename A>
	typename Dict<K, V, C, A>::Node* Dict<K, V, C, A>::node_(IteratorConst it) {
		return const_cast<Node*>(it.ptr());
	}

}

#endif