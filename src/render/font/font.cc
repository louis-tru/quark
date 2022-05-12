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
		_fts.set(style, std::move(fts));
		return _fts[style];
	}

	Font::Font(FFID FFID, FontStyle style, float fontSize)
		: _FFID(FFID)
		, _style(style)
		, _fontSize(fontSize)
	{}

	bool Font::text_blob(const ArrayBuffer<Unichar>& unichar, float startX, float endX, TextBlob* blob) {
		// TODO ...
		return true;
	}

}