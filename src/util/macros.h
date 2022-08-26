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

#ifndef __noug__util__macros__
#define __noug__util__macros__

//  ----------------------------- Compiling environment -----------------------------

#define N_HAS_CPP __cplusplus

#ifdef __EXCEPTIONS
# define N_EXCEPTIONS_SUPPORT 1
#else
# define N_EXCEPTIONS_SUPPORT 0
#endif

#ifndef NULL
# define NULL 0
#endif

#define N_GNUC          0
#define N_CLANG         0
#define N_MSC           0
#define N_ARCH_X86      0
#define N_ARCH_ARM      0
#define N_ARCH_MIPS     0
#define N_ARCH_MIPS64   0
#define N_ARCH_IA32     0
#define N_ARCH_X64      0
#define N_ARCH_ARM64    0
#define N_ARCH_ARMV7    0
#define N_ARCH_32BIT    0
#define N_ARCH_64BIT    0
#define N_APPLE         0
#define N_POSIX         0
#define N_UNIX          0
#define N_LINUX         0
#define N_BSD           0
#define N_CYGWIN        0
#define N_NACL          0
#define N_IOS           0
#define N_OSX           0
#define N_ANDROID       0
#define N_WIN           0
#define N_QNX           0

#if defined(__GNUC__)
# undef N_GNUC
# define N_GNUC       1
#endif

#if defined(__clang__)
# undef N_CLANG
# define N_CLANG       1
#endif

#if defined(_MSC_VER)
# undef N_MSC
# define N_MSC       1
#endif

#if defined(_M_X64) || defined(__x86_64__)
# undef N_ARCH_X86
# define N_ARCH_X86        1
# if defined(__native_client__)
#   undef N_ARCH_IA32
#   define N_ARCH_IA32     1
#   undef N_ARCH_32BIT
#   define N_ARCH_32BIT    1
# else // else __native_client__
#   undef N_ARCH_X64
#   define N_ARCH_X64      1
#   undef N_ARCH_64BIT
#   define N_ARCH_64BIT    1
# endif  // __native_client__

#elif defined(_M_IX86) || defined(__i386__)
# undef N_ARCH_X86
# define N_ARCH_X86        1
# undef N_ARCH_IA32
# define N_ARCH_IA32       1
# undef N_ARCH_32BIT
# define N_ARCH_32BIT      1

#elif defined(__arm64__) || defined(__AARCH64EL__)
# undef N_ARCH_ARM
# define N_ARCH_ARM        1
# undef N_ARCH_ARM64
# define N_ARCH_ARM64      1
# undef N_ARCH_64BIT
# define N_ARCH_64BIT      1

#elif defined(__ARMEL__)
# undef N_ARCH_ARM
# define N_ARCH_ARM        1
# undef N_ARCH_32BIT
# define N_ARCH_32BIT      1

#elif defined(__mips64)
# undef N_ARCH_MIPS
# define N_ARCH_MIPS       1
# undef N_ARCH_MIPS64
# define N_ARCH_MIPS64     1
# undef N_ARCH_64BIT
# define N_ARCH_64BIT      1

#elif defined(__MIPSEL__)
# undef N_ARCH_MIPS
# define N_ARCH_MIPS       1
# undef N_ARCH_32BIT
# define N_ARCH_32BIT      1

#else
# error Host architecture was not detected as supported by noug
#endif

#if defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7__)
# undef N_ARCH_ARMV7
# define N_ARCH_ARMV7  1
#endif

#if defined(__sun)
# undef N_BSD
# define N_BSD        1
# undef N_UNIX
# define N_UNIX       1
#endif

#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
# undef N_POSIX
# define N_POSIX      1
# undef N_BSD
# define N_BSD        1
# undef N_UNIX
# define N_UNIX       1
#endif

#if defined(__APPLE__)
# undef N_APPLE
# define N_APPLE      1
# undef N_POSIX
# define N_POSIX      1
# undef N_BSD
# define N_BSD        1
# undef N_UNIX
# define N_UNIX       1
#endif

