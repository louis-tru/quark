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

//@private head

#ifndef __quark__font__priv__to__
#define __quark__font__priv__to__

#include "../../../util/macros.h"

#include <limits>
#include <stdint.h>
#include <type_traits>

/**
 * std::underlying_type is only defined for enums. For integral types, we just want the type.
 */
template <typename T, class Enable = void>
struct qk_strip_enum {
	typedef T type;
};

template <typename T>
struct qk_strip_enum<T, typename std::enable_if<std::is_enum<T>::value>::type> {
	typedef typename std::underlying_type<T>::type type;
};

/**
 * In C++ an unsigned to signed cast where the source value cannot be represented in the destination
 * type results in an implementation defined destination value. Unlike C, C++ does not allow a trap.
 * This makes "(S)(D)s == s" a possibly useful test. However, there are two cases where this is
 * incorrect:
 *
 * when testing if a value of a smaller signed type can be represented in a larger unsigned type
 * (int8_t)(uint16_t)-1 == -1 => (int8_t)0xFFFF == -1 => [implementation defined] == -1
 *
 * when testing if a value of a larger unsigned type can be represented in a smaller signed type
 * (uint16_t)(int8_t)0xFFFF == 0xFFFF => (uint16_t)-1 == 0xFFFF => 0xFFFF == 0xFFFF => true.
 *
 * Consider the cases:
 *   u = unsigned, less digits
 *   U = unsigned, more digits
 *   s = signed, less digits
 *   S = signed, more digits
 *   v is the value we're considering.
 *
 * u -> U: (u)(U)v == v, trivially true
 * U -> u: (U)(u)v == v, both casts well defined, test works
 * s -> S: (s)(S)v == v, trivially true
 * S -> s: (S)(s)v == v, first cast implementation value, second cast defined, test works
 * s -> U: (s)(U)v == v, *this is bad*, the second cast results in implementation defined value
 * S -> u: (S)(u)v == v, the second cast is required to prevent promotion of rhs to unsigned
 * u -> S: (u)(S)v == v, trivially true
 * U -> s: (U)(s)v == v, *this is bad*,
 *                             first cast results in implementation defined value,
 *                             second cast is defined. However, this creates false positives
 *                             uint16_t x = 0xFFFF
 *                                (uint16_t)(int8_t)x == x
 *                             => (uint16_t)-1        == x
 *                             => 0xFFFF              == x
 *                             => true
 *
 * So for the eight cases three are trivially true, three more are valid casts, and two are special.
 * The two 'full' checks which otherwise require two comparisons are valid cast checks.
 * The two remaining checks s -> U [v >= 0] and U -> s [v <= max(s)] can be done with one op.
 */

template <typename D, typename S>
static constexpr inline
typename std::enable_if<(std::is_integral<S>::value || std::is_enum<S>::value) &&
                        (std::is_integral<D>::value || std::is_enum<D>::value), bool>::type
/*bool*/ QkTFitsIn(S src) {
	// QkTFitsIn() is used in public headers, so needs to be written targeting at most C++11.
	return

	// E.g. (int8_t)(uint8_t) int8_t(-1) == -1, but the uint8_t == 255, not -1.
	(std::is_signed<S>::value && std::is_unsigned<D>::value && sizeof(S) <= sizeof(D)) ?
			(S)0 <= src :

	// E.g. (uint8_t)(int8_t) uint8_t(255) == 255, but the int8_t == -1.
	(std::is_signed<D>::value && std::is_unsigned<S>::value && sizeof(D) <= sizeof(S)) ?
			src <= (S)std::numeric_limits<typename qk_strip_enum<D>::type>::max() :

#if !defined(DEBUG) && !defined(__MSVC_RUNTIME_CHECKS )
	// Correct (simple) version. This trips up MSVC's /RTCc run-time checking.
	(S)(D)src == src;
#else
	// More complex version that's safe with /RTCc. Used in all debug builds, for coverage.
	(std::is_signed<S>::value) ?
			(intmax_t)src >= (intmax_t)std::numeric_limits<typename qk_strip_enum<D>::type>::min() &&
			(intmax_t)src <= (intmax_t)std::numeric_limits<typename qk_strip_enum<D>::type>::max() :

	// std::is_unsigned<S> ?
			(uintmax_t)src <= (uintmax_t)std::numeric_limits<typename qk_strip_enum<D>::type>::max();
#endif
}

template <typename D, typename S> constexpr D QkTo(S s) {
	return Qk_ASSERT(QkTFitsIn<D>(s)), static_cast<D>(s);
}

