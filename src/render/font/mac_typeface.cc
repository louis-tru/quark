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

#include "mac_typeface.h"

struct RoundCGFloatToInt {
		int operator()(CGFloat s) { return s + 0.5; }
};
struct CGFloatIdentity {
		CGFloat operator()(CGFloat s) { return s; }
};

/** Convert the [0, 1000] CSS weight to [-1, 1] CTFontDescriptor weight (for system fonts).
 *
 *  The -1 to 1 weights reported by CTFontDescriptors have different mappings depending on if the
 *  CTFont is native or created from a CGDataProvider.
 */
CGFloat QkCTFontCTWeightForCSSWeight(TextWeight fontstyleWeight) {
	using Interpolator = QkLinearInterpolater<int, CGFloat, CGFloatIdentity>;

	// Note that Mac supports the old OS2 version A so 0 through 10 are as if multiplied by 100.
	// However, on this end we can't tell, so this is ignored.

	static Interpolator::Mapping nativeWeightMappings[11];
	
	static int onceInit = []{
		const CGFloat(&nsFontWeights)[11] = QkCTFontGetNSFontWeightMapping();
		for (int i = 0; i < 11; ++i) {
			nativeWeightMappings[i].src_val = i * 100;
			nativeWeightMappings[i].dst_val = nsFontWeights[i];
		}
		return 0;
	}();

	static constexpr Interpolator nativeInterpolator(
					nativeWeightMappings, Qk_ARRAY_COUNT(nativeWeightMappings));

	return nativeInterpolator.map(int(fontstyleWeight));
}

/** Convert the [0, 10] CSS weight to [-1, 1] CTFontDescriptor width. */
CGFloat QkCTFontCTWidthForCSSWidth(TextWidth fontstyleWidth) {
	using Interpolator = QkLinearInterpolater<int, CGFloat, CGFloatIdentity>;

	// Values determined by creating font data with every width, creating a CTFont,
	// and asking the CTFont for its width. See TypefaceStyle test for basics.
	static constexpr Interpolator::Mapping widthMappings[] = {
		{  0, -0.5 },
		{ 10,  0.5 },
	};
	static constexpr Interpolator interpolator(widthMappings, Qk_ARRAY_COUNT(widthMappings));
	return interpolator.map(int(fontstyleWidth));
}
