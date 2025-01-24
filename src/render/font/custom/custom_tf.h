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

#ifndef __quark__font__custom__custom_tf__
#define __quark__font__custom__custom_tf__

#include "../freetype/ft_typeface.h"
#include "../priv/styleset.h"
#include "../pool.h"

/** The base QkTypeface implementation for the custom font manager. */
class QkTypeface_Custom : public QkTypeface_FreeType {
public:
	QkTypeface_Custom(const FontStyle& style, bool isFixedPitch,
					bool sysFont, const String familyName, int index);
	bool isSysFont() const;

protected:
	String onGetFamilyName() const override;
	int getIndex() const;

private:
	const bool fIsSysFont;
	const String fFamilyName;
	const int fIndex;
	using INHERITED = QkTypeface_FreeType;
};

/** The stream QkTypeface implementation for the custom font manager. */
class QkTypeface_Stream : public QkTypeface_Custom {
public:
	QkTypeface_Stream(Sp<QkFontData> fontData,
					const FontStyle& style, bool isFixedPitch, bool sysFont,
					const String familyName);
protected:
	Sp<QkFontData> onMakeFontData() const override;
private:
	const Sp<const QkFontData> fData;
	using INHERITED = QkTypeface_Custom;
};

/** The file QkTypeface implementation for the custom font manager. */
class QkTypeface_File : public QkTypeface_Custom {
public:
	QkTypeface_File(const FontStyle& style, bool isFixedPitch, bool sysFont,
					const String familyName, cChar path[], int index);
protected:
	Sp<QkFontData> onMakeFontData() const override;
private:
	String fPath;
	using INHERITED = QkTypeface_Custom;
};

///////////////////////////////////////////////////////////////////////////////

/**
 *  FontStyleSet_Custom
 *
 *  This class is used by QkFontPool_Custom to hold QkTypeface_Custom families.
 */
class FontStyleSet_Custom : public QkFontStyleSet {
public:
	explicit FontStyleSet_Custom(const String familyName);

	/** Should only be called during the initial build phase. */
	void appendTypeface(Sp<QkTypeface_Custom> typeface);
	int count() override;
	void getStyle(int index, FontStyle* style, String* name) override;
	Typeface* createTypeface(int index) override;
	Typeface* matchStyle(const FontStyle& pattern) override;
	String getFamilyName();

private:
	Array<Sp<QkTypeface_Custom>> fStyles;
	String fFamilyName;

	friend class QkFontPool_Custom;
};

/**
 *  QkFontPool_Custom
 *
 *  This class is essentially a collection of FontStyleSet_Custom,
 *  one FontStyleSet_Custom for each family. This class may be modified
 *  to load fonts from any source by changing the initialization.
 */
class QkFontPool_Custom : public FontPool {
public:
	typedef Array<Sp<FontStyleSet_Custom>> Families;
	class SystemFontLoader {
	public:
		virtual ~SystemFontLoader() {}
		virtual void loadSystemFonts(const QkTypeface_FreeType::Scanner&, Families*) const = 0;
	};
	explicit QkFontPool_Custom(const SystemFontLoader& loader);

protected:
	uint32_t onCountFamilies() const override;
	String onGetFamilyName(int index) const override;
	FontStyleSet_Custom* onMatchFamily(cChar familyName[]) const;
	Typeface* onMatchFamilyStyle(cChar familyName[], FontStyle fontStyle) const override;
	Typeface* onMatchFamilyStyleCharacter(cChar familyName[], FontStyle,
											cChar* bcp47[], int bcp47Count, Unichar character) const override;
	Typeface* onAddFontFamily(cBuffer& data, int ttcIndex) const override;
	Typeface* onMakeFromStreamArgs(Sp<QkStream>, const FontArguments&) const;
private:
	Array<String> fFamilyNames;
	Dict<String, Sp<FontStyleSet_Custom>> fFamilies;
	FontStyleSet_Custom* fDefaultFamily;
	QkTypeface_FreeType::Scanner fScanner;
};

#endif
