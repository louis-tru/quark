/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
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

#include <math.h>
#include "../util/loop.h"
#include "../util/codec.h"
#include "../app.h"
#include "../display.h"
#include "./render.h"
#include "./gl/gl_render.h"
#include "../layout/root.h"
#include "../filter.h"
#include "../layout/image.h"
#include "../layout/flex.h"
#include "../layout/float.h"
#include "../layout/flow.h"

#define Qk_ENABLE_DRAW 1

namespace qk {

	static uint32_t integerExp(uint32_t n) {
		return (uint32_t) powf(2, floor(log2(n)));
	}

	static uint32_t massSample(uint32_t n) {
		n = integerExp(n);
		return Qk_MIN(n, 8);
	}

	RenderBackend::RenderBackend(Options opts)
		: _opts(opts)
		, _canvas(nullptr)
		, _delegate(nullptr)
		, _default_scale(1)
		, _opacity(1), _mark_recursive(0)
	{
		_opts.colorType = _opts.colorType ? _opts.colorType: kColor_Type_RGBA_8888;//kColor_Type_BGRA_8888;
		_opts.msaaSampleCnt = massSample(_opts.msaaSampleCnt);
	}

	RenderBackend::~RenderBackend() {}

	void RenderBackend::activate(bool isActive) {
	}

	const Array<Vec2>& RenderBackend::getPathTrianglesCache(const Path &path) {
		auto hash = path.hashCode();
		const Array<Vec2> *out;
		if (_PathTrianglesCache.get(hash, out)) return *out;
		if (_PathTrianglesCache.length() >= 1024)
			_PathTrianglesCache.clear();
		return _PathTrianglesCache.set(hash, path.getTriangles(1));
	}

	const Path& RenderBackend::getStrokePathCache(
		const Path &path, float width, Path::Cap cap, Path::Join join, float miterLimit) 
	{
		auto hash = path.hashCode();
		auto hash_part = ((*(int64_t*)&width) << 32) | *(int32_t*)&miterLimit;
		hash += (hash << 5) + hash_part + ((cap << 2) | join);
		const Path *out;
		if (_StrokePathCache.get(hash, out)) return *out;
		if (_StrokePathCache.length() >= 1024)
			_StrokePathCache.clear();
		auto stroke = path.strokePath(width,cap,join,miterLimit);
		return _StrokePathCache.set(hash, stroke.isNormalized() ? std::move(stroke): stroke.normalizedPath(1));
	}

	const Array<Vec3>& RenderBackend::getSDFStrokeTriangleStripCache(const Path &path, float width) {
		auto hash = path.hashCode();
		hash += (hash << 5) + *(int32_t*)&width;
		const Array<Vec3> *out;
		if (_SDFStrokeTriangleStripCache.get(hash, out)) return *out;
		if (_SDFStrokeTriangleStripCache.length() >= 1024)
			_SDFStrokeTriangleStripCache.clear();
		return _SDFStrokeTriangleStripCache.set(hash, path.getSDFStrokeTriangleStrip(width, 1));
	}

	const Path& RenderBackend::getNormalizedPathCache(const Path &path) {
		if (path.isNormalized()) return path;
		auto hash = path.hashCode();
		const Path *out;
		if (_NormalizedPathCache.get(hash, out)) return *out;
		if (_NormalizedPathCache.length() >= 1024)
			_NormalizedPathCache.clear();
		return _NormalizedPathCache.set(hash, path.normalizedPath(1));
	}

	// --------------------------------------------------------------------------

	void RenderBackend::visitView(View* view) {
		// visit child
		auto v = view->_first;
		if (v) {
			auto opacityCurr = _opacity;
			auto markCurr = _mark_recursive;
			do {
				if (v->_visible) {
					uint32_t mark = markCurr | v->layout_mark(); // inherit recursive
					if (mark) {
						v->solve_marks(mark);
						_mark_recursive = mark & Layout::kRecursive_Mark;
					}
					if (v->_visible_region && v->_opacity) {
						_opacity = opacityCurr * v->_opacity;
						v->accept(this);
					}
				}
				v = v->_next;
			} while(v);
			_opacity = opacityCurr;
			_mark_recursive = markCurr;
		}
	}