#if defined(__CYGWIN__)
# undef N_CYGWIN
# define N_CYGWIN     1
# undef N_POSIX
# define N_POSIX      1
#endif

#if defined(__unix__)
# undef N_UNIX
# define N_UNIX       1
#endif

#if defined(__linux__)
# undef N_LINUX
# define N_LINUX      1
# undef N_POSIX
# define N_POSIX      1
# undef N_UNIX
# define N_UNIX       1
#endif

#if defined(__native_client__)
# undef N_NACL
# define N_NACL       1
# undef N_POSIX
# define N_POSIX      1
#endif

#ifdef __APPLE__
# include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE
# undef N_IOS
# define N_IOS        1
#endif

#if TARGET_OS_MAC && !TARGET_OS_IPHONE
# undef N_OSX
# define N_OSX        1
#endif

#if defined(__ANDROID__)
# undef N_ANDROID
# define N_ANDROID    1
# undef N_POSIX
# define N_POSIX      1
# undef N_LINUX
# define N_LINUX      1
# undef N_UNIX
# define N_UNIX       1
#endif

#if defined(_WINDOWS) || defined(_MSC_VER)
# undef N_WIN
# define N_WIN        1
#endif

#if defined(__QNXNTO__)
# undef N_POSIX
# define N_POSIX 1
# undef N_QNX
# define N_QNX 1
#endif

// ----------------------------- Compiling environment end -----------------------------

#ifndef N_MORE_LOG
# define N_MORE_LOG 0
#endif

#if N_MORE_LOG
# undef  N_MEMORY_TRACE_MARK
# define N_MEMORY_TRACE_MARK 1
#endif

#ifndef N_STRICT_ASSERT
# define  N_STRICT_ASSERT 0
#endif

#if DEBUG || N_STRICT_ASSERT
# define N_Asset(cond, ...) if(!(cond)) noug::fatal(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
# define N_Asset(cond, ...) ((void)0)
#endif

#define N_DEFINE_INLINE_CLASS(Inl) public: class Inl; friend class Inl; private:
#define N_DEFINE_INLINE_MEMBERS(cls, Inl) \
	static N_INLINE cls::Inl* Inl##_##cls(cls* self) { \
		return reinterpret_cast<cls::Inl*>(self); \
	} class cls::Inl: public cls

#define N_LOG(msg, ...)      noug::console::log(msg, ##__VA_ARGS__)
#define N_WARN(msg, ...)     noug::console::warn(msg, ##__VA_ARGS__)
#define N_ERR(msg, ...)      noug::console::error(msg, ##__VA_ARGS__)
#define N_FATAL(...)         noug::fatal(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define N_UNIMPLEMENTED(...)    N_FATAL("Unimplemented code, %s", ##__VA_ARGS__)
#define N_UNREACHABLE(...)      N_FATAL("Unreachable code, %s", ##__VA_ARGS__)
#define N_MIN(A, B)          ((A) < (B) ? (A) : (B))
#define N_MAX(A, B)          ((A) > (B) ? (A) : (B))

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG || N_MORE_LOG
# define N_DEBUG N_LOG
#else
# define N_DEBUG(msg, ...) ((void)0)
#endif

// ------------------------------------------------------------------

