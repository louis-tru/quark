/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
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
#include <skia/core/SkTypeface.h>

namespace noug {

	#define SkTF(impl) static_cast<SkTypeface*>(impl)

	Typeface::Typeface(): _impl(nullptr) {
	}

	Typeface::Typeface(const Typeface& tf): _impl(tf._impl) {
		if(_impl) {
			SkTF(_impl)->ref();
		}
	}

	Typeface::Typeface(void* impl): _impl(impl) {
	}

	Typeface::~Typeface() {
		if (_impl) {
			SkTF(_impl)->unref();
			_impl = nullptr;
		}
	}

	FontStyle Typeface::fontStyle() const {
		SkFontStyle style = SkTF(_impl)->fontStyle();
		return *reinterpret_cast<FontStyle*>(&style);
	}

	bool Typeface::isBold() const {
		return SkTF(_impl)->isBold();
	}

	bool Typeface::isItalic() const {
		return SkTF(_impl)->isItalic();
	}

	bool Typeface::isFixedPitch() const {
		return SkTF(_impl)->isFixedPitch();
	}

	int Typeface::countGlyphs() const {
		return SkTF(_impl)->countGlyphs();
	}

	int Typeface::countTables() const {
		return SkTF(_impl)->countTables();
	}

	int Typeface::getTableTags(FontTableTag tags[]) const {
		return SkTF(_impl)->getTableTags(tags);
	}

	Buffer Typeface::getTableData(FontTableTag tag) const {
		size_t size = SkTF(_impl)->getTableSize(tag);
		Buffer buf = Buffer::alloc((uint32_t)size);
		SkTF(_impl)->getTableData(tag, 0, size, *buf);
		return buf;
	}

	int Typeface::getUnitsPerEm() const {
		return SkTF(_impl)->getUnitsPerEm();
	}

	String Typeface::getFamilyName() const {
		SkString str;
		SkTF(_impl)->getFamilyName(&str);
		return String(str.c_str(), (uint32_t)str.size());
	}

	bool Typeface::getPostScriptName(String* name) const {
		SkString str;
		bool ok = SkTF(_impl)->getPostScriptName(&str);
		if (ok)
			*name = String(str.c_str(), (uint32_t)str.size());
		return ok;
	}

	Array<GlyphID> Typeface::unicharsToGlyphs(const Array<Unichar>& unichar) const {
		Array<GlyphID> result(unichar.length());
		auto skunichar = reinterpret_cast<const SkUnichar*>(*unichar);
		SkTF(_impl)->unicharsToGlyphs(skunichar, unichar.length(), *result);
		return result;
	}

	void Typeface::unicharsToGlyphs(const Unichar unichar[], uint32_t count, GlyphID glyphs[]) const {
		auto skunichar = reinterpret_cast<const SkUnichar*>(unichar);
		SkTF(_impl)->unicharsToGlyphs(skunichar, count, glyphs);
	}

	GlyphID Typeface::unicharToGlyph(Unichar unichar) const {
		return SkTF(_impl)->unicharToGlyph(unichar);
	}

	Region Typeface::getBounds() const {
		SkRect rect = SkTF(_impl)->getBounds();
		return *reinterpret_cast<Region*>(&rect);
	}

}
