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
#include <skia/core/SkFont.h>
#include <skia/core/SkTypeface.h>
#include <skia/core/SkFontMetrics.h>
#include <skia/core/SkFontTypes.h>

namespace quark {

	// --------------- F o n t ---------------

	inline const SkFont* CastSkFont(const Font* fg) {
		return reinterpret_cast<const SkFont*>(fg);
	}

	Font::Font(const Typeface& typeface, float fontSize)
		: _typeface( *((void**)(&typeface)) )
		, _fontSize(fontSize)
		, _scaleX(1)
		, _skewX(0)
		, _flags(1 << 5)
		, _edging(static_cast<unsigned>(SkFont::Edging::kAntiAlias))
		, _hinting(static_cast<unsigned>(SkFontHinting::kNormal))
	{
	}

	Array<float> Font::get_offset(const GlyphID glyphs[], uint32_t count) const {
		auto font = CastSkFont(this);
		auto len = count + 1;
		Array<float> offset(len);
		font->getXPos(glyphs, len, *offset);
		return offset;
	}
	
	const Typeface* Font::typeface() const {
		return _typeface.value();
	}

	float Font::get_metrics(FontMetrics* metrics) const {
		return CastSkFont(this)->getMetrics( (SkFontMetrics*)metrics );
	}

	float Font::get_metrics(FontMetrics* metrics, FFID FFID, FontStyle style, float fontSize) {
		return Font(FFID->match(style)[0], fontSize).get_metrics(metrics);
	}

	float Font::get_metrics(FontMetrics* metrics, Typeface* typeface, float fontSize) {
		return Font(typeface, fontSize).get_metrics(metrics);
	}

	// --------------- F o n t . G l y p h s ---------------

	FontGlyphs::FontGlyphs(Font&& font, const GlyphID glyphs[], uint32_t count): _font(std::move(font)) {
		_glyphs.write(glyphs, 0, count);
		_glyphs.realloc(count + 1);
		(*_glyphs)[count] = 0;
	}

	Array<float> FontGlyphs::get_offset() const {
		return _font.get_offset(*_glyphs, _glyphs.length());
	}

}
