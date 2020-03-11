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

#ifndef __ngui__utils__macros__
#define __ngui__utils__macros__

#include "env.h"

// This macro allows to test for the version of the GNU C++ compiler.
// Note that this also applies to compilers that masquerade as GCC,
// for example clang and the Intel C++ compiler for Linux.
// Use like:
//  #if V8_GNUC_PREREQ(4, 3, 1)
//   ...
//  #endif
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
# define NX_GNUC_PREREQ(major, minor, patchlevel) \
((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= \
((major) * 10000 + (minor) * 100 + (patchlevel)))
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
# define NX_GNUC_PREREQ(major, minor, patchlevel) \
((__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= \
((major) * 10000 + (minor) * 100 + (patchlevel)))
#else
# define NX_GNUC_PREREQ(major, minor, patchlevel) 0
#endif

#if NX_CLANG
# define NX_HAS_ATTRIBUTE_ALWAYS_INLINE (__has_attribute(always_inline))
# define NX_HAS_ATTRIBUTE_VISIBILITY (__has_attribute(visibility))
#elif NX_GNUC
// always_inline is available in gcc 4.0 but not very reliable until 4.4.
// Works around "sorry, unimplemented: inlining failed" build errors with
// older compilers.
# define NX_HAS_ATTRIBUTE_ALWAYS_INLINE (NX_GNUC_PREREQ(4, 4, 0))
# define NX_HAS_ATTRIBUTE_VISIBILITY (NX_GNUC_PREREQ(4, 3, 0))
#elif NX_MSC
# define NX_HAS_FORCE_INLINE 1
#endif

// ------------------------------------------------------------------

#define NX_CHECK(cond, ...)	if(!(cond)) ngui::fatal(__FILE__, __LINE__, __func__, ##__VA_ARGS__)

#ifndef NX_MORE_LOG
# define NX_MORE_LOG 0
#endif

#if NX_MORE_LOG
# undef  DEBUG
# define DEBUG 1
# undef  NX_MEMORY_TRACE_MARK
# define NX_MEMORY_TRACE_MARK 1
#endif

#if DEBUG
# define NX_ASSERT(cond, ...) NX_CHECK(cond, ##__VA_ARGS__)
#else
# define NX_ASSERT(cond, ...) ((void)0)
#endif

#if DEBUG || NX_MORE_LOG
# define NX_DEBUG(msg, ...) ngui::console::log(msg, ##__VA_ARGS__)
#else
# define NX_DEBUG(msg, ...) ((void)0)
#endif

#define DLOG NX_DEBUG

#define NX_NS(name) namespace name {
#define NX_END };

#define NX_DEFINE_INLINE_CLASS(Inl) public: class Inl; friend class Inl; private:
#define NX_DEFINE_INLINE_MEMBERS(cls, Inl) \
	static NX_INLINE cls::Inl* Inl##_##cls(cls* self) { \
		return reinterpret_cast<cls::Inl*>(self); \
	} class cls::Inl: public cls

#define NX_LOG(msg, ...)      ngui::console::log(msg, ##__VA_ARGS__)
#define NX_WARN(msg, ...)     ngui::console::warn(msg, ##__VA_ARGS__)
#define NX_ERR(msg, ...)      ngui::console::error(msg, ##__VA_ARGS__)
#define NX_FATAL(...)         ngui::fatal(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define NX_UNIMPLEMENTED()    NX_FATAL("Unimplemented code")
#define NX_UNREACHABLE()      NX_FATAL("Unreachable code")
#define NX_MIN(A, B)          ((A) < (B) ? (A) : (B))
#define NX_MAX(A, B)          ((A) > (B) ? (A) : (B))
#define LOG NX_LOG

// ------------------------------------------------------------------

#if NX_MSC

#ifdef NX_BUILDING_SHARED
# define NX_EXPORT __declspec(dllexport)
#elif NX_USING_SHARED
# define NX_EXPORT __declspec(dllimport)
#else
# define NX_EXPORT
#endif  // NX_BUILDING_SHARED

#else  // NX_MSC

// Setup for Linux shared library export.
#if NX_HAS_ATTRIBUTE_VISIBILITY
// # ifdef NX_BUILDING_SHARED
#  define NX_EXPORT __attribute__ ((visibility("default")))
// # else
// #  define NX_EXPORT
// # endif
#else
# define NX_EXPORT
#endif

#endif // NX_MSC


#if NX_MSC
# pragma section(".CRT$XCU", read)
# define NX_INIT_BLOCK(fn)	\
extern void __cdecl fn(void);	\
__declspec(dllexport, allocate(".CRT$XCU"))	\
void (__cdecl*fn ## _)(void) = fn;	\
extern void __cdecl fn(void)
#else // NX_MSC
# define NX_INIT_BLOCK(fn)	\
extern __attribute__((constructor)) void __##fn(void)
#endif

// A macro used to make better inlining. Don't bother for debug builds.
//
#if NX_HAS_ATTRIBUTE_ALWAYS_INLINE && !DEBUG
# define NX_INLINE inline __attribute__((always_inline))
#elif NX_HAS_FORCE_INLINE && !DEBUG
# define NX_INLINE __forceinline
#else
# define NX_INLINE inline
#endif
#undef NX_HAS_ATTRIBUTE_ALWAYS_INLINE
#undef NX_HAS_FORCE_INLINE

#define NX_HIDDEN_COPY_CONSTRUCTOR(T)  private: T(const T& t) = delete;
#define NX_HIDDEN_ASSIGN_OPERATOR(T)   private: T& operator=(const T& t) = delete;
#define NX_HIDDEN_ALL_COPY(T)	\
	private: T(const T& t) = delete;	\
	private: T& operator=(const T& t) = delete;
#define NX_HIDDEN_HEAP_ALLOC() \
	private: static void* operator new(size_t size) = delete; \
	private: static void operator delete(void*, size_t) = delete

// Helper macros end
// -----------------------------------------------------------------------------

#endif
