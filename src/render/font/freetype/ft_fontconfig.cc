/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include <fontconfig/fontconfig.h>
#include <string.h>

#include "./ft_typeface.h"
#include "../pool.h"
#include "../../../util/fs.h"

// FC_POSTSCRIPT_NAME was added with b561ff20 which ended up in 2.10.92
// Ubuntu 14.04 is on 2.11.0
// Debian 8 and 9 are on 2.11
// OpenSUSE Leap 42.1 is on 2.11.0 (42.3 is on 2.11.1)
// Fedora 24 is on 2.11.94
#ifndef FC_POSTSCRIPT_NAME
#  define FC_POSTSCRIPT_NAME  "postscriptname"
#endif

/** Since FontConfig is poorly documented, this gives a high level overview:
 *
 *  FcConfig is a handle to a FontConfig configuration instance. Each 'configuration' is independent
 *  from any others which may exist. There exists a default global configuration which is created
 *  and destroyed by FcInit and FcFini, but this default should not normally be used.
 *  Instead, one should use FcConfigCreate and FcInit* to have a named local state.
 *
 *  FcPatterns are {objectName -> [element]} (maps from object names to a list of elements).
 *  Each element is some internal data plus an FcValue which is a variant (a union with a type tag).
 *  Lists of elements are not typed, except by convention. Any collection of FcValues must be
 *  assumed to be heterogeneous by the code, but the code need not do anything particularly
 *  interesting if the values go against convention.
 *
 *  Somewhat like DirectWrite, FontConfig supports synthetics through FC_EMBOLDEN and FC_MATRIX.
 *  Like all synthetic information, such information must be passed with the font data.
 */

namespace {

// FontConfig was thread antagonistic until 2.10.91 with known thread safety issues until 2.13.93.
// Before that, lock with a global mutex.
// See https://bug.Qkia.org/1497 and cl/339089311 for background.
static QkMutex& f_c_mutex() {
	static QkMutex& mutex = *(new QkMutex);
	return mutex;
}

class FCLocker {
	static constexpr int FontConfigThreadSafeVersion = 21393;

	// Assume FcGetVersion() has always been thread safe.
	static void lock() Qk_NO_THREAD_SAFETY_ANALYSIS {
		if (FcGetVersion() < FontConfigThreadSafeVersion) {
			f_c_mutex().lock();
		}
	}
	static void unlock() Qk_NO_THREAD_SAFETY_ANALYSIS {
		AssertHeld();
		if (FcGetVersion() < FontConfigThreadSafeVersion) {
			f_c_mutex().unlock();
		}
	}

public:
	FCLocker() { lock(); }
	~FCLocker() { unlock(); }

	static void AssertHeld() { Qk_DEBUGCODE(
		if (FcGetVersion() < FontConfigThreadSafeVersion) {
			f_c_mutex().assertHeld();
		}
	) }
};

} // namespace

template<typename T, void (*D)(T*)> void FcTDestroy(T* t) {
	if (t) {
		FCLocker::AssertHeld();
		D(t);
	}
}

template <typename T, T* (*C)(), void (*D)(T*)> class QkAutoFc
	: public Sp<T, object_traits_from<T, FcTDestroy<T, D>>> {
	using inherited = Sp<T, object_traits_from<T, FcTDestroy<T, D>>>;
public:
	QkAutoFc(): inherited(C()) {
		T* obj = this->operator T*();
		Qk_CHECK(nullptr != obj);
	}
	explicit QkAutoFc(T* obj): inherited(obj) {}
	QkAutoFc(QkAutoFc&& that): inherited(std::move(that)) {}
	operator T*() { return this->get(); }

	QkAutoFc& operator=(T *data) {
		inherited::operator=(data);
		return *this;
	}
};

