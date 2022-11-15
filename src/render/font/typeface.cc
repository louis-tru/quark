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

#include "./typeface.h"
#include "./style.h"
#include "./pool.h"

namespace quark {

	#define SkTF(impl) static_cast<SkTypeface*>(impl)

	Typeface::Typeface() {
	}

	FontStyle Typeface::fontStyle() const {
		FontStyle style;// = SkTF(_impl)->fontStyle();
		//return *reinterpret_cast<FontStyle*>(&style);
		return style;
	}

	bool Typeface::isBold() const {
		return true;
	}

	bool Typeface::isItalic() const {
		return true;
	}

	bool Typeface::isFixedPitch() const {
		return true;
	}

	int Typeface::countGlyphs() const {
		return 0;
	}

	int Typeface::countTables() const {
		return 0;
	}

	int Typeface::getTableTags(FontTableTag tags[]) const {
		return 0;
	}

	Buffer Typeface::getTableData(FontTableTag tag) const {
		return Buffer();
	}

	int Typeface::getUnitsPerEm() const {
		return 0;
	}

	String Typeface::getFamilyName() const {
		return String();
	}

	bool Typeface::getPostScriptName(String* name) const {
		return true;
	}

	Array<GlyphID> Typeface::unicharsToGlyphs(const Array<Unichar>& unichar) const {
		return Array<GlyphID>();
	}

	void Typeface::unicharsToGlyphs(const Unichar unichar[], uint32_t count, GlyphID glyphs[]) const {
	}

	GlyphID Typeface::unicharToGlyph(Unichar unichar) const {
	}

	Region Typeface::getBounds() const {
		return {};
	}

	TypefaceID Typeface::id() const {
		return TypefaceID((size_t(this)));
	}

}
