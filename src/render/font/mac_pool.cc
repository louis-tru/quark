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

#include "pool.h"
#include "mac_util.h"
#include "mac_typeface.h"
#include "style.h"
#include <dlfcn.h>

using namespace quark;

#if (Qk_IOS && defined(__IPHONE_14_0) && __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_14_0) ||  \
	  (Qk_OSX && defined(__MAC_11_0)    && __MAC_OS_VERSION_MIN_REQUIRED    >= __MAC_11_0)

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

static QkUniqueCFRef<CTFontDescriptorRef> create_descriptor(const char familyName[], const FontStyle& style) {
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
		if (style.slant() != TextSlant::DEFAULT) {
			ctFontTraits |= kCTFontItalicTrait;
		}
		QkUniqueCFRef<CFNumberRef> cfFontTraits(
						CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &ctFontTraits));
		if (cfFontTraits) {
			CFDictionaryAddValue(cfTraits.get(), kCTFontSymbolicTrait, cfFontTraits.get());
		}
	}

	// CTFontTraits (weight)
	CGFloat ctWeight = QkCTFontCTWeightForCSSWeight(style.weight());
	QkUniqueCFRef<CFNumberRef> cfFontWeight(
					CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctWeight));
	if (cfFontWeight) {
		CFDictionaryAddValue(cfTraits.get(), kCTFontWeightTrait, cfFontWeight.get());
	}
	// CTFontTraits (width)
	CGFloat ctWidth = QkCTFontCTWidthForCSSWidth(style.width());
	QkUniqueCFRef<CFNumberRef> cfFontWidth(
					CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctWidth));
	if (cfFontWidth) {
		CFDictionaryAddValue(cfTraits.get(), kCTFontWidthTrait, cfFontWidth.get());
	}
	// CTFontTraits (slant)
	// macOS 15 behaves badly when kCTFontSlantTrait is set.
	if (QkGetCoreTextVersion() != kSkiaLocalCTVersionNumber10_15) {
			
		CGFloat ctSlant = style.slant() == TextSlant::DEFAULT ? 0 : 1;
		QkUniqueCFRef<CFNumberRef> cfFontSlant(
						CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctSlant));
		if (cfFontSlant) {
			CFDictionaryAddValue(cfTraits.get(), kCTFontSlantTrait, cfFontSlant.get());
		}
	}
	// CTFontTraits
	CFDictionaryAddValue(cfAttributes.get(), kCTFontTraitsAttribute, cfTraits.get());

	// CTFontFamilyName
	if (familyName) {
		QkUniqueCFRef<CFStringRef> cfFontName = make_CFString(familyName);
		if (cfFontName) {
			CFDictionaryAddValue(cfAttributes.get(), kCTFontFamilyNameAttribute, cfFontName.get());
		}
	}

	return QkUniqueCFRef<CTFontDescriptorRef>(
					CTFontDescriptorCreateWithAttributes(cfAttributes.get()));
}

/** Creates a typeface from a descriptor, searching the cache. */
static Typeface* create_from_desc(CTFontDescriptorRef desc) {
	QkUniqueCFRef<CTFontRef> ctFont(CTFontCreateWithFontDescriptor(desc, 0, nullptr));
	if (!ctFont) {
		return nullptr;
	}
	return new QkTypeface_Mac(std::move(ctFont), OpszVariation(), WeakBuffer());
}

// -----------------------------------------------------------------------------

