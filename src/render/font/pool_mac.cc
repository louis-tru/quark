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

#include "../../util/util.h"

#if Qk_OSX
#include <ApplicationServices/ApplicationServices.h>
#endif

#if Qk_IOS
#include <CoreText/CoreText.h>
#include <CoreText/CTFontManager.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#include <dlfcn.h>
#endif

#include "../../util/handle.h"
#include "style.h"

using namespace quark;

namespace quark {
	class Typeface;
}

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

#if (defined(Qk_IOS) && defined(__IPHONE_14_0) &&  __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_14_0) ||  \
	(defined(Qk_MAC) && defined(__MAC_11_0) && __MAC_OS_VERSION_MIN_REQUIRED >= __MAC_11_0)

static uint32_t QkGetCoreTextVersion() {
	// If compiling for iOS 14.0+ or macOS 11.0+, the CoreText version number
	// must be derived from the OS version number.
	static const uint32_t kCoreTextVersionNEWER = 0x000D0000;
	return kCoreTextVersionNEWER;
}

#else

static uint32_t QkGetCoreTextVersion() {
	// Check for CoreText availability before calling CTGetCoreTextVersion().
	if (&CTGetCoreTextVersion) {
		return CTGetCoreTextVersion();
	}
	// Default to a value that's smaller than any known CoreText version.
	static const uint32_t kCoreTextVersionUNKNOWN = 0;
	return kCoreTextVersionUNKNOWN;
}

#endif


static QkUniqueCFRef<CFStringRef> make_CFString(const char s[]) {
    return QkUniqueCFRef<CFStringRef>(CFStringCreateWithCString(nullptr, s, kCFStringEncodingUTF8));
}

/** Creates a typeface from a descriptor, searching the cache. */
static Typeface* create_from_desc(CTFontDescriptorRef desc) {
    QkUniqueCFRef<CTFontRef> ctFont(CTFontCreateWithFontDescriptor(desc, 0, nullptr));
    if (!ctFont) {
        return nullptr;
    }
	
	return nullptr;

//    return QkTypeface_Mac::Make(std::move(ctFont), OpszVariation(), nullptr);
}


// ------------------------------------------------------------

template <typename S, typename D, typename C> struct LinearInterpolater {
		struct Mapping {
				S src_val;
				D dst_val;
		};
		constexpr LinearInterpolater(Mapping const mapping[], int mappingCount)
				: fMapping(mapping), fMappingCount(mappingCount) {}

		static D map(S value, S src_min, S src_max, D dst_min, D dst_max) {
				SkASSERT(src_min < src_max);
				SkASSERT(dst_min <= dst_max);
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
CGFloat CTFontCTWeightForCSSWeight(int fontstyleWeight) {
		using Interpolator = LinearInterpolater<int, CGFloat, CGFloatIdentity>;

		// Note that Mac supports the old OS2 version A so 0 through 10 are as if multiplied by 100.
		// However, on this end we can't tell, so this is ignored.

		static Interpolator::Mapping nativeWeightMappings[11];
		static SkOnce once;
		once([&] {
				const CGFloat(&nsFontWeights)[11] = SkCTFontGetNSFontWeightMapping();
				for (int i = 0; i < 11; ++i) {
						nativeWeightMappings[i].src_val = i * 100;
						nativeWeightMappings[i].dst_val = nsFontWeights[i];
				}
		});
		static constexpr Interpolator nativeInterpolator(
						nativeWeightMappings, SK_ARRAY_COUNT(nativeWeightMappings));

		return nativeInterpolator.map(fontstyleWeight);
}


// ------------------------------------------------------------


static QkUniqueCFRef<CTFontDescriptorRef> create_descriptor(const char familyName[],
                                                            const FontStyle& style) {
    QkUniqueCFRef<CFMutableDictionaryRef> cfAttributes(
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));

		QkUniqueCFRef<CFMutableDictionaryRef> cfTraits(
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));

    if (!cfAttributes || !cfTraits) {
        return nullptr;
    }

    // TODO(crbug.com/1018581) Some CoreText versions have errant behavior when
    // certain traits set.  Temporary workaround to omit specifying trait for those
    // versions.
    // Long term solution will involve serializing typefaces instead of relying upon
    // this to match between processes.
    //
    // Compare CoreText.h in an up to date SDK for where these values come from.
    static const uint32_t kSkiaLocalCTVersionNumber10_14 = 0x000B0000;
    static const uint32_t kSkiaLocalCTVersionNumber10_15 = 0x000C0000;

    // CTFontTraits (symbolic)
    // macOS 14 and iOS 12 seem to behave badly when kCTFontSymbolicTrait is set.
    // macOS 15 yields LastResort font instead of a good default font when
    // kCTFontSymbolicTrait is set.
    if (QkGetCoreTextVersion() < kSkiaLocalCTVersionNumber10_14) {
        CTFontSymbolicTraits ctFontTraits = 0;
        if (style.weight() >= TextWeight::BOLD) {
            ctFontTraits |= kCTFontBoldTrait;
        }
        if (style.slant() != TextSlant::NORMAL) {
            ctFontTraits |= kCTFontItalicTrait;
        }
        QkUniqueCFRef<CFNumberRef> cfFontTraits(
                CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &ctFontTraits));
        if (cfFontTraits) {
            CFDictionaryAddValue(cfTraits.get(), kCTFontSymbolicTrait, cfFontTraits.get());
        }
    }

    // CTFontTraits (weight)
    CGFloat ctWeight = CTFontCTWeightForCSSWeight(style.weight());
    QkUniqueCFRef<CFNumberRef> cfFontWeight(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctWeight));
    if (cfFontWeight) {
        CFDictionaryAddValue(cfTraits.get(), kCTFontWeightTrait, cfFontWeight.get());
    }
    // CTFontTraits (width)
    CGFloat ctWidth = CTFontCTWidthForCSSWidth(style.width());
    SkUniqueCFRef<CFNumberRef> cfFontWidth(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctWidth));
    if (cfFontWidth) {
        CFDictionaryAddValue(cfTraits.get(), kCTFontWidthTrait, cfFontWidth.get());
    }
    // CTFontTraits (slant)
    // macOS 15 behaves badly when kCTFontSlantTrait is set.
    if (SkGetCoreTextVersion() != kSkiaLocalCTVersionNumber10_15) {
        CGFloat ctSlant = style.slant() == SkFontStyle::kUpright_Slant ? 0 : 1;
        SkUniqueCFRef<CFNumberRef> cfFontSlant(
                CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctSlant));
        if (cfFontSlant) {
            CFDictionaryAddValue(cfTraits.get(), kCTFontSlantTrait, cfFontSlant.get());
        }
    }
    // CTFontTraits
    CFDictionaryAddValue(cfAttributes.get(), kCTFontTraitsAttribute, cfTraits.get());

    // CTFontFamilyName
    if (familyName) {
        SkUniqueCFRef<CFStringRef> cfFontName = make_CFString(familyName);
        if (cfFontName) {
            CFDictionaryAddValue(cfAttributes.get(), kCTFontFamilyNameAttribute, cfFontName.get());
        }
    }

    return SkUniqueCFRef<CTFontDescriptorRef>(
            CTFontDescriptorCreateWithAttributes(cfAttributes.get()));
}

