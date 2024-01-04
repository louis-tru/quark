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

#ifndef __quark__util__array__
#define __quark__util__array__

#include "./object.h"
#include "./iterator.h"
#include <vector>

namespace qk {

	template<typename T = char, typename A = MemoryAllocator> class Array;
	template<typename T = char, typename A = MemoryAllocator> class ArrayBuffer; // array no copy
	template<typename T = char, typename A = MemoryAllocator> class ArrayWeak;
	template<typename T = char, typename A = MemoryAllocator> using cArray = const Array<T, A>;

	typedef       ArrayBuffer<char>     Buffer; // Array No Copy
	typedef       ArrayWeak<char>       WeakBuffer;
	typedef const ArrayBuffer<char>     cBuffer;
	typedef const ArrayWeak<char>       cWeakBuffer;

	/**
	 * @class Array
	 */
	template<typename T, typename A>
	class Array: public Object {
	public:
		typedef T Type;
		typedef A Alloc;
		// constructors
		Array();
		Array(Array&& arr); // right value copy constructors
		Array(const Array& arr);
		Array(const std::initializer_list<T>& list);
		Array(const std::vector<T>& list);
		Array(uint32_t length);

		typedef SimpleIterator<      T, T> Iterator;
		typedef SimpleIterator<const T, T> IteratorConst;

		inline Iterator begin() { return Iterator(_val); }
		inline Iterator end() { return Iterator(_val + _length); }
		inline IteratorConst begin() const { return IteratorConst(_val); }
		inline IteratorConst end() const { return IteratorConst(_val + _length); }

		virtual ~Array() { clear(); }

		/**
		 * @method size() Get the memory size occupied by the data
		 */
		inline uint32_t size() const { return _length * sizeof(T); }

		/**
		 * @method is_null() Is null data available?
		*/
		inline bool isNull() const { return _length == 0; }

		/**
		 * @method isWeak() is weak array buffer object
		 */
		inline bool isWeak() const { return _capacity < 0; }

		inline uint32_t length() const { return _length; }
		inline int32_t  capacity() const { return _capacity; }

		inline operator bool() const { return _length != 0; }
		// operator=
		Array& operator=(Array&& arr);
		Array& operator=(const Array& arr);

		// get ptr
		inline       T& operator[](uint32_t index) {
			Qk_ASSERT(index < _length, "Array access violation.");
			return _val[index];
		}
		inline const T& operator[](uint32_t index) const {
			Qk_ASSERT(index < _length, "Array access violation.");
			return _val[index];
		}
		inline       T& indexAt(uint32_t index) { return operator[](index); }
		inline const T& indexAt(uint32_t index) const { return operator[](index); }
		inline       T& lastIndexAt(uint32_t index) { return operator[](_length - 1 - index); }
		inline const T& lastIndexAt(uint32_t index) const { return operator[](_length - 1 - index); }
		inline       T* operator*()       { return _val; }
		inline const T* operator*() const { return _val; }
		inline       T* val      ()       { return _val; }
		inline const T* val      () const { return _val; }

		Array& push(T&& item);
		Array& push(const T& item);
		Array& pop (uint32_t count = 1);

		const T& front() const { return _val[0]; }
		const T& back()  const { return _val[_length - 1]; }
		T&       front() { return _val[0]; }
		T&       back()  { return _val[_length - 1]; }

		/**
		 * @method write() write data items
		 * @arg src  source data items
		 * @arg size_src {int=-1} The number of items that need to be written
		 * @arg to {int=-1} The position where the current array starts to be written, -1 starts to write from the end
		 * @return {uint32_t} Returns the amount of data written
		*/
		uint32_t write(const T* src, uint32_t size_src, int to = -1);

		/**
		 * @method concat() use right value move mode concat buffer
		 */
		template<typename A2>
		Array& concat(Array<T, A2>&& arr);

		/**
		 * @method slice() weak copy array buffer
		 */
		ArrayWeak<T, A> slice(uint32_t start = 0, uint32_t end = 0xFFFFFFFF) const;

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
		ArrayString<T, A> collapseString();

		/**
		 * @method to vector
		*/
		std::vector<T> vector() const;

		/**
		 * @method join() to string
		 */
		String join(cString& sp) const;
		
		/**
		 * @func toString() to_string
		 */
		virtual String toString() const;

