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

namespace noug {

	template<> uint64_t Compare<FontStyle>::hash_code(const FontStyle& key) {
		return key.value();
	}

	// --------------------- F o n t . G l y p h s --------------------- 

	FontGlyphs::FontGlyphs(const GlyphID glyphs[], uint32_t count, const Typeface* typeface, float fontSize)
		: _typeface(typeface)
		, _fontSize(fontSize)
	{
		_glyphs.write(glyphs, 0, count);
	}

	bool FontGlyphs::text_blob(TextBlob* blob, float offsetEnd) {
		// TODO ...
		return true;
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

	struct MakeFontGlyphsCtx {
		const Array<Unichar>  &unichars;
		const Array<Typeface> &tfs;
		Array<FontGlyphs>     &result;
		float                 fontSize;
		FontPool*             pool;

		void make(GlyphID glyphs[], uint32_t count, uint32_t ftIdx) {
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
							tfs[ftIdx + 1].unicharsToGlyphs(*unichars + idx, count, glyphs + idx);
							make(glyphs + idx, count, ftIdx + 1);
						} else {
							result.push(FontGlyphs(glyphs + idx, count, &pool->last(), fontSize));
						}
						prev_idx = i - 1;
						prev_val = 0;
					}
				} else { // zero
					if (ftIdx + 1 == tfs.length()) {
						glyphs[i] = pool->last_glyphID_65533(); // use 65533 glyph
					}
					if (!prev_val) {
						b:
						int idx = prev_idx + 1;
						int count = i - idx;
						result.push(FontGlyphs(glyphs + idx, count, *tfs + ftIdx, fontSize));
						prev_idx = i - 1;
						prev_val = 1;
					}
				}
			}
		}
	};

	Array<FontGlyphs> FontFamilys::makeFontGlyphs(const Array<Unichar>& unichars, FontStyle style, float fontSize) {
		Array<FontGlyphs> result;

		if (unichars.length()) {
			MakeFontGlyphsCtx ctx = { unichars, match(style), result, fontSize, _pool };
			auto glyphs = ctx.tfs[0].unicharsToGlyphs(unichars);
			ctx.make(*glyphs, glyphs.length(), 0);
		}
		return result;
	}


}
