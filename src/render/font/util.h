//@private
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

#ifndef __quark__font__util__
#define __quark__font__util__

#include "../../util/util.h"

using namespace qk;

typedef uint8_t Qk_OT_BYTE;
#if CHAR_BIT == 8
typedef signed char Qk_OT_CHAR; //easier to debug
#else
typedef int8_t Qk_OT_CHAR;
#endif
typedef uint16_t Qk_OT_SHORT;
typedef uint16_t Qk_OT_USHORT;
typedef uint32_t Qk_OT_ULONG;
typedef uint32_t Qk_OT_LONG;
//16.16 Signed fixed point representation.
typedef int32_t Qk_OT_Fixed;
//2.14 Signed fixed point representation.
typedef uint16_t Qk_OT_F2DOT14;
//F units are the units of measurement in em space.
typedef uint16_t Qk_OT_FWORD;
typedef uint16_t Qk_OT_UFWORD;
//Number of seconds since 12:00 midnight, January 1, 1904.
typedef uint64_t Qk_OT_LONGDATETIME;

#ifdef Qk_CPU_LENDIAN
	#define Qk_UINT8_BITFIELD(f0, f1, f2, f3, f4, f5, f6, f7) \
		Qk_OT_BYTE f0 : 1; \
		Qk_OT_BYTE f1 : 1; \
		Qk_OT_BYTE f2 : 1; \
		Qk_OT_BYTE f3 : 1; \
		Qk_OT_BYTE f4 : 1; \
		Qk_OT_BYTE f5 : 1; \
		Qk_OT_BYTE f6 : 1; \
		Qk_OT_BYTE f7 : 1;
#else
	#define Qk_UINT8_BITFIELD(f0, f1, f2, f3, f4, f5, f6, f7) \
			Qk_OT_BYTE f7 : 1; \
			Qk_OT_BYTE f6 : 1; \
			Qk_OT_BYTE f5 : 1; \
			Qk_OT_BYTE f4 : 1; \
			Qk_OT_BYTE f3 : 1; \
			Qk_OT_BYTE f2 : 1; \
			Qk_OT_BYTE f1 : 1; \
			Qk_OT_BYTE f0 : 1;
#endif

#define Qk_OT_BYTE_BITFIELD Qk_UINT8_BITFIELD

/** @return the number of entries in an array (not a pointer)
*/
template <typename T, size_t N> char (&QkArrayCountHelper(T (&array)[N]))[N];
#define Qk_ARRAY_COUNT(array) (sizeof(QkArrayCountHelper(array)))

#ifdef Qk_CPU_LENDIAN
	#define QkEndian_SwapBE16(n)    QkEndianSwap16(n)
	#define QkEndian_SwapBE32(n)    QkEndianSwap32(n)
#else
	#define QkEndian_SwapBE16(n)    (n)
	#define QkEndian_SwapBE32(n)    (n)
#endif

// TODO: when C++17 the language is available, use template <auto P>
template <typename T, T* P> struct QkFunctionWrapper {
	template <typename... Args>
	auto operator()(Args&&... args) const -> decltype(P(std::forward<Args>(args)...)) {
		return P(std::forward<Args>(args)...);
	}
};

static inline constexpr uint16_t QkEndianSwap16(uint16_t value) {
	return static_cast<uint16_t>((value >> 8) | ((value & 0xFF) << 8));
}

static inline constexpr uint32_t QkEndianSwap32(uint32_t value) {
		return ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) |
					 ((value & 0xFF0000) >> 8) | (value >> 24);
}

typedef uint32_t QkFourByteTag;
static inline constexpr QkFourByteTag QkSetFourByteTag(char a, char b, char c, char d) {
		return (((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)c << 8) | (uint32_t)d);
}


/** QkOTSetUSHORTBit<N>::value is an Qk_OT_USHORT with the Nth BE bit set. */
template <unsigned N> struct QkOTSetUSHORTBit {
	static_assert(N < 16, "NTooBig");
	static const uint16_t bit = 1u << N;
	static const Qk_OT_USHORT value = QkEndian_SwapBE16(bit);
};

/** QkOTSetULONGBit<N>::value is an Qk_OT_ULONG with the Nth BE bit set. */
template <unsigned N> struct QkOTSetULONGBit {
	static_assert(N < 32, "NTooBig");
	static const uint32_t bit = 1u << N;
	static const Qk_OT_ULONG value = QkEndian_SwapBE32(bit);
};

template<typename T> class QkOTTableTAG {
public:
	/**
	 * QkOTTableTAG<T>::value is the big endian value of an OpenType table tag.
	 * It may be directly compared with raw big endian table data.
	 */
	static const Qk_OT_ULONG value = QkEndian_SwapBE32(
		QkSetFourByteTag(T::TAG0, T::TAG1, T::TAG2, T::TAG3)
	);
};


#endif
