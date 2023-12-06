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

#ifndef __quark__view_render__
#define __quark__view_render__

#include "./layout/layout.h"
#include "../render/render.h"
#include "../render/canvas.h"

namespace qk {
	class Window;
	class BoxLayout;
	class ImageLayout;
	class ScrollLayout;
	class InputLayout;
	class LabelLayout;
	class RootLayout;
	class TransformLayout;
	class ScrollLayoutBase;

	class Qk_EXPORT UIRender: public Object {
	public:
		struct BoxData {
			const RectPath *inside = nullptr;
			const RectPath *outside = nullptr;
			const RectOutlinePath *outline = nullptr;
		};
		UIRender(Window *window);
		void visitLayout(Layout* v);
		void visitBox(BoxLayout* box);
		void visitImage(ImageLayout* image);
		void visitScroll(ScrollLayout* scroll);
		void visitInput(InputLayout* input);
		void visitLabel(LabelLayout* label);
		void visitRoot(RootLayout* root);
		void visitTransform(TransformLayout* transform);
	private:
		Rect getRect(BoxLayout* box);
		void getInsideRectPath(BoxLayout *box, BoxData &out);
		void getOutsideRectPath(BoxLayout *box, BoxData &out);
		void getRRectOutlinePath(BoxLayout *box, BoxData &out);
		void drawBoxColor(BoxLayout *box, BoxData &data);
		void drawBoxFill(BoxLayout *box, BoxData &data);
		void drawBoxFillImage(BoxLayout *box, FillImage *fill, BoxData &data);
		void drawBoxFillLinear(BoxLayout *box, FillGradientLinear *fill, BoxData &data);
		void drawBoxFillRadial(BoxLayout *box, FillGradientRadial *fill, BoxData &data);
		void drawBoxShadow(BoxLayout *box, BoxData &data);
		void drawBoxBorder(BoxLayout *box, BoxData &data);
		void drawBoxEnd(BoxLayout *box, BoxData &data);
		void drawScrollBar(BoxLayout *box, ScrollLayoutBase *v);
		Window     *_window;
		Render     *_render;
		Canvas     *_canvas;
		PathvCache *_cache;
		float      _opacity;
		uint32_t   _mark_recursive;
		Vec2       _fixOrigin;
		float      _fixSize; // fix rect stroke width
	};
}
#endif
