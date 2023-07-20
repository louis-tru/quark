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

#include "./layout/view.h"
#include "./render/render.h"
#include "./display.h"

namespace qk {

	class Qk_EXPORT ViewRender: public Object, public ViewVisitor {
		Qk_HIDDEN_ALL_COPY(ViewRender);
	public:
		ViewRender(Display *display);
		// @overwrite class ViewVisitor
		virtual void  visitView(View* v) override;
		virtual void  visitBox(Box* box) override;
		virtual void  visitImage(Image* image) override;
		virtual void  visitVideo(Video* video) override;
		virtual void  visitScroll(Scroll* scroll) override;
		virtual void  visitInput(Input* input) override;
		virtual void  visitTextarea(Textarea* textarea) override;
		virtual void  visitButton(Button* btn) override;
		virtual void  visitTextLayout(TextLayout* text) override;
		virtual void  visitLabel(Label* label) override;
		virtual void  visitRoot(Root* root) override;
		virtual void  visitFloatLayout(FloatLayout* flow) override;
		virtual void  visitFlowLayout(FlowLayout* flow) override;
		virtual void  visitFlexLayout(FlexLayout* flex) override;
		// props
		Qk_DEFINE_PROP(Render*, render);
	private:
		Display      *_display;
		Canvas       *_canvas;
		float         _opacity;
		uint32_t      _mark_recursive;
		Qk_DEFINE_INLINE_CLASS(Inl);
	};

}
#endif
