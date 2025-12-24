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

#ifndef __quark__util__object__
#define __quark__util__object__

#include "./allocator.h"
#include <atomic>
#include <stdlib.h>

namespace qk {
	template<typename T = char, typename A = Allocator>
	class StringImpl;
	typedef StringImpl<> String;
	typedef const String cString;
	typedef const void cVoid;

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
		static bool isDefaultHeapAllocator(); // check is default heap allocator
		static void setHeapAllocator(HeapAllocator *allocator); // set global heap allocator
		static void* operator new(size_t size);
		static void* operator new(size_t size, void *p);
		static void  operator delete(void *p);
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

	/**
	 * @class Protocol
	 */
	class Protocol {
	public:
		virtual Object* asObject() = 0;
		typedef void* __HaveProtocol__;
	};

	/**
	 * @struct Wobj
	 * @brief Wrapper object for value types.
	 *
	 * A simple Object subclass that holds a value of type T.
	 *
	 * @tparam T Value type.
	 */
	template <typename T> struct Wobj: Object {
		inline Wobj(const T &val): value(val) {}
		inline Wobj(T &&val): value(std::move(val)) {}
		// operator T&() { return value; }
		T value;
	};

	/**
	 * @method New() Create a new object instance.
	*/
	template<class T, typename... Args>
	inline T* New(Args... args) {
		return new T(std::forward<Args>(args)...);
	}

	/**
	 * @method NewRetain() Create a new retained object instance.
	*/
	template<class T, typename... Args>
	inline T* NewRetain(Args... args) {
		T* r = new T(std::forward<Args>(args)...); return r->retain(), r;
	}

	/**
	 * @method bitwise_cast() Perform a bitwise cast between types of the same size.
	 *
	 * This function allows for safe reinterpretation of data between types
	 * that have the same size in memory. It uses a union to ensure that the
	 * bit patterns are preserved during the cast.
	 *
	 * @tparam TO   Target type to cast to.
	 * @tparam FROM Source type to cast from.
	 * @param in    Input value of type FROM.
	 * @return      Reinterpreted value of type TO.
	 */
	template<typename TO, typename FROM>
	inline TO bitwise_cast(const FROM &in) {
		static_assert(sizeof(TO) == sizeof(FROM), "reinterpret_cast_sizeof_types_is_equal");
		union {
			FROM from;
			TO to;
		} u;
		u.from = in;
		return u.to;
	}

	/**
	 * @class SafeFlag
	 * @brief A simple safety flag to detect invalid states.
	 * 
	 * This class provides a mechanism to mark an object as valid or invalid.
	 * It uses a specific magic number to indicate validity, allowing for
	 * quick checks to ensure that an object is in a safe state before use.
	 */
	class SafeFlag {
	public:
		inline ~SafeFlag() { _safeFlagValue = 0; }
		inline bool isValid() { return _safeFlagValue == 0xff73ffab; }
		inline void markAsInvalid() { _safeFlagValue = 0; }
	private:
		uint32_t _safeFlagValue = 0xff73ffab;
	};

	Qk_EXPORT void Retain(Object* obj); // retain object
	Qk_EXPORT void Release(Object* obj); // release object

	/**
	 * @struct object_traits_basic
	 * @brief Basic traits for handling different object types.
	 * This struct provides methods to retain and release objects
	 * based on their type characteristics (Object, Reference, Protocol, or non-object).
	*/
	template <typename T, int kind> struct object_traits_basic { // Object
		static inline void Retain(T* obj) { if (obj) obj->retain(); }
		static inline void Release(T* obj) { if (obj) obj->release(); }
	};
	template <typename T> struct object_traits_basic<T, 0> { // Other
		static inline void Retain(T* obj) {}
		static inline void Release(T* obj) { delete obj; }
	};
	template <typename T> struct object_traits_basic<T, 3> { //protocol
		static inline void Retain(T* obj) { if (obj) obj->asObject()->retain(); }
		static inline void Release(T* obj) { if (obj) obj->asObject()->release(); }
	};

	/**
	 * @struct object_traits
	 * @brief Traits for handling different object types.
	 * This struct provides methods to retain and release objects
	 * based on their type characteristics (Object, Reference, Protocol, or non-object).
	*/
	template<typename T> struct object_traits {
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
		inline static void Retain(T* obj) { 
			object_traits_basic<T, is::kind>::Retain(obj);
		}
		inline static void Release(T* obj) {
			object_traits_basic<T, is::kind>::Release(obj);
		}
	};

	template <>
	inline void object_traits<void>::Release(void* obj) {
		::free(obj);
	}

	/**
	 * @struct object_traits_from
	 * @brief Traits for handling different object types with custom retain/release functions.
	 * This struct allows specifying custom retain and release functions for a given type T.
	*/
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

	/**
	 * @struct IsPointer
	 * @brief Helper to determine if a type is a pointer and manage its retention.
	*/
	template <typename T> struct IsPointer {
		static constexpr bool value = false;
		inline static void Retain(T& obj) {}
		inline static void Release(T& obj) {}
	};

	/**
	 * @struct IsPointer specialization for pointer types.
	*/
	template <typename T> struct IsPointer<T*> {
		static constexpr bool value = true;
		inline static void Retain(T*& obj) { object_traits<T>::Retain(obj); }
		inline static void Release(T*& obj) { object_traits<T>::Release(obj); obj = nullptr; }
	};

	/**
	 * @method Retainp() retain plus
	*/
	template<typename T>
	inline void Retainp(T& obj) { IsPointer<T>::Retain(obj); }

	/**
	 * @method Releasep() release plus
	*/
	template<typename T>
	inline void Releasep(T& obj) { IsPointer<T>::Release(obj); }

	/**
	 * @method Retainp() retain plus for atomic pointer
	 */
	template<typename T>
	inline void Retainp(std::atomic<T*>& obj) {
		object_traits<T>::retain(obj.load()); // retain current value
	}

	/**
	 * @method Releasep() release plus for atomic pointer
	 */
	template<typename T>
	inline void Releasep(std::atomic<T*>& obj) {
		// auto v = obj.exchange(nullptr); // first set to nullptr
		// IsPointer<T*>::Release(v); // then release
		object_traits<T>::Release(obj.exchange(nullptr)); // release and set to nullptr
	}

}
#endif
