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

#include "./familys.h"
#include "./pool.h"

namespace quark {

	static void scaleFontMetrics(FontMetrics* metrics, float scale) {
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

	FontGlyphs::FontGlyphs(Typeface *typeface,
		float fontSize, const GlyphID glyphs[], uint32_t count)
		: _fontSize(fontSize)
		, _typeface(typeface)
	{
		if (count) {
			_glyphs.write(glyphs, 0, count);
			_glyphs.realloc(count + 1);
			(*_glyphs)[count] = 0;
		}
	}

	Array<float> FontGlyphs::getOffset(float origin) {
		if (!_glyphs.length()) return Array<float>({0});

		const float scale = _fontSize / 64.0;
		const bool isScale = scale != 1.0;

		Array<float> result(_glyphs.length() + 1);
		float loc = origin;
		float* cursor = *result;
		GlyphID *src = *_glyphs;
		GlyphID *end = src + result.length();

		while (src != end) {
			FontGlyph *glyph = _typeface->getGlyph(*src);
			*cursor++ = loc;
			loc += (isScale? glyph->advanceX() * scale: glyph->advanceX());
			src++;
		}
		return std::move(result);
	}

	float FontGlyphs::getMetrics(FontMetrics* metrics) {
		if (!metrics) return 0;
		memcpy(metrics, &_typeface->getMetrics(), sizeof(FontMetrics));
		float scale = _fontSize / 64.0;
		if (scale != 1.0)
			scaleFontMetrics(metrics, scale);
		return metrics->fDescent - metrics->fAscent + metrics->fLeading;
	}

	float FontGlyphs::getMetrics(FontMetricsBase* metrics) {
		return getMetrics(metrics, *_typeface, _fontSize);
	}

	float FontGlyphs::getMetrics(FontMetricsBase* metrics, Typeface *typeface, float fontSize) {
		if (!metrics) return 0;
		float scale = fontSize / 64.0;
		auto& metrics0 = typeface->getMetrics();
		metrics->fAscent = metrics0.fAscent;
		metrics->fDescent = metrics0.fDescent;
		metrics->fLeading = metrics0.fLeading;
		if (scale != 1.0) {
			metrics->fAscent *= scale;
			metrics->fDescent *= scale;
			metrics->fLeading *= scale;
		}
		return metrics->fDescent - metrics->fAscent + metrics->fLeading;
	}
}
