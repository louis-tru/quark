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

namespace qk {

	Typeface::Typeface(FontStyle fs): _fontStyle(fs)
	{
		_metrics.fAscent = 0;
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
		//Qk_DLog("Typeface::unicharsToGlyphs, %s", *getFamilyName());
		onCharsToGlyphs(unichar, count, glyphs);
	}

	Array<GlyphID> Typeface::unicharsToGlyphs(const Array<Unichar>& unichar) const {
		//Qk_DLog("Typeface::unicharsToGlyphs, %s", *getFamilyName());
		if (unichar.length() > 0) {
			Array<GlyphID> result(unichar.length());
			onCharsToGlyphs(*unichar, unichar.length(), *result);
			Qk_ReturnLocal(result);
		}
		return Array<GlyphID>();
	}

	GlyphID Typeface::unicharToGlyph(Unichar unichar) const {
		GlyphID id;
		onCharsToGlyphs(&unichar, 1, &id);
		return id;
	}

	const Path& Typeface::getPath(GlyphID glyph) {
		auto it = _paths.find(glyph);
		if (it != _paths.end())
			return it->value;
		Path path;
		onGetPath(glyph, &path);
		_paths.set(glyph, path.normalizedPath());
		return _paths[glyph];
	}

	const FontGlyphMetrics& Typeface::getGlyphMetrics(GlyphID glyph) {
		auto it = _glyphs.find(glyph);
		if (it != _glyphs.end()) {
			return it->value;
		}
		FontGlyphMetrics fontGlyph;
		onGetGlyphMetrics(glyph, &fontGlyph);
		_glyphs.set(glyph, fontGlyph);
		return _glyphs[glyph];
	}

	float Typeface::getMetrics(FontMetrics* metrics, float fontSize) {
		if (_metrics.fAscent == 0) {
			onGetMetrics(&_metrics);
		}
		if (!metrics) {
			return 0;
		}
		memcpy(metrics, &_metrics, sizeof(FontMetrics));
		float scale = fontSize / 64.0;

		if (scale != 1.0) {
			// scale font metrics
			metrics->fTop *= scale;
			metrics->fAscent *= scale;
			metrics->fDescent *= scale;
			metrics->fBottom *= scale;
			metrics->fLeading *= scale;
			metrics->fAvgCharWidth *= scale;
			metrics->fMaxCharWidth *= scale;
			metrics->fXMin *= scale;
			metrics->fXMax *= scale;
			metrics->fXHeight *= scale;
			metrics->fCapHeight *= scale;
			metrics->fUnderlineThickness *= scale;
			metrics->fUnderlinePosition *= scale;
			metrics->fStrikeoutThickness *= scale;
			metrics->fStrikeoutPosition *= scale;
		}
		return metrics->fDescent - metrics->fAscent + metrics->fLeading;
	}
	
	static float getMetricsBase(FontMetrics *ref, FontMetricsBase* out, float fontSize) {
		if (!out)
			return 0;
		float scale = fontSize / 64.0;
		out->fAscent = ref->fAscent;
		out->fDescent = ref->fDescent;
		out->fLeading = ref->fLeading;

		if (scale != 1.0) {
			out->fAscent *= scale;
			out->fDescent *= scale;
			out->fLeading *= scale;
		}
		return out->fDescent - out->fAscent + out->fLeading;
	}

	float Typeface::getMetrics(FontMetricsBase* metrics, float fontSize) {
		if (_metrics.fAscent == 0) {
			onGetMetrics(&_metrics);
		}
		return getMetricsBase(&_metrics, metrics, fontSize);
	}
	
	float FontPool::getMaxMetrics(FontMetricsBase* out, float fontSize) {
		return getMetricsBase(&_MaxMetrics64, out, fontSize);
	}

	Vec2 Typeface::getImage(const Array<GlyphID>& glyphs, float fontSize,
			cArray<Vec2> *offset, float offsetScale, Sp<ImageSource> *imgOut, RenderBackend *render)
	{
		if (offset)
			Qk_Assert(offset->length() > glyphs.length());
		return onGetImage(glyphs, fontSize, offset, offsetScale, imgOut, render);
	}
}
