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

#include <ftr/util/util.h>
#include <ftr/util/container.h>
#include <ftr/util/error.h>
#include <initializer_list>
#include <new>

namespace ftr {

	/**
	* @class ArrayBuffer
	*/
	template<class T, class Container = Container<T>> class FX_EXPORT ArrayBuffer: public Object {
		private:

		ArrayBuffer(const ArrayBuffer&) = delete;
		ArrayBuffer& operator=(const ArrayBuffer&) = delete;

		public:
		static ArrayBuffer readonly(const ArrayBuffer& arr);
		static ArrayBuffer readonly(const T* arr = nullptr, uint32_t length = 0);

		ArrayBuffer(const std::initializer_list<T>& list);
		ArrayBuffer(ArrayBuffer& arr);
		ArrayBuffer(ArrayBuffer&& arr);
		ArrayBuffer(T* data, uint32_t length, uint32_t capacity = 0);
		ArrayBuffer(uint32_t length = 0, uint32_t capacity = 0);

		virtual ~ArrayBuffer();
		
		ArrayBuffer& operator=(ArrayBuffer&);
		ArrayBuffer& operator=(ArrayBuffer&&);
		
		T      & operator[](uint32_t index);
		const T& operator[](uint32_t index) const;
		
		uint32_t push(T&& item);
		uint32_t push(const T& item);

		uint32_t pop(uint32_t count = 0);

		uint32_t concat(ArrayBuffer&& arr);

		ArrayBuffer slice(uint32_t start);
		ArrayBuffer slice(uint32_t start, uint32_t end);

		inline bool is_readonly() const { return _container.is_readonly(); }
		inline T* operator*() { return *_container; }
		inline T* value() { return *_container; }
		inline const T* operator*() const { return *_container; }
		inline const T* value() const { return *_container; }

		/**
		* @func size 获取数据占用内存大小
		*/
		inline uint32_t size() const { return _length * sizeof(T); }

		/**
		* @func is_null() Is null data available?
		*/
		inline bool is_null() const {
			return *_container == nullptr;
		}
		
		/**
		 * @func copy()
		 */
		inline ArrayBuffer copy() const {
			return ArrayBuffer(*(const ArrayBuffer*)this);
		}
		
		/**
		* @func collapse
		*/
		T* collapse() {
			T* value = _container.collapse();
			if ( value ) {
				this->_length = 0;
			}
			return value;
		}
		
		/**
		* @func collapse_string
		*/
		// inline std::basic_string<T> collapse_string() {
		// 	return std::basic_string<T>(std::move(*this));
		// }

		/**
		* @func realloc reset realloc length and return this ArrayBuffer&&
		*/
		ArrayBuffer&& realloc(uint32_t length) {
			// if ( !_container.readonly() ) {
			// 	return std::move(*this);
			// }
			// _container.realloc(length);
			// _length = length;
			return std::move(*this);
		}

		/**
		* @func write
		* @arg src 
		* @arg to {int=-1} 当前数组开始写入的位置,-1从结尾开始写入
		* @arg size {int=-1} 需要写入项目数量,超过要写入数组的长度自动取写入数组长度,-1写入全部
		* @arg form {int=0} 从要写入数组的form位置开始取数据
		* @ret {uint32_t} 返回写入数据量
		*/
		uint32_t write(const ArrayBuffer& src, int to = -1, int size = -1, uint32_t form = 0);
		uint32_t write(const T* src, int to, uint32_t size);

		void     clear();
		uint32_t length() const;
		uint32_t capacity() const;

		private:
		struct Sham { T _item; };
		uint32_t  _length;
		Container _container;
	};

	typedef       ArrayBuffer<char> Buffer;
	typedef const ArrayBuffer<char> cBuffer;

	#include "ftr/util/buffer.inl"
}

#endif