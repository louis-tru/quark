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
#include <stdlib.h>

namespace qk {
	struct Allocator;
	template<typename T = char, typename A = Allocator>
	class StringImpl;
	typedef StringImpl<> String;
	typedef const String cString;

	struct Qk_EXPORT Allocator {
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
		typedef int __HaveObject__;
	};

	/**
	* @class Reference
	*/
	class Qk_EXPORT Reference: public Object {
	public:
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
		typedef int __HaveReference__;
	private:
		typedef int __HaveObject__;
	protected:
		std::atomic_int _refCount;
	};

	class Protocol {
	public:
		virtual Object* asObject() = 0;
		typedef void* __HaveProtocol__;
	};

	class SafeFlag {
	public:
		inline ~SafeFlag() { _safeFlagValue = 0; }
		inline bool isValid() { return _safeFlagValue == 0xff00ffab; }
		inline void markAsInvalid() { _safeFlagValue = 0; }
	private:
		uint32_t _safeFlagValue = 0xff00ffab;
	};

	Qk_EXPORT void Retain(Object* obj);
	Qk_EXPORT void Release(Object* obj);
	Qk_EXPORT void Fatal(const char* file, uint32_t line, const char* func, const char* msg = 0, ...);

	template <typename T, int kind> struct object_traits_basic { // Object
		static inline void Retain(T* obj) { qk::Retain(obj); }
		static inline void Release(T* obj) { qk::Release(obj); }
	};
	template <typename T> struct object_traits_basic<T, 0> { // Other
		static inline void Retain(T* obj) {}
		static inline void Release(T* obj) { delete obj; }
	};
	template <typename T> struct object_traits_basic<T, 3> { //protocol
		static inline void Retain(T* obj) { if (obj) qk::Retain(obj->asObject()); }
		static inline void Release(T* obj) { if (obj) qk::Release(obj->asObject()); }
	};

	template<typename T>
	struct object_traits {
		typedef char __non[0];
		typedef char __obj[1];
		typedef char __ref[2];
		typedef char __pro[3];
		template<typename C> static __obj& test(typename C::__HaveObject__);
		template<typename C> static __ref& test(typename C::__HaveReference__);
		template<typename C> static __pro& test(typename C::__HaveProtocol__);
		template<typename>   static __non& test(...);
		enum Kind { kNon, kObj, kRef, kProtocol };
		template<enum Kind k> struct __is {
			static constexpr Kind kind = (k);
			static constexpr bool non = (k == 0);
			static constexpr bool obj = (k > 0);
			static constexpr bool ref = (k == 2);
			static constexpr bool protocol = (k == 3);
		};
		typedef __is<Kind(sizeof(test<T>(0)) / sizeof(char))> is;
		inline static void Retain(T* obj) { object_traits_basic<T, is::kind>::Retain(obj); }
		inline static void Release(T* obj) { object_traits_basic<T, is::kind>::Release(obj); }
	};

	template <>
	inline void object_traits<void>::Release(void* obj) {
		::free(obj);
	}

	template<
		typename T,
		void (*Rel)(T*) = object_traits<T>::Release,
		void (*Ret)(T*) = object_traits<T>::Retain,
		typename Is = typename object_traits<T>::is
	>
	struct object_traits_from: object_traits<T> {
		typedef Is is;
		inline static void Retain(T* obj) { Ret(obj); }
		inline static void Release(T* obj) { Rel(obj); }
	};

	typedef const void cVoid;

	template <typename T>
	struct IsPointer {
		static constexpr bool value = false;
		inline static void Release(T& obj) {}
	};

	template <typename T>
	struct IsPointer<T*> {
		static constexpr bool value = true;
		inline static void Release(T*& obj) {
			object_traits<T>::Release(obj); obj = nullptr;
		}
	};

	template <typename T>
	struct Wobj: Object {
		inline Wobj(const T &val): value(val) {}
		inline Wobj(T &&val): value(std::move(val)) {}
		// operator T&() { return value; }
		T value;
	};

	/**
	 * @method Releasep() release plus
	*/
	template<typename T>
	inline void Releasep(T& obj) {
		IsPointer<T>::Release(obj);
	}

	template<class T, typename... Args>
	inline T* New(Args... args) {
		return new T(std::forward<Args>(args)...);
	}

	template<class T, typename... Args>
	inline T* NewRetain(Args... args) {
		T* r = new T(std::forward<Args>(args)...); return r->retain(), r;
	}

}
#endif