		/**
		 * @method clear() clear data
		*/
		void clear();
		
		/**
		 * @method realloc reset realloc length
		*/
		void realloc(uint32_t capacity);

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

	protected:
		/** @constructors */
		Array(uint32_t length, int32_t capacity, T* data); // greedy constructors
		Array(uint32_t length, uint32_t capacity); // new array buffer from length

		/**
		 * @method concat_() concat multiple array buffer
		 */
		Array& concat_(T* src, uint32_t src_length);

		/**
		 * @method release realloc auro realloc
		 * @arg realloc_ {uint32_t}
		 */
		void realloc_(uint32_t capacity);

		/**
		 * @method copy data and output data pointer and capacity
		*/
		void copy_(T** val, int *capacity, uint32_t start, uint32_t len) const;

		struct Sham { T _item; }; // Used to call data destructors

		uint32_t  _length;
		int32_t   _capacity; // -1 means that it does not hold a pointer. This value is determined when it is constructed
		T*        _val;

		template<typename T2, typename A2> friend class Array;

		static void _Reverse(void *src, size_t size, uint32_t len);
	};

	/**
	 * @class ArrayBuffer array no copy
	 */
	template<typename T, typename A>
	class Qk_EXPORT ArrayBuffer: public Array<T, A> {
	public:
		inline ArrayBuffer() {}
		inline ArrayBuffer(Array<T, A>& arr): Array<T, A>(std::move(arr)) {}
		inline ArrayBuffer(ArrayBuffer<T, A>& arr): Array<T, A>(std::move(arr)) {}
		inline ArrayBuffer(ArrayBuffer<T, A>&& arr): Array<T, A>(std::move(arr)) {}

		ArrayBuffer(const ArrayBuffer& arr) = delete;
		ArrayBuffer& operator=(const ArrayBuffer& arr) = delete;

		/**
		 * @method from() greedy new Array from ...
		 */
		static inline ArrayBuffer from(T* data, uint32_t length, uint32_t capacity = 0) {
			return ArrayBuffer<T, A>(length, Qk_MAX(capacity, length), data);
		}
		static inline ArrayBuffer alloc(uint32_t length, uint32_t capacity = 0) {
			return ArrayBuffer<T, A>(length, capacity);
		}

		// operator=
		inline ArrayBuffer& operator=(ArrayBuffer<T, A>& arr) {
			Array<T, A>::operator=(std::move(arr)); return *this;
		}
		inline ArrayBuffer& operator=(ArrayBuffer<T, A>&& arr) {
			Array<T, A>::operator=(std::move(arr)); return *this;
		}

	protected:
		inline ArrayBuffer(uint32_t length, int32_t capacity, T* data)
			: Array<T, A>(length, capacity, data) {}
		inline ArrayBuffer(uint32_t length, uint32_t capacity)
			: Array<T, A>(length, capacity) {}

		template<typename T2, typename A2> friend class Array;
	};

	/**
	 * @class ArrayWeak
	 */
	template<typename T, typename A>
	class Qk_EXPORT ArrayWeak: public ArrayBuffer<T, A> {
	public:
		inline ArrayWeak()
			: ArrayBuffer<T, A>(0, -1, nullptr) {}
		inline ArrayWeak(const T* data, uint32_t length)
			: ArrayBuffer<T, A>(length, -1, const_cast<T*>(data)) {}
		inline ArrayWeak(const ArrayWeak& arr)
			: ArrayBuffer<T, A>(arr.length(), -1, const_cast<T*>(arr.val())) {}
		template<class A2>
		inline ArrayWeak(const Array<T, A2>& arr)
			: ArrayBuffer<T, A>(arr.length(), -1, const_cast<T*>(arr.val())) {}

		inline ArrayWeak& operator=(const ArrayWeak<T>& arr) {
			this->_length = arr._length;
			this->_val = arr._val;
			return *this;
		}
		template<class A2>
		inline ArrayWeak& operator=(const Array<T, A2>& arr) {
			this->_length = arr._length;
			this->_val = arr._val;
			return *this;
		}
	};

}

namespace qk {

	// ---------------------------------- IMPL ----------------------------------

	template<typename T, typename A>
	Array<T, A>::Array(): _length(0), _capacity(0), _val(nullptr)
	{}

