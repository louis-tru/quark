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

#ifndef __quark__util__array__
#define __quark__util__array__

#include "./object.h"
#include "./iterator.h"
#include <string.h>
#include <vector>
#include <functional>

namespace qk {

	template<typename T = char, typename A = Allocator, typename B = Object> class Array;
	template<typename T = char, typename A = Allocator, typename B = Object> class ArrayBuffer; // array no copy
	template<typename T = char, typename A = Allocator, typename B = Object> using cArray = const Array<T, A, B>;
	template<typename T = char> class ArrayWeak;
	template<typename T = char> using cArrayWeak = const ArrayWeak<T>;

	typedef       ArrayBuffer<char>   Buffer; // Array No Copy
	typedef const ArrayBuffer<char>   cBuffer;
	typedef       ArrayWeak<char>     WeakBuffer;

	/**
	 * @class Array
	 */
	template<typename T, typename A, typename B>
	class Array: public B {
	public:
		typedef T Type;
		typedef const T cT;
		// constructors
		Array();
		Array(Array&& arr); // right value copy constructors
		Array(const Array& arr);
		Array(const std::initializer_list<T>& list);
		Array(const std::vector<T>& list);
		Array(uint32_t length);
		Array(const A* a); // with allocator

		typedef SimpleIterator<T, false> Iterator;
		typedef SimpleIterator<T, true>  IteratorConst;

		Iterator begin() { return Iterator(_ptr.val); }
		Iterator end() { return Iterator(_ptr.val + _ptr.extra); }
		IteratorConst begin() const { return IteratorConst(_ptr.val); }
		IteratorConst end() const { return IteratorConst(_ptr.val + _ptr.extra); }

		~Array() { clear(); }

		/**
		 * @method size() Get the memory size occupied by the data
		 */
		uint32_t size() const { return _ptr.extra * sizeof(T); }

		/**
		 * @method isNull() Is null data available?
		*/
		bool isNull() const { return _ptr.extra == 0; }

		uint32_t length() const { return _ptr.extra; }
		uint32_t capacity() const { return _ptr.capacity; }

		operator bool() const { return _ptr.extra != 0; }
		// operator=
		Array& operator=(Array&& arr);
		Array& operator=(const Array& arr);

		// get ptr
		T& operator[](uint32_t index) {
			Qk_ASSERT(index < _ptr.extra, "Array Index Overflow.");
			return _ptr.val[index];
		}
		cT& operator[](uint32_t index) const {
			Qk_ASSERT(index < _ptr.extra, "Array Index Overflow.");
			return _ptr.val[index];
		}
		T& at(uint32_t index) { return operator[](index); }
		cT& at(uint32_t index) const { return operator[](index); }
		T& lastAt(uint32_t index) { return operator[](_ptr.extra - index - 1); }
		cT& lastAt(uint32_t index) const { return operator[](_ptr.extra - index - 1); }
		T* operator*() { return _ptr.val; }
		cT* operator*() const { return _ptr.val; }
		T* val() { return _ptr.val; }
		cT* val() const { return _ptr.val; }

		T&   push(T&& item);
		T&   push(const T& item);
		void pop(uint32_t count = 1);

		T&  front() { return _ptr.val[0]; }
		T&  back()  { return _ptr.val[_ptr.extra - 1]; }
		cT& front() const { return _ptr.val[0]; }
		cT& back()  const { return _ptr.val[_ptr.extra - 1]; }

		/**
		 * @method write() write data items
		 * @param src  source data items
		 * @param size_src {int=-1} The number of items that need to be written
		 * @param to {int=-1} The position where the current array starts to be written, -1 starts to write from the end
		 * @return {uint32_t} Returns the amount of data written
		*/
		uint32_t write(const T* src, uint32_t size_src, int to = -1);

		/**
		 * @method concat() use right value move mode concat buffer
		 */
		template<typename A2>
		Array& concat(Array<T, A2>&& arr);

		/**
		 * @method concat()
		 */
		template<typename A2>
		Array& concat(const Array<T, A2>& arr) {
			write(arr.val(), arr.length());
			return *this;
		}

		/**
		 * @method slice() weak copy array buffer
		 */
		ArrayWeak<T> slice(uint32_t start = 0, uint32_t end = 0xFFFFFFFF) const;

