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

#ifndef __quark__font__priv__util__
#define __quark__font__priv__util__

#include "../../../util/util.h"

// TODO: when C++17 the language is available, use template <auto P>
template <typename T, T* P> struct QkFunctionWrapper {
	template <typename... Args>
	auto operator()(Args&&... args) const -> decltype(P(std::forward<Args>(args)...)) {
		return P(std::forward<Args>(args)...);
	}
};

#if Qk_MAC
#include <CoreFoundation/CoreFoundation.h>

template <typename CFRef> using QkUniqueCFRef =
		std::unique_ptr<std::remove_pointer_t<CFRef>,
										QkFunctionWrapper<decltype(CFRelease), CFRelease>>;

/** Assumes src and dst are not nullptr. */
qk::String QkStringFromCFString(CFStringRef src);

#endif

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

typedef float QkScalar;
typedef int32_t QkFDot6;

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

static inline constexpr uint16_t QkEndianSwap16(uint16_t value) {
	return static_cast<uint16_t>((value >> 8) | ((value & 0xFF) << 8));
}

static inline constexpr uint32_t QkEndianSwap32(uint32_t value) {
		return ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) |
					 ((value & 0xFF0000) >> 8) | (value >> 24);
}

typedef uint32_t FontByteTag;
static inline constexpr FontByteTag QkSetFourByteTag(char a, char b, char c, char d) {
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

static inline constexpr int32_t QkLeftShift(int32_t value, int32_t shift) {
	return (int32_t) ((uint32_t) value << shift);
}

static inline constexpr int64_t QkLeftShift(int64_t value, int32_t shift) {
	return (int64_t) ((uint64_t) value << shift);
}

#define Qk_MaxS32FitsInFloat    2147483520
#define Qk_MinS32FitsInFloat    -Qk_MaxS32FitsInFloat

#define qk_float_floor(x)       floorf(x)
#define qk_float_ceil(x)        ceilf(x)
/**
 *  Return the closest int for the given float. Returns Qk_MaxS32FitsInFloat for NaN.
 */
static inline int qk_float_saturate2int(float x) {
	x = x < Qk_MaxS32FitsInFloat ? x : Qk_MaxS32FitsInFloat;
	x = x > Qk_MinS32FitsInFloat ? x : Qk_MinS32FitsInFloat;
	return (int)x;
}

#define qk_float_floor2int(x)   qk_float_saturate2int(qk_float_floor(x))
#define qk_float_round2int(x)   qk_float_saturate2int(qk_float_floor((x) + 0.5f))
#define qk_float_ceil2int(x)    qk_float_saturate2int(qk_float_ceil(x))

#define Qk_Scalar1              1.0f
#define Qk_ScalarHalf           0.5f

#define QkScalarToDouble(x)     static_cast<double>(x)
#define QkScalarFloorToInt(x)   qk_float_floor2int(x)
#define QkScalarCeilToInt(x)    qk_float_ceil2int(x)
#define QkScalarRoundToInt(x)   qk_float_round2int(x)

/** Types and macros for 16.16 fixed point
*/

/** 32 bit signed integer used to represent fractions values with 16 bits to the right of the decimal point
*/
typedef int32_t             QkFixed;
#define Qk_Fixed1           (1 << 16)
#define Qk_FixedHalf        (1 << 15)
#define QkFDot6Floor(x)     ((x) >> 6)
#define QkFDot6Ceil(x)      (((x) + 63) >> 6)
#define QkFDot6Round(x)     (((x) + 32) >> 6)
#define QkFixedToFDot6(x)   ((x) >> 10)

#define QkScalarToFDot6(x)  (QkFDot6)((x) * 64)
#define QkFDot6ToScalar(x)  ((QkScalar)(x) * 0.015625f)
#define QkFDot6ToFloat      QkFDot6ToScalar

// NOTE: QkFixedToFloat is exact. QkFloatToFixed seems to lack a rounding step. For all fixed-point
// values, this version is as accurate as possible for (fixed -> float -> fixed). Rounding reduces
// accuracy if the intermediate floats are in the range that only holds integers (adding 0.5f to an
// odd integer then snaps to nearest even). Using double for the rounding math gives maximum
// accuracy for (float -> fixed -> float), but that's usually overkill.
#define QkFixedToFloat(x)   ((x) * 1.52587890625e-5f)
#define QkFloatToFixed(x)   qk_float_saturate2int((x) * Qk_Fixed1)

#define QkFixedToScalar(x)  QkFixedToFloat(x)
#define QkScalarToFixed(x)  QkFloatToFixed(x)
#define QkIntToScalar(x)    static_cast<QkScalar>(x)

#include <limits>
#include <stdint.h>
#include <type_traits>


// bzero is safer than memset, but we can't rely on it, so... qk_bzero()
static inline void qk_bzero(void* buffer, size_t size) {
	// Please c.f. qk_careful_memcpy.  It's undefined behavior to call memset(null, 0, 0).
	if (size) {
		memset(buffer, 0, size);
	}
}

template <typename T> static inline T QkTAbs(T value) {
	if (value < 0) {
		value = -value;
	}
	return value;
}

/**
 *  Marks a local variable as known to be unused (to avoid warnings).
 *  Note that this does *not* prevent the local variable from being optimized away.
 */
template<typename T> inline void qk_ignore_unused_variable(const T&) { }


template <typename R, typename... Args> struct qk_base_callable_traits {
	using return_type = R;
	static constexpr std::size_t arity = sizeof...(Args);
	template <std::size_t N> struct argument {
		static_assert(N < arity, "");
		using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
	};
};
template <typename T> struct QkCallableTraits: QkCallableTraits<decltype(&T::operator())> {};
template <typename R, typename... Args>
struct QkCallableTraits<R(*)(Args...)>: qk_base_callable_traits<R, Args...> {};

// Can be used to bracket data types that must be dense, e.g. hash keys.
#if defined(__clang__)  // This should work on GCC too, but GCC diagnostic pop didn't seem to work!
	#define Qk_BEGIN_REQUIRE_DENSE _Pragma("GCC diagnostic push") \
																	_Pragma("GCC diagnostic error \"-Wpadded\"")
	#define Qk_END_REQUIRE_DENSE   _Pragma("GCC diagnostic pop")
#else
	#define Qk_BEGIN_REQUIRE_DENSE
	#define Qk_END_REQUIRE_DENSE
#endif

template <typename T> static constexpr T QkAlign8(T x) { return (x + 7) >> 3 << 3; }

#endif
