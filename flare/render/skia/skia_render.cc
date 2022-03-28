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

#include "../../app.h"
#include "../../display.h"
#include "./skia_render.h"
#include "skia/core/SkImage.h"

F_NAMESPACE_START

// --------------- S k i a . R e n d e r ---------------

SkImage* CastSkImage(ImageSource* v);

SkiaRender::SkiaRender(): _canvas(nullptr), _alpha32(255) {
}

void SkiaRender::solveView(View* view) {
	// visit child
	auto v = view->_first;
	if (v) {
		uint32_t alpha = _alpha32;
		do {
			if (v->_visible & v->_visible_region) {
				if (v->_opacity) {
					_alpha32 = (alpha * v->_opacity) >> 8;
					v->accept(this);
				}
			}
			v = v->_next;
		} while(v);
		_alpha32 = alpha;
	}
}

void SkiaRender::solveBox(Box* v, DrawFn fn) {
	
	_canvas->setMatrix(v->matrix());
	if (fn) {
		fn(this, v);
	} else {

	}
	if (v->_paint) {
		
		//_fill->draw(box, canvas, true);
	}
	SkiaRender::solveView(v);
}

void SkiaRender::solveImage(Image* v) {
	auto src = v->source();
	solveBox(v, src && src->ready() ? [](SkiaRender* render, Box* box) {
		Image* v = static_cast<Image*>(box);
		auto begin = Vec2(v->_padding_left - v->_transform_origin.x(), v->_padding_top - v->_transform_origin.y());
		auto end = v->layout_content_size() + begin;
		auto img = CastSkImage(src);
		SkRect rect = {begin.x(), begin.y(), end.x(), end.y()};
		SkSamplingOptions opts(SkFilterMode::kLinear, SkMipmapMode::kNearest);
		//if (!v->is_radius())
		render->_canvas->drawImageRect(img, rect, opts);
	}: nullptr);
}

void SkiaRender::solveRoot(Root* v) {
	if (v->visible() && v->visible_region() && v->opacity()) {
		solveBox(v, src && src->ready() ? [](SkiaRender* render, Box* box) {
			render->_canvas->clear(box->paint_color()->to_uint32_xrgb());
		});
	} else {
		_canvas->clear(SK_ColorBLACK);
	}
}

// --------------- P a i n t . D r a w ---------------

SkRect MakeSkRectFrom(Box *host) {
	auto begin = host->transform_origin(); // begin
	auto end = host->client_size() - begin; // end
	return {-begin.x(), -begin.y(), end.x(), end.y()};
}

void SkiaRender::solvePaintColor(Box *host, bool onlyLocal) {
	if (onlyLocal && _paint_color.a()) {
		SkPaint paint;
		paint.setColor(_paint_color.to_uint32_argb_from(alpha.val));
		if (host->is_radius()) {
			// TODO ...
		} else {
			canvas->drawRect(MakeSkRectFrom(host), paint);
		}
	}
}

void SkiaRender::solvePaintImage(Box *host, PaintImage* paint) {
	auto src = source();
	if (!src || !src->ready()) return;

	auto img = CastSkImage(src);
	SkSamplingOptions opts(SkFilterMode::kLinear, SkMipmapMode::kNearest);

	auto ori = host->transform_origin(); // begin
	auto cli = host->client_size();
	auto src_w = src->width(), src_h = src->height();

	float w, h, x, y;
	float sx, sx1, sy, sy1;
	float dx = 0, dx1;
	float dy = 0, dy1;
	float dw = cli.x(), dh = cli.y();
	float dxm = dw, dym = dh;
	
	if (PaintImage::solve_size(paint->size_x(), dw, w)) { // ok x
		if (!PaintImage::solve_size(paint->size_y(), dh, h)) { // auto y
			h = w / src_w * src_h;
		}
	} else if (PaintImage::solve_size(paint->size_y(), dh, h)) { // auto x, ok y
		w = h / src_h * src_w;
	} else { // auto x,y
		w = src_w / display()->atom_pixel();
		h = src_h / display()->atom_pixel();
	}
	
	auto scale_x = src_w / w;
	auto scale_y = src_h / h;
	auto min = [] (float a, float b) { return a < b ? a: b; };
	auto max = [] (float a, float b) { return a > b ? a: b; };

	x = PaintImage::solve_position(paint->position_x(), dw, w);
	y = PaintImage::solve_position(paint->position_y(), dh, h);

	auto _repeat = paint->repeat();

	if (_repeat == Repeat::NONE) {
		dx = max(x, 0);                 dy = max(y, 0);
		dxm = min(x + w, dw);           dym = min(y + h, dh);
		if (dx < dxm && dy < dym) {
			sx = dx - x;                  sy = dy - y;
			dx1 = min(w - sx + dx, dxm);  dy1 = min(h - sy + dy, dym);
			sx1 = dx1 - dx + sx;          sy1 = dy1 - dy + sy;
			SkRect src{sx*scale_x,sy*scale_y,sx1*scale_x,sy1*scale_y};
			SkRect dest{dx-ori.x(),dy-ori.y(),dx1-ori.x(),dy1-ori.y()};
			canvas->drawImageRect(img, src, dest, opts, nullptr, Canvas::kFast_SrcRectConstraint);
		}
	} else {
		if (_repeat == Repeat::REPEAT || _repeat == Repeat::REPEAT_X) { // repeat x
			x = fmod(fmod(x, w) - w, w);
		} else {
			dx = max(x, 0);
			dxm = min(x + w, dw);
		}
		if (_repeat == Repeat::REPEAT || _repeat == Repeat::REPEAT_Y) { // repeat y
			y = fmod(fmod(y, h) - h, h);
		} else {
			dy = max(y, 0);
			dym = min(y + h, dh);
		}
		
		if (dx < dxm && dy < dym) {
			float dy_ = dy;
			do {
				sx = fmod(dx - x, w);
				dx1 = min(w - sx + dx, dxm);
				sx1 = dx1 - dx + sx;
				do {
					sy = fmod(dy - y, h);
					dy1 = min(h - sy + dy, dym);
					sy1 = dy1 - dy + sy;
					SkRect src{sx*scale_x,sy*scale_y,sx1*scale_x,sy1*scale_y};
					SkRect dest{dx-ori.x(),dy-ori.y(),dx1-ori.x(),dy1-ori.y()};
					canvas->drawImageRect(img, src, dest, opts, nullptr, Canvas::kFast_SrcRectConstraint);
					dy = dy1;
				} while (dy < dym);
				dx = dx1;
				dy = dy_;
			} while (dx < dxm);
		}
	}
}

void SkiaRender::solvePaintGradient(Box* host, PaintGradient* paint) {
	// TODO ...
}

void SkiaRender::solvePaintShadow(Box* host, PaintShadow* paint) {
	// TODO ...
}

F_NAMESPACE_END