		/**
		 * @method copy() strong copy array buffer
		 */
		ArrayBuffer<T, A> copy(uint32_t start = 0, uint32_t end = 0xFFFFFFFF) const;

		/**
		 * @method collapse, discard data ownership
		*/
		T* collapse();

		/**
		 * @method collapse string, discard data ownership
		*/
		StringImpl<T, A> collapseString();

		/**
		 * @method to vector
		*/
		std::vector<T> vector() const;

		/**
		 * @method join() to string
		 */
		String join(cString& sp) const;

		/**
		 * @method map()
		 */
		template <typename S>
		Array<S, A> map(const std::function<S(const T& t, uint32_t i)>&) const;

		/**
		 * @func toString() to_string
		 */
		String toString() const;

		/**
		 * @method clear() clear data
		*/
		void clear();

		/**
		 * @method reset realloc length
		*/
		void reset(uint32_t length);

		/**
		 *
		 * Expand the length, call the default constructor, can only increase, can not reduce
		 *
		 * @method extend()
		 */
		void extend(uint32_t length);

		/**
		 *
		 * reverse array list
		 *
		 * @method reverse()
		 */
		Array& reverse();

		/**
		 * @method allocator() get allocator
		 */
		inline const A* allocator() const { return _ptr.allocator; }

	protected:
		typedef typename A::template Ptr<T,A> Ptr;
		/**
		 * @constructors
		 */
		Array(uint32_t length, uint32_t capacity, T* data); // greedy constructors
		Array(uint32_t length, uint32_t capacity); // new array buffer from length
		Array(Ptr ptr);

		/**
		 * @method concat_() concat multiple array buffer and release src
		 */
		void concat_(T* src, uint32_t src_length);

		struct Sham { T _item; }; // Used to call data destructors

		/**
		 * @method copy data and output data pointer and capacity
		*/
		void copy_(Ptr* dest, uint32_t start, uint32_t len) const;

		Ptr _ptr; // allocator / data pointer / length and capacity

		template<typename T2, typename A2, typename B2> friend class Array;
		template<typename T2, typename A2> friend class StringImpl;
		friend class _Str;

		static void _Reverse(void *src, size_t size, uint32_t len);
	};

	/**
	 * @class ArrayBuffer array no copy
	 */
	template<typename T, typename A, typename B>
	class ArrayBuffer: public Array<T, A, B> {
	public:
		ArrayBuffer() {}
		ArrayBuffer(Array<T, A, B>& arr): Array<T, A, B>(std::move(arr)) {}
		ArrayBuffer(ArrayBuffer<T, A>& arr): Array<T, A, B>(std::move(arr)) {}
		ArrayBuffer(ArrayBuffer<T, A>&& arr): Array<T, A, B>(std::move(arr)) {}

		ArrayBuffer(T* data, uint32_t length, uint32_t capacity = 0)
			: Array<T, A, B>(length, Qk_Max(capacity, length), data) {}
		ArrayBuffer(uint32_t length, uint32_t capacity = 0)
			: Array<T, A, B>(length, capacity) {}
		ArrayBuffer(typename A::template Ptr<T,A> ptr)
			: Array<T, A, B>(ptr) {}

		// Disable copy and assign value function
		ArrayBuffer(const ArrayBuffer& arr) = delete;
		ArrayBuffer& operator=(const ArrayBuffer& arr) = delete;

		/**
		 * @method from() greedy new Array from ...
		 */
		static ArrayBuffer from(T* data, uint32_t length, uint32_t capacity = 0) {
			return ArrayBuffer<T, A>(data, length, capacity);
		}
		static ArrayBuffer alloc(uint32_t length, uint32_t capacity = 0) {
			return ArrayBuffer<T, A>(length, capacity);
		}

		// operator=
		ArrayBuffer& operator=(ArrayBuffer<T, A>& arr) {
			Array<T, A, B>::operator=(std::move(arr)); return *this;
		}
		ArrayBuffer& operator=(ArrayBuffer<T, A>&& arr) {
			Array<T, A, B>::operator=(std::move(arr)); return *this;
		}
	};

