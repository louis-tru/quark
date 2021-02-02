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

#ifndef __ftr__util__array__
#define __ftr__util__array__

#include "./object.h"
#include <initializer_list>

namespace ftr {

	#ifndef FX_MIN_CAPACITY
	# define FX_MIN_CAPACITY (8)
	#endif

	struct AllocatorDefault {
		static void* alloc(uint32_t size);
		static void  free(void* ptr);
		static void* realloc(void* ptr, uint32_t size, uint32_t* out_size, int size_of);
	};

	template<typename T = char, typename A = AllocatorDefault> class ArrayBuffer;
	template<typename T = char, typename A = AllocatorDefault> class WeakArrayBuffer;
	template<typename T = char, typename A = AllocatorDefault> class ArrayString;

	typedef       ArrayBuffer<char>     Buffer;
	typedef       WeakArrayBuffer<char> WeakBuffer;
	typedef const ArrayBuffer<char>     cBuffer;
	typedef const WeakArrayBuffer<char> cWeakBuffer;

	/**
	 * @class ArrayBuffer
	 */
	template<typename T, typename A>
	class FX_EXPORT ArrayBuffer: public Object {
		public:
			typedef                 T     Type;
			typedef ArrayBuffer    <T, A> Strong;
			typedef WeakArrayBuffer<T, A> Weak;
			// constructors
			ArrayBuffer();
			ArrayBuffer(ArrayBuffer& arr);  // right value copy constructors
			ArrayBuffer(ArrayBuffer&& arr); // right value copy constructors
			ArrayBuffer(const std::initializer_list<T>& list);

			/**
			 * @func from() greedy new ArrayBuffer from ...
			 */
			static inline ArrayBuffer from(T* data, uint32_t length, uint32_t capacity = 0) {
				return ArrayBuffer(length, FX_MAX(capacity, length), data);
			}
			static inline ArrayBuffer from(uint32_t length, uint32_t capacity = 0) {
				return ArrayBuffer(length, capacity);
			}

			virtual ~ArrayBuffer() { clear(); }

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
			inline bool is_weak() const { return _capacity >= 0; }

			inline uint32_t length() const { return _length; }
			inline int32_t  capacity() const { return _capacity; }

			// operator=
			ArrayBuffer& operator=(ArrayBuffer& arr);
			ArrayBuffer& operator=(ArrayBuffer&& arr);

			// get ptr
			inline       T& operator[](uint32_t index) {
				ASSERT(index < _length, "ArrayBuffer access violation.");
				return _val[index];
			}
			inline const T& operator[](uint32_t index) const {
				ASSERT(index < _length, "ArrayBuffer access violation.");
				return _val[index];
			}
			inline       T* operator*()       { return _val; }
			inline const T* operator*() const { return _val; }
			inline       T* val      ()       { return _val; }
			inline const T* val      () const { return _val; }

			ArrayBuffer& push(T&& item);
			ArrayBuffer& push(const T& item);
			ArrayBuffer& pop (uint32_t count = 1);

			/**
			* @func write()
			* @arg src 
			* @arg to {int=-1} 当前数组开始写入的位置,-1从结尾开始写入
			* @arg size_src {int=-1} 需要写入项目数量,超过要写入数组的长度自动取写入数组长度,-1写入全部
			* @arg form_src {int=0} 从要写入src数组的form位置开始读取数据
			* @ret {uint32_t} 返回写入数据量
			*/
			template<typename A2>
			uint32_t write(const ArrayBuffer<T, A2>& src, int to = -1, int size_src = -1, uint32_t form_src = 0);
			uint32_t write(const T* src, int to, uint32_t size_src);

			/**
			 * @func concat() use right value move mode concat buffer 
			 */
			template<typename A2>
			inline ArrayBuffer& concat(ArrayBuffer<T, A2>&& arr) {
				return concat_(*arr, arr.length());
			}

			/**
			 * @slice() weak copy array buffer
			 */
			Weak slice(uint32_t start = 0, uint32_t end = 0xFFFFFFFF) const;

			/**
			 * @func copy() strong copy array buffer
			 */
			Strong copy(uint32_t start = 0, uint32_t end = 0xFFFFFFFF) const;

			/**
			* @func collapse, discard data ownership
			*/
			T* collapse();

			/**
			* @func collapse string, discard data ownership
			*/
			ArrayString<T, A> collapse_string();

			/**
			* @func clear() clear data
			*/
			void clear();
			
			/**
			* @func realloc reset realloc length and return this ArrayBuffer&&
			*/
			ArrayBuffer&& realloc(uint32_t capacity);

