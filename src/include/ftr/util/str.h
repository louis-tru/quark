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
#include <vector>
#include <functional>

namespace ftr {

	#ifndef FX_MIN_CAPACITY
	# define FX_MIN_CAPACITY (8)
	#endif

	enum HolderMode {
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

	typedef ArrayBuffer<char,     HolderMode::kWeak>   String;
	typedef ArrayBuffer<uint16_t, HolderMode::kWeak>   String16;
	typedef ArrayBuffer<uint32_t, HolderMode::kWeak>   String32;
	typedef ArrayBuffer<char,     HolderMode::kStrong> SString;
	typedef ArrayBuffer<uint16_t, HolderMode::kStrong> SString16;
	typedef ArrayBuffer<uint32_t, HolderMode::kStrong> SString32;
	
	// buffer
	typedef ArrayBuffer<char, HolderMode::kStrong> Buffer;
	typedef ArrayBuffer<char, HolderMode::kWeak>   WeakBuffer;

	/**
	 * @class ArrayBuffer
	 */
	template<typename T, HolderMode M, typename A>
	class FX_EXPORT ArrayBuffer: public Object {
		public:
			typedef             T                          Type;
			typedef ArrayBuffer<T, HolderMode::kWeak,   A> Weak;
			typedef ArrayBuffer<T, HolderMode::kStrong, A> Strong;
			// constructors
			ArrayBuffer();
			ArrayBuffer(ArrayBuffer& arr); // right value copy constructors
			ArrayBuffer(ArrayBuffer&& arr); // right value copy constructors
			template<HolderMode M2, typename A2>
			ArrayBuffer(const ArrayBuffer<T, M2, A2>& arr); // Only weak types can be copied
			ArrayBuffer(const ArrayBuffer& arr); // Only weak types can be copied
			ArrayBuffer(const std::initializer_list<T>& list);
			// constructors
			ArrayBuffer(const T* s); // copy constructors
			ArrayBuffer(const T* s, uint32_t len); // copy constructors
			ArrayBuffer(const T* a, uint32_t a_len, const T* b, uint32_t b_len); // copy constructors
			ArrayBuffer(char i); // strong types can call
			ArrayBuffer(int32_t i); // strong types can call
			ArrayBuffer(int64_t i); // strong types can call
			ArrayBuffer(uint32_t i); // strong types can call
			ArrayBuffer(uint64_t i); // strong types can call
			ArrayBuffer(float f); // strong types can call
			ArrayBuffer(double f); // strong types can call

			/**
			 * @func from() greedy new ArrayBuffer from ...
			 */
			static inline ArrayBuffer from(T* data, uint32_t length, uint32_t capacity = 0) {
				return ArrayBuffer(length, capacity, data);
			}
			static inline ArrayBuffer from(const T* data, uint32_t length) {
				static_assert(M == HolderMode::kWeak, "from(), Only weak types can be copied");
				return ArrayBuffer(const_cast<T*>(data), length);
			}
			static inline ArrayBuffer from(uint32_t length, uint32_t capacity = 0) {
				return ArrayBuffer(length, capacity);
			}

			virtual ~ArrayBuffer() { clear(); }

			/**
			 * @func format string
			 */
			static SString format(const char* format, ...);
		
			/**
			* @func size 获取数据占用内存大小
			*/
			inline uint32_t size() const { return _length * sizeof(T); }

			/**
			* @func is_null() Is null data available?
			*/
			inline bool is_null() const { return _length == 0; }
			inline bool is_empty() const { return _length == 0; }

			inline uint32_t length() const { return _length; }
			inline uint32_t capacity() const { return _capacity; }

			// operator=
			ArrayBuffer& operator=(ArrayBuffer&);
			ArrayBuffer& operator=(ArrayBuffer&&);
			template<HolderMode M2, typename A2>
			ArrayBuffer& operator=(const ArrayBuffer<T, M2, A2>& arr); // Only weak types can be copied assign value
			ArrayBuffer& operator=(const ArrayBuffer&);
			// operator+
			Strong operator+(const Weak& s) const; // concat new
			template<HolderMode M2, typename A2>
			Strong operator+(const ArrayBuffer<T, M2, A2>& s) const; // concat new
			// operator+=
			ArrayBuffer& operator+=(const Weak& s); // write, Only strong types can be call
			template<HolderMode M2, typename A2>
			ArrayBuffer& operator+=(const ArrayBuffer<T, M2, A2>& s); // write, Only strong types can be call

