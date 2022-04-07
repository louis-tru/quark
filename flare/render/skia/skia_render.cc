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
#include <skia/core/SkImage.h>
#include <skia/effects/SkGradientShader.h>
#include <skia/core/SkBlendMode.h>
#include <skia/core/SkMaskFilter.h>
// views
#include "../../layout/box.h"
#include "../../layout/image.h"
#include "../../layout/video.h"
#include "../../layout/scroll.h"
#include "../../layout/input.h"
#include "../../layout/text.h"
#include "../../layout/root.h"
#include "../../layout/flex.h"

F_NAMESPACE_START

// --------------- S k i a . R e n d e r ---------------

SkImage* CastSkImage(ImageSource* v);

SkRect SkiaRender::MakeSkRectFrom(Box *host) {
	auto begin = host->_transform_origin; // begin
	auto end = host->_client_size - begin; // end
	return {-begin.x(), -begin.y(), end.x(), end.y()};
}

SkiaCanvas* SkiaRender::getCanvas() {
	return _canvas;
}

SkiaRender::SkiaRender(): _canvas(nullptr), _alpha(1) {
	_paint.setAntiAlias(true);
}

int SkiaRender::flags() {
	return 0;
}

void SkiaRender::visitView(View* view) {
	// visit child
	auto v = view->_first;
	if (v) {
		float alphaCur = _alpha;
		do {
			if (v->_visible & v->_visible_region) {
				if (v->_opacity > 0) {
					_alpha = alphaCur * v->_opacity;
					_paint.setAlphaf(_alpha);
					v->accept(this);
				}
			}
			v = v->_next;
		} while(v);
		_alpha = alphaCur;
	}
}

void SkiaRender::visitBox(Box* box) {
	solveBox(box, nullptr);
}

void SkiaRender::visitImage(Image* box) {
	auto src = box->source();
	solveBox(box, src && src->ready() ? [](SkiaRender* render, Box* box) {
		Image* v = static_cast<Image*>(box);
		auto begin = Vec2(v->_padding_left - v->_transform_origin.x(), v->_padding_top - v->_transform_origin.y());
		auto end = v->layout_content_size() + begin;
		auto img = CastSkImage(v->source());
		SkRect rect = {begin.x(), begin.y(), end.x(), end.y()};
		SkSamplingOptions opts(SkFilterMode::kLinear, SkMipmapMode::kNearest);
		render->_canvas->drawImageRect(img, rect, opts, &render->_paint);
	}: nullptr);
}

void SkiaRender::visitVideo(Video* video) {
	solveBox(video, [](SkiaRender* render, Box* box) {
		// TODO ...
	});
}

void SkiaRender::visitScroll(Scroll* scroll) {
	solveBox(scroll, nullptr);
}

void SkiaRender::visitInput(Input* input) {
	solveBox(input, [](SkiaRender* render, Box* box) {
		// TODO ...
	});
}

void SkiaRender::visitText(Text* text) {
	solveBox(text, [](SkiaRender* render, Box* box) {
		// TODO ...
	});
}

void SkiaRender::visitLabel(Label* label) {
	// TODO ...
}

void SkiaRender::visitRoot(Root* v) {
	if (v->visible() && v->visible_region() && v->opacity()) {
		solveBox(v, [](SkiaRender* render, Box* box) {
			render->_canvas->clear(box->_fill_color.to_uint32_xrgb());
			render->solveFill(box, box->_fill, Color::from(0));
		});
	} else {
		_canvas->clear(SK_ColorBLACK);
	}
}

void SkiaRender::visitFlowLayout(FlowLayout* flow) {
	solveBox(flow, nullptr);
}

void SkiaRender::visitFlexLayout(FlexLayout* flex) {
	solveBox(flex, nullptr);
}

void SkiaRender::clipRect(Box* box, SkClipOp op, bool AA) {
	if (box->_is_radius) {
		SkVector radii[4] = {
			{box->_radius_left_top, box->_radius_left_top},
			{box->_radius_right_top, box->_radius_right_top},
			{box->_radius_right_bottom, box->_radius_right_bottom},
			{box->_radius_left_bottom, box->_radius_left_bottom},/*left-bottom*/
		};
		_rrect.setRectRadii(_rect, radii);
		_canvas->clipRRect(_rrect, op, AA); // SkClipOp::kIntersect
	} else {
		_canvas->clipRect(_rect, op, AA); // SkClipOp::kIntersect
	}
}

