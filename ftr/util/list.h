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

#ifndef __ftr__util__list__
#define __ftr__util__list__

#include "./object.h"
#include "./iterator.h"
#include <initializer_list>

namespace ftr {

	/**
	* @class List Double linked list
	*/
	template<typename T, typename A = MemoryAllocator> 
	class FX_EXPORT List: public Object {
		public:
		struct Node {
			typedef T     Data;
			Node* prev() const { return _prev; }
			Node* next() const { return _next; }
			T&       data() { return *reinterpret_cast<Data*>((&_next) + 1); }
			const T& data() const { return *reinterpret_cast<const Data*>((&_next) + 1); }
			private:
			friend class List;
			Node *_prev, *_next;
		};
		typedef ComplexIterator<const Node, Node> IteratorConst;
		typedef ComplexIterator<Node, Node>       Iterator;
		
		List();
		List(List&&);
		List(const List&);
		List(std::initializer_list<T>&& list);

		virtual ~List();

		List& operator=(const List&);
		List& operator=(List&&);

		Iterator push_back(const T& item);
		Iterator push_back(T&& item);
		Iterator push_front(const T& item);
		Iterator push_front(T&& item);

		void splice(IteratorConst it, List& ls);
		void splice(IteratorConst it, List& ls, IteratorConst first, IteratorConst end);

		void pop_back();
		void pop_front();
		
		Iterator insert(IteratorConst it, const T& item);
		Iterator insert(IteratorConst it, T&& item);

		Iterator erase(IteratorConst it);
		void erase(IteratorConst first, IteratorConst end);
		void clear();

		const T& front() const;
		const T& back() const;
		T&       front();
		T&       back();

		Iterator      indexed(IteratorConst it, int offset = 0);
		IteratorConst indexed(IteratorConst it, int offset = 0) const;

		String join(cString& sp) const;

		IteratorConst begin() const;
		IteratorConst end() const;
		Iterator      begin();
		Iterator      end();

		uint32_t length() const;
		
		private:

		void init_();
		void fill_(Node* first, Node* last, uint32_t len);
		void erase_(Node* node);
		Node* link_(Node* prev, Node* next);
		Node* node_(IteratorConst it);

		struct Sham { T _item; }; // Used to call data destructors

		Node _end; // { _prev = last, _next = first }
		uint32_t _length;
	};

	// -----------------------------------------------------------------

	template<typename T, typename A>
	List<T, A>::List() {
		init_();
	}

	template<typename T, typename A>
	List<T, A>::List(const List& list)
	{
		init_();
		for (auto i: list) {
			push_back(i);
		}
	}

	template<typename T, typename A>
	List<T, A>::List(List&& list)
	{
		if (list._length) {
			fill_(list._end._next, list._end._prev, list._length);
			list.init_();
		} else {
			init_();
		}
	}

	template<typename T, typename A>
	List<T, A>::List(std::initializer_list<T>&& list)
	{
		init_();
		for ( auto i : list ) {
			push_back(std::move(i));
		}
	}

	template<typename T, typename A>
	List<T, A>::~List() {
		clear();
	}

	template<typename T, typename A>
	List<T, A>& List<T, A>::operator=(const List& ls) { // copy
		clear();
		for (auto i: ls) {
			push_back(i);
		}
		return *this;
	}

	template<typename T, typename A>
	List<T, A>& List<T, A>::operator=(List&& ls) {
		clear();
		splice(IteratorConst(_end._prev), ls,
			IteratorConst(ls._end._next), IteratorConst(ls._end._prev));
		return *this;
	}

	template<typename T, typename A>
	typename List<T, A>::Iterator List<T, A>::push_back(const T& item) {
		return insert(IteratorConst(_end._prev), item);
	}

	template<typename T, typename A>
	typename List<T, A>::Iterator List<T, A>::push_back(T&& item) {
		return insert(IteratorConst(_end._prev), std::move(item));
	}

	template<typename T, typename A>
	typename List<T, A>::Iterator List<T, A>::push_front(const T& item) {
		return insert(IteratorConst(_end._next), item);
	}

	template<typename T, typename A>
	typename List<T, A>::Iterator List<T, A>::push_front(T&& item) {
		return insert(IteratorConst(_end._next), std::move(item));
	}

	template<typename T, typename A>
	void List<T, A>::splice(IteratorConst it, List& ls) {
		splice(it, ls,
			IteratorConst(ls._end._next), IteratorConst(ls._end._prev));
	}