// This macro allows to test for the version of the GNU C++ compiler.
// Note that this also applies to compilers that masquerade as GCC,
// for example clang and the Intel C++ compiler for Linux.
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
# define N_GNUC_PREREQ(major, minor, patchlevel) \
	((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= \
	((major) * 10000 + (minor) * 100 + (patchlevel)))
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
# define N_GNUC_PREREQ(major, minor, patchlevel) \
	((__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= \
	((major) * 10000 + (minor) * 100 + (patchlevel)))
#else
# define N_GNUC_PREREQ(major, minor, patchlevel) 0
#endif

#if N_CLANG
# define N_HAS_ATTRIBUTE_ALWAYS_INLINE (__has_attribute(always_inline))
# define N_HAS_ATTRIBUTE_VISIBILITY (__has_attribute(visibility))
#elif N_GNUC
// always_inline is available in gcc 4.0 but not very reliable until 4.4.
// Works around "sorry, unimplemented: inlining failed" build errors with
// older compilers.
# define N_HAS_ATTRIBUTE_ALWAYS_INLINE (N_GNUC_PREREQ(4, 4, 0))
# define N_HAS_ATTRIBUTE_VISIBILITY (N_GNUC_PREREQ(4, 3, 0))
#elif N_MSC
# define N_HAS_FORCE_INLINE 1
#endif

// ------------------------------------------------------------------

#if N_MSC

#ifdef N_BUILDING_SHARED
# define N_EXPORT __declspec(dllexport)
#elif N_USING_SHARED
# define N_EXPORT __declspec(dllimport)
#else
# define N_EXPORT
#endif  // N_BUILDING_SHARED

#else  // N_MSC

// Setup for Linux shared library export.
#if N_HAS_ATTRIBUTE_VISIBILITY
# ifdef N_BUILDING_SHARED
#  define N_EXPORT __attribute__((visibility("default")))
# else
#  define N_EXPORT
# endif
#else
# define N_EXPORT
#endif

#endif // N_MSC


#if N_MSC
	#pragma section(".CRT$XCU", read)
	# define N_INIT_BLOCK(fn)	\
	extern void __cdecl fn(void);	\
	__declspec(dllexport, allocate(".CRT$XCU"))	\
	void (__cdecl*fn##_)(void) = fn;	\
	extern void __cdecl fn(void)
#else // N_MSC
	# define N_INIT_BLOCK(fn)	\
	extern __attribute__((constructor)) void __##fn(void)
#endif

// A macro used to make better inlining. Don't bother for debug builds.
//
#if N_HAS_ATTRIBUTE_ALWAYS_INLINE && !DEBUG
# define N_INLINE inline __attribute__((always_inline))
#elif N_HAS_FORCE_INLINE && !DEBUG
# define N_INLINE __forceinline
#else
# define N_INLINE inline
#endif
#undef N_HAS_ATTRIBUTE_ALWAYS_INLINE
#undef N_HAS_FORCE_INLINE

#define N_HIDDEN_COPY_CONSTRUCTOR(T)  private: T(const T& t) = delete;
#define N_HIDDEN_ASSIGN_OPERATOR(T)   private: T& operator=(const T& t) = delete;
#define N_HIDDEN_ALL_COPY(T)	\
	private: T(const T& t) = delete;	\
	private: T& operator=(const T& t) = delete;
#define N_HIDDEN_HEAP_ALLOC() \
	private: static void* operator new(size_t size) = delete; \
	private: static void operator delete(void*, size_t) = delete

#define N_Define_Prop_Modifier const
#define N_Define_Prop_ModifierNoConst

#define N_Define_Prop_Acc_Get(type, name, ...) \
	type name () N_Define_Prop_Modifier##__VA_ARGS__; public:

#define N_Define_Prop_Acc(type, name, ...) \
	void set_##name (type val); \
	N_Define_Prop_Acc_Get(type, name, ##__VA_ARGS__) \

#define N_Define_Prop_Get(type, name, ...) \
	inline type name () N_Define_Prop_Modifier##__VA_ARGS__ { return _##name; } \
	private: type _##name; public:\

#define N_Define_Prop(type, name, ...) \
	void set_##name (type val); \
	N_Define_Prop_Get(type, name, ##__VA_ARGS__) \

#define N_Define_Class(Name) class Name;
#define N_Define_Visitor_Visit(N) virtual void visit##N(N *v) = 0;
#define N_Define_Visitor(Name, Each) \
	Each(N_Define_Class); \
	class Name##Visitor { \
	public: \
		virtual int flags() = 0; \
		Each(N_Define_Visitor_Visit); \
}

// Helper macros end
// -----------------------------------------------------------------------------

#endif