			// operator compare
			bool operator==(const T* s) const;
			bool operator!=(const T* s) const;
			bool operator> (const T* s) const;
			bool operator< (const T* s) const;
			bool operator>=(const T* s) const;
			bool operator<=(const T* s) const;
			//
			template<HolderMode M2, typename A2> 
			inline bool operator==(const ArrayBuffer<T, M2, A2>& s) const { return operator==(s._val); }
			template<HolderMode M2, typename A2> 
			inline bool operator!=(const ArrayBuffer<T, M2, A2>& s) const { return operator!=(s._val); }
			template<HolderMode M2, typename A2> 
			inline bool operator> (const ArrayBuffer<T, M2, A2>& s) const { return operator>(s._val); }
			template<HolderMode M2, typename A2> 
			inline bool operator< (const ArrayBuffer<T, M2, A2>& s) const { return operator<(s._val); }
			template<HolderMode M2, typename A2> 
			inline bool operator>=(const ArrayBuffer<T, M2, A2>& s) const { return operator>=(s._val); }
			template<HolderMode M2, typename A2> 
			inline bool operator<=(const ArrayBuffer<T, M2, A2>& s) const { return operator<=(s._val); }
		
			// get ptr
			T& operator[](uint32_t index) { // Only strong types have this method
				static_assert(M == HolderMode::kStrong, "Only for strong types");
				ASSERT(index < _length, "ArrayBuffer access violation.");
				return _val[index];
			}
			const T& operator[](uint32_t index) const {
				ASSERT(index < _length, "ArrayBuffer access violation.");
				return _val[index];
			}
			inline T * operator*() { // Only strong types have this method
				static_assert(M == HolderMode::kStrong, "Only for strong types");
				return _val;
			}
			inline const T* operator*() const { return _val; }
			inline const T* val      () const { return _val; }
		
			// substr
			inline Weak substr(uint32_t start, uint32_t length) const { return slice(start, start + length); }
			inline Weak substring(uint32_t start, uint32_t end) const { return slice(start, end); }
			inline Weak substr(uint32_t start) const { return slice(start); }
			inline Weak substring(uint32_t start) const { return slice(start); }
			// split
			std::vector<Weak> split(const Weak& sp) const;
			// trim
			Weak trim() const;
			Weak trim_left() const;
			Weak trim_right() const;
			// upper, lower
			ArrayBuffer& upper_case(); // Only strong types can be call
			ArrayBuffer& lower_case(); // Only strong types can be call
			Strong       to_upper_case() const;
			Strong       to_lower_case() const;
			// index_of
			int index_of(const Weak& s, uint32_t start = 0) const;
			int last_index_of(const Weak& s, int start) const;
			int last_index_of(const Weak& s) const;
			// replace
			Strong replace(const Weak& s, const Weak& rep) const;
			Strong replace_all(const Weak& s, const Weak& rep) const;

			ArrayBuffer& push(T&& item);
			ArrayBuffer& push(const T& item);
			ArrayBuffer& push(const Weak& src);
			ArrayBuffer& pop(uint32_t count = 1);

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
			 * @func concat() concat buffer
			 */
			 template<HolderMode M2, typename A2>
			 inline ArrayBuffer& concat(ArrayBuffer<T, M2, A2>&& arr) {
				 return concat_(arr.val(), arr.length());
			 }

			/**
			* @func slice() slice new ArrayBuffer
			*/
			inline Weak slice(uint32_t start) const {
				return slice(start, _length);
			}

			Weak slice(uint32_t start, uint32_t end) const;

			/**
			* @func copy()
			*/
			inline Strong copy() const {
				return slice_(0, _length);
			}

			/**
			 * @func weak() return weak array buffer
			 */
			Weak weak() const {
				return Weak(*this);
			}
			
			/**
			* @func collapse, discard data ownership
			*/
			T* collapse();
			
			/**
			* @func clear() clear data
			*/
			void clear();
			
			/**
			* @func realloc reset realloc length and return this ArrayBuffer&&
			*/
			ArrayBuffer&& realloc(uint32_t capacity);

			/**
			 * @func hash_code()
			 */
			uint64_t hash_code() const;
			
			// to number
			template<typename T2> T2   to_number()        const;
			template<typename T2> bool to_number(T2* out) const;

		protected:
			// constructors
			ArrayBuffer(uint32_t length, uint32_t capacity, T* data); // greedy constructors
			ArrayBuffer(uint32_t length, uint32_t capacity); // new array buffer from length

			/**
			 * @func concat_() concat multiple array buffer
			 */
			ArrayBuffer& concat_(T* src, uint32_t src_length);

			/**
			 * @func slice_() slice strong array buffer
			 */
			Strong slice_(uint32_t start, uint32_t end) const;
			
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
	};

	#include "./str.inl"
}

namespace std {
	template<typename T, ftr::HolderMode M, typename A>
	struct hash<ftr::ArrayBuffer<T, M, A>> {
		size_t operator()(const ftr::ArrayBuffer<T, M, A>& val) const {
			return val.hash_code();
		}
	};
}

#endif