	template<typename T, typename A>
	void List<T, A>::splice(IteratorConst it, List& ls, IteratorConst f, IteratorConst e) {
		if (f != e) {
			auto start = node_(f);
			auto end = node_(e);
			auto cur = node_(it);
			auto start_prev = start->_prev;

			link_(cur->_prev, start);
			while (start != end) {
				_length++;
				ls._length--;
				start = start->_next;
			}
			link_(end->_prev, cur);
			link_(start_prev, end); // link ls
		}
	}

	template<typename T, typename A>
	void List<T, A>::pop_back() {
		if (_length)
			erase(IteratorConst(_end._prev));
	}

	template<typename T, typename A>
	void List<T, A>::pop_front() {
		if (_length)
			erase(IteratorConst(_end._next));
	}
	
	template<typename T, typename A>
	typename List<T, A>::Iterator
	List<T, A>::insert(IteratorConst it, const T& item) {
		auto node = (Node*)A::alloc(sizeof(Node) + sizeof(T));
		new(node + 1) T(item);
		auto next = node_(it);
		link_(next->_prev, node);
		link_(node, next);
		_length++;
		return Iterator(node);
	}

	template<typename T, typename A>
	typename List<T, A>::Iterator
	List<T, A>::insert(IteratorConst it, T&& item) {
		auto node = (Node*)A::alloc(sizeof(Node) + sizeof(T));
		new(node + 1) T(std::move(item));
		auto next = node_(it);
		link_(next->_prev, node);
		link_(node, next);
		_length++;
		return Iterator(node);
	}

	template<typename T, typename A>
	typename List<T, A>::Iterator
	List<T, A>::erase(IteratorConst it) {
		ASSERT(_length);
		auto node = node_(it);
		if (node != &_end) {
			auto next = link_(node->_prev, node->_next);
			erase_(node);
			_length--;
			return Iterator(next);
		} else {
			return Iterator(&_end);
		}
	}

	template<typename T, typename A>
	void List<T, A>::erase(IteratorConst f, IteratorConst e) {
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
	}

	template<typename T, typename A>
	void List<T, A>::clear() {
		erase(IteratorConst(_end._next), IteratorConst(_end._prev));
	}

	template<typename T, typename A>
	const T& List<T, A>::front() const {
		return *IteratorConst(_end._next);
	}

	template<typename T, typename A>
	const T& List<T, A>::back() const {
		return *IteratorConst(_end._prev);
	}

	template<typename T, typename A>
	T& List<T, A>::front() {
		return *Iterator(_end._next);
	}

	template<typename T, typename A>
	T& List<T, A>::back() {
		return *Iterator(_end._prev);
	}

	template<typename T, typename A>
	typename List<T, A>::Iterator
	List<T, A>::indexed(IteratorConst it, int offset) {
		auto node = node_(it);
		if (offset > 0) {
			while (offset--) node = node->_next;
		} else if (offset < 0) {
			while (offset++) node = node->_prev;
		}
		return Iterator(node);
	}

	template<typename T, typename A>
	typename List<T, A>::IteratorConst
	List<T, A>::indexed(IteratorConst it, int offset) const {
		return const_cast<List*>(*this)->indexed(it, offset);
	}

	template<typename T, typename A>
	String List<T, A>::join(cString& sp) const {
		String str;
		return str;
	}

	template<typename T, typename A>
	typename List<T, A>::IteratorConst List<T, A>::begin() const {
		return IteratorConst(_end._next);
	}

	template<typename T, typename A>
	typename List<T, A>::IteratorConst List<T, A>::end() const {
		return IteratorConst(&_end);
	}

	template<typename T, typename A>
	typename List<T, A>::Iterator List<T, A>::begin() {
		return Iterator(_end._next);
	}

	template<typename T, typename A>
	typename List<T, A>::Iterator List<T, A>::end() {
		return Iterator(&_end);
	}

	template<typename T, typename A>
	uint32_t List<T, A>::length() const {
		return _length;
	}

	template<typename T, typename A>
	void List<T, A>::init_() {
		fill_(&_end, &_end, 0);
	}

	template<typename T, typename A>
	void List<T, A>::fill_(Node* first, Node* last, uint32_t len) {
		_end._prev = last;
		_end._next = first;
		_length = len;
	}

	template<typename T, typename A>
	void List<T, A>::erase_(Node* node) {
		reinterpret_cast<Sham*>(node + 1)->~Sham(); // destructor
		A::free(node);
	}

	template<typename T, typename A>
	typename List<T, A>::Node* List<T, A>::link_(Node* prev, Node* next) {
		prev->_next = next;
		next->_prev = prev;
		return next;
	}

	template<typename T, typename A>
	typename List<T, A>::Node* List<T, A>::node_(IteratorConst it) {
		return const_cast<Node*>(it.ptr());
	}

}

#endif