typedef QkAutoFc<FcCharSet, FcCharSetCreate, FcCharSetDestroy> QkAutoFcCharSet;
typedef QkAutoFc<FcConfig, FcConfigCreate, FcConfigDestroy> QkAutoFcConfig;
typedef QkAutoFc<FcFontSet, FcFontSetCreate, FcFontSetDestroy> QkAutoFcFontSet;
typedef QkAutoFc<FcLangSet, FcLangSetCreate, FcLangSetDestroy> QkAutoFcLangSet;
typedef QkAutoFc<FcObjectSet, FcObjectSetCreate, FcObjectSetDestroy> QkAutoFcObjectSet;
typedef QkAutoFc<FcPattern, FcPatternCreate, FcPatternDestroy> QkAutoFcPattern;

static bool get_bool(FcPattern* pattern, const char object[], bool missing = false) {
	FcBool value;
	if (FcPatternGetBool(pattern, object, 0, &value) != FcResultMatch) {
		return missing;
	}
	return value;
}

static int get_int(FcPattern* pattern, const char object[], int missing) {
	int value;
	if (FcPatternGetInteger(pattern, object, 0, &value) != FcResultMatch) {
		return missing;
	}
	return value;
}

static const char* get_string(FcPattern* pattern, const char object[], const char* missing = "") {
	FcChar8* value;
	if (FcPatternGetString(pattern, object, 0, &value) != FcResultMatch) {
		return missing;
	}
	return (const char*)value;
}

static const FcMatrix* get_matrix(FcPattern* pattern, const char object[]) {
	FcMatrix* matrix;
	if (FcPatternGetMatrix(pattern, object, 0, &matrix) != FcResultMatch) {
		return nullptr;
	}
	return matrix;
}

enum QkWeakReturn {
	kIsWeak_WeakReturn,
	kIsStrong_WeakReturn,
	kNoId_WeakReturn
};
/** Ideally there  would exist a call like
 *  FcResult FcPatternIsWeak(pattern, object, id, FcBool* isWeak);
 *  Sometime after 2.12.4 FcPatternGetWithBinding was added which can retrieve the binding.
 *
 *  However, there is no such call and as of Fc 2.11.0 even FcPatternEquals ignores the weak bit.
 *  Currently, the only reliable way of finding the weak bit is by its effect on matching.
 *  The weak bit only affects the matching of FC_FAMILY and FC_POSTSCRIPT_NAME object values.
 *  A element with the weak bit is scored after FC_LANG, without the weak bit is scored before.
 *  Note that the weak bit is stored on the element, not on the value it holds.
 */
static QkWeakReturn is_weak(FcPattern* pattern, const char object[], int id) {
	FCLocker::AssertHeld();

	FcResult result;

	// Create a copy of the pattern with only the value 'pattern'['object'['id']] in it.
	// Internally, FontConfig pattern objects are linked lists, so faster to remove from head.
	QkAutoFcObjectSet requestedObjectOnly(FcObjectSetBuild(object, nullptr));
	QkAutoFcPattern minimal(FcPatternFilter(pattern, requestedObjectOnly));
	FcBool hasId = true;
	for (int i = 0; hasId && i < id; ++i) {
		hasId = FcPatternRemove(minimal, object, 0);
	}
	if (!hasId) {
		return kNoId_WeakReturn;
	}
	FcValue value;
	result = FcPatternGet(minimal, object, 0, &value);
	if (result != FcResultMatch) {
		return kNoId_WeakReturn;
	}
	while (hasId) {
		hasId = FcPatternRemove(minimal, object, 1);
	}

	// Create a font set with two patterns.
	// 1. the same 'object' as minimal and a lang object with only 'nomatchlang'.
	// 2. a different 'object' from minimal and a lang object with only 'matchlang'.
	QkAutoFcFontSet fontSet;

	QkAutoFcLangSet strongLangSet;
	FcLangSetAdd(strongLangSet, (const FcChar8*)"nomatchlang");
	QkAutoFcPattern strong(FcPatternDuplicate(minimal));
	FcPatternAddLangSet(strong, FC_LANG, strongLangSet);

	QkAutoFcLangSet weakLangSet;
	FcLangSetAdd(weakLangSet, (const FcChar8*)"matchlang");
	QkAutoFcPattern weak;
	FcPatternAddString(weak, object, (const FcChar8*)"nomatchstring");
	FcPatternAddLangSet(weak, FC_LANG, weakLangSet);

	FcFontSetAdd(fontSet, strong.collapse());
	FcFontSetAdd(fontSet, weak.collapse());

	// Add 'matchlang' to the copy of the pattern.
	FcPatternAddLangSet(minimal, FC_LANG, weakLangSet);

	// Run a match against the copy of the pattern.
	// If the 'id' was weak, then we should match the pattern with 'matchlang'.
	// If the 'id' was strong, then we should match the pattern with 'nomatchlang'.

	// Note that this config is only used for FcFontRenderPrepare, which we don't even want.
	// However, there appears to be no way to match/sort without it.
	QkAutoFcConfig config;
	FcFontSet* fontSets[1] = { fontSet };
	QkAutoFcPattern match(FcFontSetMatch(config, fontSets, Qk_ARRAY_COUNT(fontSets),
										 minimal, &result));

	FcLangSet* matchLangSet;
	FcPatternGetLangSet(match, FC_LANG, 0, &matchLangSet);
	return FcLangEqual == FcLangSetHasLang(matchLangSet, (const FcChar8*)"matchlang")
						? kIsWeak_WeakReturn : kIsStrong_WeakReturn;
}

