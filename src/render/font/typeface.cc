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

	Typeface::Typeface(FontStyle fs, bool isFixedPitch)
		: _fontStyle(fs), _isFixedPitch(isFixedPitch)
	{
	}

	int Typeface::countGlyphs() const {
		return onCountGlyphs();
	}

	int Typeface::countTables() const {
		return onGetTableTags(nullptr);
	}

	int Typeface::getTableTags(FontTableTag tags[]) const {
		return onGetTableTags(tags);
	}

	Buffer Typeface::getTableData(FontTableTag tag) const {
		size_t size = getTableSize(tag);
		Buffer buf = Buffer::alloc((uint32_t)size);
		onGetTableData(tag, 0, size, *buf);
		return buf;
	}

	size_t Typeface::getTableSize(FontTableTag tag) const {
		return onGetTableData(tag, 0, ~0U, nullptr);
	}

	int Typeface::getUnitsPerEm() const {
		return onGetUPEM();
	}

	String Typeface::getFamilyName() const {
		return onGetFamilyName();
	}

	bool Typeface::getPostScriptName(String* name) const {
		return onGetPostScriptName(name);
	}

	void Typeface::unicharsToGlyphs(const Unichar unichar[], uint32_t count, GlyphID glyphs[]) const {
		onCharsToGlyphs(unichar, count, glyphs);
	}

	Array<GlyphID> Typeface::unicharsToGlyphs(const Array<Unichar>& unichar) const {
		if (unichar.length() > 0) {
			Array<GlyphID> result(unichar.length());
			onCharsToGlyphs(*unichar, unichar.length(), *result);
			return std::move(result);
		}
		return Array<GlyphID>();
	}

	GlyphID Typeface::unicharToGlyph(Unichar unichar) const {
		GlyphID id;
		onCharsToGlyphs(&unichar, 1, &id);
		return id;
	}

	float Typeface::getMetrics(FontMetrics* metrics, float fontSize) {
		auto it = _MetricsCaches.find(fontSize);
		if (it != _MetricsCaches.end()) {
			*metrics = it->value;
		} else {
			onGetMetrics(metrics, fontSize);
			_MetricsCaches.set(fontSize, *metrics);
		}
		return metrics->fDescent - metrics->fAscent + metrics->fLeading;
	}
}
