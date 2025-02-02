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

#include "./android_font_parser.h"
#include "../pool.h"
#include "../priv/styleset.h"

#include <algorithm>
#include <limits>

class QkTypeface_Android : public QkTypeface_FreeType {
public:
	QkTypeface_Android(const FontStyle& style, bool isFixedPitch, cString& familiesName)
		: QkTypeface_FreeType(style, 0)
		, fFamilyName(familiesName) {}
protected:
	String onGetFamilyName() const override { return fFamilyName; }
	String fFamilyName;
};

class QkTypeface_AndroidSystem : public QkTypeface_Android {
public:
	QkTypeface_AndroidSystem(cString& pathName,
							const bool cacheFontFiles,
							int index,
							const QkFixed* axes, int axesCount,
							const FontStyle& style,
							bool isFixedPitch,
							cString& familiesName,
							const Array<QkLanguage>& lang,
							FontVariant variantStyle)
		: QkTypeface_Android(style, isFixedPitch, familiesName)
		, fPathName(pathName)
		, fIndex(index)
		, fAxes(axes, axesCount)
		, fLang(lang)
		, fVariantStyle(variantStyle)
		, fCacheFontFiles(cacheFontFiles) {
		initFreeType();
	}

	Sp<QkStream> makeStream() const {
		if (fCacheFontFiles) {
			Buffer data;
			Qk_Try(data = fs_read_file_sync(fPathName)) { return nullptr; }
			return QkStream::Make(data);
		} else {
			return QkStream::Make(fPathName);
		}
	}

	Sp<QkFontData> onMakeFontData() const override {
		return new QkFontData(this->makeStream(), fIndex, fAxes.begin(), fAxes.length());
	}

	bool fCacheFontFiles;
	cString fPathName;
	int fIndex;
	const Array<QkFixed> fAxes;
	const Array<QkLanguage> fLang;
	const FontVariant fVariantStyle;
};

class QkTypeface_AndroidStream : public QkTypeface_Android {
public:
	QkTypeface_AndroidStream(Sp<QkFontData> data,
							const FontStyle& style,
							bool isFixedPitch,
							cString& familiesName)
		: QkTypeface_Android(style, isFixedPitch, familiesName)
		, fData(std::move(data))
	{
		initFreeType();
	}

	Sp<QkFontData> onMakeFontData() const override {
		return new QkFontData(*fData.value());
	}

	const Sp<const QkFontData> fData;
};

class QkFontStyleSet_Android : public QkFontStyleSet {
public:
	typedef QkTypeface_FreeType::Scanner Scanner;

	QkFontStyleSet_Android(const FontFamily& families, const Scanner& scanner, bool cacheFontFiles) {
		cString* cannonicalFamilyName = nullptr;
		if (families.fNames.length() > 0) {
			cannonicalFamilyName = &families.fNames[0];
		}
		fFallbackFor = families.fFallbackFor;

		// TODO? make this lazy
		for (int i = 0; i < families.fFonts.length(); ++i) {
			const FontFileInfo& fontFile = families.fFonts[i];

			String pathName(families.fBasePath);
			pathName.append(fontFile.fFileName);

			Sp<QkStream> stream = QkStream::Make(pathName);
			if (!stream) {
				Qk_DLog("Requested font file %s does not exist or cannot be opened.\n",
						pathName.c_str());
				continue;
			}

			const int ttcIndex = fontFile.fIndex;
			String familiesName;
			FontStyle style;
			bool isFixedWidth;
			Scanner::AxisDefinitions axisDefinitions;
			if (!scanner.scanFont(stream.value(), ttcIndex,
								&familiesName, &style, &isFixedWidth, &axisDefinitions))
			{
				Qk_DLog("Requested font file %s exists, but is not a valid font.\n",
						pathName.c_str());
				continue;
			}

			int weight = fontFile.fWeight != 0 ? fontFile.fWeight : style.weight();
			TextSlant slant = style.slant();
			switch (fontFile.fStyle) {
				case FontFileInfo::Style::kAuto: slant = style.slant(); break;
				case FontFileInfo::Style::kNormal: slant = TextSlant::Normal; break;
				case FontFileInfo::Style::kItalic: slant = TextSlant::Italic; break;
				default: Qk_ASSERT(false); break;
			}
			style = FontStyle(TextWeight(weight), style.width(), slant);

			uint32_t variant = families.fVariant;
			if (kDefault_FontVariant == variant) {
				variant = kCompact_FontVariant | kElegant_FontVariant;
			}

			// The first specified families name overrides the families name found in the font.
			// TODO: QkTypeface_AndroidSystem::onCreateFamilyNameIterator should return
			// all of the specified families names in addition to the names found in the font.
			if (cannonicalFamilyName != nullptr) {
				familiesName = *cannonicalFamilyName;
			}

			Array<QkFixed> axisValues(axisDefinitions.length());
			FontArguments::VariationPosition position = {
				*fontFile.fVariationDesignPosition.begin(),
				 fontFile.fVariationDesignPosition.length()
			};
			Scanner::computeAxisValues(axisDefinitions, position, *axisValues, familiesName);

			auto tf = new QkTypeface_AndroidSystem(
					pathName, cacheFontFiles, ttcIndex, axisValues.val(), axisDefinitions.length(),
					style, isFixedWidth, familiesName, families.fLanguages, variant);

			fStyles.push(tf);
		}
	}