	void RenderBackend::visitBox(Box* box) {
		_canvas->setMatrix(box->matrix());
		const RectPath *outside = nullptr;
		if (box->_background_color.a())
			drawBoxColor(box, outside);
		if (box->background())
			drawBoxFill(box, outside);
		if (box->box_shadow())
			drawBoxShadow(box, outside);
		if (box->_border)
			drawBoxBorder(box);
		drawBoxEnd(box);
	}

	void RenderBackend::visitImage(Image* v) {
		_canvas->setMatrix(v->matrix());
		const RectPath *outside = nullptr;
		if (v->_background_color.a())
			drawBoxColor(v, outside);
		if (v->background())
			drawBoxFill(v, outside);
		// TODO ... image
		if (v->box_shadow())
			drawBoxShadow(v, outside);
		if (v->_border)
			drawBoxBorder(v);
		drawBoxEnd(v);
	}

	void RenderBackend::visitVideo(Video* video) {
		// TODO ...
	}

	void RenderBackend::visitScroll(Scroll* scroll) {
		// TODO ...
	}

	void RenderBackend::visitInput(Input* input) {
		// TODO ...
	}

	void RenderBackend::visitTextarea(Textarea* textarea) {
		// TODO ...
	}

	void RenderBackend::visitButton(Button* btn) {
		// TODO ...
	}

	void RenderBackend::visitTextLayout(TextLayout* text) {
		// TODO ...
	}

	void RenderBackend::visitLabel(Label* label) {
		// TODO ...
	}

	void RenderBackend::visitRoot(Root* v) {
		if (v->_visible) {
			uint32_t mark = v->layout_mark();
			if (mark) {
				v->solve_marks(mark);
				_mark_recursive = mark & Layout::kRecursive_Mark;
			}
			if (v->_visible_region && v->_opacity != 0) {
				_canvas->setMatrix(v->matrix());
				_canvas->clearColor(v->_background_color.to_color4f());
				const RectPath *outside = nullptr;
				if (v->background())
					drawBoxFill(v, outside);
				if (v->box_shadow())
					drawBoxShadow(v, outside);
				if (v->_border)
					drawBoxBorder(v);
				drawBoxEnd(v);
			} else {
				_canvas->clearColor(Color4f(0,0,0,0));
			}
			_mark_recursive = 0;
		}
	}

	void RenderBackend::visitFloatLayout(FloatLayout* box) {
		RenderBackend::visitBox(box);
	}

	void RenderBackend::visitFlowLayout(FlowLayout* flow) {
		RenderBackend::visitBox(flow);
	}

	void RenderBackend::visitFlexLayout(FlexLayout* flex) {
		RenderBackend::visitBox(flex);
	}

	// --------------------------------------------------------------------------

	void RenderBackend::drawBoxColor(Box *box, const RectPath *&outside) {
		// TODO ...
	}

	void RenderBackend::drawBoxFill(Box *box, const RectPath *&outside) {
		auto fill = box->background();
		do {
			switch(fill->type()) {
				case Filter::M_IMAGE: // fill
					break;
				case Filter::M_GRADIENT_Linear: // fill
					break;
				case Filter::M_GRADIENT_Radial: // fill
					break;
			}
			fill = fill->next();
		} while(fill);
	}

	void RenderBackend::drawBoxShadow(Box *box, const RectPath *&outside) {
		auto shadow = box->box_shadow();
		do {
			if (shadow->type() == Filter::M_SHADOW) {
			}
			shadow = shadow->next();
		} while(shadow);
	}

	void RenderBackend::drawBoxBorder(Box *box) {
		// TODO: RectOutlinePath
	}

	void RenderBackend::drawBoxEnd(Box *box) {
		if (box->_is_clip) {
			if (box->_first) {
				// TODO: RectPath inside
				// _canvas->clipPath(); // clip
				Render::visitView(box);
				// _canvas->restore(); // cancel clip
			}
		} else {
			Render::visitView(box);
		}
	}

}
