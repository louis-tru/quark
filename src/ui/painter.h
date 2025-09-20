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

// @private head

#ifndef __quark__ui_painter__
#define __quark__ui_painter__

#include "./view/view.h"
#include "../render/render.h"
#include "../render/canvas.h"
#include "./filter.h"
#include "./text/text_blob.h"

namespace qk {
	class Box;
	class ScrollView;
	typedef const Mat cMat;

	constexpr PaintImage::FilterMode default_FilterMode = PaintImage::kLinear_FilterMode;
	constexpr PaintImage::MipmapMode default_MipmapMode = PaintImage::kLinear_MipmapMode;

	class Qk_EXPORT Painter: public Object {
	public:
		struct BoxData {
			const RectPath *inside = nullptr;
			const RectPath *outside = nullptr;
			const RectOutlinePath *outline = nullptr;
		};
		Qk_DEFINE_PROP_GET(Window*, window);
		Qk_DEFINE_PROP_GET(Canvas*, canvas);
		Qk_DEFINE_PROP_GET(PathvCache*, cache);
		Qk_DEFINE_PROPERTY(cMat*, matrix); // current matrix
		Qk_DEFINE_PROPERTY(Vec2, origin);  // box origin
		Qk_DEFINE_PROPERTY(Vec2, originAA);  // box origin and fix aa stroke width
		Qk_DEFINE_PROP_GET(Color4f, color); // current color
		Qk_DEFINE_PROP_GET(float, AAShrink); // anti alias shrink, fix rect stroke width for AA
		Qk_DEFINE_PROP_GET(float, AAShrinkBorder); // anti alias shrink border
		Qk_DEFINE_PROP_GET(bool, isMsaa); // is MSAA
		Painter(Window *window);
		void set_origin_reverse(Vec2 origin);
		Rect getRect(Box* v);
		void getInsideRectPath(Box *v, BoxData &out);
		void getOutsideRectPath(Box *v, BoxData &out);
		void getRRectOutlinePath(Box *v, BoxData &out);
		void visitView(View* v);
		void visitView(View* v, cMat *mat);
		void drawBoxBasic(Box *v, BoxData &data);
		void drawBoxFill(Box *v, BoxData &data);
		void drawBoxFillImage(Box *v, FillImage *fill, BoxData &data);
		void drawBoxFillLinear(Box *v, FillGradientLinear *fill, BoxData &data);
		void drawBoxFillRadial(Box *v, FillGradientRadial *fill, BoxData &data);
		void drawBoxShadow(Box *v, BoxData &data);
		void drawBoxColor(Box *v, BoxData &data);
		void drawBoxBorder(Box *v, BoxData &data);
		void drawBoxEnd(Box *v, BoxData &data);
		void drawScrollBar(ScrollView *v);
		void drawTextBlob(TextOptions *opts, Vec2 inOffset,
			TextLines *lines, Array<TextBlob> &blob, Array<uint32_t> &blob_visible
		);
	private:
		template<CascadeColor parentCascadeColor>
		void visitView_(View *view, View *v);
		Render     *_render;
		uint32_t   _mark_recursive;
		Buffer     _tempBuff;
		LinearAllocator _tempAllocator[2]; // Reset when starting every frame

		friend class Spine;
		friend class Root;
	};
}
#endif