/** Removes weak elements from either FC_FAMILY or FC_POSTSCRIPT_NAME objects in the property.
 *  This can be quite expensive, and should not be used more than once per font lookup.
 *  This removes all of the weak elements after the last strong element.
 */
static void remove_weak(FcPattern* pattern, const char object[]) {
	FCLocker::AssertHeld();

	QkAutoFcObjectSet requestedObjectOnly(FcObjectSetBuild(object, nullptr));
	QkAutoFcPattern minimal(FcPatternFilter(pattern, requestedObjectOnly));

	int lastStrongId = -1;
	int numIds;
	QkWeakReturn result;
	for (int id = 0; ; ++id) {
		result = is_weak(minimal, object, 0);
		if (kNoId_WeakReturn == result) {
			numIds = id;
			break;
		}
		if (kIsStrong_WeakReturn == result) {
			lastStrongId = id;
		}
		Qk_ASSERT_NE(0, FcPatternRemove(minimal, object, 0));
	}

	// If they were all weak, then leave the pattern alone.
	if (lastStrongId < 0) {
		return;
	}

	// Remove everything after the last strong.
	for (int id = lastStrongId + 1; id < numIds; ++id) {
		Qk_ASSERT_NE(0, FcPatternRemove(pattern, object, lastStrongId + 1));
	}
}

static int map_range(QkScalar value,
					 QkScalar old_min, QkScalar old_max,
					 QkScalar new_min, QkScalar new_max)
{
	Qk_ASSERT(old_min < old_max);
	Qk_ASSERT(new_min <= new_max);
	return new_min + ((value - old_min) * (new_max - new_min) / (old_max - old_min));
}

struct MapRanges {
	QkScalar old_val;
	QkScalar new_val;
};

static QkScalar map_ranges(QkScalar val, MapRanges const ranges[], int rangesCount) {
	// -Inf to [0]
	if (val < ranges[0].old_val) {
		return ranges[0].new_val;
	}

	// Linear from [i] to [i+1]
	for (int i = 0; i < rangesCount - 1; ++i) {
		if (val < ranges[i+1].old_val) {
			return map_range(val, ranges[i].old_val, ranges[i+1].old_val,
								  ranges[i].new_val, ranges[i+1].new_val);
		}
	}

	// From [n] to +Inf
	// if (fcweight < Inf)
	return ranges[rangesCount-1].new_val;
}

#ifndef FC_WEIGHT_DEMILIGHT
#define FC_WEIGHT_DEMILIGHT        65
#endif

