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

#ifndef __quark__font__ct_typeface__
#define __quark__font__ct_typeface__

#include "../../../types.h"
#include "./ct_util.h"
#include "../typeface.h"

using namespace qk;

struct OpszVariation {
	bool isSet = false;
	double value = 0;
};

struct CTFontVariation {
	QkUniqueCFRef<CFDictionaryRef> variation;
	QkUniqueCFRef<CFDictionaryRef> wrongOpszVariation;
	OpszVariation opsz;
};

QkUniqueCFRef<CTFontRef> SkCTFontCreateExactCopy(CTFontRef baseFont, CGFloat textSize, OpszVariation opsz);

FontStyle QkCTFontDescriptorGetSkFontStyle(CTFontDescriptorRef desc, bool fromDataProvider);

CGFloat QkCTFontCTWeightForCSSWeight(TextWeight fontstyleWeight);
CGFloat QkCTFontCTWidthForCSSWidth(TextWidth fontstyleWidth);

class Typeface_Mac: public Typeface {
public:
	Typeface_Mac(QkUniqueCFRef<CTFontRef> fontRef, OpszVariation opszVariation, bool isData);
protected:
	int onCountGlyphs() const override;
	int onGetUPEM() const override;
	int onGetTableTags(FontTableTag tags[]) const override;
	bool onGetPostScriptName(String*) const override;
	String onGetFamilyName() const override;
	size_t onGetTableData(FontTableTag, size_t offset, size_t length, void* data) const override;
	void onCharsToGlyphs(const Unichar* chars, int count, GlyphID glyphs[]) const override;
	void onGetMetrics(FontMetrics* metrics) const override;
	void onGetGlyphMetrics(GlyphID glyph, FontGlyphMetrics* metrics) const override;
	bool onGetPath(GlyphID glyph, Path *path) const override;
	Vec2 onGetImage(const Array<GlyphID>& glyphs, float fontSize,
		const Array<Vec2> *offset, Sp<ImageSource> *imgOut) override;
private:
	QkUniqueCFRef<CTFontRef> ctFont(float fontSize) const;
	QkUniqueCFRef<CGColorSpaceRef> fRGBSpace;
	QkUniqueCFRef<CTFontRef> fFontRef;
	const OpszVariation fOpszVariation;
	const bool fHasColorGlyphs;
	const bool fIsData;
};

#endif
