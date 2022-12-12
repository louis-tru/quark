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
#include <initializer_list>
#include <vector>

namespace quark {

	template<typename T = char, typename A = MemoryAllocator> class Array;
	template<typename T = char, typename A = MemoryAllocator> class ArrayBuffer;
	template<typename T = char, typename A = MemoryAllocator> class ArrayWeak;

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
		typedef T     Type;
		typedef A     Alloc;
		// constructors
		Array();
		Array(Array& arr);  // right value copy constructors
		Array(Array&& arr); // right value copy constructors
		Array(const std::initializer_list<T>& list);
		Array(const std::vector<T>& list);
		Array(const Array& arr);
		Array(uint32_t length);

		typedef SimpleIterator<T,       T> Iterator;
		typedef SimpleIterator<const T, T> IteratorConst;

		inline Iterator begin() { return Iterator(_val); }
		inline Iterator end() { return Iterator(_val + _length); }
		inline IteratorConst begin() const { return IteratorConst(_val); }
		inline IteratorConst end() const { return IteratorConst(_val + _length); }

		virtual ~Array() { clear(); }

		/**
		 * @func size 获取数据占用内存大小
		 */
		inline uint32_t size() const { return _length * sizeof(T); }

		/**
		* @func is_null() Is null data available?
		*/
		inline bool is_null() const { return _length == 0; }

		/**
			* @func is_weak() is weak array buffer object
			*/
		inline bool is_weak() const { return _capacity < 0; }

		inline uint32_t length() const { return _length; }
		inline int32_t  capacity() const { return _capacity; }

		inline operator bool() const { return _length != 0; }
		// operator=
		Array& operator=(Array& arr);
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
		inline       T* operator*()       { return _val; }
		inline const T* operator*() const { return _val; }
		inline       T* val      ()       { return _val; }
		inline const T* val      () const { return _val; }

		Array& push(T&& item);
		Array& push(const T& item);
		Array& pop (uint32_t count = 1);

		const T& front() const { return _val[0]; }
		const T& back() const { return _val[_length - 1]; }
		T&       front() { return _val[0]; }
		T&       back() { return _val[_length - 1]; }

		/**
		* @func write()
		* @arg src 
		* @arg to {int=-1} 当前数组开始写入的位置,-1从结尾开始写入
		* @arg size_src {int=-1} 需要写入项目数量,超过要写入数组的长度自动取写入数组长度,-1写入全部
		* @arg form_src {int=0} 从要写入src数组的form位置开始读取数据
		* @ret {uint32_t} 返回写入数据量
		*/
		template<typename A2>
		uint32_t write(const Array<T, A2>& src, int to = -1, int size_src = -1, uint32_t form_src = 0);
		uint32_t write(const T* src, int to, uint32_t size_src);

		/**
			* @func concat() use right value move mode concat buffer 
			*/
		template<typename A2>
		Array& concat(Array<T, A2>&& arr);

		/**
		 * @slice() weak copy array buffer
		 */
		ArrayWeak<T, A> slice(uint32_t start = 0, uint32_t end = 0xFFFFFFFF) const;

		/**
		 * @func copy() strong copy array buffer
		 */
		ArrayBuffer<T, A> copy(uint32_t start = 0, uint32_t end = 0xFFFFFFFF) const;

		/**
		* @func collapse, discard data ownership
		*/
		T* collapse();

		/**
		* @func collapse string, discard data ownership
		*/
		ArrayString<T, A> collapse_string();

		/**
		* @func to vector
		*/
		std::vector<T> vector() const;

		/**
			* @func join() to string
			*/
		String join(cString& sp) const;
		
		/**
		 * @func joto_stringin() to_string
		 */
		virtual String to_string() const;

		/**
		* @func clear() clear data
		*/
		void clear();
		
		/**
		* @func realloc reset realloc length
		*/
		void realloc(uint32_t capacity);

		/**
		 *
		 * Expand the length, call the default constructor, can only increase, can not reduce
		 *
		 * @func extend()
		 */
		void extend(uint32_t length, uint32_t capacity = 0);

		/**
		 *
		 * reverse array list
		 *
		 * @func reverse()
		 */
		Array& reverse();