void SkiaRender::solveBox(Box* box, void (*fillFn)(SkiaRender* render, Box* v)) {

	_canvas->setMatrix(box->matrix());

	_rect = MakeSkRectFrom(box);

	// step 1: if exist radius or overflow clip and exist draw task then clip rect
	bool is_clip = false;
	if (
		(box->_is_radius && (box->_fill_color.a() || box->_fill || fillFn)) ||
		(box->_is_clip   && (box->_first))
	) {
		_canvas->save();
		clipRect(box, SkClipOp::kIntersect, true); // exec reverse clip
		is_clip = true;
	}

	// step 2: if exist fillFn then exec fillFn else exec draw color and image and gradient background ...
	if (fillFn) {
		fillFn(this, box);
	} else {
		solveFill(box, box->_fill, box->_fill_color);
	}

	// step 3: exec child draw task
	SkiaRender::visitView(box);

	// step 4: if exist clip then cancel clip rect
	if (is_clip) {
		_canvas->restore(); // cancel clip
	}

	// step 5: if exist shadow then reverse clip rect and draw effect
	if (box->_effect)
		solveEffect(box, box->_effect);
}

void SkiaRender::solveEffect(Box* box, Effect* effect) {
	int clip = 0; // 1 kDifference, 2 kIntersect
	do {
		switch (effect->type()) {
			case Effect::M_SHADOW: {
				if (clip != 1) {
					if (clip) {
						_canvas->restore(); // cancel reverse clip
					}
					clip = 1;
					_canvas->save();
					clipRect(box, SkClipOp::kDifference, true); // exec reverse clip
				}
				SkPaint paint;// = _paint;
				auto shadow = static_cast<BoxShadow*>(effect)->value();
				auto c4f = SkColor4f::FromColor(shadow.color.to_uint32_argb());
				c4f.fA *= _alpha;
				paint.setColor4f(c4f);
				paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, shadow.size));
				_canvas->save();
				_canvas->translate(shadow.offset_x, shadow.offset_y);
				if (box->_is_radius) {
					_canvas->drawRRect(_rrect, paint);
				} else {
					_canvas->drawRect(_rect, paint);
				}
				_canvas->restore();
				break;
			}
			case Effect::M_BLUR:
				// TODO ...
				break;
			default:
				break;
		}
		effect = effect->next();
	} while(effect);

	if (clip)
		_canvas->restore(); // cancel reverse clip
}

void SkiaRender::solveFill(Box* box, Fill* fill, Color color) {

	if (color.a()) {
		SkPaint paint = _paint;
		auto c4f = SkColor4f::FromColor(color.to_uint32_argb());
		c4f.fA *= _alpha;
		paint.setColor4f(c4f);
		_canvas->drawRect(_rect, paint);
	}

	while(fill) {
		switch(fill->type()) {
			case Effect::M_IMAGE:
				solveFillImage(box, static_cast<FillImage*>(fill)); break;
			case Effect::M_GRADIENT_Linear:
				solveFillGradientLinear(box, static_cast<FillGradientLinear*>(fill)); break;
			case Effect::M_GRADIENT_Radial:
				solveFillGradientRadial(box, static_cast<FillGradientRadial*>(fill)); break;
			default: break;
		}
		fill = fill->next();
	}
}

