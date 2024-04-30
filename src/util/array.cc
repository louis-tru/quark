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

#include "./array.h"
#include "./string.h"
#include <string.h>
#include <math.h>

namespace qk {

	#define Qk_DEF_ARRAY_SPECIAL_IMPLEMENTATION_(T, A, APPEND_ZERO) \
		\
		template<> void Array<T, A>::reset(uint32_t length) { \
			if (length < _length) { /* clear Partial data */ \
				_length = length;\
			} \
			_ptr.realloc(length+APPEND_ZERO); \
			if (APPEND_ZERO) _ptr.val[_length] = 0; \
		} \
		\
		template<> void Array<T, A>::extend(uint32_t length) \
		{ \
			if (length > _length) {  \
				_length = length; \
				increase_(_length + APPEND_ZERO); \
				if (APPEND_ZERO) _ptr.val[_length] = 0; \
			}\
		}\
		\
		template<> std::vector<T> Array<T, A>::vector() const { \
			std::vector<T> r(_length); \
			if (_length) \
				memcpy(r.data(), _ptr.val, sizeof(T) * _length); \
			Qk_ReturnLocal(r); \
		} \
		\
		template<> void Array<T, A>::concat_(T* src, uint32_t src_length) { \
			if (src_length) {\
				_length += src_length; \
				increase_(_length + APPEND_ZERO); \
				T* src = _ptr.val; \
				T* to = _ptr.val + _length - src_length; \
				memcpy((void*)to, src, src_length * sizeof(T)); \
				if (APPEND_ZERO) _ptr.val[_length] = 0; \
			} \
		} \
		\
		template<> uint32_t Array<T, A>::write(const T* src, uint32_t size, int to) { \
			if (size) { \
				if ( to == -1 ) to = _length; \
				_length = Qk_MAX(to + size, _length); \
				increase_(_length + APPEND_ZERO); \
				memcpy((void*)(_ptr.val + to), src, size * sizeof(T) ); \
				if (APPEND_ZERO) _ptr.val[_length] = 0; \
			} \
			return size; \
		} \
		\
		template<> \
		Array<T, A>& Array<T, A>::push(const T& item) { \
			increase_(_length + APPEND_ZERO + 1); \
			_ptr.val[_length++] = item;\
			return *this; \
		} \
		template<> \
		Array<T, A>& Array<T, A>::push(T&& item) { \
			increase_(_length + APPEND_ZERO + 1); \
			_ptr.val[_length++] = item;\
			return *this; \
		} \
		template<> Array<T, A>& Array<T, A>::pop(uint32_t count) { \
			uint32_t j = uint32_t(Qk_MAX(_length - count, 0)); \
			if (_length > j) {  \
				_length = j;  \
				_ptr.reduce(_length + APPEND_ZERO); \
			} \
			return *this; \
		} \
		\
		template<> void Array<T, A>::clear() { \
			if (_ptr.val) { \
				A::free(_ptr.val); /* free */ \
				_ptr.capacity = 0; \
				_ptr.val = nullptr; \
				_length = 0; \
			} \
		} \
		\
		template<> void Array<T, A>::copy_(Ptr* ptr, uint32_t start, uint32_t len) const { \
			ptr->realloc(len+APPEND_ZERO);\
			memcpy(ptr->val, _ptr.val + start, len * sizeof(T)); \
			if (APPEND_ZERO) ptr->val[len] = 0; \
		} \

	#define Qk_DEF_ARRAY_SPECIAL_IMPLEMENTATION(T, APPEND_ZERO) \
		Qk_DEF_ARRAY_SPECIAL_IMPLEMENTATION_(T, MemoryAllocator, APPEND_ZERO)

#ifndef Qk_ARRAY_SKIP_DEFAULT_IMPL
	template<> void Array<char, MemoryAllocator>::_Reverse(void *src, size_t size, uint32_t len) {
		if (len > 1) {
			char* _src = (char*)src;
			void* tmp = malloc(size);
			uint32_t len2 = floor(len / 2);
			uint32_t i = 0;
			while (i < len2) {
				char* src = _src + (i * size);
				char* dest = _src + ((len - i - 1) * size);
				memcpy(tmp, src, size); // swap data
				memcpy(src, dest, size);
				memcpy(dest, tmp, size);
				i++;
			}
			free(tmp);
		}
	}
	
	Qk_DEF_ARRAY_SPECIAL_IMPLEMENTATION(char,1);
	Qk_DEF_ARRAY_SPECIAL_IMPLEMENTATION(uint8_t,1);
	Qk_DEF_ARRAY_SPECIAL_IMPLEMENTATION(int16_t,1);
	Qk_DEF_ARRAY_SPECIAL_IMPLEMENTATION(uint16_t,1);
	Qk_DEF_ARRAY_SPECIAL_IMPLEMENTATION(int32_t,1);
	Qk_DEF_ARRAY_SPECIAL_IMPLEMENTATION(uint32_t,1);
	Qk_DEF_ARRAY_SPECIAL_IMPLEMENTATION(int64_t,1);
	Qk_DEF_ARRAY_SPECIAL_IMPLEMENTATION(uint64_t,1);
	Qk_DEF_ARRAY_SPECIAL_IMPLEMENTATION(float,1);
	Qk_DEF_ARRAY_SPECIAL_IMPLEMENTATION(double,1);
#endif

}
