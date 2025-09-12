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

#include "./custom_typeface.h"

#include <limits>
#include <memory>

QkTypeface_Custom::QkTypeface_Custom(const FontStyle& style, bool isFixedPitch,
									 bool sysFont, cString &familyName, int index)
	: INHERITED(style, isFixedPitch)
	, fIsSysFont(sysFont), fFamilyName(familyName), fIndex(index)
{}

bool QkTypeface_Custom::isSysFont() const { return fIsSysFont; }

String QkTypeface_Custom::onGetFamilyName() const {
	return fFamilyName;
}

int QkTypeface_Custom::getIndex() const { return fIndex; }

QkTypeface_Stream::QkTypeface_Stream(Sp<QkFontData> fontData,
									 const FontStyle& style, bool isFixedPitch, bool sysFont,
									 const String familyName)
	: INHERITED(style, isFixedPitch, sysFont, familyName, fontData->getIndex())
	, fData(std::move(fontData))
{
	initFreeType();
}

Sp<QkFontData> QkTypeface_Stream::onMakeFontData() const {
	return new QkFontData(**fData);
}

QkTypeface_File::QkTypeface_File(const FontStyle& style, bool isFixedPitch, bool sysFont,
								 cString &familyName, cChar path[], int index)
	: INHERITED(style, isFixedPitch, sysFont, familyName, index)
	, fPath(path)
{
	initFreeType();
}

Sp<QkFontData> QkTypeface_File::onMakeFontData() const {
	int index = this->getIndex();
	auto stream = QkStream::Make(fPath);
	if (!stream) {
		return nullptr;
	}
	return new QkFontData(stream, index, nullptr, 0);
}

///////////////////////////////////////////////////////////////////////////////

FontStyleSet_Custom::FontStyleSet_Custom(const String familyName): fFamilyName(familyName) {}

void FontStyleSet_Custom::appendTypeface(Sp<QkTypeface_Custom> typeface) {
	fStyles.push(std::move(typeface));
}

int FontStyleSet_Custom::count() {
	return fStyles.length();
}

void FontStyleSet_Custom::getStyle(int index, FontStyle* style, String* name) {
	Qk_ASSERT(index < fStyles.length());
	if (style) {
		*style = fStyles[index]->fontStyle();
	}
	if (name) {
		*name = String();
	}
}

Typeface* FontStyleSet_Custom::createTypeface(int index) {
	Qk_ASSERT(index < fStyles.length());
	return fStyles[index].get();
}

Typeface* FontStyleSet_Custom::matchStyle(FontStyle pattern) {
	return this->matchStyleCSS3(pattern);
}

String FontStyleSet_Custom::getFamilyName() { return fFamilyName; }


///////////////////////////////////////////////////////////////////////////////

QkFontPool_Custom::QkFontPool_Custom(const SystemFontLoader& loader) : fDefaultFamily(nullptr) {
	Families families;
	loader.loadSystemFonts(fScanner, &families);

	for (auto &it: families) {
		auto name = it.get()->getFamilyName();
		fFamilyNames.push(name);
		fFamilies.set(name.lowerCase(), std::move(it));
		// Qk_DLog("FamilyName, %s", *name);
	}
	Qk_CHECK(fFamilies.length() == fFamilyNames.length(), "QkFontPool_Custom repeat set object");

	// Try to pick a default font.
	static cChar* defaultNames[] = {
		"Arial", "Verdana", "Times New Roman", "Droid Sans", nullptr
	};
	for (size_t i = 0; i < Qk_ARRAY_COUNT(defaultNames); ++i) {
		auto set = this->onMatchFamily(defaultNames[i]);
		if (nullptr == set) {
			continue;
		}
		if (nullptr == set->matchStyle(FontStyle())) {
			continue;
		}

		fDefaultFamily = set;
		break;
	}
	if (nullptr == fDefaultFamily) {
		fDefaultFamily = fFamilies.begin()->value.get();
	}
	
	initFontPool();
}

uint32_t QkFontPool_Custom::onCountFamilies() const {
	return fFamilyNames.length();
}

String QkFontPool_Custom::onGetFamilyName(int index) const {
	Qk_ASSERT(index < fFamilyNames.length());
	return fFamilyNames[index];
}

FontStyleSet_Custom* QkFontPool_Custom::onMatchFamily(cChar familyName[]) const {
	const Sp<FontStyleSet_Custom> *set;
	if (fFamilies.get(String(familyName).lowerCase(), set)) {
		return const_cast<FontStyleSet_Custom*>(set->get());
	}
	return nullptr;
}

Typeface* QkFontPool_Custom::onMatchFamilyStyle(cChar familyName[], FontStyle fontStyle) const {
	if (familyName) {
		auto sset = this->onMatchFamily(familyName);
		return sset ? sset->matchStyle(fontStyle): nullptr;
	}
	return fDefaultFamily->matchStyle(fontStyle);
}

Typeface* QkFontPool_Custom::onMatchFamilyStyleCharacter(cChar familyName[], FontStyle style,
		cChar* bcp47[], int bcp47Count, Unichar character) const
{
	// TODO ...
	return nullptr;
}

Typeface* QkFontPool_Custom::onAddFontFamily(cBuffer& data, int ttcIndex) const {
	return onMakeFromStreamArgs(
		QkStream::Make(data.copy()), FontArguments().setCollectionIndex(ttcIndex)
	);
}

Typeface* QkFontPool_Custom::onMakeFromStreamArgs(Sp<QkStream> stream,
														 const FontArguments& args) const {
	using Scanner = QkTypeface_FreeType::Scanner;
	bool isFixedPitch;
	FontStyle style;
	String name;
	Scanner::AxisDefinitions axisDefinitions;
	if (!fScanner.scanFont(stream.get(), args.getCollectionIndex(),
							&name, &style, &isFixedPitch, &axisDefinitions))
	{
		return nullptr;
	}

	const FontArguments::VariationPosition position = args.getVariationDesignPosition();
	Array<QkFixed> axisValues(axisDefinitions.length());
	Scanner::computeAxisValues(axisDefinitions, position, axisValues.val(), name);

	auto data = new QkFontData(std::move(stream), args.getCollectionIndex(),
											   axisValues.val(), axisDefinitions.length());
	auto face = new QkTypeface_Stream(data, style, isFixedPitch, false, name);
	return face;
}
