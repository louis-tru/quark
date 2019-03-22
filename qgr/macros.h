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

#ifndef __qgr__utils__macros__
#define __qgr__utils__macros__

#include "env.h"

// This macro allows to test for the version of the GNU C++ compiler.
// Note that this also applies to compilers that masquerade as GCC,
// for example clang and the Intel C++ compiler for Linux.
// Use like:
//  #if V8_GNUC_PREREQ(4, 3, 1)
//   ...
//  #endif
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
# define XX_GNUC_PREREQ(major, minor, patchlevel) \
((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= \
((major) * 10000 + (minor) * 100 + (patchlevel)))
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
# define XX_GNUC_PREREQ(major, minor, patchlevel) \
((__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= \
((major) * 10000 + (minor) * 100 + (patchlevel)))
#else
# define XX_GNUC_PREREQ(major, minor, patchlevel) 0
#endif

#if XX_CLANG
# define XX_HAS_ATTRIBUTE_ALWAYS_INLINE (__has_attribute(always_inline))
# define XX_HAS_ATTRIBUTE_VISIBILITY (__has_attribute(visibility))
#elif XX_GNUC
// always_inline is available in gcc 4.0 but not very reliable until 4.4.
// Works around "sorry, unimplemented: inlining failed" build errors with
// older compilers.
# define XX_HAS_ATTRIBUTE_ALWAYS_INLINE (XX_GNUC_PREREQ(4, 4, 0))
# define XX_HAS_ATTRIBUTE_VISIBILITY (XX_GNUC_PREREQ(4, 3, 0))
#elif XX_MSC
# define XX_HAS_FORCE_INLINE 1
#endif

// ------------------------------------------------------------------

#define XX_CHECK(cond, ...)	if(!(cond)) qgr::fatal(__FILE__, __LINE__, __func__, ##__VA_ARGS__)

#ifndef XX_MORE_LOG
# define XX_MORE_LOG 0
#endif

#if XX_MORE_LOG
# undef  DEBUG
# define DEBUG 1
# undef  XX_MEMORY_TRACE_MARK
# define XX_MEMORY_TRACE_MARK 1
#endif

#if DEBUG
# define XX_ASSERT(cond, ...) XX_CHECK(cond, ##__VA_ARGS__)
#else
# define XX_ASSERT(cond, ...) ((void)0)
#endif

#if DEBUG || XX_MORE_LOG
# define XX_DEBUG(msg, ...) qgr::console::log(msg, ##__VA_ARGS__)
#else
# define XX_DEBUG(msg, ...) ((void)0)
#endif

#define DLOG XX_DEBUG

#define XX_NS(name) namespace name {
#define XX_END };

#define XX_DEFINE_INLINE_CLASS(Inl) public: class Inl; friend class Inl; private:
#define XX_DEFINE_INLINE_MEMBERS(cls, Inl) \
	static XX_INLINE cls::Inl* Inl##_##cls(cls* self) { \
		return reinterpret_cast<cls::Inl*>(self); \
	} class cls::Inl: public cls

#define XX_LOG(msg, ...)      qgr::console::log(msg, ##__VA_ARGS__)
#define XX_WARN(msg, ...)     qgr::console::warn(msg, ##__VA_ARGS__)
#define XX_ERR(msg, ...)      qgr::console::error(msg, ##__VA_ARGS__)
#define XX_FATAL(...)         qgr::fatal(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define XX_UNIMPLEMENTED()    XX_FATAL("Unimplemented code")
#define XX_UNREACHABLE()      XX_FATAL("Unreachable code")
#define XX_MIN(A, B)          ((A) < (B) ? (A) : (B))
#define XX_MAX(A, B)          ((A) > (B) ? (A) : (B))
#define LOG XX_LOG

// ------------------------------------------------------------------

#if XX_MSC

#ifdef XX_BUILDING_SHARED
# define XX_EXPORT __declspec(dllexport)
#elif XX_USING_SHARED
# define XX_EXPORT __declspec(dllimport)
#else
# define XX_EXPORT
#endif  // XX_BUILDING_SHARED

#else  // XX_MSC

// Setup for Linux shared library export.
#if XX_HAS_ATTRIBUTE_VISIBILITY
# ifdef XX_BUILDING_SHARED
#  define XX_EXPORT __attribute__ ((visibility("default")))
# else
#  define XX_EXPORT
# endif
#else
# define XX_EXPORT
#endif

#endif // XX_MSC


#if XX_MSC
# pragma section(".CRT$XCU", read)
# define XX_INIT_BLOCK(fn)	\
extern void __cdecl fn(void);	\
__declspec(dllexport, allocate(".CRT$XCU"))	\
void (__cdecl*fn ## _)(void) = fn;	\
extern void __cdecl fn(void)
#else // XX_MSC
# define XX_INIT_BLOCK(fn)	\
extern __attribute__((constructor)) void __##fn(void)
#endif

// A macro used to make better inlining. Don't bother for debug builds.
//
#if XX_HAS_ATTRIBUTE_ALWAYS_INLINE && !DEBUG
# define XX_INLINE inline __attribute__((always_inline))
#elif XX_HAS_FORCE_INLINE && !DEBUG
# define XX_INLINE __forceinline
#else
# define XX_INLINE inline
#endif
#undef XX_HAS_ATTRIBUTE_ALWAYS_INLINE
#undef XX_HAS_FORCE_INLINE

#define XX_HIDDEN_COPY_CONSTRUCTOR(T)  private: T(const T& t) = delete;
#define XX_HIDDEN_ASSIGN_OPERATOR(T)   private: T& operator=(const T& t) = delete;
#define XX_HIDDEN_ALL_COPY(T)	\
	private: T(const T& t) = delete;	\
	private: T& operator=(const T& t) = delete;
#define XX_HIDDEN_HEAP_ALLOC() \
	private: static void* operator new(size_t size) = delete; \
	private: static void operator delete(void*, size_t) = delete

// Helper macros end
// -----------------------------------------------------------------------------

#endif
