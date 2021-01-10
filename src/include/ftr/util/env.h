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

#ifndef __ftr__util__env__
#define __ftr__util__env__

#define FX_HAS_CPP __cplusplus

#ifdef __EXCEPTIONS
# define FX_EXCEPTIONS_SUPPORT 1
#else
# define FX_EXCEPTIONS_SUPPORT 0
#endif

#ifndef NULL
# define NULL 0
#endif

#define FX_GNUC          0
#define FX_CLANG         0
#define FX_MSC           0
#define FX_ARCH_X86      0
#define FX_ARCH_ARM      0
#define FX_ARCH_MIPS     0
#define FX_ARCH_MIPS64   0
#define FX_ARCH_IA32     0
#define FX_ARCH_X64      0
#define FX_ARCH_ARM64    0
#define FX_ARCH_ARMV7    0
#define FX_ARCH_32BIT    0
#define FX_ARCH_64BIT    0
#define FX_APPLE         0
#define FX_POSIX         0
#define FX_UNIX          0
#define FX_LINUX         0
#define FX_BSD           0
#define FX_CYGWIN        0
#define FX_NACL          0
#define FX_IOS           0
#define FX_OSX           0
#define FX_ANDROID       0
#define FX_WIN           0
#define FX_QNX           0

#if defined(__GNUC__)
# undef FX_GNUC
# define FX_GNUC       1
#endif

#if defined(__clang__)
# undef FX_CLANG
# define FX_CLANG       1
#endif

#if defined(_MSC_VER)
# undef FX_MSC
# define FX_MSC       1
#endif

#if defined(_M_X64) || defined(__x86_64__)
# undef FX_ARCH_X86
# define FX_ARCH_X86        1
# if defined(__native_client__)
#   undef FX_ARCH_IA32
#   define FX_ARCH_IA32     1
#   undef FX_ARCH_32BIT
#   define FX_ARCH_32BIT    1
# else // else __native_client__
#   undef FX_ARCH_X64
#   define FX_ARCH_X64      1
#   undef FX_ARCH_64BIT
#   define FX_ARCH_64BIT    1
# endif  // __native_client__

#elif defined(_M_IX86) || defined(__i386__)
# undef FX_ARCH_X86
# define FX_ARCH_X86        1
# undef FX_ARCH_IA32
# define FX_ARCH_IA32       1
# undef FX_ARCH_32BIT
# define FX_ARCH_32BIT      1

#elif defined(__arm64__) || defined(__AARCH64EL__)
# undef FX_ARCH_ARM
# define FX_ARCH_ARM        1
# undef FX_ARCH_ARM64
# define FX_ARCH_ARM64      1
# undef FX_ARCH_64BIT
# define FX_ARCH_64BIT      1

#elif defined(__ARMEL__)
# undef FX_ARCH_ARM
# define FX_ARCH_ARM        1
# undef FX_ARCH_32BIT
# define FX_ARCH_32BIT      1

#elif defined(__mips64)
# undef FX_ARCH_MIPS
# define FX_ARCH_MIPS       1
# undef FX_ARCH_MIPS64
# define FX_ARCH_MIPS64     1
# undef FX_ARCH_64BIT
# define FX_ARCH_64BIT      1

#elif defined(__MIPSEL__)
# undef FX_ARCH_MIPS
# define FX_ARCH_MIPS       1
# undef FX_ARCH_32BIT
# define FX_ARCH_32BIT      1

#else
# error Host architecture was not detected as supported by ftr
#endif

#if defined(__ARM_ARCH_7A__) || \
defined(__ARM_ARCH_7R__) || \
defined(__ARM_ARCH_7__)
# undef FX_ARCH_ARMV7
# define FX_ARCH_ARMV7  1
#endif

#if defined(__sun)
# undef FX_BSD
# define FX_BSD        1
# undef FX_UNIX
# define FX_UNIX       1
#endif

#if defined(__OpenBSD__) || \
defined(__NetBSD__)   || \
defined(__FreeBSD__)  || \
defined(__DragonFly__)
# undef FX_POSIX
# define FX_POSIX      1
# undef FX_BSD
# define FX_BSD        1
# undef FX_UNIX
# define FX_UNIX       1
#endif

#if defined(__APPLE__)
# undef FX_APPLE
# define FX_APPLE      1
# undef FX_POSIX
# define FX_POSIX      1
# undef FX_BSD
# define FX_BSD        1
# undef FX_UNIX
# define FX_UNIX       1
#endif

#if defined(__CYGWIN__)
# undef FX_CYGWIN
# define FX_CYGWIN     1
# undef FX_POSIX
# define FX_POSIX      1
#endif

#if defined(__unix__)
# undef FX_UNIX
# define FX_UNIX       1
#endif

#if defined(__linux__)
# undef FX_LINUX
# define FX_LINUX      1
# undef FX_POSIX
# define FX_POSIX      1
# undef FX_UNIX
# define FX_UNIX       1
#endif

#if defined(__native_client__)
# undef FX_NACL
# define FX_NACL       1
# undef FX_POSIX
# define FX_POSIX      1
#endif

#ifdef __APPLE__
# include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE
# undef FX_IOS
# define FX_IOS        1
#endif

#if TARGET_OS_MAC && !TARGET_OS_IPHONE
# undef FX_OSX
# define FX_OSX        1
#endif

#if defined(__ANDROID__)
# undef FX_ANDROID
# define FX_ANDROID    1
# undef FX_POSIX
# define FX_POSIX      1
# undef FX_LINUX
# define FX_LINUX      1
# undef FX_UNIX
# define FX_UNIX       1
#endif

#if defined(_WINDOWS)
# undef FX_WIN
# define FX_WIN        1
#endif

#if defined(__QNXNTO__)
# undef FX_POSIX
# define FX_POSIX 1
# undef FX_QNX
# define FX_QNX 1
#endif

#endif