		protected:
			// constructors
			ArrayBuffer(uint32_t length, uint32_t capacity, T* data); // greedy constructors
			ArrayBuffer(uint32_t length, uint32_t capacity); // new array buffer from length

			/**
			 * @func concat_() concat multiple array buffer
			 */
			ArrayBuffer& concat_(T* src, uint32_t src_length);

			/**
			* @func realloc auro realloc
			* @arg realloc_ {uint32_t}
			*/
			void realloc_(uint32_t capacity);

			struct Sham { T _item; }; // Used to call data destructors

			uint32_t  _length;
			int32_t   _capacity; // -1 means that it does not hold a pointer. This value is determined when it is constructed
			T*        _val;

			template<typename T2, typename A2> friend class ArrayBuffer;
	};

	/**
		* @class WeakArrayBuffer
		*/
	template<typename T, typename A>
	class FX_EXPORT WeakArrayBuffer: public ArrayBuffer<T, A> {
		public:
			WeakArrayBuffer(): ArrayBuffer<T, A>(0, -1, nullptr) {}
			WeakArrayBuffer(const T* data, uint32_t length)
				: ArrayBuffer<T, A>(length, -1, const_cast<T*>(data)) {}
			WeakArrayBuffer(const WeakArrayBuffer& arr)
				: ArrayBuffer<T, A>(arr._length, -1, const_cast<T*>(arr._val)) {}
			template<class A2>
			WeakArrayBuffer(const ArrayBuffer<T, A2>& arr)
				: ArrayBuffer<T, A>(arr._length, -1, const_cast<T*>(arr._val)) {}

			WeakArrayBuffer& operator=(const WeakArrayBuffer<T>& arr) {
				this->_length = arr._length;
				this->_val = arr._val;
				return *this;
			}
			template<class A2>
			WeakArrayBuffer& operator=(const ArrayBuffer<T, A2>& arr) {
				this->_length = arr._length;
				this->_val = arr._val;
				return *this;
			}
	};

}

namespace ftr {

	// -------------------------------------- IMPL --------------------------------------

	template<typename T, typename A>
	ArrayBuffer<T, A>::ArrayBuffer(): _length(0), _capacity(0), _val(nullptr) {
	}

	template<typename T, typename A>
	ArrayBuffer<T, A>::ArrayBuffer(ArrayBuffer& arr): ArrayBuffer(std::move(arr))
	{}

	template<typename T, typename A>
	ArrayBuffer<T, A>::ArrayBuffer(ArrayBuffer&& arr): _length(0), _capacity(0), _val(nullptr)
	{
		operator=(std::move(arr));
	}

	template<typename T, typename A>
	ArrayBuffer<T, A>::ArrayBuffer(const std::initializer_list<T>& list)
		: _length((uint32_t)list.size()), _capacity(0), _val(nullptr)
	{
		realloc_(_length);
		T* begin = _val;
		for (auto& i : list) {
			new(begin) T(std::move(i)); // 调用默认构造
			begin++;
		}
	}

	template<typename T, typename A>
	ArrayBuffer<T, A>::ArrayBuffer(uint32_t length, uint32_t capacity, T* data)
		: _length(length), _capacity(capacity), _val(data)
	{
	}

	template<typename T, typename A>
	ArrayBuffer<T, A>::ArrayBuffer(uint32_t length, uint32_t capacity)
		: _length(length), _capacity(0), _val(nullptr)
	{
		realloc_(FX_MAX(length, capacity));
		if (_length) {
			T* begin = _val;
			T* end = begin + _length;
			while (begin < end) {
				new(begin) T(); // 调用默认构造
				begin++;
			}
		}
	}

	template<typename T, typename A>
	ArrayBuffer<T, A>& ArrayBuffer<T, A>::operator=(ArrayBuffer& arr) {
		return operator=(std::move(arr));
	}

