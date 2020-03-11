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

#ifndef __ngui__utils__env__
#define __ngui__utils__env__

#define NX_HAS_CPP __cplusplus

#ifdef __EXCEPTIONS
# define NX_EXCEPTIONS_SUPPORT 1
#else
# define NX_EXCEPTIONS_SUPPORT 0
#endif

#ifndef NULL
# define NULL 0
#endif

#define NX_GNUC          0
#define NX_CLANG         0
#define NX_MSC           0
#define NX_ARCH_X86      0
#define NX_ARCH_ARM      0
#define NX_ARCH_MIPS     0
#define NX_ARCH_MIPS64   0
#define NX_ARCH_IA32     0
#define NX_ARCH_X64      0
#define NX_ARCH_ARM64    0
#define NX_ARCH_ARMV7    0
#define NX_ARCH_32BIT    0
#define NX_ARCH_64BIT    0
#define NX_APPLE         0
#define NX_POSIX         0
#define NX_UNIX          0
#define NX_LINUX         0
#define NX_BSD           0
#define NX_CYGWIN        0
#define NX_NACL          0
#define NX_IOS           0
#define NX_OSX           0
#define NX_ANDROID       0
#define NX_WIN           0
#define NX_QNX           0

#if defined(__GNUC__)
# undef NX_GNUC
# define NX_GNUC       1
#endif

#if defined(__clang__)
# undef NX_CLANG
# define NX_CLANG       1
#endif

#if defined(_MSC_VER)
# undef NX_MSC
# define NX_MSC       1
#endif

#if defined(_M_X64) || defined(__x86_64__)
# undef NX_ARCH_X86
# define NX_ARCH_X86        1
# if defined(__native_client__)
#   undef NX_ARCH_IA32
#   define NX_ARCH_IA32     1
#   undef NX_ARCH_32BIT
#   define NX_ARCH_32BIT    1
# else // else __native_client__
#   undef NX_ARCH_X64
#   define NX_ARCH_X64      1
#   undef NX_ARCH_64BIT
#   define NX_ARCH_64BIT    1
# endif  // __native_client__

#elif defined(_M_IX86) || defined(__i386__)
# undef NX_ARCH_X86
# define NX_ARCH_X86        1
# undef NX_ARCH_IA32
# define NX_ARCH_IA32       1
# undef NX_ARCH_32BIT
# define NX_ARCH_32BIT      1

#elif defined(__arm64__) || defined(__AARCH64EL__)
# undef NX_ARCH_ARM
# define NX_ARCH_ARM        1
# undef NX_ARCH_ARM64
# define NX_ARCH_ARM64      1
# undef NX_ARCH_64BIT
# define NX_ARCH_64BIT      1

#elif defined(__ARMEL__)
# undef NX_ARCH_ARM
# define NX_ARCH_ARM        1
# undef NX_ARCH_32BIT
# define NX_ARCH_32BIT      1

#elif defined(__mips64)
# undef NX_ARCH_MIPS
# define NX_ARCH_MIPS       1
# undef NX_ARCH_MIPS64
# define NX_ARCH_MIPS64     1
# undef NX_ARCH_64BIT
# define NX_ARCH_64BIT      1

#elif defined(__MIPSEL__)
# undef NX_ARCH_MIPS
# define NX_ARCH_MIPS       1
# undef NX_ARCH_32BIT
# define NX_ARCH_32BIT      1

#else
# error Host architecture was not detected as supported by ngui
#endif

#if defined(__ARM_ARCH_7A__) || \
defined(__ARM_ARCH_7R__) || \
defined(__ARM_ARCH_7__)
# undef NX_ARCH_ARMV7
# define NX_ARCH_ARMV7  1
#endif

#if defined(__sun)
# undef NX_BSD
# define NX_BSD        1
# undef NX_UNIX
# define NX_UNIX       1
#endif

#if defined(__OpenBSD__) || \
defined(__NetBSD__)   || \
defined(__FreeBSD__)  || \
defined(__DragonFly__)
# undef NX_POSIX
# define NX_POSIX      1
# undef NX_BSD
# define NX_BSD        1
# undef NX_UNIX
# define NX_UNIX       1
#endif

#if defined(__APPLE__)
# undef NX_APPLE
# define NX_APPLE      1
# undef NX_POSIX
# define NX_POSIX      1
# undef NX_BSD
# define NX_BSD        1
# undef NX_UNIX
# define NX_UNIX       1
#endif

#if defined(__CYGWIN__)
# undef NX_CYGWIN
# define NX_CYGWIN     1
# undef NX_POSIX
# define NX_POSIX      1
#endif

#if defined(__unix__)
# undef NX_UNIX
# define NX_UNIX       1
#endif

#if defined(__linux__)
# undef NX_LINUX
# define NX_LINUX      1
# undef NX_POSIX
# define NX_POSIX      1
# undef NX_UNIX
# define NX_UNIX       1
#endif

#if defined(__native_client__)
# undef NX_NACL
# define NX_NACL       1
# undef NX_POSIX
# define NX_POSIX      1
#endif

#ifdef __APPLE__
# include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE
# undef NX_IOS
# define NX_IOS        1
#endif

#if TARGET_OS_MAC && !TARGET_OS_IPHONE
# undef NX_OSX
# define NX_OSX        1
#endif

#if defined(__ANDROID__)
# undef NX_ANDROID
# define NX_ANDROID    1
# undef NX_POSIX
# define NX_POSIX      1
# undef NX_LINUX
# define NX_LINUX      1
# undef NX_UNIX
# define NX_UNIX       1
#endif

#if defined(_WINDOWS)
# undef NX_WIN
# define NX_WIN        1
#endif

#if defined(__QNXNTO__)
# undef NX_POSIX
# define NX_POSIX 1
# undef NX_QNX
# define NX_QNX 1
#endif

#endif