	/**
	 * @class ArrayWeak
	 */
	template<typename T>
	class ArrayWeak {
	public:
		ArrayWeak() {}
		ArrayWeak(const T* data, uint32_t length)
			: _ptr(Allocator::shared(),const_cast<T*>(data),length,length) {}
		template<class A2>
		ArrayWeak(cArray<T, A2>& arr)
			: _ptr(Allocator::shared(),const_cast<T*>(*arr),arr.length(),arr.length()) {}

		operator bool() const { return _ptr.val != nullptr; }
		const T* operator*() const { return _ptr.val; }
		const ArrayBuffer<T>* operator->() const {
			return reinterpret_cast<const ArrayBuffer<T>*>(this);
		}
		ArrayBuffer<T> copy(uint32_t start = 0, uint32_t end = 0xFFFFFFFF) const {
			Qk_ReturnLocal(operator->()->copy(start, end));
		}
		const ArrayBuffer<T>& buffer() const { return *(operator->()); }
		uint32_t length() const { return _ptr.extra; }
		const T* val() const { return _ptr.val; }

	private:
		char __[sizeof(Object)];
		Allocator::Ptr<T> _ptr;
	};

}

namespace qk {

	// ---------------------------------- IMPL ----------------------------------

	template<typename T, typename A, typename B>
	Array<T, A, B>::Array(): _ptr()
	{}

	template<typename T, typename A, typename B>
	Array<T, A, B>::Array(Array&& arr): _ptr()
	{
		if (arr._ptr.extra)
			operator=(std::move(arr));
	}

	template<typename T, typename A, typename B>
	Array<T, A, B>::Array(const Array& arr): _ptr{arr._ptr.allocator,0,0,arr._ptr.extra}
	{
		if (_ptr.extra)
			arr.copy_(&_ptr, 0, _ptr.extra);
	}

	template<typename T, typename A, typename B>
	Array<T, A, B>::Array(const std::initializer_list<T>& list)
		: _ptr()
	{
		write(list.begin(), (uint32_t)list.size());
	}

	template<typename T, typename A, typename B>
	Array<T, A, B>::Array(const std::vector<T>& list)
		: _ptr()
	{
		write(list.data(), (uint32_t)list.size());
	}

	template<typename T, typename A, typename B>
	Array<T, A, B>::Array(uint32_t length, uint32_t capacity, T* data)
		: _ptr(A::shared(),data,capacity,length)
	{}

	template<typename T, typename A, typename B>
	Array<T, A, B>::Array(uint32_t length): Array(length, length)
	{}

	template<typename T, typename A, typename B>
	Array<T, A, B>::Array(uint32_t length, uint32_t capacity)
		: _ptr()
	{
		extend(length);
		_ptr.extend(Qk_Max(length, capacity));
	}

	template<typename T, typename A, typename B>
	Array<T, A, B>::Array(Ptr ptr)
		: _ptr(ptr)
	{
	}

	template<typename T, typename A, typename B>
	Array<T, A, B>::Array(const A* a): _ptr(a,0,0,0) {
	}

	template<typename T, typename A, typename B>
	Array<T, A, B>& Array<T, A, B>::operator=(Array&& arr) {
		if ( arr._ptr.val != _ptr.val ) {
			clear();
			_ptr = arr._ptr;
			arr._ptr.extra = 0;
			arr._ptr.capacity = 0;
			arr._ptr.val = nullptr;
		}
		return *this;
	}
	
	template<typename T, typename A, typename B>
	Array<T, A, B>& Array<T, A, B>::operator=(const Array& arr) {
		if (arr._ptr.val != _ptr.val) {
			clear();
			if (arr._ptr.extra) {
				_ptr.extra = arr._ptr.extra;
				arr.copy_(&_ptr, 0, _ptr.extra);
			}
		}
		return *this;
	}

	template<typename T, typename A, typename B>
	T& Array<T, A, B>::push(const T& item) {
		_ptr.extend(++_ptr.extra);
		if (IsPointer<T>::value) {
			_ptr.val[_ptr.extra-1] = item;
			return _ptr.val[_ptr.extra-1];
		} else {
			return *new(_ptr.val + _ptr.extra-1) T(item);
		}
	}

	template<typename T, typename A, typename B>
	T& Array<T, A, B>::push(T&& item) {
		_ptr.extend(++_ptr.extra);
		if (IsPointer<T>::value) {
			return (_ptr.val[_ptr.extra - 1] = std::move(item));
		} else {
			return *new(_ptr.val + _ptr.extra - 1) T(std::move(item));
		}
	}