	template<typename T, typename A>
	ArrayBuffer<T, A>& ArrayBuffer<T, A>::operator=(ArrayBuffer&& arr) {
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
	ArrayBuffer<T, A>& ArrayBuffer<T, A>::push(const T& item) {
		_length++;
		realloc_(_length);
		new(_val + _length - 1) T(item);
		return *this;
	}

	template<typename T, typename A>
	ArrayBuffer<T, A>& ArrayBuffer<T, A>::push(T&& item) {
		_length++;
		realloc_(_length);
		new(_val + _length - 1) T(std::move(item));
		return *this;
	}

	template<typename T, typename A>
	ArrayBuffer<T, A>& ArrayBuffer<T, A>::pop(uint32_t count) {
		int j = FX_MAX(_length - count, 0);
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
	uint32_t ArrayBuffer<T, A>::write(
		const ArrayBuffer<T, A2>& arr, int to, int size_src, uint32_t form_src)
	{
		int s = FX_MIN(arr._length - form_src, size_src < 0 ? arr._length : size_src);
		if (s > 0) {
			return write(arr._val + form_src, to, s);
		}
		return 0;
	}

	/**
	* @func write
	*/
	template<typename T, typename A>
	uint32_t ArrayBuffer<T, A>::write(const T* src, int to, uint32_t size_src) {
		if (size_src) {
			if ( to == -1 ) to = _length;
			uint32_t old_len = _length;
			uint32_t end = to + size_src;
			_length = FX_MAX(end, _length);
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
	ArrayBuffer<T, A>& ArrayBuffer<T, A>::concat_(T* src, uint32_t src_length) {
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
	WeakArrayBuffer<T, A> ArrayBuffer<T, A>::slice(uint32_t start, uint32_t end) const {
		end = FX_MIN(end, _length);
		if (start < end) {
			return ArrayBuffer<T, A>(_val + start, end - start);
		} else {
			return ArrayBuffer<T, A>();
		}
	}

	template<typename T, typename A>
	ArrayBuffer<T, A> ArrayBuffer<T, A>::copy(uint32_t start, uint32_t end) const {
		end = FX_MIN(end, _length);
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
	T* ArrayBuffer<T, A>::collapse() {
		if (is_weak())
			return nullptr;
		T* r = _val;
		_capacity = 0;
		_length = 0;
		_val = nullptr;
		return r;
	}

	template<typename T, typename A>
	void ArrayBuffer<T, A>::clear() {
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
	ArrayBuffer<T, A>&& ArrayBuffer<T, A>::realloc(uint32_t capacity) {
		FX_ASSERT(!is_weak(), "the weak holder cannot be changed");
		if (capacity < _length) { // clear Partial data
			T* i = _val + capacity;
			T* end = i + _length;
			while (i < end)
				reinterpret_cast<Sham*>(i++)->~Sham(); // 释放
			_length = capacity;
		}
		realloc_(capacity);
		return std::move(*this);
	}

	template<typename T, typename A>
	void ArrayBuffer<T, A>::realloc_(uint32_t capacity) {
		FX_ASSERT(!is_weak(), "the weak holder cannot be changed");
		_val = (T*)A::realloc(_val, capacity, &_capacity, sizeof(T));
	}

	#define FX_DEF_ARRAY_SPECIAL(T, A) \
		template<>                       ArrayBuffer<T, A>::ArrayBuffer(uint32_t length, uint32_t capacity); \
		template<>                       ArrayBuffer<T, A>::ArrayBuffer(const std::initializer_list<T>& list); \
		template<> ArrayBuffer<T, A>&    ArrayBuffer<T, A>::concat_(T* src, uint32_t src_length); \
		template<> uint32_t              ArrayBuffer<T, A>::write(const T* src, int to, uint32_t size); \
		template<> ArrayBuffer<T, A>&    ArrayBuffer<T, A>::pop(uint32_t count); \
		template<> void                  ArrayBuffer<T, A>::clear(); \
		template<> ArrayBuffer<T, A>&&   ArrayBuffer<T, A>::realloc(uint32_t capacity); \
		template<> ArrayBuffer<T, A>     ArrayBuffer<T, A>::copy(uint32_t start, uint32_t end) const \

	#define FX_DEF_ARRAY_SPECIAL_ALL(T) \
		FX_DEF_ARRAY_SPECIAL(T, AllocatorDefault)

	FX_DEF_ARRAY_SPECIAL_ALL(char);
	FX_DEF_ARRAY_SPECIAL_ALL(unsigned char);
	FX_DEF_ARRAY_SPECIAL_ALL(int16_t);
	FX_DEF_ARRAY_SPECIAL_ALL(uint16_t );
	FX_DEF_ARRAY_SPECIAL_ALL(int32_t);
	FX_DEF_ARRAY_SPECIAL_ALL(uint32_t);
	FX_DEF_ARRAY_SPECIAL_ALL(int64_t);
	FX_DEF_ARRAY_SPECIAL_ALL(uint64_t);
	FX_DEF_ARRAY_SPECIAL_ALL(float);
	FX_DEF_ARRAY_SPECIAL_ALL(double);

	#undef FX_DEF_ARRAY_SPECIAL
	#undef FX_DEF_ARRAY_SPECIAL_ALL
}

#endif
