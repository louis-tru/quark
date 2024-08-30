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

#ifndef __quark__util__object__
#define __quark__util__object__

#include "./macros.h"
#include <utility>
#include <atomic>

namespace qk {
	struct Allocator;
	struct ObjectTraits;
	struct ReferenceTraits;
	struct ProtocolTraits;

	template<typename T = char, typename A = Allocator>
	class StringImpl;
	typedef StringImpl<> String;
	typedef const String cString;

	struct Allocator {
		template<typename T, typename A = Allocator>
		struct Prt {
			void realloc(uint32_t size) { A::realloc((Prt<void>*)this, size, sizeof(T)); }
			void increase(uint32_t size) { A::increase((Prt<void>*)this, size, sizeof(T)); }
			void reduce(uint32_t size) { A::reduce((Prt<void>*)this, size, sizeof(T)); }
			T       *val = nullptr;
			uint32_t capacity = 0;
		};
		static void* alloc(uint32_t size);
		static void  free(void *ptr);
		static void  realloc(Prt<void> *ptr, uint32_t size, uint32_t sizeOf);
		static void  increase(Prt<void> *ptr, uint32_t size, uint32_t sizeOf);
		static void  reduce(Prt<void> *ptr, uint32_t size, uint32_t sizeOf);
	};

	/**
	* @class Object
	*/
	class Qk_EXPORT Object {
	public:
		class HeapAllocator {
		public:
			virtual ~HeapAllocator();
			/**
			* @method alloc() alloc memory from heap
			*/
			virtual void* alloc(size_t size);
			/**
			* @method free() free memory from heap
			*/
			virtual void free(void *ptr);
			/**
			* Remove the weak mark to indicate that the destruction operation needs to be revoked.
			* Before calling ensure that the object has not been destroyed yet
			* @method  strong()
			*/
			virtual void strong(Object* obj);
			/**
			* Mark as weak object, indicating that the object needs to be destroyed,
			* but it may not be immediately destroyed.
			* This is determined by the heap allocator implementation
			* @method weak()
			*/
			virtual void weak(Object* obj);
		};
		typedef ObjectTraits Traits;
		typedef int HasObjectType;
		virtual ~Object();
		virtual void destroy(); // Heap allocation destructor function call and memory free
		virtual void retain(); // Heap allocation strong
		virtual void release(); // Heap allocation weak
		virtual bool isReference() const;
		virtual String toString() const;
		static void* operator new(size_t size);
		static void* operator new(size_t size, void *p);
		static void  operator delete(void *p);
		static HeapAllocator* heapAllocator();
		static void setHeapAllocator(HeapAllocator *allocator);
	};

	/**
	* @class Reference
	*/
	class Qk_EXPORT Reference: public Object {
	public:
		typedef ReferenceTraits Traits;
		typedef int HasReferenceType;
		inline Reference(): _refCount(0) {}
		inline Reference(const Reference& ref): _refCount(0) {}
		inline Reference& operator=(const Reference& ref) { return *this; }
		virtual ~Reference();
		// It is not completely thread safe,
		// and it must be confirmed before calling 
		// that it has not been released by another thread at the same time
		virtual void retain(); // ref++
		virtual void release(); // --ref
		virtual bool isReference() const;
		inline  int  refCount() const { return _refCount.load(); }
	private:
		typedef int HasObjectType;
	protected:
		std::atomic_int _refCount;
	};

	class SafeFlag {
	public:
		inline ~SafeFlag() { _flagValue = 0; }
		inline bool isValid() { return _flagValue == 0xff00ffab; }
	private:
		uint32_t _flagValue = 0xff00ffab;
	};

	Qk_EXPORT void Retain(Object* obj);
	Qk_EXPORT void Release(Object* obj);
	Qk_EXPORT void Fatal(const char* file, uint32_t line, const char* func, const char* msg = 0, ...);

	/**
	* @class Protocol protocol base
	*/
	class Protocol {
	public:
		typedef ProtocolTraits Traits;
		virtual Object* toObject() = 0;
	};

	/**
	* @class ObjectTraits
	*/
	struct ObjectTraits {
		inline static bool Retain(Object* obj) {
			if (obj) obj->retain();
		}
		inline static void Release(Object* obj) {
			if (obj) obj->release();
		}
		static constexpr bool isObject = true;
		static constexpr bool isReference = false;
	};

	/**
	* @class ReferenceTraits
	*/
	struct ReferenceTraits: ObjectTraits {
		static constexpr bool isReference = true;
	};

	/**
	* @class ProtocolTraits
	*/
	struct ProtocolTraits {
		template<class T> inline static void Retain(T* obj) {
			if (obj) qk::Retain(obj->toObject());
		}
		template<class T> inline static void Release(T* obj) {
			if (obj) qk::Release(obj->toObject());
		}
		static constexpr bool isReference = false;
	};

	/**
	 * @class NonObjectTraits
	 */
	struct NonObjectTraits {
		template<class T> inline static void Retain(T* obj) {
		}
		template<class T> inline static void Release(T* obj) {
			delete obj;
		}
		static constexpr bool isReference = false;
	};

	template<typename T>
	struct has_object_type {
		typedef char Non[1]; typedef char Obj[2];
		typedef char Ref[3];
		template<typename C> static Obj& test(typename C::HasObjectType);
		template<typename C> static Ref& test(typename C::HasReferenceType);
		template<typename>   static Non& test(...);
		static const int  type = sizeof(test<T>(0)) / sizeof(char) - 1;
		static const bool isObj = sizeof(test<T>(0)) / sizeof(char) > 1;
		static const bool isRef = sizeof(test<T>(0)) / sizeof(char) == 3;
	};

	template<class T, typename... Args>
	inline T* New(Args... args) { return new T(args...); }

	template<class T, typename... Args>
	inline T* NewRetain(Args... args) { T* r = new T(args...); return r->retain(), r; }

}
#endif
