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

#ifndef __ftr__util__buffer__
#define __ftr__util__buffer__

#include <ftr/util/object.h>
#include <initializer_list>
#include <math.h>

namespace ftr {

	#ifndef FX_MIN_CAPACITY
	# define FX_MIN_CAPACITY (8)
	#endif

	enum class HolderMode {
		kWeak, kStrong
	};

	struct AllocatorDefault {
		static void* alloc(size_t size);
		static void* realloc(void* ptr, size_t size);
		static void  free(void* ptr);
	};

  template<
    typename T = char,
    HolderMode M = HolderMode::kStrong,
    typename A = AllocatorDefault
  > class ArrayBuffer;

  template<
    typename T = char,
    HolderMode M = HolderMode::kWeak,
    typename A = AllocatorDefault
  > class BasicString;

  typedef ArrayBuffer<char, HolderMode::kStrong> Buffer;
  typedef ArrayBuffer<char, HolderMode::kWeak>   WeakBuffer;

	/**
	* @class ArrayBuffer
	*/
	template<typename T, HolderMode M, typename A>
	class FX_EXPORT ArrayBuffer: public Object {
		public:
			typedef T Type;
      ArrayBuffer();
			ArrayBuffer(T* data, uint32_t length, uint32_t capacity = 0);
			ArrayBuffer(uint32_t length, uint32_t capacity = 0);
			template<HolderMode M2, typename A2>
			ArrayBuffer(const ArrayBuffer<T, M2, A2>& arr); // Only weak types can be copied
			ArrayBuffer(      ArrayBuffer& arr);
			ArrayBuffer(      ArrayBuffer&& arr);
			ArrayBuffer(const std::initializer_list<T>& list);

			virtual ~ArrayBuffer() { clear(); }
			
			template<HolderMode M2, typename A2>
			ArrayBuffer& operator=(const ArrayBuffer<T, M2, A2>& arr); // Only weak types can be copied assign value
			ArrayBuffer& operator=(      ArrayBuffer&);
			ArrayBuffer& operator=(      ArrayBuffer&&);

			T      & operator[](uint32_t index); // Only strong types have this method
			const T& operator[](uint32_t index) const;
      T      * operator*(); // Only strong types have this method
      const T* operator*() const { return _val; }
      const T* val()       const { return _val; }

			uint32_t push(T&& item);
			uint32_t push(const T& item);

			uint32_t pop(uint32_t count = 1);

			/**
			* @func concat() concat buffer
			*/
			template<HolderMode M2, typename A2>
			inline ArrayBuffer& concat(ArrayBuffer<T, M2, A2>&& arr) {
				return concat_(arr.val(), arr.length());
			}

			/**
			* @func write()
			* @arg src 
			* @arg to {int=-1} 当前数组开始写入的位置,-1从结尾开始写入
			* @arg size_src {int=-1} 需要写入项目数量,超过要写入数组的长度自动取写入数组长度,-1写入全部
			* @arg form_src {int=0} 从要写入src数组的form位置开始读取数据
			* @ret {uint32_t} 返回写入数据量
			*/
			template<HolderMode M2, typename A2>
			uint32_t write(const ArrayBuffer<T, M2, A2>& src, int to = -1, int size_src = -1, uint32_t form_src = 0);
			uint32_t write(const T* src, int to, uint32_t size_src);
			
			/**
			* @func slice() slice new ArrayBuffer
			*/
			inline ArrayBuffer<T, HolderMode::kWeak, A> slice(uint32_t start) const {
				return slice(start, _length);
			}

			ArrayBuffer<T, HolderMode::kWeak, A> slice(uint32_t start, uint32_t end) const;

			/**
			* @func collapse
			*/
			T* collapse();
			
			/**
			* @func copy()
			*/
			inline ArrayBuffer<T, HolderMode::kStrong, A> copy() const {
        return slice_(0, _length);
			}

			/**
			 * @func weak() return weak array buffer
			 */
			ArrayBuffer<T, HolderMode::kWeak, A> weak() const {
				return ArrayBuffer<T, HolderMode::kWeak, A>(*this);
			}
			
			/**
			* @func size 获取数据占用内存大小
			*/
			inline uint32_t size() const { return _length * sizeof(T); }

			/**
			* @func is_null() Is null data available?
			*/
			inline bool is_null() const { return _length == 0; }

			inline uint32_t length() const { return _length; }
			inline uint32_t capacity() const { return _capacity; }

			/**
			* @func clear() clear data
			*/
			void clear();
				
			/**
			* @func collapse_string
			*/
			BasicString<T, M, A> collapse_string();

			/**
			* @func realloc reset realloc length and return this ArrayBuffer&&
			*/
			ArrayBuffer&& realloc(uint32_t capacity) {
				// TODO ...
				return std::move(*this);
			}

			/**
			 * @func hash_code()
			 */
			uint64_t hash_code() const;

		protected:

			/**
			 * @func concat_() concat multiple array buffer
			 */
			ArrayBuffer& concat_(T* src, uint32_t src_length);

			/**
			 * @func slice_() slice strong array buffer
			 */
			ArrayBuffer<T, HolderMode::kStrong, A> slice_(uint32_t start, uint32_t end) const;
			
			/**
			* @func realloc auro realloc
			* @arg realloc_ {uint32_t}
			*/
			void realloc_(uint32_t capacity);

			struct Sham { T _item; }; // Used to call data destructors

			uint32_t  _length;
			uint32_t  _capacity;
			T*        _val;
		
			template<typename T2, HolderMode M2, typename A2> friend class ArrayBuffer;
			template<typename T2, HolderMode M2, typename A2> friend class BasicString;
	};

	#include "./buffer.inl"
}

#endif
