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

#ifndef __ftr__util__container__
#define __ftr__util__container__

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ftr/util/util.h>

namespace ftr {

	#ifndef FX_MIN_CAPACITY
	# define FX_MIN_CAPACITY (8)
	#endif

	struct FX_EXPORT AllocatorDefault {
		inline static void* alloc(size_t size) {
			return ::malloc(size);
		}
		inline static void* realloc(void* ptr, size_t size) {
			return ::realloc(ptr, size);
		}
		inline static void free(void* ptr) {
			::free(ptr);
		}
	};

	/**
	* @class Container
	*/
	template <typename T, class Allocator = AllocatorDefault> class FX_EXPORT Container {
		public:
		
		~Container() {
			free();
		}
		
		inline uint32_t is_readonly() const { return _readonly; }
		inline uint32_t capacity() const { return _capacity; }
		inline uint32_t size() const { return sizeof(T) * _capacity; }
		inline T* operator*() { return _value; }
		inline const T* operator*() const { return _value; }
		
		//
		Container(uint32_t capacity = 0): _capacity(0), _value(nullptr), _readonly(false) {
			if ( capacity ) {
				_capacity = powf(2, ceil(log2(FX_MAX(FX_MIN_CAPACITY, capacity))));
				_value = static_cast<T*>(Allocator::alloc(size()));
				ASSERT(_value);
			}
		}
		
		Container(uint32_t capacity, T* value, bool readonly = false)
		: _capacity(capacity), _value(value), _readonly(readonly)
		{}
		
		Container(const Container& container): _capacity(0), _value(nullptr), _readonly(false) {
			operator=(container);
		}
		
		Container(Container&& container): _capacity(0), _value(nullptr), _readonly(false) {
			operator=(std::move(container));
		}
		
		Container& operator=(const Container& container) {
			free();
			_capacity = container._capacity;
			if (_capacity) {
				if (container._readonly) {
					_value = container->_value;
					_readonly = true;
				} else {
					_value = static_cast<T*>(Allocator::alloc(size()));
					::memcpy((void*)_value, (void*)*container, size());
					_readonly = false;
				}
			}
			return *this;
		}
		
		Container& operator=(Container&& container) {
			free();
			_capacity = container._capacity;
			_value = container.collapse();
			return *this;
		}
		
		/**
		* @func realloc auro realloc
		* @arg capacity {uint32_t}
		*/
		void realloc(uint32_t capacity) {
			if (_readonly) {
				FX_UNREACHABLE();
			} else {
				if ( capacity ) {
					capacity = FX_MAX(FX_MIN_CAPACITY, capacity);
					if ( capacity > _capacity || capacity < _capacity / 4.0 ) {
						realloc_(powf(2, ceil(log2(capacity))));
					}
				} else {
					free();
				}
			}
		}
		
		/**
		* @func collapse
		*/
		T* collapse() {
			if (_readonly) {
				return nullptr;
			}
			T* rev = _value;
			_capacity = 0;
			_value = nullptr;
			return rev;
		}
		
		/**
		* @func free
		*/
		void free() {
			if ( _value ) {
				if (!_readonly) {
					Allocator::free(_value);
				}
				_capacity = 0;
				_value = nullptr;
			}
		}

		private:

		void realloc_(uint32_t capacity) {
			if (capacity == 0) {
				free();
			} else if ( capacity != _capacity ) {
				uint32_t size = sizeof(T) * capacity;
				_capacity = capacity;
				_value = (T*)(_value ? Allocator::realloc(_value, size) : Allocator::alloc(size));
			}
		}

		template<class S, class A> friend class Container;

		uint32_t _capacity;
		T*       _value;
		bool     _readonly;
	};

}
#endif
