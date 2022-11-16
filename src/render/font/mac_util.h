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

#if Qk_OSX
# include <ApplicationServices/ApplicationServices.h>
#endif
#if Qk_IOS
# include <CoreText/CoreText.h>
# include <CoreText/CTFontManager.h>
# include <CoreGraphics/CoreGraphics.h>
# include <CoreFoundation/CoreFoundation.h>
#endif

#include "../../util/util.h"
#include "util.h"

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

template <typename CFRef> using QkUniqueCFRef =
		std::unique_ptr<std::remove_pointer_t<CFRef>,
										QkFunctionWrapper<decltype(CFRelease), CFRelease>>;

/** Assumes src and dst are not nullptr. */
String QkStringFromCFString(CFStringRef src);

#endif
