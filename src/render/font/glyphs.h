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

#ifndef __quark__font__glphs__
#define __quark__font__glphs__

#include "./typeface.h"
#include "./style.h"
#include "./metrics.h"

namespace quark {

	class Qk_EXPORT FontGlyphs {
	public:
		FontGlyphs(const Typeface& typeface, float fontSize, const GlyphID glyphs[] = nullptr, uint32_t count = 0);

		/**
		 * Returns the current typeface array object
		*/
		inline const Array<GlyphID>& glyphs() const { return _glyphs; }

		/**
		 * Returns the current typeface
		*/
		const Typeface& typeface() const;

		/**
		 * Returns offset values for GlyphID
		 * @return array floar number
		 */
		Array<float> get_offset();

		/**
		 * @brief Returns Font Metrics associated with Typeface.
		 * @param[out] font metrics value the output param
		 * @return returns recommended spacing between lines
		*/
		float get_metrics(FontMetrics* metrics) const;

		/**
		 * Returns font metrics associated with Typeface.
		 * @param[out] metrics
		 * @param[in] FFID
		 * @param[in] style
		 * @param[in] fontSize
		 * @return returns recommended spacing between lines
		*/
		static float get_metrics(FontMetrics* metrics, FFID FFID, FontStyle style, float fontSize);
		static float get_metrics(FontMetrics* metrics, const Typeface& typeface, float fontSize);
		
	private:
		Array<GlyphID> _glyphs;
		void          *_typeface;
	public:
		Qk_Define_Prop(float, fontSize);
		Qk_Define_Prop(float, scaleX);
		Qk_Define_Prop(float, skewX);
	private:
		uint8_t _flags, _edging, _hinting, ___[5];
	};

}
#endif
