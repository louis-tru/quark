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

#if Qk_MEMORY_TRACE_MARK
# include <vector>
#endif
namespace qk {

#ifndef Qk_MIN_CAPACITY
# define Qk_MIN_CAPACITY (4)
#endif

	struct ObjectTraits;
	struct ReferenceTraits;
	struct ProtocolTraits;
	struct MemoryAllocator;

	template<
		typename T = char, typename A = MemoryAllocator
	> class ArrayString;
	typedef ArrayString<> String;
	typedef const String cString;

	/**
	* @class Object
	*/
	class Qk_EXPORT Object {
	public:
		typedef ObjectTraits Traits;
		virtual bool retain();
		virtual void release(); // "new" method alloc can call，Otherwise, fatal exception will be caused
		virtual bool isReference() const;
		virtual String toString() const;
		static void* operator new(size_t size);
		static void* operator new(size_t size, void* p);
		static void  operator delete(void* p);
		static void setAllocator(
			void* (*alloc)(size_t size), void (*release)(Object* obj),
			void (*retain)(Object* obj)
		);
		typedef void* __has_object_type;
#if Qk_MEMORY_TRACE_MARK
		static std::vector<Object*> mark_objects();
		static int mark_objects_count();
		Object();
		virtual ~Object();
	private:
		int initialize_mark_();
		int mark_index_;
#else
		virtual ~Object() = default;
#endif
		// Use the following method to override to restore the default call
		// virtual void release() { static_assert(!Traits::isReference, ""); ::delete this; }
		// static void* operator new(std::size_t size) { return ::operator new(size); }
		// static void  operator delete(void* p) { ::operator delete(p); }
	};

	/**
	* @class Reference
	*/
	class Qk_EXPORT Reference: public Object {
	public:
		typedef ReferenceTraits Traits;
		inline Reference(): _refCount(0) {}
		inline Reference(const Reference& ref): _refCount(0) {}
		inline Reference& operator=(const Reference& ref) { return *this; }
		virtual ~Reference();
		virtual bool retain();
		virtual void release();
		virtual bool isReference() const;
		inline int refCount() const { return _refCount; }
		typedef void* __has_ref_type; private:
		typedef void* __has_object_type;
	protected:
		std::atomic_int _refCount;
	};

	Qk_EXPORT bool Retain(Object* obj);
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
		inline static bool Retain(Object* obj) { return obj ? obj->retain(): 0; }
		inline static void Release(Object* obj) { if (obj) obj->release(); }
		static constexpr bool isReference = false;
		static constexpr bool isObject = true;
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
		template<class T> inline static bool Retain(T* obj) {
			return obj ? qk::Retain(obj->toObject()) : 0;
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
		template<class T> inline static bool Retain(T* obj) {
			/* Non referential pairs need not be Retain */ return 0;
		}
		template<class T> inline static void Release(T* obj) { delete obj; }
		static constexpr bool isReference = false;
	};

	struct MemoryAllocator {
		template<typename T, typename A = MemoryAllocator>
		struct Prt {
			void realloc(uint32_t size) {
				A::realloc((Prt<void>*)this, size, sizeof(T));
			}
			void increase(uint32_t size) {
				A::increase((Prt<void>*)this, size, sizeof(T));
			}
			void reduce(uint32_t size) {
				A::reduce((Prt<void>*)this, size, sizeof(T));
			}
			uint32_t capacity = 0;
			T*       val = nullptr;
		};
		static void*alloc(uint32_t size);
		static void free(void *ptr);
		static void realloc(Prt<void> *ptr, uint32_t size, uint32_t sizeOf);
		static void increase(Prt<void> *ptr, uint32_t size, uint32_t sizeOf);
		static void reduce(Prt<void> *ptr, uint32_t size, uint32_t sizeOf);
	};

	template<typename T>
	struct has_object_type {
		typedef char Non[1]; typedef char Obj[2];
		typedef char Ref[3];
		template<typename C> static Obj& test(typename C::__has_object_type);
		template<typename C> static Ref& test(typename C::__has_ref_type);
		template<typename> static Non& test(...);
		static const int type = sizeof(test<T>(0)) / sizeof(char) - 1;
		static const bool isObj = sizeof(test<T>(0)) / sizeof(char) > 1;
		static const bool isRef = sizeof(test<T>(0)) / sizeof(char) == 3;
	};

	template<class T, typename... Args>
	inline T* New(Args... args) { return new T(args...); }

	template<class T, typename... Args>
	inline T* NewRetain(Args... args) { T* r = new T(args...); return r->retain(), r; }

}
#endif