		/**
		 * @func reduce()
		 * @line https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/reduce
		*/
		template<typename S, typename C = void>
		S reduce(void (*cb)(S& total, const T& item, uint32_t i, C* ctx), S initialValue, C* ctx = nullptr) const;

	protected:
		// constructors
		Array(uint32_t length, int32_t capacity, T* data); // greedy constructors
		Array(uint32_t length, uint32_t capacity); // new array buffer from length

		/**
			* @func concat_() concat multiple array buffer
			*/
		Array& concat_(T* src, uint32_t src_length);

		/**
		* @func realloc auro realloc
		* @arg realloc_ {uint32_t}
		*/
		void realloc_(uint32_t capacity);

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
		 * @func from() greedy new Array from ...
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

namespace quark {

	// ---------------------------------- IMPL ----------------------------------

	template<typename T, typename A>
	Array<T, A>::Array(): _length(0), _capacity(0), _val(nullptr)
	{}

	template<typename T, typename A>
	Array<T, A>::Array(Array& arr): Array(arr.copy())//Array(std::move(arr))
	{}

	template<typename T, typename A>
	Array<T, A>::Array(Array&& arr): _length(0), _capacity(0), _val(nullptr)
	{
		operator=(std::move(arr));
	}

	template<typename T, typename A>
	Array<T, A>::Array(const Array& arr): Array(arr.copy())
	{}
	
	template<typename T, typename A>
	Array<T, A>::Array(const std::initializer_list<T>& list)
		: _length(0), _capacity(0), _val(nullptr)
	{
		write(list.begin(), 0, (uint32_t)list.size());
	}

	template<typename T, typename A>
	Array<T, A>::Array(const std::vector<T>& list)
		: _length(0), _capacity(0), _val(nullptr)
	{
		write(list.data(), 0, (uint32_t)list.size());
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
		extend(length, capacity);
	}

	template<typename T, typename A>
	Array<T, A>& Array<T, A>::operator=(Array& arr) {
		return operator=(std::move(arr));
	}

	template<typename T, typename A>
	Array<T, A>& Array<T, A>::operator=(Array&& arr) {
		if ( arr._val != _val ) {
			clear();
			_length = arr._length;
			_val = arr._val;
			if (!is_weak()) {
				_capacity = arr._capacity;
				arr._length = 0;
				arr._capacity = 0;
				arr._val = nullptr;
			}
		}
		return *this;
	}
	
	template<typename T, typename A>
	Array<T, A>& Array<T, A>::operator=(const Array& arr) {
		return operator=(is_weak() ? arr.slice(): arr.copy());
	}

	template<typename T, typename A>
	Array<T, A>& Array<T, A>::push(const T& item) {
		_length++;
		realloc_(_length);
		new(_val + _length - 1) T(item);
		return *this;
	}

	template<typename T, typename A>
	Array<T, A>& Array<T, A>::push(T&& item) {
		_length++;
		realloc_(_length);
		new(_val + _length - 1) T(std::move(item));
		return *this;
	}

	template<typename T, typename A>
	Array<T, A>& Array<T, A>::pop(uint32_t count) {
		int j = Qk_MAX(_length - count, 0);
		if (_length > j) {
			do {
				_length--;
				reinterpret_cast<Sham*>(_val + _length)->~Sham(); // 释放
			} while (_length > j);
			realloc_(_length);
		}
		return *this;
	}

	template<typename T, typename A>
	template<typename A2>
	uint32_t Array<T, A>::write(
		const Array<T, A2>& arr, int to, int size_src, uint32_t form_src)
	{
		int s = Qk_MIN(arr._length - form_src, size_src < 0 ? arr._length : size_src);
		if (s > 0) {
			return write(arr._val + form_src, to, s);
		}
		return 0;
	}

