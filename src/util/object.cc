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

#include "./object.h"
#include "./array.h"
#include "./loop.h"
#include <math.h>

#ifndef Qk_Min_CAPACITY
# define Qk_Min_CAPACITY (4)
#endif

namespace qk {

	void* Allocator::alloc(uint32_t size) {
		return ::malloc(size);
	}
	
	void Allocator::free(void* ptr) {
		::free(ptr);
	}

	static void increase_(Allocator::Prt<void> *ptr, uint32_t size,uint32_t sizeOf) {
		size = Qk_Max(Qk_Min_CAPACITY, size);
		size = powf(2, ceilf(log2f(size)));
		ptr->val = ::realloc(ptr->val, sizeOf * size);
		ptr->capacity = size;
		Qk_Assert(ptr->val);
	}

	void Allocator::realloc(Prt<void> *ptr, uint32_t size, uint32_t sizeOf) {
		if ( size > ptr->capacity ) {
			increase_(ptr, size, sizeOf);
		} else {
			reduce(ptr, size, sizeOf);
		}
	}

	void Allocator::increase(Prt<void> *ptr, uint32_t size, uint32_t sizeOf) {
		if ( size > ptr->capacity ) {
			increase_(ptr, size, sizeOf);
		}
	}

	void Allocator::reduce(Prt<void> *ptr, uint32_t size, uint32_t sizeOf) {
		uint32_t capacity = ptr->capacity;
		if ( size > Qk_Min_CAPACITY && size < (capacity >> 2) ) { // > 8
			capacity >>= 1;
			size = powf(2, ceilf(log2f(size)));
			size <<= 1;
			size = Qk_Min(size, capacity);
			ptr->val = ::realloc(ptr->val, sizeOf * size);
			ptr->capacity = size;
			Qk_Assert(ptr->val);
		}
	}
	
	typedef Object::HeapAllocator HeapAllocator;

	HeapAllocator::~HeapAllocator() {}

	void* HeapAllocator::alloc(size_t size) {
		return ::malloc(size);
	}

	void HeapAllocator::free(void *ptr) {
		::free(ptr);
	}

	void HeapAllocator::strong(Object* obj) {
	}

	void HeapAllocator::weak(Object* obj) {
		obj->destroy(); // The default weak will be directly destroyed
	}

	static HeapAllocator *object_heap_allocator = new HeapAllocator();

	HeapAllocator* Object::heapAllocator() {
		return object_heap_allocator;
	}

	void Object::setHeapAllocator(HeapAllocator *allocator) {
		delete object_heap_allocator;
		object_heap_allocator = allocator;
	}

	// ---------------- O b j e c t ----------------

	Object::~Object() {}

	void Object::destroy() {
		this->~Object();
		object_heap_allocator->free(this); // free heap memory
	}

	void Object::retain() {
		object_heap_allocator->strong(this);
	}

	void Object::release() {
		object_heap_allocator->weak(this);
	}

	bool Object::isReference() const {
		return false;
	}

	void* Object::operator new(size_t size) {
		return object_heap_allocator->alloc(size);
	}

	void* Object::operator new(size_t size, void* p) {
		return p;
	}

	void Object::operator delete(void* p) {
		Qk_Unreachable("Modify to `Release(obj)`");
	}

	// ---------------- R e f e r e n c e ----------------

	Reference::~Reference() {
		Qk_Assert( _refCount <= 0 );
	}

	void Reference::retain() {
		Qk_Assert(_refCount >= 0);
		if ( _refCount++ == 0 ) {
			object_heap_allocator->strong(this);
		}
	}

	void Reference::release() {
		Qk_Assert(_refCount >= 0);
		if ( --_refCount <= 0 ) {
			object_heap_allocator->weak(this);
		}
	}

	bool Reference::isReference() const {
		return true;
	}

	void Retain(Object* obj) {
		if (obj) obj->retain();
	}

	void Release(Object* obj) {
		if (obj) obj->release();
	}
}