static FontStyle Qkfontstyle_from_fcpattern(FcPattern* pattern) {

	// FcWeightToOpenType was buggy until 2.12.4
	static constexpr MapRanges weightRanges[] = {
		{ FC_WEIGHT_THIN,       float(TextWeight::Thin) },
		{ FC_WEIGHT_EXTRALIGHT, float(TextWeight::Ultralight) },
		{ FC_WEIGHT_LIGHT,      float(TextWeight::Light) },
		{ FC_WEIGHT_DEMILIGHT,  350 },
		{ FC_WEIGHT_BOOK,       380 },
		{ FC_WEIGHT_REGULAR,    float(TextWeight::Regular) },
		{ FC_WEIGHT_MEDIUM,     float(TextWeight::Medium) },
		{ FC_WEIGHT_DEMIBOLD,   float(TextWeight::Semibold) },
		{ FC_WEIGHT_BOLD,       float(TextWeight::Bold) },
		{ FC_WEIGHT_EXTRABOLD,  float(TextWeight::Heavy) },
		{ FC_WEIGHT_BLACK,      float(TextWeight::Black) },
		{ FC_WEIGHT_EXTRABLACK, float(TextWeight::ExtraBlack) },
	};
	QkScalar weight = map_ranges(get_int(pattern, FC_WEIGHT, FC_WEIGHT_REGULAR),
								weightRanges, Qk_ARRAY_COUNT(weightRanges));

	static constexpr MapRanges widthRanges[] = {
		{ FC_WIDTH_ULTRACONDENSED, float(TextWidth::UltraCondensed) },
		{ FC_WIDTH_EXTRACONDENSED, float(TextWidth::ExtraCondensed) },
		{ FC_WIDTH_CONDENSED,      float(TextWidth::Condensed) },
		{ FC_WIDTH_SEMICONDENSED,  float(TextWidth::SemiCondensed) },
		{ FC_WIDTH_NORMAL,         float(TextWidth::Normal) },
		{ FC_WIDTH_SEMIEXPANDED,   float(TextWidth::SemiExpanded) },
		{ FC_WIDTH_EXPANDED,       float(TextWidth::Expanded) },
		{ FC_WIDTH_EXTRAEXPANDED,  float(TextWidth::ExtraExpanded) },
		{ FC_WIDTH_ULTRAEXPANDED,  float(TextWidth::UltraExpanded) },
	};
	QkScalar width = map_ranges(get_int(pattern, FC_WIDTH, FC_WIDTH_NORMAL),
								widthRanges, Qk_ARRAY_COUNT(widthRanges));

	TextSlant slant = TextSlant::Normal;
	switch (get_int(pattern, FC_SLANT, FC_SLANT_ROMAN)) {
		case FC_SLANT_ROMAN:   slant = TextSlant::Normal; break;
		case FC_SLANT_ITALIC : slant = TextSlant::Italic ; break;
		case FC_SLANT_OBLIQUE: slant = TextSlant::Oblique; break;
		default: Qk_ASSERT(false); break;
	}

	return FontStyle(TextWeight(QkScalarRoundToInt(weight)),
		TextWidth(QkScalarRoundToInt(width)), slant);
}

