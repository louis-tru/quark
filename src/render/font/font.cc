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
			_glyphs.write(glyphs, 0, count);
		}
	}
	
	FontGlyphs::FontGlyphs(float fontSize, Typeface *ft, Array<GlyphID> &&glyphs)
		: _fontSize(fontSize), _typeface(ft), _glyphs(std::move(glyphs))
	{}

	Array<Vec2> FontGlyphs::getHorizontalOffset(Vec2 origin) const {
		if (!_glyphs.length())
			return Array<Vec2>({origin});

		constexpr float _1_64 = 1.0 / 64.0;
		const float scale = _fontSize * _1_64;
		const bool isScale = scale != 1.0;

		Array<Vec2> arr(_glyphs.length() + 1);
		float x = origin.x(), y = origin.y();
		Vec2 *dst = *arr;
		const GlyphID *src = *_glyphs;
		const GlyphID *end = src + arr.length();
		auto tf = typeface();

		while (src != end) {
			auto &glyph = tf->getGlyphMetrics(*src);
			*dst++ = Vec2(x, y);
			x += (isScale? glyph.advanceX * scale: glyph.advanceX);
			src++;
		}
		*dst = Vec2(x, y);
		
		Qk_ReturnLocal(arr);
	}

	// -------------------- F o n t . F a m i l y s --------------------

	FontFamilys::FontFamilys(FontPool* pool, Array<String>& familys)
		: _pool(pool), _familys(std::move(familys))
	{}

	const Array<String>& FontFamilys::familys() const {
		return _familys;
	}

	Sp<Typeface> FontFamilys::match(FontStyle style, uint32_t index) {
		return matchs(style)[index];
	}

	Array<Sp<Typeface>>& FontFamilys::matchs(FontStyle style) {
		auto it = _TFs.find(style);
		if (it != _TFs.end()) {
			return it->value;
		}
		Array<Sp<Typeface>> arr;
		Dict<String, bool> set;

		auto match = [&](cString& name) {
			if (!set.has(name)) {
				auto tf = _pool->match(name, style);
				if (tf)
					arr.push(std::move(tf));
				set.set(name, true);
			}
		};
		for (auto& name: _familys)
			match(name);
		for (auto& name: _pool->second())
			match(name);

		_TFs.set(style, std::move(arr));

		return _TFs[style];
	}

	struct FontGlyphsBuilder {
		Array<Sp<Typeface>>   &tfs;
		float                 fontSize;
		FontPool*             pool;
		Array<FontGlyphs>     result;

		/**
		 * @method make() build FontGlyphs
		*/
		void make(const Unichar *unichars, GlyphID glyphs[], const uint32_t count, const uint32_t ftIdx) {
			int prev_idx = -1;
			int prev_val = glyphs[0] ? 0: 1;
			for (int i = 0; i < count + 1; i++) {
				if (count == i) {
					if (prev_val) goto a;
					else goto b;
				}
				if (glyphs[i]) { // valid
					if (prev_val) {
					a:
						// exec recursion
						int idx = prev_idx + 1;
						int count = i - idx;
						if (ftIdx + 1 < tfs.length()) {
							tfs[ftIdx + 1]->unicharsToGlyphs(unichars + idx, count, glyphs + idx);
							make(unichars + idx, glyphs + idx, count, ftIdx + 1);
						} else {
							result.push(FontGlyphs(fontSize, pool->last().value(), glyphs + idx, count));
						}
						prev_idx = i - 1;
						prev_val = 0;
					}
				} else { // zero
					if (ftIdx + 1 == tfs.length()) {
						glyphs[i] = pool->last_65533(); // use 65533 glyph
					}
					if (!prev_val) {
					b:
						int idx = prev_idx + 1;
						int count = i - idx;
						result.push(FontGlyphs(fontSize, tfs[ftIdx].value(), glyphs + idx, count));
						prev_idx = i - 1;
						prev_val = 1;
					}
				}
			}
		}
	};

	Array<FontGlyphs> FontFamilys::makeFontGlyphs(const Array<Unichar>& unichars, FontStyle style, float fontSize) {
		if (unichars.length()) {
			FontGlyphsBuilder builder = { matchs(style), fontSize, _pool };
			auto glyphs = builder.tfs[0]->unicharsToGlyphs(unichars);
			builder.make(*unichars, *glyphs, glyphs.length(), 0);
			Qk_ReturnLocal(builder.result);
		} else {
			return Array<FontGlyphs>();
		}
	}


}
