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

#ifndef __ftr__util__object__
#define __ftr__util__object__

#include "./macros.h"
#include <atomic>

#ifndef FX_MEMORY_TRACE_MARK
# define FX_MEMORY_TRACE_MARK 0
#endif

#if FX_MEMORY_TRACE_MARK
# include <vector>
#endif

namespace ftr {

	#define FX_DEFAULT_ALLOCATOR() \
		static void* operator new(std::size_t size) { return ::operator new(size); } \
		static void  operator delete(void* p) { ::operator delete(p); } \
		virtual void release() { static_assert(!Traits::is_reference, ""); ::delete this; }

	#ifndef FX_MIN_CAPACITY
	# define FX_MIN_CAPACITY (8)
	#endif

	// -------------------------------------------------------

	class ObjectTraits;
	class ReferenceTraits;
	class ProtocolTraits;
	
	struct MemoryAllocator {
		static void* alloc(uint32_t size);
		static void  free(void* ptr);
		static void* realloc(void* ptr, uint32_t size);
		// auto alloc Memory
		static void* aalloc(void* ptr, uint32_t size, uint32_t* size_out, uint32_t size_of);
	};
	
	template<typename T = char, typename A = MemoryAllocator> class ArrayString;
	
	typedef       ArrayString<char, MemoryAllocator> String;
	typedef const ArrayString<char, MemoryAllocator> cString;

	/**
	* @class Object
	*/
	class FX_EXPORT Object {
		public:
			typedef ObjectTraits Traits;
			virtual bool is_reference() const;
			virtual bool retain();
			virtual void release(); // "new" method alloc can call，Otherwise, fatal exception will be caused
			virtual ArrayString<char, MemoryAllocator> to_string() const;
			static void* operator new(std::size_t size);
			static void* operator new(std::size_t size, void* p);
			static void  operator delete(void* p);
			static void set_object_allocator(
				void* (*alloc)(size_t size) = nullptr,
				void (*release)(Object* obj) = nullptr, void (*retain)(Object* obj) = nullptr
			);
		#if FX_MEMORY_TRACE_MARK
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
	};

	/**
	* @class Reference
	*/
	class FX_EXPORT Reference: public Object {
		public:
			typedef ReferenceTraits Traits;
			inline Reference(): _ref_count(0) {}
			inline Reference(const Reference& ref): _ref_count(0) {}
			inline Reference& operator=(const Reference& ref) { return *this; }
			virtual ~Reference();
			virtual bool retain();
			virtual void release();
			virtual bool is_reference() const;
			inline int ref_count() const { return _ref_count; }
		protected:
			std::atomic_int _ref_count;
	};

	/**
	* @class Protocol
	*/
	class FX_EXPORT Protocol {
		public:
			typedef ProtocolTraits Traits;
			virtual Object* to_object() = 0;
	};

	/**
	* @class ObjectTraits
	*/
	class FX_EXPORT ObjectTraits {
		public:
			inline static bool Retain(Object* obj) { return obj ? obj->retain() : 0; }
			inline static void Release(Object* obj) { if (obj) obj->release(); }
			static constexpr bool is_reference = false;
			static constexpr bool is_object = true;
	};

	/**
	* @class ReferenceTraits
	*/
	class FX_EXPORT ReferenceTraits: public ObjectTraits {
		public:
			static constexpr bool is_reference = true;
	};

	/**
	* @class ProtocolTraits
	*/
	class FX_EXPORT ProtocolTraits {
		public:
			template<class T> inline static bool Retain(T* obj) {
				return obj ? obj->to_object()->retain() : 0;
			}
			template<class T> inline static void Release(T* obj) {
				if (obj) obj->to_object()->release();
			}
			static constexpr bool is_reference = false;
	};

	typedef ProtocolTraits InterfaceTraits;

	/**
	* @class NonObjectTraits
	*/
	class FX_EXPORT NonObjectTraits {
		public:
			template<class T> inline static bool Retain(T* obj) {
				/* Non referential pairs need not be Retain */ return 0;
			}
			template<class T> inline static void Release(T* obj) { delete obj; }
			static constexpr bool is_reference = false;
	};

	FX_EXPORT void fatal(const char* file, uint32_t line, const char* func, const char* msg = 0, ...);

	FX_EXPORT bool Retain(Object* obj);
	FX_EXPORT void Release(Object* obj);

	template<class T, typename... Args>
	inline T* New(Args... args) {
		return new T(args...);
	}

	template<class T, typename... Args>
	inline T* NewRetain(Args... args) {
		T* r = new T(args...);
		r->retain();
		return r;
	}

}
#endif
