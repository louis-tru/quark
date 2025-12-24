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

#ifndef __quark__util__iterator__
#define __quark__util__iterator__

namespace qk {

	template<typename T, bool IsConst>
	struct IteratorType          { typedef T Type; };
	template<typename T>
	struct IteratorType<T, true> { typedef const T Type; };

	template <typename T, bool IsConst>
	class SimpleIterator {
		template<typename, bool> friend class SimpleIterator;
	public:
		//! Constant iterator type
		typedef SimpleIterator<T, true>  IteratorConst;
		//! Non-constant iterator type
		typedef SimpleIterator<T, false> Iterator;
		//! Pointer to (const) GenericMember
		typedef typename IteratorType<T, IsConst>::Type* Pointer;
		//! Reference to (const) GenericMember
		typedef typename IteratorType<T, IsConst>::Type& Reference;

		SimpleIterator(): _ptr(nullptr) {}
		explicit
		SimpleIterator(Pointer ptr): _ptr(ptr) {}
		SimpleIterator(const Iterator& it): _ptr(it._ptr) {}

		SimpleIterator& operator++() {    // ++i
			++_ptr; return *this;
		}
		SimpleIterator& operator--() {    // --i
			--_ptr; return *this;
		}
		SimpleIterator operator++(int) { // i++
			SimpleIterator old(*this); ++_ptr; return old;
		}
		SimpleIterator operator--(int) { // i--
			SimpleIterator old(*this); --_ptr; return old;
		}
		SimpleIterator prev() const {
			return SimpleIterator(_ptr-1);
		}
		SimpleIterator next() const {
			return SimpleIterator(_ptr+1);
		}
		bool operator==(const IteratorConst& that) const {
			return _ptr == that._ptr;
		}
		bool operator!=(const IteratorConst& that) const {
			return !(_ptr == that._ptr);
		}
		bool operator==(const Iterator& that) const {
			return _ptr == that._ptr;
		}
		bool operator!=(const Iterator& that) const {
			return !(_ptr == that._ptr);
		}
		Pointer   operator->() const { return _ptr; }
		Reference operator*() const { return *_ptr; }
		//
	private:
		Pointer _ptr; //!< raw pointer
	};

	template <typename T, bool IsConst>
	class ComplexIterator {
		template<typename, bool> friend class ComplexIterator;
	public:
		typedef typename IteratorType<T, IsConst>::Type Type;
		//! Constant iterator type
		typedef ComplexIterator<T, true> IteratorConst;
		//! Non-constant iterator type
		typedef ComplexIterator<T, false> Iterator;
		//! Pointer to (const) GenericMember
		typedef typename IteratorType<typename T::Data, IsConst>::Type* Pointer;
		//! Reference to (const) GenericMember
		typedef typename IteratorType<typename T::Data, IsConst>::Type& Reference;

		ComplexIterator(): _ptr(nullptr) {}
		explicit
		ComplexIterator(Type* ptr): _ptr(ptr) {}
		ComplexIterator(const Iterator& it): _ptr(it._ptr) {}

		Type* ptr() const { return _ptr; }

		ComplexIterator& operator++() {    // ++i
			_ptr = _ptr->next(); return *this;
		}
		ComplexIterator& operator--() {    // --i
			_ptr = _ptr->prev(); return *this;
		}
		ComplexIterator operator++(int) { // i++
			ComplexIterator old(*this); _ptr = _ptr->next(); return old;
		}
		ComplexIterator operator--(int) { // i--
			ComplexIterator old(*this); _ptr = _ptr->prev(); return old;
		}
		ComplexIterator prev() const {
			return ComplexIterator(_ptr->prev());
		}
		ComplexIterator next() const {
			return ComplexIterator(_ptr->next());
		}
		
		bool operator==(const IteratorConst& that) const {
			return _ptr == that._ptr;
		}
		bool operator!=(const IteratorConst& that) const {
			return !(_ptr == that._ptr);
		}
		bool operator==(const Iterator& that) const {
			return _ptr == that._ptr;
		}
		bool operator!=(const Iterator& that) const {
			return !(_ptr == that._ptr);
		}

		Pointer   operator->() const { return &_ptr->data(); }
		Reference operator*() const { return _ptr->data(); }

	private:
		Type* _ptr; //!< raw pointer
	};

}
#endif
