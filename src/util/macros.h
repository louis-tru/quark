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

#ifndef __quark__util__macros__
#define __quark__util__macros__

#if defined(_M_X64) || defined(__x86_64__)
# define Qk_ARCH_X86 1
# ifndef __native_client__
#  define Qk_ARCH_X86_64 1
#  define Qk_ARCH_64BIT 1
# endif
#elif defined(_M_IX86) || defined(__i386__)
# define Qk_ARCH_X86 1
#elif defined(__arm64__) || defined(__AARCH64EL__)
# define Qk_ARCH_ARM 1
# define Qk_ARCH_ARM64 1
# define Qk_ARCH_64BIT 1
#elif defined(__ARMEL__)
# define Qk_ARCH_ARM 1
#elif defined(__mips64)
# define Qk_ARCH_MIPS 1
# define Qk_ARCH_MIPS64 1
# define Qk_ARCH_64BIT 1
#elif defined(__MIPSEL__)
# define Qk_ARCH_MIPS 1
#else
# error Host architecture was not detected as supported by quark
#endif

#ifndef Qk_ARCH_64BIT
# define Qk_ARCH_64BIT 0
#endif

#if defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7__)
# define Qk_ARCH_ARMV7 1
#endif

#define Qk_POSIX 1

#if defined(__ANDROID__)
# define Qk_ANDROID 1
# define Qk_LINUX 1
#elif defined(__APPLE__)
# include <TargetConditionals.h>
# define Qk_BSD 1
# define Qk_MAC TARGET_OS_MAC
# if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#  define Qk_iOS 1
# else
#  define Qk_OSX 1
# endif
#elif defined(__CYGWIN__)
# define Qk_CYGWIN 1
#elif defined(__linux__)
# define Qk_LINUX 1
#elif defined(__sun)
# define Qk_SOLARIS 1
#elif defined(_AIX)
# define Qk_AIX 1
#elif defined(__FreeBSD__)
# define Qk_BSD 1
# define Qk_FREEBSD 1
#elif defined(__Fuchsia__)
# define Qk_FUCHSIA 1
#elif defined(__DragonFly__)
# define Qk_BSD 1
# define Qk_DRAGONFLYBSD 1
#elif defined(__NetBSD__)
# define Qk_BSD 1
# define Qk_NETBSD 1
#elif defined(__OpenBSD__)
# define Qk_BSD 1
# define Qk_OPENBSD 1
#elif defined(__QNXNTO__)
# define Qk_QNX 1
#elif defined(__native_client__)
# undef Qk_POSIX
# define Qk_NACL 1
#elif defined(_WIN32)
# undef Qk_POSIX
# define Qk_WIN 1
#endif

// ------------------------------------------------------------------

#ifdef __GNUC__
# define Qk_LIKELY(expr) __builtin_expect(!!(expr), 1)
# define Qk_UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#else
# define Qk_LIKELY(expr) expr
# define Qk_UNLIKELY(expr) expr
#endif

