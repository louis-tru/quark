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

#ifndef __quark__util__iterator__
#define __quark__util__iterator__

namespace quark {

	template <typename T, typename T2>
	class SimpleIterator {
		template<typename, typename> friend class SimpleIterator;
	public:
		//! Constant iterator type
		typedef SimpleIterator<const T2, T2> IteratorConst;
		//! Non-constant iterator type
		typedef SimpleIterator<T2, T2>       NonIteratorConst;
		//! Pointer to (const) GenericMember
		typedef   T*       Pointer;
		//! Reference to (const) GenericMember
		typedef   T&       Reference;

		SimpleIterator(): _ptr(nullptr) {}
		explicit SimpleIterator(Pointer ptr): _ptr(ptr) {}
		SimpleIterator(const NonIteratorConst& it): _ptr(it._ptr) {}

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

		bool operator==(const IteratorConst& that) const {
			return _ptr == that._ptr;
		}
		bool operator!=(const IteratorConst& that) const {
			return !(_ptr == that._ptr);
		}
		bool operator==(const NonIteratorConst& that) const {
			return _ptr == that._ptr;
		}
		bool operator!=(const NonIteratorConst& that) const {
			return !(_ptr == that._ptr);
		}

		Pointer   operator->() const { return _ptr; }
		Reference operator*() const { return *_ptr; }
		//
	protected:
		Pointer _ptr; //!< raw pointer
	};

	template <typename T, typename T2>
	class ComplexIterator {
		template<typename, typename> friend class ComplexIterator;
	public:
		//! Constant iterator type
		typedef ComplexIterator<const T2, T2> IteratorConst;
		//! Non-constant iterator type
		typedef ComplexIterator<      T2, T2> NonIteratorConst;
		//! Pointer to (const) GenericMember
		typedef typename T::Data* Pointer;
		//! Reference to (const) GenericMember
		typedef typename T::Data& Reference;

		ComplexIterator(): _ptr(nullptr) {}
		explicit ComplexIterator(T* ptr): _ptr(ptr) {}
		ComplexIterator(const NonIteratorConst& it): _ptr(it._ptr) {}
		
		T* ptr() const { return _ptr; }

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
		
		bool operator==(const IteratorConst& that) const {
			return _ptr == that._ptr;
		}
		bool operator!=(const IteratorConst& that) const {
			return !(_ptr == that._ptr);
		}
		bool operator==(const NonIteratorConst& that) const {
			return _ptr == that._ptr;
		}
		bool operator!=(const NonIteratorConst& that) const {
			return !(_ptr == that._ptr);
		}
		
		Pointer   operator->() const { return &((T2*)_ptr)->data(); }
		Reference operator*() const { return ((T2*)_ptr)->data(); }
		
	private:
		T* _ptr; //!< raw pointer
	};

}
#endif