	/**
	* @func write
	*/
	template<typename T, typename A>
	uint32_t Array<T, A>::write(const T* src, int to, uint32_t size_src) {
		if (size_src) {
			if ( to == -1 ) to = _length;
			uint32_t old_len = _length;
			uint32_t end = to + size_src;
			_length = Qk_MAX(end, _length);
			realloc_(_length);
			T* to_ = _val + to;
			
			for (int i = to; i < end; i++) {
				if (i < old_len) {
					reinterpret_cast<Sham*>(to_)->~Sham(); // 先释放原对像
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
			T* end = _val + _length;
			T* to = end - src_length;
			while (to < end) {
				new(to) T(std::move(*src)); // 调用移动构造
				src++; to++;
			}
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
			arr.realloc_(arr._length);
			T* to = arr._val;
			T* e = to + arr._length;
			const T* src = _val + start;
			while (to < e) {
				new(to) T(*src);
				to++; src++;
			}
			return std::move(arr);
		}
		return ArrayBuffer<T, A>();
	}

	template<typename T, typename A>
	T* Array<T, A>::collapse() {
		if (is_weak())
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
		return std::move(r);
	}

	template<typename T, typename A>
	void Array<T, A>::clear() {
		if (_val) {
			if (!is_weak()) {
				T* i = _val;
				T* end = i + _length;
				while (i < end)
					reinterpret_cast<Sham*>(i++)->~Sham(); // 释放
				A::free(_val); /* free */
				_capacity = 0;
			}
			_length = 0;
			_val = nullptr;
		}
	}

	template<typename T, typename A>
	void Array<T, A>::realloc(uint32_t capacity) {
		Qk_ASSERT(!is_weak(), "the weak holder cannot be changed");
		if (capacity < _length) { // clear Partial data
			T* i = _val + capacity;
			T* end = i + _length;
			while (i < end)
				reinterpret_cast<Sham*>(i++)->~Sham(); // 释放
			_length = capacity;
		}
		realloc_(capacity);
		// return std::move(*this);
	}

	template<typename T, typename A>
	void Array<T, A>::extend(uint32_t length, uint32_t capacity) {
		if (length > _length) {
			realloc_(Qk_MAX(length, capacity));
			T* begin = _val + _length;
			T* end = _val + length;
			while (begin < end) {
				new(begin) T; // 调用默认构造
				begin++;
			}
			_length = length;
		}
	}

	template<typename T, typename A>
	Array<T, A>& Array<T, A>::reverse() {
		Array<char, MemoryAllocator>::_Reverse(_val, sizeof(T), _length);
		return *this;
	}

	template<typename T, typename A>
	template<typename S, typename C>
	S Array<T, A>::reduce(void (*cb)(S& total, const T& item, uint32_t i, C* ctx), S initialValue, C* ctx) const {
		for (int i = 0; i < _length; i++)
			cb(initialValue, _val[i], i, ctx);
		return std::move(initialValue);
	}

	template<typename T, typename A>
	void Array<T, A>::realloc_(uint32_t capacity) {
		Qk_ASSERT(!is_weak(), "the weak holder cannot be changed");
		_val = (T*)A::aalloc(_val, capacity, (uint32_t*)&_capacity, sizeof(T));
	}

	template<> Qk_EXPORT
	void Array<char, MemoryAllocator>::_Reverse(void *src, size_t size, uint32_t len);

	#define Qk_DEF_ARRAY_SPECIAL_(T, A) \
		template<> Qk_EXPORT void              Array<T, A>::extend(uint32_t length, uint32_t capacity); \
		template<> Qk_EXPORT std::vector<T>    Array<T, A>::vector() const; \
		template<> Qk_EXPORT Array<T, A>&      Array<T, A>::concat_(T* src, uint32_t src_length); \
		template<> Qk_EXPORT uint32_t          Array<T, A>::write(const T* src, int to, uint32_t size); \
		template<> Qk_EXPORT Array<T, A>&      Array<T, A>::pop(uint32_t count); \
		template<> Qk_EXPORT void              Array<T, A>::clear(); \
		template<> Qk_EXPORT void              Array<T, A>::realloc(uint32_t capacity); \
		template<> Qk_EXPORT ArrayBuffer<T, A> Array<T, A>::copy(uint32_t start, uint32_t end) const \

	#define Qk_DEF_ARRAY_SPECIAL(T) \
		Qk_DEF_ARRAY_SPECIAL_(T, MemoryAllocator)

	Qk_DEF_ARRAY_SPECIAL(char);
	Qk_DEF_ARRAY_SPECIAL(unsigned char);
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
