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
#include "./metrics.h"

namespace noug {

	class FontGlyphs;

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

	class N_EXPORT FontGlyphs {
	public:
		FontGlyphs(const GlyphID glyphs[], uint32_t count, const Typeface& typeface, float fontSize);

		/**
		 * glyphs in typeface object
		 * @func glyphs()
		*/
		inline const Array<GlyphID>& glyphs() const { return _glyphs; }
		
		/**
		 * @func typeface()
		*/
		const Typeface& typeface() const;

		/**
		 * @func get_offset()
		 * @return offset values for GlyphID
		 */
		Array<float> get_offset();

		/**
		 * Returns FontMetrics associated with Typeface.
		 * @func get_metrics()
		 * @return recommended spacing between lines
		*/
		float get_metrics(FontMetrics* metrics) const;

		/**
		 * Returns FontMetrics associated with Typeface.
		 * @func get_metrics()
		*/
		static float get_metrics(FontMetrics* metrics, FFID FFID, FontStyle style, float fontSize);
		
	private:
		Array<GlyphID> _glyphs;
		void          *_typeface;
	public:
		N_DEFINE_PROP(float, fontSize);
		N_DEFINE_PROP(float, scaleX);
		N_DEFINE_PROP(float, skewX);
	private:
		uint8_t _flags, _edging, _hinting, ___[5];
	};

}
#endif
