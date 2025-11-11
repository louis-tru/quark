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

#ifndef __quark_render_blend__
#define __quark_render_blend__


namespace qk {

	enum BlendMode {
		kClear_BlendMode,         //!< r = (1-sa)*d
		kSrc_BlendMode,           //!< r = s
		kDst_BlendMode,           //!< r = d
		kSrcOver_BlendMode,       //!< r = sa*s + (1-sa)*d
		kSrcOverPre_BlendMode,    //!< r = s + (1-sa)*d, with src premultiplied alpha, but this mode of aaclip will fail
		kDstOver_BlendMode,       //!< r = (1-da)*s + da*d
		kSrcIn_BlendMode,         //!< r = da*s
		kDstIn_BlendMode,         //!< r = sa*d
		kSrcOut_BlendMode,        //!< r = (1-da)*s
		kDstOut_BlendMode,        //!< r = (1-sa)*d
		kSrcATop_BlendMode,       //!< r = da*s + (1-sa)*d
		kDstATop_BlendMode,       //!< r = (1-da)*s + sa*d
		kXor_BlendMode,           //!< r = (1-da)*s + (1-sa)*d
		kPlus_BlendMode,          //!< r = min(s + d, 1)
		kModulate_BlendMode,      //!< r = s*d
		kScreen_BlendMode,        //!< r = s + d - s*d
		kMultiply_BlendMode,      //!< r = d*s + (1-sa)*d
		kAdditive_BlendMode,      //!< r = sa*s + d
		// Note: All modes that do not handle source alpha channels do not support aaclipping.
	};
}

#endif
