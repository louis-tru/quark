/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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
#include "./thread.h"
#include <math.h>

namespace qk {

	// ---------------- H e a p A l l o c a t o r ----------------
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

	static HeapAllocator *objectHeapAllocator = nullptr;

	void Object::setHeapAllocator(HeapAllocator *allocator) {
		delete objectHeapAllocator;
		objectHeapAllocator = allocator;
	}

	// ---------------- O b j e c t ----------------

	Object::~Object() {}

	void Object::destroy() {
		this->~Object();
		objectHeapAllocator->free(this); // free heap memory
	}

	void Object::retain() {
		objectHeapAllocator->strong(this);
	}

	void Object::release() {
		objectHeapAllocator->weak(this);
	}

	bool Object::isReference() const {
		return false;
	}

	void* Object::operator new(size_t size) {
		if (!objectHeapAllocator)
			objectHeapAllocator = new HeapAllocator;
		return objectHeapAllocator->alloc(size);
	}

	void* Object::operator new(size_t size, void* p) {
		return p;
	}

	void Object::operator delete(void* p) {
		Qk_Unreachable("Modify to `Release(obj)`");
	}

	// ---------------- R e f e r e n c e ----------------

	Reference::~Reference() {
		Qk_ASSERT( _refCount <= 0 );
	}

	void Reference::retain() {
		Qk_ASSERT(_refCount >= 0);
		if ( _refCount++ == 0 ) {
			objectHeapAllocator->strong(this);
		}
	}

	void Reference::release() {
		Qk_ASSERT(_refCount >= 0);
		if ( --_refCount <= 0 ) {
			objectHeapAllocator->weak(this);
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