static QkUniqueCFRef<CFDataRef> cfdata_from_skdata(cBuffer& data) {
	void const * const addr = data.val();
	size_t const size = data.length();

	CFAllocatorContext ctx = {
			0, // CFIndex version
			(void*)addr, // void* info
			nullptr, // const void *(*retain)(const void *info);
			nullptr, // void (*release)(const void *info);
			nullptr, // CFStringRef (*copyDescription)(const void *info);
			nullptr, // void * (*allocate)(CFIndex size, CFOptionFlags hint, void *info);
			nullptr, // void*(*reallocate)(void* ptr,CFIndex newsize,CFOptionFlags hint,void* info);
			[](void*,void* info) -> void { // void (*deallocate)(void *ptr, void *info);
				// Qk_ASSERT(info);
				// Buffer::Alloc::free(info);
			},
			nullptr, // CFIndex (*preferredSize)(CFIndex size, CFOptionFlags hint, void *info);
	};
	QkUniqueCFRef<CFAllocatorRef> alloc(CFAllocatorCreate(kCFAllocatorDefault, &ctx));
	return QkUniqueCFRef<CFDataRef>(CFDataCreateWithBytesNoCopy(
					kCFAllocatorDefault, (const UInt8 *)addr, size, alloc.get()));
}

static QkUniqueCFRef<CTFontRef> ctfont_from_skdata(cBuffer& data, int ttcIndex) {
	// TODO: Use CTFontManagerCreateFontDescriptorsFromData when available.
	if (ttcIndex != 0) {
		return nullptr;
	}

	QkUniqueCFRef<CFDataRef> cfData(cfdata_from_skdata(data));

	QkUniqueCFRef<CTFontDescriptorRef> desc(CTFontManagerCreateFontDescriptorFromData(cfData.get()));
	if (!desc) {
		return nullptr;
	}
	return QkUniqueCFRef<CTFontRef>(CTFontCreateWithFontDescriptor(desc.get(), 0, nullptr));
}

QkUniqueCFRef<CFArrayRef> QkCopyAvailableFontFamilyNames(CTFontCollectionRef collection) {
		// Create a CFArray of all available font descriptors.
	QkUniqueCFRef<CFArrayRef> descriptors(
			CTFontCollectionCreateMatchingFontDescriptors(collection));

	// Copy the font family names of the font descriptors into a CFSet.
	auto addDescriptorFamilyNameToSet = [](const void* value, void* context) -> void {
			CTFontDescriptorRef descriptor = static_cast<CTFontDescriptorRef>(value);
			CFMutableSetRef familyNameSet = static_cast<CFMutableSetRef>(context);
		QkUniqueCFRef<CFTypeRef> familyName(
					CTFontDescriptorCopyAttribute(descriptor, kCTFontFamilyNameAttribute));
		if (familyName) {
			CFSetAddValue(familyNameSet, familyName.get());
		}
	};
	QkUniqueCFRef<CFMutableSetRef> familyNameSet(
			CFSetCreateMutable(kCFAllocatorDefault, 0, &kCFTypeSetCallBacks));
	CFArrayApplyFunction(descriptors.get(), CFRangeMake(0, CFArrayGetCount(descriptors.get())),
											 addDescriptorFamilyNameToSet, familyNameSet.get());

	// Get the set of family names into an array; this does not retain.
	CFIndex count = CFSetGetCount(familyNameSet.get());
	std::unique_ptr<const void*[]> familyNames(new const void*[count]);
	CFSetGetValues(familyNameSet.get(), familyNames.get());

	// Sort the array of family names (to match CTFontManagerCopyAvailableFontFamilyNames).
	std::sort(familyNames.get(), familyNames.get() + count, [](const void* a, const void* b){
		return CFStringCompare((CFStringRef)a, (CFStringRef)b, 0) == kCFCompareLessThan;
	});

	// Copy family names into a CFArray; this does retain.
	return QkUniqueCFRef<CFArrayRef>(
			CFArrayCreate(kCFAllocatorDefault, familyNames.get(), count, &kCFTypeArrayCallBacks));
}