	template<typename T, typename A>
	Array<T, A>::Array(Array&& arr): _length(0), _capacity(0), _val(nullptr)
	{
		if (arr._length)
			operator=(std::move(arr));
	}

	template<typename T, typename A>
	Array<T, A>::Array(const Array& arr): _length(arr._length), _capacity(0), _val(nullptr)
	{
		if (_length)
			arr.copy_(&_val, &_capacity, 0, _length);
	}
	
	template<typename T, typename A>
	Array<T, A>::Array(const std::initializer_list<T>& list)
		: _length(0), _capacity(0), _val(nullptr)
	{
		write(list.begin(), (uint32_t)list.size());
	}

	template<typename T, typename A>
	Array<T, A>::Array(const std::vector<T>& list)
		: _length(0), _capacity(0), _val(nullptr)
	{
		write(list.data(), (uint32_t)list.size());
	}

	template<typename T, typename A>
	Array<T, A>::Array(uint32_t length, int32_t capacity, T* data)
		: _length(length), _capacity(capacity), _val(data)
	{}

	template<typename T, typename A>
	Array<T, A>::Array(uint32_t length): Array(length, length)
	{}

	template<typename T, typename A>
	Array<T, A>::Array(uint32_t length, uint32_t capacity)
		: _length(0), _capacity(0), _val(nullptr)
	{
		extend(length);
		realloc_(Qk_MAX(length, capacity));
	}

	template<typename T, typename A>
	Array<T, A>& Array<T, A>::operator=(Array&& arr) {
		if ( arr._val != _val ) {
			if (isWeak()) {
				_length = arr._length;
				_val = arr._val;
			} else {
				clear();
				_capacity = arr._capacity;
				_length = arr._length;
				_val = arr._val;
				if (arr._capacity != -1) {
					arr._capacity = 0;
					arr._length = 0;
					arr._val = nullptr;
				}
			}
		}
		return *this;
	}
	
	template<typename T, typename A>
	Array<T, A>& Array<T, A>::operator=(const Array& arr) {
		if (arr._val != _val) {
			if (isWeak()) {
				_length = arr._length;
				_val = arr._val;
			} else {
				clear();
				if (arr._length) {
					_length = arr._length;
					arr.copy_(&_val, &_capacity, 0, _length);
				}
			}
		}
		return *this;
	}

	template<typename T, typename A>
	Array<T, A>& Array<T, A>::push(const T& item) {
		realloc_(++_length);
		new(_val + _length - 1) T(item);
		return *this;
	}

	template<typename T, typename A>
	Array<T, A>& Array<T, A>::push(T&& item) {
		realloc_(++_length);
		new(_val + _length - 1) T(std::move(item));
		return *this;
	}

	template<typename T, typename A>
	Array<T, A>& Array<T, A>::pop(uint32_t count) {
		int j = Qk_MAX(_length - count, 0);
		if (_length > j) {
			do {
				_length--;
				reinterpret_cast<Sham*>(_val + _length)->~Sham(); // release
			} while (_length > j);
			realloc_(_length);
		}
		return *this;
	}

	template<typename T, typename A>
	uint32_t Array<T, A>::write(const T* src, uint32_t size_src, int to) {
		if (size_src) {
			if ( to == -1 ) to = _length;
			uint32_t old_len = _length;
			uint32_t end = to + size_src;
			_length = Qk_MAX(end, _length);
			realloc_(_length);
			T* to_ = _val + to;
			
			for (int i = to; i < end; i++) {
				if (i < old_len) {
					reinterpret_cast<Sham*>(to_)->~Sham(); // release old object first
				}
				new(to_) T(*src);
				to_++; src++;
			}
		}
		return size_src;
	}

	template<typename T, typename A>
	Array<T, A>& Array<T, A>::concat_(T* src, uint32_t src_length) {
		if (src_length) {
			_length += src_length;
			realloc_(_length);
			T* end = _val + _length, *to = end - src_length;
			do {
				new(to) T(std::move(*src)); // call move constructor
				src++; to++;
			} while (to < end);
		}
		return *this;
	}

	template<typename T, typename A>
	template<typename A2>
	Array<T, A>& Array<T, A>::concat(Array<T, A2>&& arr) {
		concat_(*arr, arr.length());
		arr.clear();
		return *this;
	}

