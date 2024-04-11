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
#include <math.h>

namespace qk {

	void* MemoryAllocator::alloc(uint32_t size) {
		return ::malloc(size);
	}
	
	void MemoryAllocator::free(void* ptr) {
		::free(ptr);
	}

	static void increase_f(MemoryAllocator::Prt<void> *ptr, uint32_t size,uint32_t sizeOf) {
		size = Qk_MAX(Qk_MIN_CAPACITY, size);
		size = powf(2, ceilf(log2f(size)));
		ptr->val = ::realloc(ptr->val, sizeOf * size);
		ptr->capacity = size;
		Qk_ASSERT(ptr->val);
	}

	void MemoryAllocator::realloc(Prt<void> *ptr, uint32_t size, uint32_t sizeOf) {
		if ( size > ptr->capacity ) {
			increase_f(ptr, size, sizeOf);
		} else {
			reduce(ptr, size, sizeOf);
		}
	}

	void MemoryAllocator::increase(Prt<void> *ptr, uint32_t size, uint32_t sizeOf) {
		if ( size > ptr->capacity ) {
			increase_f(ptr, size, sizeOf);
		}
	}

	void MemoryAllocator::reduce(Prt<void> *ptr, uint32_t size, uint32_t sizeOf) {
		uint32_t capacity = ptr->capacity;
		if ( size > Qk_MIN_CAPACITY && size < (capacity >> 2) ) { // > 8
			capacity >>= 1;
			size = powf(2, ceilf(log2f(size)));
			size <<= 1;
			size = Qk_MIN(size, capacity);
			ptr->val = ::realloc(ptr->val, sizeOf * size);
			ptr->capacity = size;
			Qk_ASSERT(ptr->val);
		}
	}

	static void* default_object_alloc(size_t size) {
		return ::malloc(size);
	}
	static void default_object_free(void *ptr) {
		::free(ptr);
	}
	static void default_object_strong(Object* obj) {
	}
	static void default_object_weak(Object* obj) {
		obj->destroy(); // The default weak will be directly destroyed
	}

	struct ObjectAllocator {
		void* (*alloc)(size_t size);
		void  (*free)(void *ptr);
		void  (*strong)(Object* obj);
		void  (*weak)(Object* obj);
	} object_allocator = {
		&default_object_alloc, &default_object_free, &default_object_strong, &default_object_weak
	};

	void Object::setAllocator(
		void* (*alloc)(size_t size), void (*free)(void *ptr),
		void (*strong)(Object* obj), void (*weak)(Object* obj)
	) {
		object_allocator = {
			alloc ? alloc: &default_object_alloc,
			free ? free: &default_object_free,
			strong ? strong: &default_object_strong,
			weak ? weak: &default_object_weak,
		};
	}

	void* Object::objectAlloc(size_t size) {
		return object_allocator.alloc(size);
	}

	void Object::objectFree(void *ptr) {
		object_allocator.free(ptr);
	}

	void Object::objectStrong(Object* obj) {
		object_allocator.strong(obj);
	}

	void Object::objectWeak(Object* obj) {
		object_allocator.weak(obj);
	}

	// ---------------- O b j e c t ----------------

#if Qk_MEMORY_TRACE_MARK
	static int active_mark_objects_count_ = 0;
	static Mutex mark_objects_mutex;
	static Array<Object*> mark_objects_;

	int Object::initialize_mark_() {
		if ( mark_index_ == 123456 ) {
			ScopeLock scope(mark_objects_mutex);
			uint32_t index = mark_objects_.length();
			mark_objects_.push(this);
			active_mark_objects_count_++;
			return index;
		}
		return -1;
	}

	Object::Object(): mark_index_(initialize_mark_()) {
	}

	Object::~Object() {
		if ( mark_index_ > -1 ) {
			ScopeLock scope(mark_objects_mutex);
			mark_objects_[mark_index_] = nullptr;
			Qk_ASSERT(active_mark_objects_count_);
			active_mark_objects_count_--;
		}
	}

	std::vector<Object*> Object::mark_objects() {
		ScopeLock scope(mark_objects_mutex);
		Array<Object*> new_mark_objects;
		std::vector<Object*> rv;
		
		for ( auto& i : mark_objects_ ) {
			Object* obj = i.value();
			if ( i.value() ) {
				obj->mark_index_ = new_mark_objects.length();
				new_mark_objects.push(obj);
				rv.pushBack(obj);
			}
		}
		
		Qk_ASSERT( new_mark_objects.length() == active_mark_objects_count_ );
		
		mark_objects_ = std::move(new_mark_objects);
		return rv;
	}

	int Object::mark_objects_count() {
		return active_mark_objects_count_;
	}
#endif

	void Object::destroy() {
		this->~Object();
		object_allocator.free(this); // free heap memory
	}

	bool Object::retain() {
		return 0;
	}

	void Object::release() {
		object_allocator.weak(this);
	}

	bool Object::isReference() const {
		return false;
	}

	void* Object::operator new(size_t size) {
#if Qk_MEMORY_TRACE_MARK
		void* p = object_allocator.alloc(size);
		((Object*)p)->mark_index_ = 123456;
		return p;
#else
		return object_allocator.alloc(size);
#endif
	}

	void* Object::operator new(size_t size, void* p) {
		return p;
	}

	void Object::operator delete(void* p) {
		Qk_UNREACHABLE("Modify to `Release(obj)`");
	}

	// ---------------- R e f e r e n c e ----------------

	Reference::~Reference() {
		Qk_ASSERT( _refCount <= 0 );
	}

	bool Reference::retain() {
		Qk_ASSERT(_refCount >= 0);
		if ( _refCount++ == 0 ) {
			object_allocator.strong(this);
		}
		return true;
	}

	void Reference::release() {
		Qk_ASSERT(_refCount >= 0);
		if ( --_refCount <= 0 ) {
			object_allocator.weak(this);
		}
	}

	bool Reference::isReference() const {
		return true;
	}

	bool Retain(Object* obj) {
		return obj ? obj->retain() : false;
	}

	void Release(Object* obj) {
		if ( obj ) {
			obj->release();
		}
	}

}