#if DEBUG
#define Qk_ASSERT_RAW(cond, ...) \
	if (Qk_UNLIKELY(!(cond))) ::qk::Fatal(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define Qk_ASSERT_RAW(cond, ...) if (Qk_UNLIKELY(!(cond))) ::qk::Fatal("", 0, "", ##__VA_ARGS__)
#endif
#if DEBUG
# define Qk_ASSERT Qk_ASSERT_RAW
# define Qk_ASSERT_OP(a, op, b, ...) Qk_ASSERT_RAW(((a) op (b)), ##__VA_ARGS__)
#else
# define Qk_ASSERT(cond, ...) ((void)(cond))
# define Qk_ASSERT_OP(a, op, b, ...) ((void)((a) op (b)))
#endif
#define Qk_ASSERT_EQ(a, b, ...) Qk_ASSERT_OP((a), ==, (b), ##__VA_ARGS__)
#define Qk_ASSERT_NE(a, b, ...) Qk_ASSERT_OP((a), !=, (b), ##__VA_ARGS__)
#define Qk_ASSERT_GE(a, b, ...) Qk_ASSERT_OP((a), >=, (b), ##__VA_ARGS__)
#define Qk_ASSERT_LE(a, b, ...) Qk_ASSERT_OP((a), <=, (b), ##__VA_ARGS__)
#define Qk_ASSERT_GT(a, b, ...) Qk_ASSERT_OP((a), >,  (b), ##__VA_ARGS__)
#define Qk_ASSERT_LT(a, b, ...) Qk_ASSERT_OP((a), <,  (b), ##__VA_ARGS__)

#define Qk_DEFINE_INLINE_CLASS(Inl) public: class Inl; friend class Inl; private:
#define Qk_DEFINE_INLINE_MEMBERS(cls, Inl) \
	static Qk_INLINE cls::Inl* Inl##_##cls(cls* self) { \
		return reinterpret_cast<cls::Inl*>(self); \
	} class cls::Inl: public cls

#define Qk_Unreachable(msg)   Qk_Fatal("Unreachable code, %s", msg)
#define Qk_Log(msg, ...)      ::qk::log_println(msg, ##__VA_ARGS__)
#define Qk_Warn(msg, ...)     ::qk::log_println_warn(msg, ##__VA_ARGS__)
#define Qk_ELog(msg, ...)     ::qk::log_println_error(msg, ##__VA_ARGS__)
#define Qk_Fatal(...)         ::qk::Fatal(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define Qk_Min(A, B)          ((A) < (B) ? (A) : (B))
#define Qk_Max(A, B)          ((A) > (B) ? (A) : (B))
// return and move local
#define Qk_ReturnLocal(x)     return (x)

#if DEBUG
# define Qk_DLog Qk_Log
# define Qk_DEBUGCODE(...) __VA_ARGS__
#else
# define Qk_DLog(msg, ...) ((void)0)
# define Qk_DEBUGCODE(...)
#endif

#ifndef Qk_DEBUG
# define Qk_DEBUG DEBUG
#endif
#if __cplusplus >= 201703L
#define throw(...)
#endif

#define Qk_Type_Check(Base, Sub) \
	while (false) { *(static_cast<Base* volatile*>(0)) = static_cast<Sub*>(0); }

// ------------------------------------------------------------------

// This macro allows to test for the version of the GNU C++ compiler.
// Note that this also applies to compilers that masquerade as GCC,
// for example clang and the Intel C++ compiler for Linux.
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
# define Qk_GNUC_PREREQ(major, minor, patchlevel) \
	((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= \
	((major) * 10000 + (minor) * 100 + (patchlevel)))
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
# define Qk_GNUC_PREREQ(major, minor, patchlevel) \
	((__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= \
	((major) * 10000 + (minor) * 100 + (patchlevel)))
#else
# define Qk_GNUC_PREREQ(major, minor, patchlevel) 0
#endif

#if __clang__
# define __Qk_HAS_ATTRIBUTE_ALWAYS_INLINE (__has_attribute(always_inline))
# define __Qk_HAS_ATTRIBUTE_VISIBILITY (__has_attribute(visibility))
#elif __GNUC__
// always_inline is available in gcc 4.0 but not very reliable until 4.4.
// Works around "sorry, unimplemented: inlining failed" build errors with
// older compilers.
# define __Qk_HAS_ATTRIBUTE_ALWAYS_INLINE (Qk_GNUC_PREREQ(4, 4, 0))
# define __Qk_HAS_ATTRIBUTE_VISIBILITY (Qk_GNUC_PREREQ(4, 3, 0))
#elif defined(_MSC_VER)
# define __Qk_HAS_FORCE_INLINE 1
#endif

// ------------------------------------------------------------------

#ifdef _MSC_VER
	#ifdef Qk_BUILDING_SHARED
	# define Qk_EXPORT __declspec(dllexport)
	#elif Qk_USING_SHARED
	# define Qk_EXPORT __declspec(dllimport)
	#else
	# define Qk_EXPORT
	#endif  // Qk_BUILDING_SHARED
#else  // _MSC_VER
	// Setup for Linux shared library export.
	#if __Qk_HAS_ATTRIBUTE_VISIBILITY
	# ifdef Qk_BUILDING_SHARED
	#  define Qk_EXPORT __attribute__((visibility("default")))
	# else
	#  define Qk_EXPORT
	# endif
	#else
	# define Qk_EXPORT
	#endif
#endif // _MSC_VER

// A macro used to make better inlining. Don't bother for debug builds.
//
#if __Qk_HAS_ATTRIBUTE_ALWAYS_INLINE && !DEBUG
# define Qk_INLINE inline __attribute__((always_inline))
#elif __Qk_HAS_FORCE_INLINE && !DEBUG
# define Qk_INLINE __forceinline
#else
# define Qk_INLINE inline
#endif
#undef __Qk_HAS_ATTRIBUTE_ALWAYS_INLINE
#undef __Qk_HAS_FORCE_INLINE
#undef __Qk_HAS_ATTRIBUTE_VISIBILITY

// ------------------------------------------------------------------

#ifdef _MSC_VER
	#pragma section(".CRT$XCU", read)
	# define Qk_Init_Func(fn)\
	extern void __cdecl fn(void);\
	__declspec(dllexport, allocate(".CRT$XCU"))\
	void (__cdecl*fn##_)(void) = fn;\
	extern void __cdecl fn(void)
#else // _MSC_VER
	# define Qk_Init_Func(fn)\
	extern __attribute__((constructor)) void fn##__(void)
#endif

#if !defined(Qk_CPU_LENDIAN)
	#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
		#define Qk_CPU_LENDIAN 0
		// #define Qk_CPU_BENDIAN 1
	#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
		#define Qk_CPU_LENDIAN 1
	#elif defined(__sparc) || defined(__sparc__) || \
		defined(_POWER) || defined(__powerpc__) || \
		defined(__ppc__) || defined(__hppa) || \
		defined(__PPC__) || defined(__PPC64__) || \
		defined(_MIPSEB) || defined(__ARMEB__) || \
		defined(__s390__) || \
		(defined(__sh__) && defined(__BIG_ENDIAN__)) || \
		(defined(__ia64) && defined(__BIG_ENDIAN__))
			#define Qk_CPU_LENDIAN 0
			// #define Qk_CPU_BENDIAN 1
	#else
		#define Qk_CPU_LENDIAN 1
	#endif
#endif

#define Qk_HIDDEN_ALL_COPY(T)	\
	private: T(const T& t) = delete;	\
	private: T& operator=(const T& t) = delete;
#define Qk_HIDDEN_HEAP_ALLOC() \
	private: static void* operator new(size_t size) = delete; \
	private: static void operator delete(void*, size_t) = delete

#define __Qk_DEFINE_PROPERTY_
#define __Qk_DEFINE_PROPERTY_Const const
#define __Qk_DEFINE_PROPERTY_Protected
#define __Qk_DEFINE_PROPERTY_ProtectedConst const
#define __Qk_DEFINE_PROPERTY_Modifier private
#define __Qk_DEFINE_PROPERTY_ModifierConst private
#define __Qk_DEFINE_PROPERTY_ModifierProtected protected
#define __Qk_DEFINE_PROPERTY_ModifierProtectedConst protected

#define Qk_DEFINE_ACCE_GET(type, name, ...) public: \
	type name () __Qk_DEFINE_PROPERTY_##__VA_ARGS__

#define Qk_DEFINE_ACCESSOR(type, name, ...) \
	Qk_DEFINE_ACCE_GET(type, name, ##__VA_ARGS__); void set_##name (type val)

#define Qk_DEFINE_PROP_GET(type, name, ...) \
	__Qk_DEFINE_PROPERTY_Modifier##__VA_ARGS__: type _##name; public:\
	inline type name () __Qk_DEFINE_PROPERTY_##__VA_ARGS__ { return _##name; }

#define Qk_DEFINE_PROPERTY(type, name, ...) \
	Qk_DEFINE_PROP_GET(type, name, ##__VA_ARGS__) void set_##name (type val)

#define Qk_DEFINE_PROP_GET_Atomic(type, name, ...) \
	__Qk_DEFINE_PROPERTY_Modifier##__VA_ARGS__: std::atomic<type> _##name; public:\
	inline type name () __Qk_DEFINE_PROPERTY_##__VA_ARGS__ { return _##name.load(); }

#define Qk_DEFINE_PROPERTY_Atomic(type, name, ...) \
	Qk_DEFINE_PROP_GET_Atomic(type, name, ##__VA_ARGS__) void set_##name (type val)

#define __Qk_DEFINE_CLASS(Name) class Name;
#define __Qk_DEFINE_VISITOR_VISIT(N) virtual void visit##N(N *v) = 0;
#define Qk_DEFINE_VISITOR(Name, Each) \
	Each(__Qk_DEFINE_CLASS); \
	class Name##Visitor { \
	public: \
		virtual uint32_t flags() = 0; \
		Each(__Qk_DEFINE_VISITOR_VISIT) \
}

// Helper macros end
// -----------------------------------------------------------------------------

#endif