static void fcpattern_from_Qkfontstyle(FontStyle style, FcPattern* pattern) {
	FCLocker::AssertHeld();

	// FcWeightFromOpenType was buggy until 2.12.4
	static constexpr MapRanges weightRanges[] = {
		{ float(TextWeight::Thin),       FC_WEIGHT_THIN },
		{ float(TextWeight::Ultralight), FC_WEIGHT_EXTRALIGHT },
		{ float(TextWeight::Light),      FC_WEIGHT_LIGHT },
		{ 350,                           FC_WEIGHT_DEMILIGHT },
		{ 380,                           FC_WEIGHT_BOOK },
		{ float(TextWeight::Regular),    FC_WEIGHT_REGULAR },
		{ float(TextWeight::Medium),     FC_WEIGHT_MEDIUM },
		{ float(TextWeight::Semibold),   FC_WEIGHT_DEMIBOLD },
		{ float(TextWeight::Bold),       FC_WEIGHT_BOLD },
		{ float(TextWeight::Heavy),      FC_WEIGHT_EXTRABOLD },
		{ float(TextWeight::Black),      FC_WEIGHT_BLACK },
		{ float(TextWeight::ExtraBlack), FC_WEIGHT_EXTRABLACK },
	};
	int weight = map_ranges(float(style.weight()), weightRanges, Qk_ARRAY_COUNT(weightRanges));

	static constexpr MapRanges widthRanges[] = {
		{ float(TextWidth::UltraCondensed), FC_WIDTH_ULTRACONDENSED },
		{ float(TextWidth::ExtraCondensed), FC_WIDTH_EXTRACONDENSED },
		{ float(TextWidth::Condensed),      FC_WIDTH_CONDENSED },
		{ float(TextWidth::SemiCondensed),  FC_WIDTH_SEMICONDENSED },
		{ float(TextWidth::Normal),         FC_WIDTH_NORMAL },
		{ float(TextWidth::SemiExpanded),   FC_WIDTH_SEMIEXPANDED },
		{ float(TextWidth::Expanded),       FC_WIDTH_EXPANDED },
		{ float(TextWidth::ExtraExpanded),  FC_WIDTH_EXTRAEXPANDED },
		{ float(TextWidth::UltraExpanded),  FC_WIDTH_ULTRAEXPANDED },
	};
	int width = map_ranges(float(style.width()), widthRanges, Qk_ARRAY_COUNT(widthRanges));

	int slant = FC_SLANT_ROMAN;
	switch (style.slant()) {
		case TextSlant::Normal:  slant = FC_SLANT_ROMAN  ; break;
		case TextSlant::Italic : slant = FC_SLANT_ITALIC ; break;
		case TextSlant::Oblique: slant = FC_SLANT_OBLIQUE; break;
		default: Qk_ASSERT(false); break;
	}

	FcPatternAddInteger(pattern, FC_WEIGHT, weight);
	FcPatternAddInteger(pattern, FC_WIDTH , width);
	FcPatternAddInteger(pattern, FC_SLANT , slant);
}

class Typeface_stream : public QkTypeface_FreeType {
public:
	Typeface_stream(Sp<QkFontData> data,
					String familyName, const FontStyle& style, bool fixedWidth)
		: INHERITED(style, 0)
		, fFamilyName(std::move(familyName))
		, fData(std::move(data))
	{
		initFreeType();
	}

	String onGetFamilyName() const override { 
		return fFamilyName;
	}

	Sp<QkFontData> onMakeFontData() const override {
		return new QkFontData(*fData.get());
	}

private:
	String fFamilyName;
	const Sp<QkFontData> fData;

	using INHERITED = QkTypeface_FreeType;
};

class Typeface_fontconfig : public QkTypeface_FreeType {
public:
	static Sp<Typeface_fontconfig> Make(QkAutoFcPattern pattern, cString& sysroot) {
		return new Typeface_fontconfig(pattern, sysroot);
	}
	mutable QkAutoFcPattern fPattern;  // Mutable for passing to FontConfig API.
	const String fSysroot;

	String onGetFamilyName() const override {
		return get_string(fPattern, FC_FAMILY);
	}

	QkStream* onOpenStream(int* ttcIndex) const {
		FCLocker lock;
		*ttcIndex = get_int(fPattern, FC_INDEX, 0);
		const char* filename = get_string(fPattern, FC_FILE);
		// See FontAccessible for note on searching sysroot then non-sysroot path.
		String resolvedFilename;
		if (!fSysroot.is_empty()) {
			resolvedFilename = fSysroot;
			resolvedFilename += filename;
			if (fs_readable_sync(resolvedFilename)) {
				filename = resolvedFilename.c_str();
			}
		}
		return QkStream::Make(filename);
	}

	Sp<QkFontData> onMakeFontData() const override {
		int index;
		auto stream = this->onOpenStream(&index);
		if (!stream) {
			return nullptr;
		}
		// TODO: FC_VARIABLE and FC_FONT_VARIATIONS
		return new QkFontData(stream, index, nullptr, 0);
	}