template <typename S> constexpr int8_t   QkToS8(S x)    { return QkTo<int8_t>(x);   }
template <typename S> constexpr uint8_t  QkToU8(S x)    { return QkTo<uint8_t>(x);  }
template <typename S> constexpr int16_t  QkToS16(S x)   { return QkTo<int16_t>(x);  }
template <typename S> constexpr uint16_t QkToU16(S x)   { return QkTo<uint16_t>(x); }
template <typename S> constexpr int32_t  QkToS32(S x)   { return QkTo<int32_t>(x);  }
template <typename S> constexpr uint32_t QkToU32(S x)   { return QkTo<uint32_t>(x); }
template <typename S> constexpr int      QkToInt(S x)   { return QkTo<int>(x);      }
template <typename S> constexpr unsigned QkToUInt(S x)  { return QkTo<unsigned>(x); }
template <typename S> constexpr size_t   QkToSizeT(S x) { return QkTo<size_t>(x);   }

/** @return false or true based on the condition
*/
template <typename T> static constexpr bool QkToBool(const T& x) {
	return 0 != x;  // NOLINT(modernize-use-nullptr)
}


////////////////////////////////////////////////////////////////////////////////////////////
// Convert a 16bit pixel to a 32bit pixel

#define Qk_A32_BITS     8
#define Qk_R32_BITS     8
#define Qk_G32_BITS     8
#define Qk_B32_BITS     8

#define Qk_A32_MAQk     ((1 << Qk_A32_BITS) - 1)
#define Qk_R32_MAQk     ((1 << Qk_R32_BITS) - 1)
#define Qk_G32_MAQk     ((1 << Qk_G32_BITS) - 1)
#define Qk_B32_MAQk     ((1 << Qk_B32_BITS) - 1)

#define Qk_R16_BITS     5
#define Qk_G16_BITS     6
#define Qk_B16_BITS     5

#define Qk_R16_SHIFT    (Qk_B16_BITS + Qk_G16_BITS)
#define Qk_G16_SHIFT    (Qk_B16_BITS)
#define Qk_B16_SHIFT    0

#define Qk_R16_MAQk     ((1 << Qk_R16_BITS) - 1)
#define Qk_G16_MAQk     ((1 << Qk_G16_BITS) - 1)
#define Qk_B16_MAQk     ((1 << Qk_B16_BITS) - 1)

#define QkGetPackedR16(color)   (((unsigned)(color) >> Qk_R16_SHIFT) & Qk_R16_MAQk)
#define QkGetPackedG16(color)   (((unsigned)(color) >> Qk_G16_SHIFT) & Qk_G16_MAQk)
#define QkGetPackedB16(color)   (((unsigned)(color) >> Qk_B16_SHIFT) & Qk_B16_MAQk)

#define QkA32Assert(a)  Qk_ASSERT((unsigned)(a) <= Qk_A32_MAQk)
#define QkR32Assert(r)  Qk_ASSERT((unsigned)(r) <= Qk_R32_MAQk)
#define QkG32Assert(g)  Qk_ASSERT((unsigned)(g) <= Qk_G32_MAQk)
#define QkB32Assert(b)  Qk_ASSERT((unsigned)(b) <= Qk_B32_MAQk)

#define QkR32ToR16_MACRO(r)   ((unsigned)(r) >> (Qk_R32_BITS - Qk_R16_BITS))
#define QkG32ToG16_MACRO(g)   ((unsigned)(g) >> (Qk_G32_BITS - Qk_G16_BITS))
#define QkB32ToB16_MACRO(b)   ((unsigned)(b) >> (Qk_B32_BITS - Qk_B16_BITS))

#if Qk_DEBUG
	static inline unsigned QkR32ToR16(unsigned r) {
		QkR32Assert(r);
		return QkR32ToR16_MACRO(r);
	}
	static inline unsigned QkG32ToG16(unsigned g) {
		QkG32Assert(g);
		return QkG32ToG16_MACRO(g);
	}
	static inline unsigned QkB32ToB16(unsigned b) {
		QkB32Assert(b);
		return QkB32ToB16_MACRO(b);
	}
#else
	#define QkR32ToR16(r)   QkR32ToR16_MACRO(r)
	#define QkG32ToG16(g)   QkG32ToG16_MACRO(g)
	#define QkB32ToB16(b)   QkB32ToB16_MACRO(b)
#endif

/** Fast type for unsigned 8 bits. Use for parameter passing and local
    variables, not for storage
*/
typedef unsigned U8CPU;

/** Fast type for unsigned 16 bits. Use for parameter passing and local
    variables, not for storage
*/
typedef unsigned U16CPU;

static inline U16CPU QkPack888ToRGB16(U8CPU r, U8CPU g, U8CPU b) {
	return  (QkR32ToR16(r) << Qk_R16_SHIFT) |
					(QkG32ToG16(g) << Qk_G16_SHIFT) |
					(QkB32ToB16(b) << Qk_B16_SHIFT);
}

#endif
