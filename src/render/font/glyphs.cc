/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./glyphs.h"

namespace qk {

	FontGlyphs::FontGlyphs(float fontSize, Typeface *ft, const GlyphID glyphs[], uint32_t count)
		: _fontSize(fontSize), _typeface(ft)
	{
		if (count) {
			_glyphs.write(glyphs, count, 0);
		}
	}
	
	FontGlyphs::FontGlyphs(float fontSize, Typeface *ft, Array<GlyphID> &&glyphs)
		: _fontSize(fontSize), _typeface(ft), _glyphs(std::move(glyphs))
	{}

	Array<Vec2> FontGlyphs::getHorizontalOffset(Vec2 origin) const {
		if (!_glyphs.length())
			return Array<Vec2>({origin});

		const float one_div_64 = 1.0 / 64.0;
		const float scale = one_div_64 * _fontSize;

		Array<Vec2> offset(_glyphs.length() + 1);

		float x = origin.x(), y = origin.y();
		Vec2 *dst = *offset;

		for (auto &gm: _typeface->getGlyphsMetrics(_glyphs)) {
			*dst++ = Vec2(x, y);
			x += gm.fAdvanceX * scale;
		}
		*dst = Vec2(x, y);

		Qk_ReturnLocal(offset);
	}

}
