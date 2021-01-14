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

#include <ftr/util/buffer.h>
#include <ftr/util/hash.h>

namespace ftr {

	void* AllocatorDefault::alloc(size_t size) {
		return ::malloc(size);
	}
	void* AllocatorDefault::realloc(void* ptr, size_t size) {
		return ::realloc(ptr, size);
	}
	void AllocatorDefault::free(void* ptr) {
		::free(ptr);
	}

	#define FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(T, M, A, APPEND_ZERO) \
		\
		template<> ArrayBuffer<T, M, A>::ArrayBuffer(uint32_t length, uint32_t capacity) \
		: _length(length), _capacity(0), _val(nullptr) \
		{ \
			if (_length) {  \
				realloc_(_length); \
				memset(_val, 0, sizeof(T) * _length); \
			}\
		}\
		\
		template<> ArrayBuffer<T, M, A>::ArrayBuffer(const std::initializer_list<T>& list) \
		: _length((uint32_t)list.size()), _capacity(0), _val(nullptr) \
		{ \
			if (_length) { \
				realloc_(_length); \
				memcpy(_val, list.begin(), sizeof(T) * _length); \
			}\
		} \
		\
		template<> ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::concat_(T* src, uint32_t src_length) { \
			if (src_length) {\
				_length += src_length; \
				realloc_(_length + APPEND_ZERO); \
				T* src = _val; \
				T* to = _val + _length - src_length; \
				memcpy((void*)to, src, src_length * sizeof(T)); \
				if (APPEND_ZERO) _val[_length] = 0; \
			} \
			return *this; \
		} \
		\
		template<> uint32_t ArrayBuffer<T, M, A>::write(const T* src, int to, uint32_t size) { \
			if (size) { \
				if ( to == -1 ) to = _length; \
				_length = FX_MAX(to + size, _length); \
				realloc_(_length + APPEND_ZERO); \
				memcpy((void*)(_val + to), src, size * sizeof(T) ); \
				if (APPEND_ZERO) _val[_length] = 0; \
			} \
			return size; \
		} \
		\
		template<> uint32_t ArrayBuffer<T, M, A>::pop(uint32_t count) { \
			uint32_t j = uint32_t(FX_MAX(_length - count, 0)); \
			if (_length > j) {  \
				_length = j;  \
				realloc_(_length + APPEND_ZERO); \
				if (APPEND_ZERO) _val[_length] = 0; \
			} \
			return _length; \
		} \
		\
		template<> void ArrayBuffer<T, M, A>::clear() { \
			if (_capacity) { \
				if (M == HolderMode::kStrong) { \
					A::free(_val); /* free */ \
				} \
				_length = 0; \
				_capacity = 0; \
				_val = nullptr; \
			} \
		} \
		\
		FX_DEF_ARRAY_SPECIAL_SLICE_IMPLEMENTATION(T, M, A, APPEND_ZERO)
	
	#define FX_DEF_ARRAY_SPECIAL_SLICE_IMPLEMENTATION(T, M, A, APPEND_ZERO) \
		template<> ArrayBuffer<T, HolderMode::kStrong, A> \
		ArrayBuffer<T, M, A>::slice_(uint32_t start, uint32_t end) const { \
			end = FX_MIN(end, _length); \
			if (start < end) { \
				ArrayBuffer<T, HolderMode::kStrong, A> arr(end - start, end - start + APPEND_ZERO); \
				memcpy((void*)arr.val(), _val + start, arr.length() * sizeof(T)); \
				if (APPEND_ZERO) arr[arr.length()] = 0; \
				return std::move(arr); \
			} \
			return ArrayBuffer<T, HolderMode::kStrong, A>();\
		}

	#define FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION_ALL(T) \
		FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(T, HolderMode::kStrong, AllocatorDefault, 1); \
		FX_DEF_ARRAY_SPECIAL_SLICE_IMPLEMENTATION(T, HolderMode::kWeak, AllocatorDefault, 1)

	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION_ALL(char);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION_ALL(unsigned char);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION_ALL(int16_t);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION_ALL(uint16_t );
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION_ALL(int32_t);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION_ALL(uint32_t);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION_ALL(int64_t);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION_ALL(uint64_t);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION_ALL(float);
	FX_DEF_ARRAY_SPECIAL_IMPLEMENTATION_ALL(double);
}
