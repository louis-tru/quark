// @private head
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


#ifndef __ftr__render_skia_render__
#define __ftr__render_skia_render__

#include "../../util/macros.h"

#ifdef __OBJC__
#include "../metal.h"
#endif

#if !Qk_APPLE || Qk_ENABLE_GL
#include "../gl.h"
#endif

#include "../render.h"
#include "../../effect.h"
#include "./skia_canvas.h"
#include "skia/core/SkSurface.h"
#include "skia/gpu/GrBackendSurface.h"
#include "skia/gpu/GrDirectContext.h"

namespace quark {
	class BaseScroll;

	/**
	* @class SkiaRender
	*/
	class SkiaRender: public ViewVisitor {
	public:
		SkiaRender(Application* app);
		Qk_DEFINE_PROP_GET(Application*, app);
		virtual int  flags();
		virtual void visitView(View* v);
		virtual void visitBox(Box* box);
		virtual void visitImage(Image* image);
		virtual void visitVideo(Video* video);
		virtual void visitScroll(Scroll* scroll);
		virtual void visitInput(Input* input);
		virtual void visitTextarea(Textarea* textarea);
		virtual void visitButton(Button* btn);
		virtual void visitTextLayout(TextLayout* text);
		virtual void visitLabel(Label* label);
		virtual void visitRoot(Root* root);
		virtual void visitFloatLayout(FloatLayout* flow);
		virtual void visitFlowLayout(FlowLayout* flow);
		virtual void visitFlexLayout(FlexLayout* flex);
		virtual SkiaCanvas* getCanvas();
		void solveBox(Box* box, void (*fillFn)(SkiaRender* render, Box* v, int &clip));
	private:
		// make
		void MakeSkRectFrom(Box *box);
		void MakeRRectInside(Box *box, SkRRect* rrect);
		void MakeRRectOuter(Box *box, SkRRect* rrect);
		void MakeRPathInside(Box *box, SkPath* rrect);
		void MakeRPathOuter(Box *box, SkPath* rrect);
		// clip
		void clipRect(Box* box, SkClipOp op, bool AA);
		void clipRectInside(Box* box, SkClipOp op, bool AA);
		void clipPathInside(Box* box, SkClipOp op, bool AA);
		void clipPath(Box* box, SkClipOp op, bool AA);
		// solve
		void solveEffect(Box* box, Effect* effect, int &clip);
		void solveFill(Box* box, Fill* fill, Color fill_color);
		void solveFillImage(Box* box, FillImage* fill);
		void solveFillGradientLinear(Box* box, FillGradientLinear* fill);
		void solveFillGradientRadial(Box* box, FillGradientRadial* fill);
		void solveBorder(Box* box);
		void solveBorderRadius(Box* box);
		void solveBorderNoRadius(Box* box);
		void solveScrollBar(BaseScroll* scroll, Box *box);
		void useInsideClip(Box* box, int &clip);
	protected:
		sk_sp<GrDirectContext> _direct;
		sk_sp<SkSurface> _surface, _rasterSurface;
		SkiaCanvas*      _canvas;
		SkRect          _rect_inside, _rect;
		SkRRect         _rrect_inside, _rrect;
		SkPath          _rrect_path;
		float           _alpha;
		uint32_t        _mark_recursive;
		SkPaint         _paint;
		bool            _raster; // software raster
	};

#if !Qk_APPLE || Qk_ENABLE_GL
	/**
	* @class SkiaGLRender
	*/
	class SkiaGLRender: public GLRender, public SkiaRender {
	public:
		virtual ViewVisitor* render() override;
	protected:
		virtual void onReload() override;
		virtual void onSubmit() override;
		SkiaGLRender(Application* host, const Options& opts, bool raster);
	};
#endif

#ifdef __OBJC__
	/**
	* @class SkiaMetalRender
	*/
	class SkiaMetalRender: public MetalRender, public SkiaRender {
	public:
		virtual ViewVisitor* visitor() override;
		virtual SkiaCanvas* getCanvas() override;
	protected:
		virtual void onReload() override;
		virtual void onBegin() override;
		virtual void onSubmit() override;
		SkiaMetalRender(Application* host, const Options& opts, bool raster);
	};
#endif

}
#endif
