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

#ifndef __quark__font__metrics__
#define __quark__font__metrics__

#include "../../util/util.h"

namespace quark {

	struct FontMetrics {

		/** \enum FontMetricsFlags
		 FontMetricsFlags indicate when certain metrics are valid;
			the underline or strikeout metrics may be valid and zero.
			Fonts with embedded bitmaps may not have valid underline or strikeout metrics.
			*/
		enum FontMetricsFlags {
			kUnderlineThicknessIsValid_Flag = 1 << 0, //!< set if fUnderlineThickness is valid
			kUnderlinePositionIsValid_Flag  = 1 << 1, //!< set if fUnderlinePosition is valid
			kStrikeoutThicknessIsValid_Flag = 1 << 2, //!< set if fStrikeoutThickness is valid
			kStrikeoutPositionIsValid_Flag  = 1 << 3, //!< set if fStrikeoutPosition is valid
			kBoundsInvalid_Flag             = 1 << 4, //!< set if fTop, fBottom, fXMin, fXMax invalid
		};

		uint32_t fFlags;           //!< FontMetricsFlags indicating which metrics are valid
		float fTop;                //!< greatest extent above origin of any glyph bounding box, typically negative; deprecated with variable fonts
		float fAscent;             //!< distance to reserve above baseline, typically negative
		float fDescent;            //!< distance to reserve below baseline, typically positive
		float fBottom;             //!< greatest extent below origin of any glyph bounding box, typically positive; deprecated with variable fonts
		float fLeading;            //!< distance to add between lines, typically positive or zero
		float fAvgCharWidth;       //!< average character width, zero if unknown
		float fMaxCharWidth;       //!< maximum character width, zero if unknown
		float fXMin;               //!< greatest extent to left of origin of any glyph bounding box, typically negative; deprecated with variable fonts
		float fXMax;               //!< greatest extent to right of origin of any glyph bounding box, typically positive; deprecated with variable fonts
		float fXHeight;            //!< height of lower-case 'x', zero if unknown, typically negative
		float fCapHeight;          //!< height of an upper-case letter, zero if unknown, typically negative
		float fUnderlineThickness; //!< underline thickness
		float fUnderlinePosition;  //!< distance from baseline to top of stroke, typically positive
		float fStrikeoutThickness; //!< strikeout thickness
		float fStrikeoutPosition;  //!< distance from baseline to bottom of stroke, typically negative
	};

}
#endif