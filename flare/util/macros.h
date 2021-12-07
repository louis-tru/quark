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

#ifndef __flare__util__macros__
#define __flare__util__macros__

//  ----------------------------- Compiling environment -----------------------------

#define F_HAS_CPP __cplusplus

#ifdef __EXCEPTIONS
# define F_EXCEPTIONS_SUPPORT 1
#else
# define F_EXCEPTIONS_SUPPORT 0
#endif

#ifndef NULL
# define NULL 0
#endif

#define F_GNUC          0
#define F_CLANG         0
#define F_MSC           0
#define F_ARCH_X86      0
#define F_ARCH_ARM      0
#define F_ARCH_MIPS     0
#define F_ARCH_MIPS64   0
#define F_ARCH_IA32     0
#define F_ARCH_X64      0
#define F_ARCH_ARM64    0
#define F_ARCH_ARMV7    0
#define F_ARCH_32BIT    0
#define F_ARCH_64BIT    0
#define F_APPLE         0
#define F_POSIX         0
#define F_UNIX          0
#define F_LINUX         0
#define F_BSD           0
#define F_CYGWIN        0
#define F_NACL          0
#define F_IOS           0
#define F_OSX           0
#define F_ANDROID       0
#define F_WIN           0
#define F_QNX           0

#if defined(__GNUC__)
# undef F_GNUC
# define F_GNUC       1
#endif

#if defined(__clang__)
# undef F_CLANG
# define F_CLANG       1
#endif

#if defined(_MSC_VER)
# undef F_MSC
# define F_MSC       1
#endif

#if defined(_M_X64) || defined(__x86_64__)
# undef F_ARCH_X86
# define F_ARCH_X86        1
# if defined(__native_client__)
#   undef F_ARCH_IA32
#   define F_ARCH_IA32     1
#   undef F_ARCH_32BIT
#   define F_ARCH_32BIT    1
# else // else __native_client__
#   undef F_ARCH_X64
#   define F_ARCH_X64      1
#   undef F_ARCH_64BIT
#   define F_ARCH_64BIT    1
# endif  // __native_client__

#elif defined(_M_IX86) || defined(__i386__)
# undef F_ARCH_X86
# define F_ARCH_X86        1
# undef F_ARCH_IA32
# define F_ARCH_IA32       1
# undef F_ARCH_32BIT
# define F_ARCH_32BIT      1

#elif defined(__arm64__) || defined(__AARCH64EL__)
# undef F_ARCH_ARM
# define F_ARCH_ARM        1
# undef F_ARCH_ARM64
# define F_ARCH_ARM64      1
# undef F_ARCH_64BIT
# define F_ARCH_64BIT      1

#elif defined(__ARMEL__)
# undef F_ARCH_ARM
# define F_ARCH_ARM        1
# undef F_ARCH_32BIT
# define F_ARCH_32BIT      1

#elif defined(__mips64)
# undef F_ARCH_MIPS
# define F_ARCH_MIPS       1
# undef F_ARCH_MIPS64
# define F_ARCH_MIPS64     1
# undef F_ARCH_64BIT
# define F_ARCH_64BIT      1

#elif defined(__MIPSEL__)
# undef F_ARCH_MIPS
# define F_ARCH_MIPS       1
# undef F_ARCH_32BIT
# define F_ARCH_32BIT      1

#else
# error Host architecture was not detected as supported by flare
#endif

#if defined(__ARM_ARCH_7A__) || \
defined(__ARM_ARCH_7R__) || \
defined(__ARM_ARCH_7__)
# undef F_ARCH_ARMV7
# define F_ARCH_ARMV7  1
#endif

#if defined(__sun)
# undef F_BSD
# define F_BSD        1
# undef F_UNIX
# define F_UNIX       1
#endif

#if defined(__OpenBSD__) || \
defined(__NetBSD__)   || \
defined(__FreeBSD__)  || \
defined(__DragonFly__)
# undef F_POSIX
# define F_POSIX      1
# undef F_BSD
# define F_BSD        1
# undef F_UNIX
# define F_UNIX       1
#endif

#if defined(__APPLE__)
# undef F_APPLE
# define F_APPLE      1
# undef F_POSIX
# define F_POSIX      1
# undef F_BSD
# define F_BSD        1
# undef F_UNIX
# define F_UNIX       1
#endif

#if defined(__CYGWIN__)
# undef F_CYGWIN
# define F_CYGWIN     1
# undef F_POSIX
# define F_POSIX      1
#endif

#if defined(__unix__)
# undef F_UNIX
# define F_UNIX       1
#endif

#if defined(__linux__)
# undef F_LINUX
# define F_LINUX      1
# undef F_POSIX
# define F_POSIX      1
# undef F_UNIX
# define F_UNIX       1
#endif

#if defined(__native_client__)
# undef F_NACL
# define F_NACL       1
# undef F_POSIX
# define F_POSIX      1
#endif

#ifdef __APPLE__
# include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE
# undef F_IOS
# define F_IOS        1
#endif

#if TARGET_OS_MAC && !TARGET_OS_IPHONE
# undef F_OSX
# define F_OSX        1
#endif

#if defined(__ANDROID__)
# undef F_ANDROID
# define F_ANDROID    1
# undef F_POSIX
# define F_POSIX      1
# undef F_LINUX
# define F_LINUX      1
# undef F_UNIX
# define F_UNIX       1
#endif

#if defined(_WINDOWS) || defined(_MSC_VER)
# undef F_WIN
# define F_WIN        1
#endif

#if defined(__QNXNTO__)
# undef F_POSIX
# define F_POSIX 1
# undef F_QNX
# define F_QNX 1
#endif

// ----------------------------- Compiling environment end -----------------------------