	~Typeface_fontconfig() override {
		// Hold the lock while unrefing the pattern.
		FCLocker lock;
		fPattern.release();
	}

private:
	Typeface_fontconfig(QkAutoFcPattern& pattern, cString& sysroot)
		: INHERITED(Qkfontstyle_from_fcpattern(pattern),
					FC_PROPORTIONAL != get_int(pattern, FC_SPACING, FC_PROPORTIONAL))
		, fPattern(std::move(pattern))
		, fSysroot(sysroot)
	{
		initFreeType();
	}

	using INHERITED = QkTypeface_FreeType;
};

class QkFontPool_fontconfig : public FontPool {
	mutable QkAutoFcConfig fFC;  // Only mutable to avoid const cast when passed to FontConfig API.
	const String fSysroot;
	const Array<String> fFamilyNames;
	const QkTypeface_FreeType::Scanner fScanner;

	static Array<String> GetFamilyNames(FcConfig* fcconfig) {
		FCLocker lock;
		Set<String> data;

		static const FcSetName fcNameSet[] = { FcSetSystem, FcSetApplication };
		for (int setIndex = 0; setIndex < (int)Qk_ARRAY_COUNT(fcNameSet); ++setIndex) {
			// Return value of FcConfigGetFonts must not be destroyed.
			FcFontSet* allFonts(FcConfigGetFonts(fcconfig, fcNameSet[setIndex]));
			if (nullptr == allFonts) {
				continue;
			}

			for (int fontIndex = 0; fontIndex < allFonts->nfont; ++fontIndex) {
				FcPattern* current = allFonts->fonts[fontIndex];
				for (int id = 0; ; ++id) {
					FcChar8* fcFamilyName;
					FcResult result = FcPatternGetString(current, FC_FAMILY, id, &fcFamilyName);
					if (FcResultNoId == result) {
						break;
					}
					if (FcResultMatch != result) {
						continue;
					}
					const char* familyName = reinterpret_cast<const char*>(fcFamilyName);
					if (familyName && !data.has(familyName)) {
						data.add(familyName);
					}
				}
			}
		}

		return data.keys();
	}

	mutable QkMutex fTFCacheMutex;
	mutable Array<Sp<Typeface_fontconfig>> fTFCache;

	/** Creates a typeface using a typeface cache.
	 *  @param pattern a complete pattern from FcFontRenderPrepare.
	 */
	Typeface* createTypefaceFromFcPattern(QkAutoFcPattern pattern) const {
		if (!pattern) {
			return nullptr;
		}
		// Cannot hold FCLocker when calling fTFCache.add; an evicted typeface may need to lock.
		// Must hold fTFCacheMutex when interacting with fTFCache.
		QkAutoMutexExclusive ama(fTFCacheMutex);
		{
			FCLocker lock;
			for (auto& it: fTFCache) {
				if (FcTrue == FcPatternEqual(it->fPattern, pattern)) {
					pattern.release();
					return it.get();
				}
			}
		}
		auto face = Typeface_fontconfig::Make(std::move(pattern), fSysroot);
		if (face) {
			// Cannot hold FCLocker in fTFCache.add; evicted typefaces may need to lock.
			fTFCache.push(std::move(face));
		}
		return face.get();
	}

public:
	/** Takes control of the reference to 'config'. */
	explicit QkFontPool_fontconfig(FcConfig* config)
		: fFC(config ? config : FcInitLoadConfigAndFonts())
		, fSysroot(reinterpret_cast<const char*>(FcConfigGetSysRoot(fFC)))
		, fFamilyNames(GetFamilyNames(fFC)) 
	{
		initFontPool();
	}

	~QkFontPool_fontconfig() override {
		// Hold the lock while unrefing the config.
		FCLocker lock;
		fFC.release();
	}

protected:
	uint32_t onCountFamilies() const override {
		return fFamilyNames.length();
	}

	String onGetFamilyName(int index) const override {
		return fFamilyNames.at(index);
	}