	int count() override {
		return fStyles.length();
	}

	void getStyle(int index, FontStyle* style, String* name) override {
		if (index < 0 || fStyles.length() <= index) {
			return;
		}
		if (style) {
			*style = fStyles[index]->fontStyle();
		}
		if (name) {
			*name = String();
		}
	}

	QkTypeface_AndroidSystem* createTypeface(int index) override {
		if (index < 0 || fStyles.length() <= index) {
			return nullptr;
		}
		return fStyles[index].value();
	}

	QkTypeface_AndroidSystem* matchStyle(FontStyle pattern) override {
		return static_cast<QkTypeface_AndroidSystem*>(this->matchStyleCSS3(pattern));
	}

private:
	Array<Sp<QkTypeface_AndroidSystem>> fStyles;
	String fFallbackFor;

	friend class QkFontPool_Android;
};

typedef Dict<String, QkFontStyleSet_Android*> NameToFamilySet;

class QkFontPool_Android : public FontPool {
public:
	QkFontPool_Android(const QkAndroid_CustomFonts* custom) {
		Array<FontFamily*> families;
		if (custom && QkAndroid_CustomFonts::kPreferSystem != custom->fSystemFontUse) {
			String base(custom->fBasePath);
			GetCustomFontFamilies(families, base, custom->fFontsXml, custom->fFallbackFontsXml);
		}
		if (!custom || (custom && QkAndroid_CustomFonts::kOnlyCustom != custom->fSystemFontUse)) {
			GetSystemFontFamilies(families);
		}
		if (custom && QkAndroid_CustomFonts::kPreferSystem == custom->fSystemFontUse) {
			String base(custom->fBasePath);
			GetCustomFontFamilies(families, base, custom->fFontsXml, custom->fFallbackFontsXml);
		}
		this->buildNameToFamilyMap(families, custom ? custom->fIsolated : false);
		this->findDefaultStyleSet();

		for (auto i: families) delete i; // delete all

		initFontPool();
	}

protected:
	/** Returns not how many families we have, but how many unique names
	 *  exist among the families.
	 */
	uint32_t onCountFamilies() const override {
		Qk_ASSERT_EQ(fNameToFamilyMap.length(), fFamilyNames.length());
		return fNameToFamilyMap.length();
	}

	String onGetFamilyName(int index) const override {
		if (index < 0 || fFamilyNames.length() <= index) {
			return String();
		}
		return fFamilyNames[index];
	}

	Typeface* onMatchFamilyStyle(cChar familiesName[], FontStyle style) const override {
		if (familiesName) {
			auto sset = this->onMatchFamily(familiesName);
			return sset ? sset->matchStyle(style): nullptr;
		}
		return fDefaultStyleSet->matchStyle(style);
	}

	QkFontStyleSet* onMatchFamily(cChar familiesName[]) const {
		if (!familiesName) {
			return nullptr;
		}
		auto tolc = String(familiesName).lowerCase();

		QkFontStyleSet_Android *set;
		if (fNameToFamilyMap.get(tolc, set)) {
			return set;
		}
		// TODO: eventually we should not need to name fallback families.
		if (fFallbackNameToFamilyMap.get(tolc, set)) {
			return set;
		}
		return nullptr;
	}

	static Typeface* find_families_style_character(cString& familiesName,
			const NameToFamilySet& fallbackNameToFamilyMap,
			FontStyle style, bool elegant,
			cString& langTag, Unichar character)
	{
		for (auto &it: fallbackNameToFamilyMap) {
			auto families = it.value;
			if (familiesName != families->fFallbackFor) {
				continue;
			}
			QkTypeface_AndroidSystem* face(families->matchStyle(style));

			if (!langTag.isEmpty() &&
				std::none_of(face->fLang.begin(), face->fLang.end(), [&](QkLanguage lang) {
					return lang.getTag().startsWith(langTag);
				}))
			{
				continue;
			}

			if (QkToBool(face->fVariantStyle & kElegant_FontVariant) != elegant) {
				continue;
			}

			if (face->unicharToGlyph(character) != 0) {
				return face;
			}
		}
		return nullptr;
	}

