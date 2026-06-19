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
#include <type_traits>

namespace qk {
	template<typename T = char>
	class StringImpl;
	typedef StringImpl<> String;
	typedef const String cString;
	typedef const void cVoid;

	class NonObject {};

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
		static void setHeapAllocator(HeapAllocator *allocator); // set global heap allocator
		static void* operator new(size_t size);
		static void* operator new(size_t size, void *p);
		static void  operator delete(void *p);
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
	protected:
		std::atomic_int _refCount;
	};

	/**
	 * @class Protocol
	 */
	class Protocol {
	public:
		virtual Object* asObject() = 0;
	};

	/**
	 * @struct WObject
	 * @brief Wrapper object for value types.
	 *
	 * A simple Object subclass that holds a value of type T.
	 *
	 * @tparam T Value type.
	 */
	template <typename T> struct WObject: Object {
		inline WObject(const T &val): value(val) {}
		inline WObject(T &&val): value(std::move(val)) {}
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
		inline bool is_valid() { return _safeFlagValue == 0xff73ffab; }
		inline void mark_as_invalid() { _safeFlagValue = 0; }
	private:
		uint32_t _safeFlagValue = 0xff73ffab;
	};

	/**
	 * @struct ObjectTraits
	 * @brief Traits for handling different object types.
	 * This struct provides methods to retain and release objects
	 * based on their type characteristics (Object, Reference, Protocol, or non-object).
	*/
	template<typename T, typename = void>
	struct ObjectTraitsBase {
		static constexpr bool isComplete = false;
		static constexpr bool isObj = false;
		static constexpr bool isRef = false;
		static constexpr bool isProtocol = false;
		static constexpr bool isNonObj = true;
		static constexpr bool isTriviallyCopyable = false;
		static constexpr bool isTriviallyDestructible = false;
		static constexpr bool isTriviallyDefaultConstructible = false;
		static constexpr bool isOrdinary = false;
		static void Retain(T*) = delete;
		static void Release(T*) = delete;
		template<typename U>
		using isBase = std::false_type;
	};

	template<typename T>
	struct ObjectTraitsBase<T, decltype(void(sizeof(T)))> {
		enum Kind { kNonObj, kObj, kProtocol };
		template<typename U>
		using isBase = std::is_base_of<U, T>;
		static constexpr bool isComplete = true;
		static constexpr bool isObj = isBase<Object>::value;
		static constexpr bool isRef = isBase<Reference>::value;
		static constexpr bool isProtocol = isBase<Protocol>::value;
		static constexpr bool nonObj = !(isObj || isRef || isProtocol);
		static constexpr Kind kind = isObj ? kObj : (isProtocol ? kProtocol : kNonObj);
		static constexpr bool isTriviallyDefaultConstructible = std::is_trivially_default_constructible<T>::value;
		static constexpr bool isTriviallyDestructible = std::is_trivially_destructible<T>::value;
		static constexpr bool isTriviallyCopyable = std::is_trivially_copyable<T>::value;
		static constexpr bool isOrdinary = isTriviallyDefaultConstructible && isTriviallyDestructible && isTriviallyCopyable;
		inline static void Retain(T* obj) {
			retain(obj, std::integral_constant<Kind, kind>());
		}
		inline static void Release(T* obj) {
			release(obj, std::integral_constant<Kind, kind>());
		}
	private:
		static inline void retain(T*, std::integral_constant<Kind, kNonObj>) {}
		static inline void release(T* obj, std::integral_constant<Kind, kNonObj>) {
			delete obj;
		}
		static inline void retain(T* obj, std::integral_constant<Kind, kObj>) {
			if (obj) obj->retain();
		}
		static inline void release(T* obj, std::integral_constant<Kind, kObj>) {
			if (obj) obj->release();
		}
		static inline void retain(T* obj, std::integral_constant<Kind, kProtocol>) {
			if (obj) obj->asObject()->retain();
		}
		static inline void release(T* obj, std::integral_constant<Kind, kProtocol>) {
			if (obj) obj->asObject()->release();
		}
	};

	template <typename T>
	struct ObjectTraits: ObjectTraitsBase<T> {};

	/**
	 * @struct ObjectTraitsFrom
	 * @brief Traits for handling different object types with custom retain/release functions.
	 * This struct allows specifying custom retain and release functions for a given type T.
	*/
	template<
		typename T,
		void (*Rel)(T*) = ObjectTraits<T>::Release,
		void (*Ret)(T*) = ObjectTraits<T>::Retain
	>
	struct ObjectTraitsFrom: ObjectTraits<T> {
		inline static void Retain(T* obj) { Ret(obj); }
		inline static void Release(T* obj) { Rel(obj); }
	};

	/**
	 * @method Retain() retain object
	 */
	Qk_EXPORT void Retain(Object* obj); // retain object
	/**
	 * @method Release() release object
	 */
	Qk_EXPORT void Release(Object* obj); // release object

	/**
	 * @method Retainp() retain plus
	*/
	template<typename T>
	inline void Retainp(T*& obj) { ObjectTraits<T>::Retain(obj); }

	template<typename T>
	inline void Retainp(T& obj) {}

	/**
	 * @method Releasep() release plus
	*/
	template<typename T>
	inline void Releasep(T*& obj) {
		ObjectTraits<T>::Release(obj);
		obj = nullptr;
	}

	template<typename T>
	inline void Releasep(T& obj) {}

	/**
	 * @method Retainp() retain plus for atomic pointer
	 */
	template<typename T>
	inline void Retainp(std::atomic<T*>& obj) {
		ObjectTraits<T>::Retain(obj.load()); // retain current value
	}

	/**
	 * @method Releasep() release plus for atomic pointer
	 */
	template<typename T>
	inline void Releasep(std::atomic<T*>& obj) {
		ObjectTraits<T>::Release(obj.exchange(nullptr)); // release and set to nullptr
	}

}
#endif