void SkiaRender::solveFillImage(Box *box, FillImage* fill) {
	auto src = fill->source();
	if (!src || !src->ready()) return;

	auto img = CastSkImage(src);
	SkSamplingOptions opts(SkFilterMode::kLinear, SkMipmapMode::kNearest);

	auto ori = box->transform_origin(); // begin
	auto cli = box->client_size();
	auto src_w = src->width(), src_h = src->height();

	float w, h, x, y;
	float sx, sx1, sy, sy1;
	float dx = 0, dx1;
	float dy = 0, dy1;
	float dw = cli.x(), dh = cli.y();
	float dxm = dw, dym = dh;
	
	if (FillImage::compute_size(fill->size_x(), dw, w)) { // ok x
		if (!FillImage::compute_size(fill->size_y(), dh, h)) { // auto y
			h = w / src_w * src_h;
		}
	} else if (FillImage::compute_size(fill->size_y(), dh, h)) { // auto x, ok y
		w = h / src_h * src_w;
	} else { // auto x,y
		w = src_w / display()->atom_pixel();
		h = src_h / display()->atom_pixel();
	}
	
	auto scale_x = src_w / w;
	auto scale_y = src_h / h;
	auto min = [] (float a, float b) { return a < b ? a: b; };
	auto max = [] (float a, float b) { return a > b ? a: b; };

	x = FillImage::compute_position(fill->position_x(), dw, w);
	y = FillImage::compute_position(fill->position_y(), dh, h);

	auto _repeat = fill->repeat();

	if (_repeat == Repeat::NONE) {
		dx = max(x, 0);                 dy = max(y, 0);
		dxm = min(x + w, dw);           dym = min(y + h, dh);
		if (dx < dxm && dy < dym) {
			sx = dx - x;                  sy = dy - y;
			dx1 = min(w - sx + dx, dxm);  dy1 = min(h - sy + dy, dym);
			sx1 = dx1 - dx + sx;          sy1 = dy1 - dy + sy;
			SkRect src{sx*scale_x,sy*scale_y,sx1*scale_x,sy1*scale_y};
			SkRect dest{dx-ori.x(),dy-ori.y(),dx1-ori.x(),dy1-ori.y()};
			_canvas->drawImageRect(img, src, dest, opts, &_paint, SkCanvas::kFast_SrcRectConstraint);
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
					_canvas->drawImageRect(img, src, dest, opts, &_paint, SkCanvas::kFast_SrcRectConstraint);
					dy = dy1;
				} while (dy < dym);
				dx = dx1;
				dy = dy_;
			} while (dx < dxm);
		}
	}
}

void SkiaRender::solveFillGradientLinear(Box* box, FillGradientLinear* gradient) {
	const SkColor *colors = gradient->colors_argb_uint32_t().val();
	const SkScalar *pos = gradient->positions().val();
	float R = gradient->_radian;
	/*
	 a = w/2
	 b = h/2
	 c = √(a^2+b^2)
	 θA=atan(a/b)
	 θA'=θA-θR
	 d=cosθA' * c
	 p0x = cosθR * d
	 p0y = sinθR * d
	 */
	float w = _rect.fRight - _rect.fLeft;
	float h = _rect.fBottom - _rect.fTop;
	float a = h / 2;
	float b = w / 2;
	float c = sqrtf(a*a + b*b);
	float A = gradient->_quadrant ? -atanf(a/b): atanf(a/b);
	float A_ = A - R;
	float d = cosf(A_) * c;
	float p0x = cosf(R) * d;
	float p0y = sinf(R) * d;
	float p1x = -p0x;
	float p1y = -p0y;
	
	float centerX = _rect.fLeft + b;
	float centerY = _rect.fTop + a;

	SkPoint pts[2] = {
		SkPoint::Make(p0x + centerX, p0y + centerY),
		SkPoint::Make(p1x + centerX, p1y + centerY),
	};
	auto shader = SkGradientShader::MakeLinear
		(pts, colors, pos, gradient->count(), SkTileMode::kDecal, 0, nullptr);
	SkPaint paint = _paint;
	paint.setShader(shader);
	_canvas->drawRect(_rect, paint);
}

void SkiaRender::solveFillGradientRadial(Box* box, FillGradientRadial* gradient) {
	const SkColor *colors = gradient->colors_argb_uint32_t().val();
	const SkScalar *pos = gradient->positions().val();
	SkTileMode mode = SkTileMode::kClamp;
	float centerX = (_rect.fLeft + _rect.fRight) / 2;
	SkPoint center = SkPoint::Make(centerX, centerX);
	double r = _rect.fRight - _rect.fLeft;
	double r2 = _rect.fBottom - _rect.fTop;
	auto mat = SkMatrix::Scale(1, r2 / r);
	auto shader = SkGradientShader::MakeRadial
		(center, r / 2, colors, pos, gradient->count(), SkTileMode::kClamp, 0, &mat);
	SkPaint paint = _paint;
	paint.setShader(shader);
	_canvas->drawRect(_rect, paint);
}

F_NAMESPACE_END
