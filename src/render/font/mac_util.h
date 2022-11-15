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

#ifndef __quark__font__mac_util__
#define __quark__font__mac_util__

#include "../../util/util.h"

#if Qk_OSX
# include <ApplicationServices/ApplicationServices.h>
#endif
#if Qk_IOS
# include <CoreText/CoreText.h>
# include <CoreText/CTFontManager.h>
# include <CoreGraphics/CoreGraphics.h>
# include <CoreFoundation/CoreFoundation.h>
#endif

using namespace quark;

enum class QkCTFontSmoothBehavior {
	none, // SmoothFonts produces no effect.
	some, // SmoothFonts produces some effect, but not subpixel coverage.
	subpixel, // SmoothFonts produces some effect and provides subpixel coverage.
};

QkCTFontSmoothBehavior QkCTFontGetSmoothBehavior();

using QkCTFontWeightMapping = const CGFloat[11];

/** Returns the [-1, 1] CTFontDescriptor weights for the
 *  <0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000> CSS weights.
 *
 *  It is assumed that the values will be interpolated linearly between these points.
 *  NSFontWeightXXX were added in 10.11, appear in 10.10, but do not appear in 10.9.
 *  The actual values appear to be stable, but they may change without notice.
 *  These values are valid for system fonts only.
 */
QkCTFontWeightMapping& QkCTFontGetNSFontWeightMapping();

/** Returns the [-1, 1] CTFontDescriptor weights for the
 *  <0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000> CSS weights.
 *
 *  It is assumed that the values will be interpolated linearly between these points.
 *  The actual values appear to be stable, but they may change without notice.
 *  These values are valid for fonts created from data only.
 */
QkCTFontWeightMapping& QkCTFontGetDataFontWeightMapping();

// -------------------------------------------------------------------------------------


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

template <typename S, typename D, typename C> struct QkLinearInterpolater {
	struct Mapping {
		S src_val;
		D dst_val;
	};
	constexpr QkLinearInterpolater(Mapping const mapping[], int mappingCount)
		: fMapping(mapping), fMappingCount(mappingCount) {}

	static D map(S value, S src_min, S src_max, D dst_min, D dst_max) {
		Qk_Assert(src_min < src_max);
		Qk_Assert(dst_min <= dst_max);
		return C()(dst_min + (((value - src_min) * (dst_max - dst_min)) / (src_max - src_min)));
	}

	D map(S val) const {
		// -Inf to [0]
		if (val < fMapping[0].src_val) {
			return fMapping[0].dst_val;
		}
		// Linear from [i] to [i+1]
		for (int i = 0; i < fMappingCount - 1; ++i) {
			if (val < fMapping[i+1].src_val) {
				return map(val, fMapping[i].src_val, fMapping[i+1].src_val,
												fMapping[i].dst_val, fMapping[i+1].dst_val);
			}
		}
		// From [n] to +Inf
		// if (fcweight < Inf)
		return fMapping[fMappingCount - 1].dst_val;
	}

	Mapping const * fMapping;
	int fMappingCount;
};

// TODO: when C++17 the language is available, use template <auto P>
template <typename T, T* P> struct QkFunctionWrapper {
	template <typename... Args>
	auto operator()(Args&&... args) const -> decltype(P(std::forward<Args>(args)...)) {
		return P(std::forward<Args>(args)...);
	}
};

template <typename CFRef> using QkUniqueCFRef =
		std::unique_ptr<std::remove_pointer_t<CFRef>,
										QkFunctionWrapper<decltype(CFRelease), CFRelease>>;

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

/** Assumes src and dst are not nullptr. */
String QkStringFromCFString(CFStringRef src);

#endif
