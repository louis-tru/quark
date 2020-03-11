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

#ifndef __ngui__utils__object__
#define __ngui__utils__object__

#include "nxkit/macros.h"
#include <atomic>

#ifndef NX_MEMORY_TRACE_MARK
# define NX_MEMORY_TRACE_MARK 0
#endif

#if NX_MEMORY_TRACE_MARK
# include <vector>
#endif

/**
 * @ns ngui
 */

NX_NS(ngui)

typedef const char cchar;
typedef unsigned char byte;
typedef const unsigned char cbyte;
typedef signed char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef unsigned int uint;
typedef int int32;
typedef unsigned int uint32;

#if NX_ARCH_64BIT
typedef long int int64;
typedef unsigned long int uint64;
#else
typedef long long int int64;
typedef unsigned long long int uint64;
#endif

typedef std::size_t size_t;

#define NX_DEFAULT_ALLOCATOR()  \
static void* operator new(std::size_t size) { return ::operator new(size); } \
static void  operator delete(void* p) { ::operator delete(p); } \
virtual void release() { static_assert(!Traits::is_reference, ""); ::delete this; }

// -------------------------------------------------------

class Object;
class Reference;

struct NX_EXPORT DefaultAllocator {
	static void* alloc(size_t size);
	static void* realloc(void* ptr, size_t size);
	static void free(void* ptr);
};

struct ObjectAllocator {
	void* (*alloc)(size_t size);
	void (*release)(Object* obj);
	void (*retain)(Object* obj);
};

template<typename T, class A = DefaultAllocator> class Container;
template<class T, class Container = Container<T>> class Array;
template<class Item, class ItemAllocator = DefaultAllocator> class List;
template<typename Char = char, class Container = Container<Char>> class BasicString;
typedef BasicString<> String;
typedef const String cString;
typedef BasicString<uint16, Container<uint16>> Ucs2String;
typedef const Ucs2String cUcs2String;
typedef BasicString<uint32, Container<uint32>> Ucs4String;
typedef const Ucs4String cUcs4String;

NX_EXPORT void set_object_allocator(ObjectAllocator* allocator = nullptr);
NX_EXPORT bool Retain(Object* obj);
NX_EXPORT void Release(Object* obj);

template<class T, typename... Args>
NX_INLINE T* New(Args... args) { return new T(args...); }

template<class T, typename... Args>
NX_INLINE T* NewRetain(Args... args) { 
	T* r = new T(args...); r->retain(); return r; 
}

class DefaultTraits;
class ReferenceTraits;
class ProtocolTraits;

/**
 * @class Object
 */
class NX_EXPORT Object {
 public:
	typedef DefaultTraits Traits;
	virtual String to_string() const;
	virtual bool is_reference() const;
	virtual bool retain();
  // "new" method alloc can call，Otherwise, fatal exception will be caused
	virtual void release();
	static void* operator new(std::size_t size);
	static void* operator new(std::size_t size, void* p);
	static void  operator delete(void* p);
#if NX_MEMORY_TRACE_MARK
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
class NX_EXPORT Reference: public Object {
 public:
	typedef ReferenceTraits Traits;
	inline Reference(): m_ref_count(0) { }
	inline Reference(const Reference& ref): m_ref_count(0) { }
	inline Reference& operator=(const Reference& ref) { return *this; }
	virtual ~Reference();
	virtual bool retain();
	virtual void release();
	virtual bool is_reference() const;
	inline int ref_count() const { return m_ref_count; }
 protected:
	std::atomic_int m_ref_count;
};

/**
 * @class Protocol
 */
class NX_EXPORT Protocol {
 public:
	typedef ProtocolTraits Traits;
	virtual Object* to_object() = 0;
};

/**
 * @class DefaultTraits
 */
class NX_EXPORT DefaultTraits {
 public:
	inline static bool Retain(Object* obj) { return obj ? obj->retain() : 0; }
	inline static void Release(Object* obj) { if (obj) obj->release(); }
	static constexpr bool is_reference = false;
	static constexpr bool is_object = true;
};

/**
 * @class ReferenceTraits
 */
class NX_EXPORT ReferenceTraits: public DefaultTraits {
 public:
	static constexpr bool is_reference = true;
};

/**
 * @class ProtocolTraits
 */
class NX_EXPORT ProtocolTraits {
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
class NX_EXPORT NonObjectTraits {
 public:
	template<class T> inline static bool Retain(T* obj) {
		/* Non referential pairs need not be Retain */ return 0;
	}
	template<class T> inline static void Release(T* obj) { delete obj; }
	static constexpr bool is_reference = false;
};

NX_END
#endif
