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

#include "./font.h"
#include "./pool.h"

namespace qk {

	// -------------------- F o n t . G l y p h s --------------------

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

		constexpr float _1_div_64 = 1.0 / 64.0;
		const float scale = _fontSize * _1_div_64;
		const bool isScale = scale != 1.0;

		Array<Vec2> offset(_glyphs.length() + 1);

		float x = origin.x(), y = origin.y();
		Vec2 *dst = *offset;
		const GlyphID *src = *_glyphs;
		const GlyphID *end = src + offset.length()-1;
		auto tf = typeface();

		while (src != end) {
			auto &glyph = tf->getGlyphMetrics(*src);
			*dst++ = Vec2(x, y);
			x += (isScale? glyph.advanceX * scale: glyph.advanceX);
			src++;
		}
		*dst = Vec2(x, y);

		Qk_ReturnLocal(offset);
	}

	// -------------------- F o n t . F a m i l y s --------------------

	FontFamilys::FontFamilys(FontPool* pool, cArray<String>& familys)
		: _pool(pool)
	{
		for (auto &i: familys) {
			_familys.add(i.trim());
		}
		for (auto &s: pool->defaultFamilyNames()) {
			_familys.add(s);
		}
	}

	Sp<Typeface> FontFamilys::match(FontStyle style, uint32_t index) {
		return matchs(style)[index];
	}

	Array<Sp<Typeface>>& FontFamilys::matchs(FontStyle style) {
		auto it = _typefaces.find(style);
		if (it != _typefaces.end()) {
			return it->value;
		}
		Array<Sp<Typeface>> arr;
		for (auto& name: _familys) {
			auto tf = _pool->match(name.key, style);
			if (tf)
				arr.push(std::move(tf));
		}
		return _typefaces.set(style, std::move(arr));
	}

	struct FontGlyphsBuilder {
		Array<Sp<Typeface>>   &tfs;
		float                 fontSize;
		FontPool*             pool;
		Array<FontGlyphs>     result;

		void makeNext(const Unichar *unichars, GlyphID glyphs[], uint32_t count, uint32_t ftIdx) {
			if (ftIdx < tfs.length()) {
				tfs[ftIdx]->unicharsToGlyphs(unichars, count, glyphs);
				make(unichars, glyphs, count, ftIdx);
			} else {
				for (int i = 0; i < count; i++)
					glyphs[i] = pool->tf65533GlyphID(); // use 65533 glyph
				result.push(FontGlyphs(fontSize, *pool->tf65533(), glyphs, count));
			}
		}

		void make(
			const Unichar *unichars, GlyphID glyphs[], uint32_t count, uint32_t ftIdx
		) {
			int i = 0, j = 0;
			bool isValidPrev = glyphs[0] ? false: true; // init prev group

			while (j < count) {
				if (isValidPrev) { // prev valid, find invalid glyphs, find valid end
					do {
						if (glyphs[j]) { // valid
							makeNext(unichars + i, glyphs + i, j-i, ftIdx+1);
							isValidPrev = false;
							i = j++;
							break;
						}
					} while (++j < count);
				} else { // prev invalid
					do {
						if (!glyphs[j]) { // invalid
							result.push(FontGlyphs(fontSize, tfs[ftIdx].value(), glyphs+i, j-i));
							isValidPrev = true;
							i = j++;
							break;
						}
					} while (++j < count);
				}
			}

			if (i < j) {
				if (isValidPrev) {
					makeNext(unichars+i, glyphs+i, j-i, ftIdx+1);
				} else {
					result.push(FontGlyphs(fontSize, tfs[ftIdx].value(), glyphs+i, j-i));
				}
			}
		}
	};

	Array<FontGlyphs> FontFamilys::makeFontGlyphs(
		cArray<Unichar>& unichars, FontStyle style, float fontSize
	) {
		if (unichars.length()) {
			FontGlyphsBuilder builder = { matchs(style), fontSize, _pool };
			auto glyphs = builder.tfs[0]->unicharsToGlyphs(unichars);
			builder.make(*unichars, *glyphs, glyphs.length(), 0);
			return std::move(builder.result);
		} else {
			return Array<FontGlyphs>();
		}
	}


}