	template<typename T, typename A, typename B>
	void Array<T, A, B>::pop(uint32_t count) {
		uint32_t newLen = Qk_Max(int(_ptr.extra - count), 0);
		if (_ptr.extra > newLen) {
			if (IsPointer<T>::value) {
				_ptr.extra = newLen;
			} else {
				do {
					_ptr.extra--;
					reinterpret_cast<Sham*>(_ptr.val + _ptr.extra)->~Sham(); // release
				} while (_ptr.extra > newLen);
			}
			_ptr.shrink(_ptr.extra);
		}
	}

	template<typename T, typename A, typename B>
	uint32_t Array<T, A, B>::write(const T* src, uint32_t size_src, int to) {
		if (size_src) {
			if ( to == -1 ) to = _ptr.extra;
			uint32_t end = to + size_src;
			uint32_t old_len = _ptr.extra;
			_ptr.extra = Qk_Max(end, _ptr.extra);
			_ptr.extend(_ptr.extra);

			if (IsPointer<T>::value) {
				memcpy((void*)(_ptr.val + to), src, size_src * sizeof(T) );
			} else {
				T* to_ = _ptr.val + to;
				for (uint32_t i = to; i < end; i++) {
					if (i < old_len) {
						reinterpret_cast<Sham*>(to_)->~Sham(); // release old object first
					}
					new(to_) T(*src);
					to_++; src++;
				}
			}
		}
		return size_src;
	}

	template<typename T, typename A, typename B>
	void Array<T, A, B>::concat_(T* src, uint32_t src_length) {
		if (src_length) {
			_ptr.extra += src_length;
			_ptr.extend(_ptr.extra);
			if (IsPointer<T>::value) {
				T* src = _ptr.val;
				T* to = _ptr.val + _ptr.extra - src_length;
				memcpy((void*)to, src, src_length * sizeof(T));
			} else {
				T* end = _ptr.val + _ptr.extra, *to = end - src_length;
				do {
					new(to) T(std::move(*src)); // call move constructor
					reinterpret_cast<Sham*>(src)->~Sham();
					src++; to++;
				} while (to < end);
			}
		}
	}

	template<typename T, typename A, typename B>
	template<typename A2>
	Array<T, A, B>& Array<T, A, B>::concat(Array<T, A2>&& arr) {
		concat_(*arr, arr.length());
		arr._ptr.extra = 0;
		return *this;
	}

	template<typename T, typename A, typename B>
	ArrayWeak<T> Array<T, A, B>::slice(uint32_t start, uint32_t end) const {
		end = Qk_Min(end, _ptr.extra);
		if (start < end) {
			return ArrayWeak<T>(_ptr.val + start, end - start);
		} else {
			return ArrayWeak<T>();
		}
	}

	template<typename T, typename A, typename B>
	ArrayBuffer<T, A> Array<T, A, B>::copy(uint32_t start, uint32_t end) const {
		end = Qk_Min(end, _ptr.extra);
		if (start < end) {
			ArrayBuffer<T, A> arr;
			arr._ptr.extra = end - start;
			copy_(&arr._ptr, start, arr._ptr.extra);
			Qk_ReturnLocal(arr);
		}
		return ArrayBuffer<T, A>();
	}

	template<typename T, typename A, typename B>
	void Array<T, A, B>::copy_(Ptr* dest, uint32_t start, uint32_t len) const {
		dest->resize(len);
		if (IsPointer<T>::value) {
			memcpy(dest->val, _ptr.val + start, len * sizeof(T));
		} else {
			T* to = dest->val, *e = to + len;
			const T* src = _ptr.val + start;
			do {
				new(to) T(*src);
				to++; src++;
			} while(to < e);
		}
	}

	template<typename T, typename A, typename B>
	T* Array<T, A, B>::collapse() {
		T* r = _ptr.val;
		_ptr.extra = 0;
		_ptr.capacity = 0;
		_ptr.val = nullptr;
		return r;
	}

	template<typename T, typename A, typename B>
	std::vector<T> Array<T, A, B>::vector() const {
		if (IsPointer<T>::value) {
			std::vector<T> r(_ptr.extra);
			if (_ptr.extra)
				memcpy(r.data(), _ptr.val, sizeof(T) * _ptr.extra);
		} else {
			std::vector<T> r;
			for (auto& i: *this)
				r.push_back(i);
			Qk_ReturnLocal(r);
		}
	}