/** Use CTFontManagerCopyAvailableFontFamilyNames if available, simulate if not. */
QkUniqueCFRef<CFArrayRef> QkCTFontManagerCopyAvailableFontFamilyNames() {
#if Qk_IOS
	using CTFontManagerCopyAvailableFontFamilyNamesProc = CFArrayRef (*)(void);
	CTFontManagerCopyAvailableFontFamilyNamesProc ctFontManagerCopyAvailableFontFamilyNames;
	*(void**)(&ctFontManagerCopyAvailableFontFamilyNames) =
		dlsym(RTLD_DEFAULT, "CTFontManagerCopyAvailableFontFamilyNames");
	if (ctFontManagerCopyAvailableFontFamilyNames) {
		return QkUniqueCFRef<CFArrayRef>(ctFontManagerCopyAvailableFontFamilyNames());
	}
	QkUniqueCFRef<CTFontCollectionRef> collection(
		CTFontCollectionCreateFromAvailableFonts(nullptr));
	return QkUniqueCFRef<CFArrayRef>(QkCopyAvailableFontFamilyNames(collection.get()));
#else
	return QkUniqueCFRef<CFArrayRef>(CTFontManagerCopyAvailableFontFamilyNames());
#endif
}

class QkFontPool_Mac : public FontPool {
	QkUniqueCFRef<CFArrayRef> fNames;
	int fCount;

	CFStringRef getFamilyNameAt(int index) const {
		Qk_ASSERT((unsigned)index < (unsigned)fCount);
		return (CFStringRef)CFArrayGetValueAtIndex(fNames.get(), index);
	}

public:
	QkFontPool_Mac(CTFontCollectionRef fontCollection)
		: fNames(fontCollection ? QkCopyAvailableFontFamilyNames(fontCollection)
														: QkCTFontManagerCopyAvailableFontFamilyNames())
		, fCount(fNames ? int(CFArrayGetCount(fNames.get())) : 0)
	{}

protected:
	int onCountFamilies() const override {
		return fCount;
	}

	String onGetFamilyName(int index) const override {
		if ((unsigned)index < (unsigned)fCount) {
			return QkStringFromCFString(getFamilyNameAt(index));
		} else {
			return String();
		}
	}

	Typeface* onMatchFamilyStyle(const char familyName[],
																FontStyle style) const override {
		QkUniqueCFRef<CTFontDescriptorRef> desc = create_descriptor(familyName, style);
		return create_from_desc(desc.get());
	}

	Typeface* onMatchFamilyStyleCharacter(const char familyName[],
																				FontStyle style,
																				Unichar character) const override {
		QkUniqueCFRef<CTFontDescriptorRef> desc = create_descriptor(familyName, style);
		QkUniqueCFRef<CTFontRef> familyFont(CTFontCreateWithFontDescriptor(desc.get(), 0, nullptr));

		// kCFStringEncodingUTF32 is BE unless there is a BOM.
		// Since there is no machine endian option, explicitly state machine endian.
#ifdef Qk_CPU_LENDIAN
		constexpr CFStringEncoding encoding = kCFStringEncodingUTF32LE;
#else
		constexpr CFStringEncoding encoding = kCFStringEncodingUTF32BE;
#endif
		QkUniqueCFRef<CFStringRef> string(CFStringCreateWithBytes(
						kCFAllocatorDefault, reinterpret_cast<const UInt8 *>(&character), sizeof(character),
						encoding, false));
		// If 0xD800 <= codepoint <= 0xDFFF || 0x10FFFF < codepoint 'string' may be nullptr.
		// No font should be covering such codepoints (even the magic fallback font).
		if (!string) {
			return nullptr;
		}
		CFRange range = CFRangeMake(0, CFStringGetLength(string.get()));  // in UniChar units.
		QkUniqueCFRef<CTFontRef> fallbackFont(
						CTFontCreateForString(familyFont.get(), string.get(), range));
		return new QkTypeface_Mac(std::move(fallbackFont), OpszVariation(), WeakBuffer());
	}

	Typeface* onMakeFromData(cBuffer& data, int ttcIndex) const override {
		if (ttcIndex != 0) {
			return nullptr;
		}
		QkUniqueCFRef<CTFontRef> ct = ctfont_from_skdata(data, ttcIndex);
		if (!ct) {
			return nullptr;
		}
		return new QkTypeface_Mac(std::move(ct), OpszVariation(), data);
	}

};