// ------------------------------------------------------------

// Same as the above function except style is included so we can
// compare whether the created font conforms to the style. If not, we need
// to recreate the font with symbolic traits. This is needed due to MacOS 10.11
// font creation problem https://bugs.chromium.org/p/skia/issues/detail?id=8447.
static sk_sp<SkTypeface> create_from_desc_and_style(CTFontDescriptorRef desc,
                                                    const SkFontStyle& style) {
    QkUniqueCFRef<CTFontRef> ctFont(CTFontCreateWithFontDescriptor(desc, 0, nullptr));
    if (!ctFont) {
        return nullptr;
    }

    const CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(ctFont.get());
    CTFontSymbolicTraits expected_traits = traits;
    if (style.slant() != SkFontStyle::kUpright_Slant) {
        expected_traits |= kCTFontItalicTrait;
    }
    if (style.weight() >= SkFontStyle::kBold_Weight) {
        expected_traits |= kCTFontBoldTrait;
    }

    if (expected_traits != traits) {
        SkUniqueCFRef<CTFontRef> ctNewFont(CTFontCreateCopyWithSymbolicTraits(
                    ctFont.get(), 0, nullptr, expected_traits, expected_traits));
        if (ctNewFont) {
            ctFont = std::move(ctNewFont);
        }
    }

    return SkTypeface_Mac::Make(std::move(ctFont), OpszVariation(), nullptr);
}

/** Creates a typeface from a name, searching the cache. */
static sk_sp<SkTypeface> create_from_name(const char familyName[], const SkFontStyle& style) {
	QkUniqueCFRef<CTFontDescriptorRef> desc = create_descriptor(familyName, style);
	if (!desc) {
		return nullptr;
	}
	return create_from_desc_and_style(desc.get(), style);
}

static const char* map_css_names(const char* name) {
	static const struct {
		const char* fFrom;  // name the caller specified
		const char* fTo;    // "canonical" name we map to
	} gPairs[] = {
		{ "sans-serif", "Helvetica" },
		{ "serif",      "Times"     },
		{ "monospace",  "Courier"   }
	};

	for (size_t i = 0; i < SK_ARRAY_COUNT(gPairs); i++) {
		if (strcmp(name, gPairs[i].fFrom) == 0) {
			return gPairs[i].fTo;
		}
	}
	return name;    // no change
}