	/** True if any string object value in the font is the same
	 *         as a string object value in the pattern.
	 */
	static bool AnyMatching(FcPattern* font, FcPattern* pattern, const char* object) {
		FcChar8* fontString;
		FcChar8* patternString;
		FcResult result;
		// Set an arbitrary limit on the number of pattern object values to consider.
		// TODO: re-write this to avoid N*M
		static const int maxId = 16;
		for (int patternId = 0; patternId < maxId; ++patternId) {
			result = FcPatternGetString(pattern, object, patternId, &patternString);
			if (FcResultNoId == result) {
				break;
			}
			if (FcResultMatch != result) {
				continue;
			}
			for (int fontId = 0; fontId < maxId; ++fontId) {
				result = FcPatternGetString(font, object, fontId, &fontString);
				if (FcResultNoId == result) {
					break;
				}
				if (FcResultMatch != result) {
					continue;
				}
				if (0 == FcStrCmpIgnoreCase(patternString, fontString)) {
					return true;
				}
			}
		}
		return false;
	}

	bool FontAccessible(FcPattern* font) const {
		// FontConfig can return fonts which are unreadable.
		const char* filename = get_string(font, FC_FILE, nullptr);
		if (nullptr == filename) {
			return false;
		}

		// When sysroot was implemented in e96d7760886a3781a46b3271c76af99e15cb0146 (before 2.11.0)
		// it was broken;  mostly fixed in d17f556153fbaf8fe57fdb4fc1f0efa4313f0ecf (after 2.11.1).
		// This leaves Debian 8 and 9 with broken support for this feature.
		// As a result, this feature should not be used until at least 2.11.91.
		// The broken support is mostly around not making all paths relative to the sysroot.
		// However, even at 2.13.1 it is possible to get a mix of sysroot and non-sysroot paths,
		// as any added file path not lexically starting with the sysroot will be unchanged.
		// To allow users to add local app files outside the sysroot,
		// prefer the sysroot but also look without the sysroot.
		if (!fSysroot.is_empty()) {
			String resolvedFilename;
			resolvedFilename = fSysroot;
			resolvedFilename += filename;
			if (fs_readable_sync(resolvedFilename)) {
				return true;
			}
		}
		return fs_readable_sync(filename);
	}

	static bool FontFamilyNameMatches(FcPattern* font, FcPattern* pattern) {
		return AnyMatching(font, pattern, FC_FAMILY);
	}

	static bool FontContainsCharacter(FcPattern* font, uint32_t character) {
		FcResult result;
		FcCharSet* matchCharSet;
		for (int charSetId = 0; ; ++charSetId) {
			result = FcPatternGetCharSet(font, FC_CHARSET, charSetId, &matchCharSet);
			if (FcResultNoId == result) {
				break;
			}
			if (FcResultMatch != result) {
				continue;
			}
			if (FcCharSetHasChar(matchCharSet, character)) {
				return true;
			}
		}
		return false;
	}

	Typeface* onMatchFamilyStyle(cChar familyName[], FontStyle style) const override {
		QkAutoFcPattern font([this, &familyName, &style]() {
			FCLocker lock;
			QkAutoFcPattern pattern;
			FcPatternAddString(pattern, FC_FAMILY, (FcChar8*)familyName);
			fcpattern_from_Qkfontstyle(style, pattern);
			FcConfigSubstitute(fFC, pattern, FcMatchPattern);
			FcDefaultSubstitute(pattern);

			// We really want to match strong (preferred) and same (acceptable) only here.
			// If a family name was specified, assume that any weak matches after the last strong
			// match are weak (default) and ignore them.
			// After substitution the pattern for 'sans-serif' looks like "wwwwwwwwwwwwwwswww" where
			// there are many weak but preferred names, followed by defaults.
			// So it is possible to have weakly matching but preferred names.
			// In aliases, bindings are weak by default, so this is easy and common.
			// If no family name was specified, we'll probably only get weak matches, but that's ok.
			FcPattern* matchPattern;
			QkAutoFcPattern strongPattern(nullptr);
			if (familyName) {
				strongPattern = FcPatternDuplicate(pattern);
				remove_weak(strongPattern, FC_FAMILY);
				matchPattern = strongPattern;
			} else {
				matchPattern = pattern;
			}

			FcResult result;
			QkAutoFcPattern font(FcFontMatch(fFC, pattern, &result));
			if (!font || !FontAccessible(font) || !FontFamilyNameMatches(font, matchPattern)) {
				Qk_CHECK(0, "onMatchFamilyStyle, can't match");
			}
			return font;
		}());
		return createTypefaceFromFcPattern(std::move(font));
	}

