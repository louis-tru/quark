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

#include "ftr/util/buffer.h"

namespace ftr {

	#define FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(T, Container, append_zero) \
	\
	template<> ArrayBuffer<T, Container<T>>::ArrayBuffer(uint32_t length, uint32_t capacity) \
	: _length(length), _container(FX_MAX(length, capacity)) { \
		if (_length) {  \
			memset(*_container, 0, sizeof(T) * _length); \
		}\
	}\
	\
	template<> ArrayBuffer<T, Container<T>>::ArrayBuffer(const std::initializer_list<T>& list) \
	: _length(uint32_t(list.size())), _container(uint32_t(list.size())) { \
		if (_length) {  \
			memcpy((void*)*_container, list.begin(), sizeof(T) * _length); \
		}\
	} \
	\
	/*template<> uint32_t ArrayBuffer<T, Container<T>>::push(const ArrayBuffer& arr) { \
		if (arr._length) { \
			_length += arr._length; \
			_container.realloc(_length + append_zero); \
			const T* item = *arr._container; \
			T* begin = (*_container) + _length - arr._length; \
			memcpy((void*)begin, item, arr._length * sizeof(T)); \
			if (append_zero) begin[_length] = 0; \
		} \
		return _length; \
	}*/\
	\
	template<> uint32_t ArrayBuffer<T, Container<T>>::push(ArrayBuffer&& arr) { \
		if (arr._length) { \
			_length += arr._length; \
			_container.realloc(_length + append_zero); \
			const T* item = *arr._container; \
			T* begin = (*_container) + _length - arr._length; \
			memcpy((void*)begin, item, arr._length * sizeof(T)); \
			if (append_zero) begin[_length] = 0; \
		} \
		return _length; \
	} \
	\
	/*template<> uint32_t ArrayBuffer<T, Container<T>>::pop() { \
		if (_length) {  \
			_length--;  \
			_container.realloc(_length); \
		} \
		return _length; \
	}*/ \
	\
	template<> ArrayBuffer<T, Container<T>> ArrayBuffer<T, Container<T>>::slice(uint32_t start, uint32_t end) { \
		end = FX_MIN(end, _length); \
		if (start < end) { \
			ArrayBuffer arr; \
			arr._length = end - start; \
			arr._container.realloc(arr._length); \
			memcpy((void*)*arr._container, (*_container) + start, arr._length * sizeof(T) ); \
			return arr; \
		} \
		return ArrayBuffer(); \
	}\
	\
	template<> uint32_t ArrayBuffer<T, Container<T>>::write(const T* src, int to, uint32_t size) { \
		if (size) { \
			if ( to == -1 ) to = _length; \
			_length = FX_MAX(to + size, _length); \
			_container.realloc(_length); \
			memcpy((void*)((*_container) + to), src, size * sizeof(T) ); \
		} \
		return size; \
	} \
	\
	template<> uint32_t ArrayBuffer<T, Container<T>>::pop(uint32_t count) { \
		uint32_t j = uint32_t(FX_MAX(_length - count, 0)); \
		if (_length > j) {  \
			_length = j;  \
			_container.realloc(_length); \
		} \
		return _length; \
	} \
	\
	template<> void ArrayBuffer<T, Container<T>>::clear() { \
		_length = 0;  \
		_container.free();  \
	}

	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(char, Container, 1);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(unsigned char, Container, 0);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(int16_t, Container, 0);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(uint16_t, Container, 0);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(int, Container, 0);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(uint32_t, Container, 0);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(int64_t, Container, 0);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(uint64_t, Container, 0);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(float, Container, 0);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(double, Container, 0);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(bool, Container, 0);

}