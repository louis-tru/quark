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

#include "./font.h"
#include "./pool.h"
#include <skia/core/SkFont.h>
#include <skia/core/SkTypeface.h>
#include <skia/core/SkFontMetrics.h>
#include <skia/core/SkFontTypes.h>

namespace noug {

	template<> uint64_t Compare<FontStyle>::hash_code(const FontStyle& key) {
		return key.value();
	}

	// --------------------- F o n t . F a m i l y s ---------------------

	FontFamilys::FontFamilys(FontPool* pool, Array<String>& familys)
		: _pool(pool), _familys(std::move(familys))
	{}

	const Array<String>& FontFamilys::familys() const {
		return _familys;
	}

	const Array<Typeface>& FontFamilys::match(FontStyle style) {
		auto it = _fts.find(style);
		if (it != _fts.end())
			return it->value;

		Array<Typeface> fts;
		for (auto& name: _familys) {
			auto tf = _pool->match(name, style);
			if (tf.isValid())
				fts.push(std::move(tf));
		}

		fts.write(_pool->second()); // append default typeface
		_fts.set(style, std::move(fts));

		return _fts[style];
	}

	struct FontGlyphsBuilder {
		const Array<Typeface> &tfs;
		float                 fontSize;
		FontPool*             pool;
		Array<FontGlyphs>     result;
		/**
		 * @func make() build FontGlyphs
		*/
		void make(const Unichar *unichars,
			GlyphID glyphs[], const uint32_t count, const uint32_t ftIdx);
	};

	void FontGlyphsBuilder::make(const Unichar *unichars,
		GlyphID glyphs[], const uint32_t count, const uint32_t ftIdx)
	{
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
						tfs[ftIdx + 1].unicharsToGlyphs(unichars + idx, count, glyphs + idx);
						make(unichars + idx, glyphs + idx, count, ftIdx + 1);
					} else {
						result.push(FontGlyphs(glyphs + idx, count, pool->last(), fontSize));
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
					result.push(FontGlyphs(glyphs + idx, count, tfs[ftIdx], fontSize));
					prev_idx = i - 1;
					prev_val = 1;
				}
			}
		}
	}

	Array<FontGlyphs> FontFamilys::makeFontGlyphs(const Array<Unichar>& unichars, FontStyle style, float fontSize) {
		if (unichars.length()) {
			FontGlyphsBuilder builder = { match(style), fontSize, _pool };
			auto glyphs = builder.tfs[0].unicharsToGlyphs(unichars);
			builder.make(*unichars, *glyphs, glyphs.length(), 0);
			return builder.result;
		} else {
			return Array<FontGlyphs>();
		}
	}

	// --------------------- F o n t . G l y p h s ---------------------
	
	inline const SkFont* CastSkFont(const FontGlyphs* fg) {
		return reinterpret_cast<const SkFont*>(reinterpret_cast<const Array<GlyphID>*>(fg) + 1);
	}

	FontGlyphs::FontGlyphs(const GlyphID glyphs[], uint32_t count, const Typeface& typeface, float fontSize)
		: _typeface( *((void**)(&typeface)) )
		, _fontSize(fontSize)
		, _scaleX(1)
		, _skewX(0)
		, _flags(1 << 5)
		, _edging(static_cast<unsigned>(SkFont::Edging::kAntiAlias))
		, _hinting(static_cast<unsigned>(SkFontHinting::kNormal))
	{
		_glyphs.write(glyphs, 0, count);
		_glyphs.realloc(count + 1);
		(*_glyphs)[count] = 0;
	}

	Array<float> FontGlyphs::get_offset() {
		auto font = CastSkFont(this);
		auto len = _glyphs.length() + 1;
		Array<float> offset(len);
		font->getXPos(*_glyphs, len, *offset);
		return offset;
	}
	
	const Typeface& FontGlyphs::typeface() const {
		return *((const Typeface*)&_typeface);
	}

	float FontGlyphs::get_metrics(FontMetrics* metrics) const {
		return CastSkFont(this)->getMetrics( (SkFontMetrics*)metrics );
	}

	float FontGlyphs::get_metrics(FontMetrics* metrics, FFID FFID, FontStyle style, float fontSize) {
		return FontGlyphs(nullptr, 0, FFID->match(style)[0], fontSize).get_metrics(metrics);
	}

	float FontGlyphs::get_metrics(FontMetrics* metrics, const Typeface& typeface, float fontSize) {
		return FontGlyphs(nullptr, 0, typeface, fontSize).get_metrics(metrics);
	}

}