// This macro allows to test for the version of the GNU C++ compiler.
// Note that this also applies to compilers that masquerade as GCC,
// for example clang and the Intel C++ compiler for Linux.
// Use like:
//  #if V8_GNUC_PREREQ(4, 3, 1)
//   ...
//  #endif
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
# define F_GNUC_PREREQ(major, minor, patchlevel) \
((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= \
((major) * 10000 + (minor) * 100 + (patchlevel)))
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
# define F_GNUC_PREREQ(major, minor, patchlevel) \
((__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= \
((major) * 10000 + (minor) * 100 + (patchlevel)))
#else
# define F_GNUC_PREREQ(major, minor, patchlevel) 0
#endif

#if F_CLANG
# define F_HAS_ATTRIBUTE_ALWAYS_INLINE (__has_attribute(always_inline))
# define F_HAS_ATTRIBUTE_VISIBILITY (__has_attribute(visibility))
#elif F_GNUC
// always_inline is available in gcc 4.0 but not very reliable until 4.4.
// Works around "sorry, unimplemented: inlining failed" build errors with
// older compilers.
# define F_HAS_ATTRIBUTE_ALWAYS_INLINE (F_GNUC_PREREQ(4, 4, 0))
# define F_HAS_ATTRIBUTE_VISIBILITY (F_GNUC_PREREQ(4, 3, 0))
#elif F_MSC
# define F_HAS_FORCE_INLINE 1
#endif

// ------------------------------------------------------------------

#ifndef F_MORE_LOG
# define F_MORE_LOG 0
#endif

#if F_MORE_LOG
# undef  F_MEMORY_TRACE_MARK
# define F_MEMORY_TRACE_MARK 1
#endif

#ifndef F_STRICT_ASSERT
# define  F_STRICT_ASSERT 1
#endif

#if DEBUG || F_STRICT_ASSERT
# define F_ASSERT(cond, ...) if(!(cond)) flare::fatal(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
# define F_ASSERT(cond, ...) ((void)0)
#endif

#define F_DEFINE_INLINE_CLASS(Inl) public: class Inl; friend class Inl; private:
#define F_DEFINE_INLINE_MEMBERS(cls, Inl) \
	static F_INLINE cls::Inl* Inl##_##cls(cls* self) { \
		return reinterpret_cast<cls::Inl*>(self); \
	} class cls::Inl: public cls

#define F_LOG(msg, ...)      flare::console::log(msg, ##__VA_ARGS__)
#define F_WARN(msg, ...)     flare::console::warn(msg, ##__VA_ARGS__)
#define F_ERR(msg, ...)      flare::console::error(msg, ##__VA_ARGS__)
#define F_FATAL(...)         flare::fatal(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define F_UNIMPLEMENTED(...)    F_FATAL("Unimplemented code, %s", ##__VA_ARGS__)
#define F_UNREACHABLE(...)      F_FATAL("Unreachable code, %s", ##__VA_ARGS__)
#define F_MIN(A, B)          ((A) < (B) ? (A) : (B))
#define F_MAX(A, B)          ((A) > (B) ? (A) : (B))

#if DEBUG || F_MORE_LOG
# define F_DEBUG F_LOG
#else
# define F_DEBUG(msg, ...) ((void)0)
#endif

// ------------------------------------------------------------------

#if F_MSC

#ifdef F_BUILDING_SHARED
# define F_EXPORT __declspec(dllexport)
#elif F_USING_SHARED
# define F_EXPORT __declspec(dllimport)
#else
# define F_EXPORT
#endif  // F_BUILDING_SHARED

#else  // F_MSC

// Setup for Linux shared library export.
#if F_HAS_ATTRIBUTE_VISIBILITY
# ifdef F_BUILDING_SHARED
#  define F_EXPORT __attribute__((visibility("default")))
# else
#  define F_EXPORT
# endif
#else
# define F_EXPORT
#endif

#endif // F_MSC


#if F_MSC
	#pragma section(".CRT$XCU", read)
	# define F_INIT_BLOCK(fn)	\
	extern void __cdecl fn(void);	\
	__declspec(dllexport, allocate(".CRT$XCU"))	\
	void (__cdecl*fn##_)(void) = fn;	\
	extern void __cdecl fn(void)
#else // F_MSC
	# define F_INIT_BLOCK(fn)	\
	extern __attribute__((constructor)) void __##fn(void)
#endif

// A macro used to make better inlining. Don't bother for debug builds.
//
#if F_HAS_ATTRIBUTE_ALWAYS_INLINE && !DEBUG
# define F_INLINE inline __attribute__((always_inline))
#elif F_HAS_FORCE_INLINE && !DEBUG
# define F_INLINE __forceinline
#else
# define F_INLINE inline
#endif
#undef F_HAS_ATTRIBUTE_ALWAYS_INLINE
#undef F_HAS_FORCE_INLINE

#define F_HIDDEN_COPY_CONSTRUCTOR(T)  private: T(const T& t) = delete;
#define F_HIDDEN_ASSIGN_OPERATOR(T)   private: T& operator=(const T& t) = delete;
#define F_HIDDEN_ALL_COPY(T)	\
	private: T(const T& t) = delete;	\
	private: T& operator=(const T& t) = delete;
#define F_HIDDEN_HEAP_ALLOC() \
	private: static void* operator new(size_t size) = delete; \
	private: static void operator delete(void*, size_t) = delete

#define F_DEFINE_PROP_READ(type, name) \
	private: type _##name; \
	public: inline type name () const { return _##name; } \

#define F_DEFINE_PROP(type, name) \
	F_DEFINE_PROP_READ(type, name) \
	void set_##name (type val); \

// Helper macros end
// -----------------------------------------------------------------------------

#endif
