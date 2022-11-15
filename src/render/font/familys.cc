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

	FontFamilys::FontFamilys(FontPool* pool, Array<String>& familys)
		: _pool(pool), _familys(std::move(familys))
	{}

	const Array<String>& FontFamilys::familys() const {
		return _familys;
	}

	Sp<Typeface> FontFamilys::match(FontStyle style, uint32_t index) {
		return match(style)[index];
	}

	Array<Sp<Typeface>>& FontFamilys::match(FontStyle style) {
		auto it = _fts.find(style);
		if (it != _fts.end())
			return it->value;

		auto familys = _familys.copy();
		familys.write(_pool->second());
		Array<Sp<Typeface>> fts;
		Dict<String, bool> set;

		for (auto& name: familys) {
			if (!set.has(name)) {
				auto tf = _pool->matchFamilyStyle(name, style);
				if (tf)
					fts.push(tf);
				set.set(name, true);
			}
		}
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
							tfs[ftIdx + 1].unicharsToGlyphs(unichars + idx, count, glyphs + idx);
							make(unichars + idx, glyphs + idx, count, ftIdx + 1);
						} else {
							result.push(FontGlyphs(Font(pool->last(), fontSize), glyphs + idx, count));
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
						result.push(FontGlyphs(Font(tfs[ftIdx], fontSize), glyphs + idx, count));
						prev_idx = i - 1;
						prev_val = 1;
					}
				}
			}
		}
	};

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

}
