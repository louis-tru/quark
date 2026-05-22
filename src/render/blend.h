/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __quark_render_blend__
#define __quark_render_blend__


namespace qk {
	/**
	 * Blend modes used by the Qk rendering pipeline.
	 *
	 * Qk internally uses premultiplied-alpha (PMA) rendering:
	 *
	 *   src.rgb = original.rgb * src.a
	 *   src.a   = src.a
	 *
	 * Most rendering paths, framebuffer accumulation, filtering,
	 * antialiasing coverage (such as aafuzz), and compositing
	 * operations are designed around PMA behavior.
	 *
	 * Blend modes without the "Legacy" suffix assume PMA source
	 * color input and produce correct Porter-Duff compositing
	 * results in the PMA framebuffer pipeline.
	 *
	 * Blend modes with the "Legacy" suffix are legacy straight-alpha
	 * compatibility modes:
	 *
	 *   src.rgb = original.rgb
	 *   src.a   = src.a
	 *
	 * These modes are mainly intended for artistic blending
	 * operations such as modulate, screen, and multiply.
	 *
	 * Since the framebuffer is accumulated in PMA form, and AA
	 * coverage naturally behaves as premultiplied-alpha contribution,
	 * legacy straight-alpha modes are not mathematically exact under
	 * fixed-function blending and may produce incorrect edge blending
	 * results when combined with coverage-based antialiasing
	 * (for example aafuzz).
	 *
	 * Legacy modes should therefore be treated as approximate visual
	 * effects rather than strict Photoshop-style blend behavior.
	 *
	 * References:
	 *   Porter-Duff compositing operators
	 *   Premultiplied alpha compositing
	 */
	enum BlendMode {
		kInvalid_BlendMode, //!< Invalid blend mode, used for error handling

		kClear_BlendMode,   //!< r = 0
		kSrc_BlendMode,     //!< r = s
		kSrcOver_BlendMode, //!< r = s + (1-sa)*d

		kDst_BlendMode,     //!< r = d
		kDstOver_BlendMode, //!< r = (1-da)*s + d

		kSrcIn_BlendMode,   //!< r = da*s
		kDstIn_BlendMode,   //!< r = sa*d

		kSrcOut_BlendMode,  //!< r = (1-da)*s
		kDstOut_BlendMode,  //!< r = (1-sa)*d

		kSrcATop_BlendMode, //!< r = da*s + (1-sa)*d
		kDstATop_BlendMode, //!< r = (1-da)*s + sa*d

		kXor_BlendMode,     //!< r = (1-da)*s + (1-sa)*d
		kPlus_BlendMode,    //!< r = s + d

		// Legacy straight-alpha artistic blend modes.
		// These are approximate under the PMA framebuffer pipeline.
		kSrcOverLegacy_BlendMode,  //!< r = sa*s + (1-sa)*d
		kPlusLegacy_BlendMode,     //!< r = sa*s + d

		// These modes are not mathematically exact under fixed-function blending
		// and may produce incorrect edge blending results when combined with coverage-based antialiasing.
		kModulateLegacy_BlendMode, //!< r = s*d
		kScreenLegacy_BlendMode,   //!< r = s + (1-s)*d
		kMultiplyLegacy_BlendMode, //!< r = d*s + (1-sa)*d
	};
}
#endif
