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
	
	void* MemoryAllocator::realloc(void* ptr, uint32_t size) {
		return ::realloc(ptr, size);
	}

	void MemoryAllocator::aalloc(void** ptrOut, uint32_t size, uint32_t* sizeOut, uint32_t sizeOf) {
		uint32_t capacity = *sizeOut;
		if ( size > capacity ) {
			size = Qk_MAX(Qk_MIN_CAPACITY, size);
			size = powf(2, ceilf(log2f(size)));
			*sizeOut = size;
			*ptrOut = *ptrOut ? ::realloc(*ptrOut, sizeOf * size) : ::malloc(sizeOf * size);
			Qk_ASSERT(*ptrOut);
		} else if ( size > Qk_MIN_CAPACITY && size < (capacity >> 2) ) { // > 8
			capacity >>= 1;
			size = powf(2, ceilf(log2f(size)));
			size <<= 1;
			size = Qk_MIN(size, capacity);
			*sizeOut = size;
			*ptrOut = ::realloc(*ptrOut, sizeOf * size);
			Qk_ASSERT(*ptrOut);
		}
	}

	static void* default_object_alloc(size_t size) {
		return ::malloc(size);
	}
	static void default_object_release(Object* obj) {
		obj->~Object();
		::free(obj);
	}
	static void default_object_retain(Object* obj) {
		/* NOOP */
	}

	static void* (*object_allocator_alloc)(size_t size) = &default_object_alloc;
	static void  (*object_allocator_release)(Object* obj) = &default_object_release;
	static void  (*object_allocator_retain)(Object* obj) = &default_object_retain;

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
					rv.push_back(obj);
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

	bool Object::is_reference() const {
		return false;
	}

	bool Object::retain() {
		return 0;
	}

	void Object::release() {
		object_allocator_release(this);
	}

	void* Object::operator new(size_t size) {
#if Qk_MEMORY_TRACE_MARK
			void* p = object_allocator_alloc(size);
			((Object*)p)->mark_index_ = 123456;
			return p;
#else
			return object_allocator_alloc(size);
#endif
	}

	void* Object::operator new(size_t size, void* p) {
		return p;
	}

	void Object::operator delete(void* p) {
		Qk_UNREACHABLE("Modify to `Release(obj)`");
	}

	void Object::set_object_allocator(
		void* (*alloc)(size_t size),
		void (*release)(Object* obj), void (*retain)(Object* obj)
	) {
		if (alloc) {
			object_allocator_alloc = alloc;
		} else {
			object_allocator_alloc = &default_object_alloc;
		}
		if (release) {
			object_allocator_release = release;
		} else {
			object_allocator_release = &default_object_release;
		}
		if (retain) {
			object_allocator_retain = retain;
		} else {
			object_allocator_retain = &default_object_retain;
		}
	}

	bool Retain(Object* obj) {
		return obj ? obj->retain() : false;
	}

	void Release(Object* obj) {
		if ( obj )
			obj->release();
	}

	Reference::~Reference() {
		Qk_ASSERT( _ref_count <= 0 );
	}

	bool Reference::retain() {
		Qk_ASSERT(_ref_count >= 0);
		if ( _ref_count++ == 0 ) {
			object_allocator_retain(this);
		}
		return true;
	}

	void Reference::release() {
		Qk_ASSERT(_ref_count >= 0);
		if ( --_ref_count <= 0 ) { // 当引用记数小宇等于0释放
			object_allocator_release(this);
		}
	}

	bool Reference::is_reference() const {
		return true;
	}

}