	Typeface* onMatchFamilyStyleCharacter(cChar familyName[], FontStyle style,
			cChar* bcp47[], int bcp47Count, Unichar character) const override 
	{
		QkAutoFcPattern font([&](){
			FCLocker lock;

			QkAutoFcPattern pattern;
			if (familyName) {
				FcValue familyNameValue;
				familyNameValue.type = FcTypeString;
				familyNameValue.u.s = reinterpret_cast<const FcChar8*>(familyName);
				FcPatternAddWeak(pattern, FC_FAMILY, familyNameValue, FcFalse);
			}
			fcpattern_from_Qkfontstyle(style, pattern);

			QkAutoFcCharSet charSet;
			FcCharSetAddChar(charSet, character);
			FcPatternAddCharSet(pattern, FC_CHARSET, charSet);

			if (bcp47Count > 0) {
				Qk_ASSERT(bcp47);
				QkAutoFcLangSet langSet;
				for (int i = bcp47Count; i --> 0;) {
					FcLangSetAdd(langSet, (const FcChar8*)bcp47[i]);
				}
				FcPatternAddLangSet(pattern, FC_LANG, langSet);
			}

			FcConfigSubstitute(fFC, pattern, FcMatchPattern);
			FcDefaultSubstitute(pattern);

			FcResult result;
			QkAutoFcPattern font(FcFontMatch(fFC, pattern, &result));
			if (!font || !FontAccessible(font) || !FontContainsCharacter(font, character)) {
				Qk_CHECK(0, "onMatchFamilyStyleCharacter, can't match");
			}
			return font;
		}());
		return createTypefaceFromFcPattern(std::move(font));
	}

	Typeface* onAddFontFamily(cBuffer& data, int ttcIndex) const override {
		return this->onMakeFromStreamIndex(QkStream::Make(data.copy()), ttcIndex);
	}

	Typeface* onMakeFromStreamIndex(Sp<QkStream> stream, int ttcIndex) const {
		const size_t length = stream->getLength();
		if (length <= 0 || (1u << 30) < length) {
			return nullptr;
		}
		String name;
		FontStyle style;
		bool isFixedWidth;
		if (!fScanner.scanFont(*stream, ttcIndex, &name, &style, &isFixedWidth, nullptr)) {
			return nullptr;
		}
		auto data = new QkFontData(stream, ttcIndex, nullptr, 0);
		auto face = new Typeface_stream(data, name, style, isFixedWidth);
		return face;
	}

	Typeface* onMakeFromStreamArgs(Sp<QkStream> stream, const FontArguments& args) const {
		using Scanner = QkTypeface_FreeType::Scanner;
		bool isFixedPitch;
		FontStyle style;
		String name;
		Scanner::AxisDefinitions axisDefinitions;
		if (!fScanner.scanFont(*stream, args.getCollectionIndex(),
							   &name, &style, &isFixedPitch, &axisDefinitions))
		{
			return nullptr;
		}

		Array<QkFixed> axisValues(axisDefinitions.length());
		Scanner::computeAxisValues(axisDefinitions, args.getVariationDesignPosition(),
									axisValues.val(), name);

		auto data = new QkFontData(stream, args.getCollectionIndex(),
												   axisValues.val(), axisDefinitions.length());
		auto face = new Typeface_stream(data, name, style, isFixedPitch);
		return face;
	}

};

qk::FontPool* qk::FontPool::Make() {
	return new QkFontPool_fontconfig(nullptr);
}