	template<typename T, typename A>
	ArrayWeak<T, A> Array<T, A>::slice(uint32_t start, uint32_t end) const {
		end = Qk_MIN(end, _length);
		if (start < end) {
			return ArrayWeak<T, A>(_val + start, end - start);
		} else {
			return ArrayWeak<T, A>();
		}
	}

	template<typename T, typename A>
	ArrayBuffer<T, A> Array<T, A>::copy(uint32_t start, uint32_t end) const {
		end = Qk_MIN(end, _length);
		if (start < end) {
			ArrayBuffer<T, A> arr;
			arr._length = end - start;
			copy_(&arr._val, &arr._capacity, start, arr._length);
			Qk_ReturnLocal(arr);
		}
		return ArrayBuffer<T, A>();
	}

	template<typename T, typename A>
	void Array<T, A>::copy_(T** val, int *capacity, uint32_t start, uint32_t len) const {
		A::aalloc((void**)val, len, (uint32_t*)capacity, sizeof(T));
		T* to = *val, *e = to + len;
		const T* src = _val + start;
		do {
			new(to) T(*src);
			to++; src++;
		} while(to < e);
	}

	template<typename T, typename A>
	T* Array<T, A>::collapse() {
		if (isWeak())
			return nullptr;
		T* r = _val;
		_capacity = 0;
		_length = 0;
		_val = nullptr;
		return r;
	}

	template<typename T, typename A>
	std::vector<T> Array<T, A>::vector() const {
		std::vector<T> r;
		for (auto& i: *this)
			r.push_back(i);
		Qk_ReturnLocal(r);
	}

	template<typename T, typename A>
	void Array<T, A>::clear() {
		if (_val) {
			if (!isWeak()) {
				T *i = _val, *end = i + _length;
				while (i < end)
					reinterpret_cast<Sham*>(i++)->~Sham(); // release
				A::free(_val); // free
				_capacity = 0;
			}
			_length = 0;
			_val = nullptr;
		}
	}

	template<typename T, typename A>
	void Array<T, A>::realloc(uint32_t capacity) {
		if (capacity < _length) { // clear Partial data
			T* i = _val + capacity;
			T* end = i + _length;
			while (i < end)
				reinterpret_cast<Sham*>(i++)->~Sham(); // release
			_length = capacity;
		}
		realloc_(capacity);
	}

	template<typename T, typename A>
	void Array<T, A>::extend(uint32_t length) {
		if (length > _length) {
			realloc_(length);
			new(_val + _length) T[length - _length];
			_length = length;
		}
	}

	template<typename T, typename A>
	Array<T, A>& Array<T, A>::reverse() {
		Array<char, MemoryAllocator>::_Reverse(_val, sizeof(T), _length);
		return *this;
	}

	template<typename T, typename A>
	void Array<T, A>::realloc_(uint32_t capacity) {
		Qk_STRICT_ASSERT(_capacity >= 0, "the weak holder cannot be changed");
		A::aalloc((void**)&_val, capacity, (uint32_t*)&_capacity, sizeof(T));
	}

	template<> Qk_EXPORT
	void Array<char, MemoryAllocator>::_Reverse(void *src, size_t size, uint32_t len);

	#define Qk_DEF_ARRAY_SPECIAL_(T, A) \
		template<> Qk_EXPORT void            Array<T, A>::extend(uint32_t length); \
		template<> Qk_EXPORT std::vector<T>  Array<T, A>::vector() const; \
		template<> Qk_EXPORT Array<T, A>&    Array<T, A>::concat_(T* src, uint32_t src_length); \
		template<> Qk_EXPORT uint32_t        Array<T, A>::write(const T* src, uint32_t size, int to); \
		template<> Qk_EXPORT Array<T, A>&    Array<T, A>::pop(uint32_t count); \
		template<> Qk_EXPORT void            Array<T, A>::clear(); \
		template<> Qk_EXPORT void            Array<T, A>::realloc(uint32_t capacity); \
		template<> Qk_EXPORT void            Array<T, A>::copy_(T** val, int *capacity, uint32_t start, uint32_t len) const \

	#define Qk_DEF_ARRAY_SPECIAL(T) \
		Qk_DEF_ARRAY_SPECIAL_(T, MemoryAllocator)

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

	//#undef Qk_DEF_ARRAY_SPECIAL
	//#undef Qk_DEF_ARRAY_SPECIAL_
}

#endif
