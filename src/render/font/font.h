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

#ifndef __noug__font__font__
#define __noug__font__font__

#include "./typeface.h"
#include "./style.h"
#include "../source.h"

namespace noug {

	struct TextBlob {
		Typeface        typeface;
		Array<GlyphID>  glyphs;
		Array<float>    offset;
		float           origin;
		Sp<ImageSource> cache;
		uint32_t        row;
	};

	class N_EXPORT FontGlyphs {
	public:
		FontGlyphs(const GlyphID glyphs[], uint32_t count, const Typeface* typeface, float fontSize);

		/**
		 * Consume Unichar output text blob and return whether to wrap
		 * @func text_blob()
		 */
		bool text_blob(TextBlob* blob, float offsetEnd);

		/**
		 * glyphs in typeface object
		 * @func glyphs()
		*/
		inline const Array<GlyphID>& glyphs() const { return _glyphs; }

		// define props
		N_DEFINE_PROP_READ(const Typeface*, typeface);
		N_DEFINE_PROP_READ(float, fontSize);
	private:
		Array<GlyphID> _glyphs;
	};

	class N_EXPORT FontFamilys {
	public:
		FontFamilys(FontPool* pool, Array<String>& familys);
		const Array<String>&   familys() const;
		const Array<Typeface>& match(FontStyle style);
		Array<FontGlyphs> makeFontGlyphs(const Array<Unichar>& unichars, FontStyle style, float fontSize);
		N_DEFINE_PROP_READ(FontPool*, pool);
	private:
		Array<String> _familys;
		Dict<FontStyle, Array<Typeface>> _fts;
		friend class FontPool;
	};

}
#endif