	template<typename T, typename A, typename B>
	void Array<T, A, B>::clear() {
		if (_ptr.val) {
			if (!IsPointer<T>::value) {
				T *i = _ptr.val, *end = i + _ptr.extra;
				while (i < end)
					reinterpret_cast<Sham*>(i++)->~Sham(); // release
			}
			_ptr.free(); // free
			_ptr.extra = 0;
		}
	}

	template<typename T, typename A, typename B>
	void Array<T, A, B>::reset(uint32_t length) {
		if (length < _ptr.extra) { // clear Partial data
			if (!IsPointer<T>::value) {
				T* i = _ptr.val + length;
				T* end = _ptr.val + _ptr.extra;
				while (i < end)
					reinterpret_cast<Sham*>(i++)->~Sham(); // release
			}
			_ptr.extra = length;
		} else {
			extend(length);
		}
	}

	template<typename T, typename A, typename B>
	void Array<T, A, B>::extend(uint32_t length) {
		if (length > _ptr.extra) {
			_ptr.extend(length);
			if (!IsPointer<T>::value)
				new(_ptr.val + _ptr.extra) T[length - _ptr.extra]; // call default constructor
			_ptr.extra = length;
		}
	}

	template<typename T, typename A, typename B>
	template<typename S>
	Array<S, A> Array<T, A, B>::map(const std::function<S(const T& t, uint32_t i)> &cb) const {
		Array<S, A> arr;
		if (_ptr.extra) {
			arr._ptr.extend(_ptr.extra);
			arr._ptr.extra = _ptr.extra;
			for (uint32_t i = 0; i < _ptr.extra; i++) {
				new(arr._ptr.val + i) S(cb(_ptr.val[i], i));
			}
		}
		Qk_ReturnLocal(arr);
	}

	template<typename T, typename A, typename B>
	Array<T, A, B>& Array<T, A, B>::reverse() {
		Array<char, Allocator>::_Reverse(_ptr.val, sizeof(T), _ptr.extra);
		return *this;
	}

	template<> Qk_EXPORT
	void Array<char, Allocator, Object>::_Reverse(void *src, size_t size, uint32_t len);

	#define Qk_DEF_ARRAY_SPECIAL_(T, A, B) \
		template<> Qk_EXPORT void            Array<T, A, B>::reset(uint32_t length); \
		template<> Qk_EXPORT void            Array<T, A, B>::extend(uint32_t length); \
		template<> Qk_EXPORT std::vector<T>  Array<T, A, B>::vector() const; \
		template<> Qk_EXPORT void            Array<T, A, B>::concat_(T* src, uint32_t src_length); \
		template<> Qk_EXPORT uint32_t        Array<T, A, B>::write(const T* src, uint32_t size, int to); \
		template<> Qk_EXPORT T&              Array<T, A, B>::push(T&& item); \
		template<> Qk_EXPORT T&              Array<T, A, B>::push(const T& item); \
		template<> Qk_EXPORT void            Array<T, A, B>::pop(uint32_t count); \
		template<> Qk_EXPORT void            Array<T, A, B>::clear(); \
		template<> Qk_EXPORT void            Array<T, A, B>::copy_(Ptr* dest, uint32_t start, uint32_t len) const;

	#define Qk_DEF_ARRAY_SPECIAL(T) \
		Qk_DEF_ARRAY_SPECIAL_(T, Allocator, Object)

	Qk_DEF_ARRAY_SPECIAL(char);
	Qk_DEF_ARRAY_SPECIAL(uint8_t);
	Qk_DEF_ARRAY_SPECIAL(int16_t);
	Qk_DEF_ARRAY_SPECIAL(uint16_t);
	Qk_DEF_ARRAY_SPECIAL(int32_t);
	Qk_DEF_ARRAY_SPECIAL(uint32_t);
	Qk_DEF_ARRAY_SPECIAL(int64_t);
	Qk_DEF_ARRAY_SPECIAL(uint64_t);
	Qk_DEF_ARRAY_SPECIAL(float);
	Qk_DEF_ARRAY_SPECIAL(double);
}

#endif