	Typeface* onMatchFamilyStyleCharacter(cChar familiesName[], FontStyle style,
			cChar* bcp47[], int bcp47Count, Unichar character) const override
	{
		// The variant 'elegant' is 'not squashed', 'compact' is 'stays in ascent/descent'.
		// The variant 'default' means 'compact and elegant'.
		// As a result, it is not possible to know the variant context from the font alone.
		// TODO: add 'is_elegant' and 'is_compact' bits to 'style' request.

		Array<String> familiesNameStrings;
		if (familiesName)
			familiesNameStrings.push(familiesName);
		familiesNameStrings.push(String());

		for (cString& familyName : familiesNameStrings) {
			// The first time match anything elegant, second time anything not elegant.
			for (int elegant = 2; elegant-- > 0;) {
				for (int bcp47Index = bcp47Count; bcp47Index-- > 0;) {
					QkLanguage lang(bcp47[bcp47Index]);
					while (!lang.getTag().isEmpty()) {
						auto matchingTypeface =
							find_families_style_character(familyName, fFallbackNameToFamilyMap,
														style, QkToBool(elegant),
														lang.getTag(), character);
						if (matchingTypeface) {
							return matchingTypeface;
						}

						lang = lang.getParent();
					}
				}
				auto matchingTypeface =
					find_families_style_character(familyName, fFallbackNameToFamilyMap,
												style, QkToBool(elegant),
												String(), character);
				if (matchingTypeface) {
					return matchingTypeface;
				}
			}
		}
		return nullptr;
	}

	Typeface* onAddFontFamily(cBuffer& data, int ttcIndex) const override {
		return this->onMakeFromStreamIndex(QkStream::Make(data.copy()), ttcIndex);
	}

	Typeface* onMakeFromStreamIndex(Sp<QkStream> stream, int ttcIndex) const {
		bool isFixedPitch;
		FontStyle style;
		String name;
		if (!fScanner.scanFont(*stream, ttcIndex, &name, &style, &isFixedPitch, nullptr)) {
			return nullptr;
		}
		auto data = new QkFontData(stream, ttcIndex, nullptr, 0);
		auto face = new QkTypeface_AndroidStream(data, style, isFixedPitch, name);
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
									axisValues, name);

		auto data = new QkFontData(stream, args.getCollectionIndex(),
											axisValues.val(), axisDefinitions.length());
		auto face = new QkTypeface_AndroidStream(data, style, isFixedPitch, name);
		return face;
	}

private:

	void addFamily(FontFamily& families, const bool isolated, int familiesIndex) {
		auto nameToFamily = &fNameToFamilyMap;
		if (families.fIsFallbackFont) {
			nameToFamily = &fFallbackNameToFamilyMap;

			if (0 == families.fNames.length()) {
				families.fNames.push(String::format("%.2x##fallback", familiesIndex));
			}
		} else {
			fFamilyNames.write(families.fNames.val(), families.fNames.length());
		}

		auto newSet = new QkFontStyleSet_Android(families, fScanner, isolated);
		if (0 == newSet->count()) {
			return;
		}

		for (cString& name : families.fNames) {
			Qk_ASSERT_EQ(nameToFamily->has(name), false);
			//nameToFamily->push(NameToFamily{name, newSet.value()});
			nameToFamily->set(name, newSet.value());
		}
		fStyleSets.push(newSet);
	}

	void buildNameToFamilyMap(Array<FontFamily*> families, const bool isolated) {
		int familiesIndex = 0;
		for (FontFamily* families : families) {
			addFamily(*families, isolated, familiesIndex++);
			for (const auto& [unused, fallbackFamily] : families->fallbackFamilies) {
				addFamily(*fallbackFamily, isolated, familiesIndex++);
			}
		}
	}

	void findDefaultStyleSet() {
		Qk_ASSERT(!fStyleSets.isNull());

		static cChar* defaultNames[] = { "sans-serif" };
		for (cChar* defaultName : defaultNames) {
			fDefaultStyleSet = this->onMatchFamily(defaultName);
			if (fDefaultStyleSet)
				return;
		}
		if (nullptr == fDefaultStyleSet) {
			fDefaultStyleSet = fStyleSets[0];
		}
		Qk_ASSERT(fDefaultStyleSet);
	}

	QkTypeface_FreeType::Scanner fScanner;

	Array<Sp<QkFontStyleSet_Android>> fStyleSets;
	mutable Sp<QkFontStyleSet> fDefaultStyleSet;

	Array<String> fFamilyNames;
	NameToFamilySet fNameToFamilyMap;
	NameToFamilySet fFallbackNameToFamilyMap;
};

static char const * const gSystemFontUseStrings[] = {
	"OnlyCustom", "PreferCustom", "PreferSystem"
};

QkFontPool_Android* QkFontPool_New_Android(const QkAndroid_CustomFonts* custom) {
	if (custom) {
		Qk_ASSERT(0 <= custom->fSystemFontUse);
		Qk_ASSERT(custom->fSystemFontUse < Qk_ARRAY_COUNT(gSystemFontUseStrings));
		Qk_DLog("SystemFontUse: %s BasePath: %s Fonts: %s FallbackFonts: %s\n",
				gSystemFontUseStrings[custom->fSystemFontUse],
				custom->fBasePath,
				custom->fFontsXml,
				custom->fFallbackFontsXml);
	}
	return new QkFontPool_Android(custom);
}

qk::FontPool* qk::FontPool::Make() {
	return QkFontPool_New_Android(nullptr);
}
