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
// skia
#include <skia/core/SkImage.h>
#include <skia/effects/SkGradientShader.h>
#include <skia/core/SkBlendMode.h>
#include <skia/core/SkMaskFilter.h>
#include <skia/core/SkTypeface.h>
// views
#include "../../layout/box.h"
#include "../../layout/image.h"
#include "../../layout/video.h"
#include "../../layout/scroll.h"
#include "../../layout/input.h"
#include "../../layout/text.h"
#include "../../layout/root.h"
#include "../../layout/flow.h"

#define N_ENABLE_DRAW 1
#define N_ENABLE_CLIP 1
#define N_USE_PATH_DRAW 0

namespace noug {

	// --------------- S k i a . R e n d e r ---------------

	SkImage* CastSkImage(ImageSource* v);

	SkiaCanvas* SkiaRender::getCanvas() {
		return _canvas;
	}

	SkiaRender::SkiaRender(): _canvas(nullptr), _alpha(1), _mark_recursive(0) {
		_paint.setAntiAlias(true);
		_rrect_path.setIsVolatile(true);
	}

	int SkiaRender::flags() {
		return 0;
	}

	void SkiaRender::visitView(View* view) {
		// visit child
		auto v = view->_first;
		if (v) {
			float alphaCurr = _alpha;
			uint32_t markCurr = _mark_recursive;
			do {
				if (v->_visible) {
					_mark_recursive = markCurr | v->layout_mark();
					if (_mark_recursive) {
						v->solve_recursive_marks(_mark_recursive);
					}
					if (v->_visible_region && v->_opacity > 0) {
						_alpha = alphaCurr * v->_opacity;
						_paint.setAlphaf(_alpha);
						v->accept(this);
					}
				}
				v = v->_next;
			} while(v);
			_alpha = alphaCurr;
			_mark_recursive = markCurr;
		}
	}

	void SkiaRender::visitBox(Box* box) {
		solveBox(box, nullptr);
	}

	void SkiaRender::visitImage(Image* box) {
		auto src = box->source();
		solveBox(box, src && src->ready() ? [](SkiaRender* render, Box* box) {
			Image* v = static_cast<Image*>(box);
			auto begin = Vec2(
				v->_padding_left - v->_transform_origin.x(),
				v->_padding_top - v->_transform_origin.y()
			);
			if (box->_border) {
				begin.val[0] += box->_border->width_left;
				begin.val[1] += box->_border->width_top;
			}
			auto end = v->content_size() + begin;
			auto img = CastSkImage(v->source());
			SkRect rect = {begin.x(), begin.y(), end.x(), end.y()};
			SkSamplingOptions opts(SkFilterMode::kLinear, SkMipmapMode::kNearest);
			if (N_ENABLE_DRAW) render->_canvas->drawImageRect(img, rect, opts, &render->_paint);
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

	void SkiaRender::visitTextLayout(TextLayout* text) {
		solveBox(text, nullptr);
	}

	void SkiaRender::visitLabel(Label* label) {

		if (label->_blob.length()) {
			auto lines = *label->_lines;
			auto size = label->text_size().value;

			SkPaint paint = _paint;
			auto c4f = SkColor4f::FromColor(label->text_color().value.to_uint32_argb());
			c4f.fA *= _alpha;
			paint.setColor4f(c4f);
			// if (N_ENABLE_DRAW) _canvas->drawRect(_rect_inside, paint);

			for (auto& blob: label->_blob) {
				auto tf = *reinterpret_cast<SkTypeface**>(&blob.typeface);
				tf->ref();

				if (N_ENABLE_DRAW)
					_canvas->drawGlyphs(
						blob.glyphs.length(), *blob.glyphs, *blob.offset,
						Vec2(blob.origin, lines->line(blob.line).baseline),
						SkFont(sk_sp<SkTypeface>(tf), size), _paint
					);
			}
		}

		SkiaRender::visitView(box);

	}

	void SkiaRender::visitRoot(Root* v) {
		if (v->_visible) {
			_mark_recursive = v->layout_mark();
			if (_mark_recursive) {
				v->solve_recursive_marks(_mark_recursive);
			}

			if (v->_visible_region && v->_opacity > 0) {
				solveBox(v, [](SkiaRender* render, Box* box) {
					if (N_ENABLE_DRAW)
						render->_canvas->clear(box->_fill_color.to_uint32_xrgb());
					render->solveFill(box, box->_fill, Color::from(0));
				});
			} else {
				if (N_ENABLE_DRAW) _canvas->clear(SK_ColorBLACK);
			}
		}
	}

	void SkiaRender::visitFlowLayout(FlowLayout* flow) {
		solveBox(flow, nullptr);
	}

	void SkiaRender::visitFlexLayout(FlexLayout* flex) {
		solveBox(flex, nullptr);
	}

	void SkiaRender::MakeSkRectFrom(Box *host) {
		auto begin = host->_transform_origin; // begin
		auto end = host->_client_size - begin; // end
		_rect_inside = _rect = {-begin.x(), -begin.y(), end.x(), end.y()};
		if (host->_border) {
			_rect_inside.fLeft += host->_border->width_left;
			_rect_inside.fTop += host->_border->width_top;
			_rect_inside.fRight -= host->_border->width_right;
			_rect_inside.fBottom -= host->_border->width_bottom;
		}
	}

	void SkiaRender::MakeRRectInside(Box *box, SkRRect *rrect) {
		SkVector radii[4];
		auto border = box->_border;
		if (border) {
			radii[0].fX = Float::max(box->_radius_left_top - border->width_left, 0);
			radii[0].fY = Float::max(box->_radius_left_top - border->width_top, 0);
			radii[1].fX = Float::max(box->_radius_right_top - border->width_right, 0);
			radii[1].fY = Float::max(box->_radius_right_top - border->width_top, 0);
			radii[2].fX = Float::max(box->_radius_right_bottom - border->width_right, 0);
			radii[2].fY = Float::max(box->_radius_right_bottom - border->width_bottom, 0);
			radii[3].fX = Float::max(box->_radius_left_bottom - border->width_left, 0);
			radii[3].fY = Float::max(box->_radius_left_bottom - border->width_bottom, 0);
		} else {
			radii[0] = {box->_radius_left_top, box->_radius_left_top};
			radii[1] = {box->_radius_right_top, box->_radius_right_top};
			radii[2] = {box->_radius_right_bottom, box->_radius_right_bottom};
			radii[3] = {box->_radius_left_bottom, box->_radius_left_bottom};/*left-bottom*/
		}
		rrect->setRectRadii(_rect_inside, radii);
	}

	void SkiaRender::MakeRRectOuter(Box *box, SkRRect *rrect) {
		SkVector radii[4] = {
			{box->_radius_left_top, box->_radius_left_top},
			{box->_radius_right_top, box->_radius_right_top},
			{box->_radius_right_bottom, box->_radius_right_bottom},
			{box->_radius_left_bottom, box->_radius_left_bottom},/*left-bottom*/
		};
		rrect->setRectRadii(_rect, radii);
	}

	void SkiaRender::MakeRPathInside(Box *box, SkPath* path) {
		auto border = box->_border;

		float rA = box->_radius_left_top; // border radius
		float rB = box->_radius_right_top;
		float rC = box->_radius_right_bottom;
		float rD = box->_radius_left_bottom;

		float rAA = rA + rA;
		float rBB = rB + rB;
		float rCC = rC + rC;
		float rDD = rD + rD;

		SkRect rectA{ _rect.fLeft, _rect.fTop, _rect.fLeft + rAA, _rect.fTop + rAA };
		SkRect rectB{ _rect.fRight - rBB, _rect.fTop, _rect.fRight, _rect.fTop + rBB };
		SkRect rectC{ _rect.fRight - rCC, _rect.fBottom - rCC, _rect.fRight, _rect.fBottom };
		SkRect rectD{ _rect.fLeft, _rect.fBottom - rDD, _rect.fLeft + rDD, _rect.fBottom };

		if (border) {
			float A = border->width_top; // border
			float B = border->width_right;
			float C = border->width_bottom;
			float D = border->width_left;
			SkRect rectAI{ _rect_inside.fLeft, _rect_inside.fTop, rectA.fRight - D, rectA.fBottom - C};
			SkRect rectBI{rectB.fLeft + B,_rect_inside.fTop,_rect_inside.fRight,rectB.fBottom - A};
			SkRect rectCI{rectC.fLeft + B,rectC.fTop - C,_rect_inside.fRight,_rect_inside.fBottom};
			SkRect rectDI{_rect_inside.fLeft,rectD.fTop + C,rectD.fRight - D,_rect_inside.fBottom};
			
			path->arcTo(rectAI, 180, 90, false);
			path->arcTo(rectBI, 270, 90, false);
			path->arcTo(rectCI, 0, 90, false);
			path->arcTo(rectDI, 90, 90, false);
		} else {
			path->arcTo(rectA, 180, 90, false);
			path->arcTo(rectB, 270, 90, false);
			path->arcTo(rectC, 0, 90, false);
			path->arcTo(rectD, 90, 90, false);
		}
		path->close();
	}

	void SkiaRender::MakeRPathOuter(Box *box, SkPath* path) {
		auto border = box->_border;

		float rA = box->_radius_left_top; // border radius
		float rB = box->_radius_right_top;
		float rC = box->_radius_right_bottom;
		float rD = box->_radius_left_bottom;
		float rAA = rA + rA;
		float rBB = rB + rB;
		float rCC = rC + rC;
		float rDD = rD + rD;

		SkRect rectA{ _rect.fLeft, _rect.fTop, _rect.fLeft + rAA, _rect.fTop + rAA };
		SkRect rectB{ _rect.fRight - rBB, _rect.fTop, _rect.fRight, _rect.fTop + rBB };
		SkRect rectC{ _rect.fRight - rCC, _rect.fBottom - rCC, _rect.fRight, _rect.fBottom };
		SkRect rectD{ _rect.fLeft, _rect.fBottom - rDD, _rect.fLeft + rDD, _rect.fBottom };

		path->arcTo(rectA, 180, 90, false);
		path->arcTo(rectB, 270, 90, false);
		path->arcTo(rectC, 0, 90, false);
		path->arcTo(rectD, 90, 90, false);
		path->close();
	}

	void SkiaRender::clipRectInside(Box* box, SkClipOp op, bool AA) {
		if (box->_is_radius) {
			MakeRRectInside(box, &_rrect_inside);
			if (N_ENABLE_CLIP) _canvas->clipRRect(_rrect_inside, op, AA); // SkClipOp::kIntersect
		} else {
			if (N_ENABLE_CLIP) _canvas->clipRect(_rect_inside, op, AA); // SkClipOp::kIntersect
		}
	}

	void SkiaRender::clipRect(Box* box, SkClipOp op, bool AA) {
		if (box->_is_radius) {
			MakeRRectOuter(box, &_rrect);
			if (N_ENABLE_CLIP) _canvas->clipRRect(_rrect, op, AA); // SkClipOp::kIntersect
		} else {
			if (N_ENABLE_CLIP) _canvas->clipRect(_rect, op, AA); // SkClipOp::kIntersect
		}
	}

	void SkiaRender::clipPathInside(Box* box, SkClipOp op, bool AA) {
		if (box->_is_radius) {
			SkPath path;
			MakeRPathInside(box, &path);
			if (N_ENABLE_CLIP) _canvas->clipPath(path, op, AA); // SkClipOp::kIntersect
		} else {
			if (N_ENABLE_CLIP) _canvas->clipRect(_rect_inside, op, AA); // SkClipOp::kIntersect
		}
	}

	void SkiaRender::clipPath(Box* box, SkClipOp op, bool AA) {
		if (box->_is_radius) {
			_rrect_path.reset();
			MakeRPathOuter(box, &_rrect_path);
			if (N_ENABLE_CLIP) _canvas->clipPath(_rrect_path, op, AA); // SkClipOp::kIntersect
		} else {
			if (N_ENABLE_CLIP) _canvas->clipRect(_rect, op, AA); // SkClipOp::kIntersect
		}
	}

	void SkiaRender::solveBox(Box* box, void (*fillFn)(SkiaRender* render, Box* v)) {

		_canvas->setMatrix(box->matrix());

		MakeSkRectFrom(box);

		// step 1: if exist radius or overflow clip and exist draw task then clip rect
		int clip = 0; // 1 kDifference, 2 kIntersect
		if (
			(box->_is_radius && (box->_fill_color.a() || box->_fill || fillFn)) ||
			(box->_is_clip   && (box->_first))
		) {
			_canvas->save();
			if (N_USE_PATH_DRAW)
				clipPathInside(box, SkClipOp::kIntersect, true);
			else
				clipRectInside(box, SkClipOp::kIntersect, true); // exec reverse clip
			clip = 2;
		}

		// step 2: if exist fillFn then exec fillFn else exec draw color and image and gradient background ...
		if (fillFn) {
			fillFn(this, box);
		} else {
			solveFill(box, box->_fill, box->_fill_color);
		}

		// step 3: if exist clip then cancel clip rect
		if (clip) {
			_canvas->restore(); // cancel clip
		}

		// step 4: if exist border then draw border
		if (box->_border) {
			solveBorder(box);
		}

		// step 5: exec child draw task
		SkiaRender::visitView(box);

		// step 6: if exist effect then reverse clip rect and draw effect
		if (box->_effect) {
			solveEffect(box, box->_effect);
		}
	}

	void SkiaRender::solveBorder(Box* box) {
		auto border = box->_border;

		SkColor top = border->color_top.to_uint32_argb();
		SkColor right = border->color_right.to_uint32_argb();
		SkColor bottom = border->color_bottom.to_uint32_argb();
		SkColor left = border->color_left.to_uint32_argb();
		if (
			(top == right || border->width_top == 0) &&
			(right == bottom || border->width_right == 0) &&
			(bottom == left || border->width_bottom == 0)
		) {
			if (!border->color_top.a())
				return;
			SkPath path;
			path.setFillType(SkPathFillType::kEvenOdd);
			if (box->_is_radius) {
				if (N_USE_PATH_DRAW) {
					MakeRPathOuter(box, &path); MakeRPathInside(box, &path);
				} else {
					SkRRect outer, inner;
					MakeRRectOuter(box, &outer); path.addRRect(outer);
					MakeRRectInside(box, &inner); path.addRRect(inner);
				}
			} else {
				path.addRect(_rect);
				path.addRect(_rect_inside);
			}
			auto c4f = SkColor4f::FromColor(top);
			c4f.fA *= _alpha;
			_paint.setColor4f(c4f);
			if (N_ENABLE_DRAW) _canvas->drawPath(path, _paint);
		}
		else
		if (box->_is_radius) { // multi bolor color
			solveBorderRadius(box);
		} else {
			solveBorderNoRadius(box);
		}
	}

	void SkiaRender::solveBorderRadius(Box* box) {
		auto border = box->_border;

		float A = border->width_top; // border
		float B = border->width_right;
		float C = border->width_bottom;
		float D = border->width_left;
		float rA = box->_radius_left_top; // border radius
		float rB = box->_radius_right_top;
		float rC = box->_radius_right_bottom;
		float rD = box->_radius_left_bottom;

		float rAA = rA + rA;
		float rBB = rB + rB;
		float rCC = rC + rC;
		float rDD = rD + rD;

		SkRect rectA{ _rect.fLeft, _rect.fTop, _rect.fLeft + rAA, _rect.fTop + rAA };
		SkRect rectB{ _rect.fRight - rBB, _rect.fTop, _rect.fRight, _rect.fTop + rBB };
		SkRect rectC{ _rect.fRight - rCC, _rect.fBottom - rCC, _rect.fRight, _rect.fBottom };
		SkRect rectD{ _rect.fLeft, _rect.fBottom - rDD, _rect.fLeft + rDD, _rect.fBottom };
		SkRect rectAI{ _rect_inside.fLeft, _rect_inside.fTop, rectA.fRight - D, rectA.fBottom - C};
		SkRect rectBI{rectB.fLeft + B,_rect_inside.fTop,_rect_inside.fRight,rectB.fBottom - A};
		SkRect rectCI{rectC.fLeft + B,rectC.fTop - C,_rect_inside.fRight,_rect_inside.fBottom};
		SkRect rectDI{_rect_inside.fLeft,rectD.fTop + C,rectD.fRight - D,_rect_inside.fBottom};

		float sweep0, sweep1;

		if (border->width_top > 0.0 && border->color_top.a()) {
			SkPath path;
			sweep0 = (A / (A + D)) * 90.0; // (A / (A + D)) 圆角权重接近1时绘制1/4个圆弧
			sweep1 = (A / (A + B)) * 90.0;
			if (rA > 0) path.arcTo(rectA,  270.0 - sweep0, sweep0, false);
			else        path.moveTo(_rect.fLeft, _rect.fTop);
			if (rB > 0) path.arcTo(rectB,  270.0, sweep1, false);
			else        path.lineTo(_rect.fRight, _rect.fTop);
			if (rectBI.width() > 0 && rectBI.height() > 0)
									path.arcTo(rectBI, 270.0 + sweep1, -sweep1, false);
			else				path.lineTo(_rect_inside.fRight, _rect_inside.fTop);
			if (rectAI.width() > 0 && rectAI.height() > 0)
									path.arcTo(rectAI, 270.0, -sweep0, false);
			else				path.lineTo(_rect_inside.fLeft, _rect_inside.fTop);
			path.close();
			auto c4f = SkColor4f::FromColor(border->color_top.to_uint32_argb());
			c4f.fA *= _alpha;
			_paint.setColor4f(c4f);
			if (N_ENABLE_DRAW) _canvas->drawPath(path, _paint);
		}
		if (border->width_right > 0.0 && border->color_right.a()) {
			SkPath path;
			sweep0 = (B / (B + A)) * 90.0;
			sweep1 = (B / (B + C)) * 90.0;
			if (rB > 0) path.arcTo(rectB,  0.0 - sweep0, sweep0, false);
			else        path.moveTo(_rect.fRight, _rect.fTop);
			if (rC > 0) path.arcTo(rectC,  0.0, sweep1, false);
			else        path.lineTo(_rect.fRight, _rect.fBottom);
			if (rectCI.width() > 0 && rectCI.height() > 0)
									path.arcTo(rectCI, 0.0 + sweep1, -sweep1, false);
			else				path.lineTo(_rect_inside.fRight, _rect_inside.fBottom);
			if (rectBI.width() > 0 && rectBI.height() > 0)
									path.arcTo(rectBI, 0.0, -sweep0, false);
			else				path.lineTo(_rect_inside.fRight, _rect_inside.fTop);
			path.close();
			auto c4f = SkColor4f::FromColor(border->color_right.to_uint32_argb());
			c4f.fA *= _alpha;
			_paint.setColor4f(c4f);
			if (N_ENABLE_DRAW) _canvas->drawPath(path, _paint);
		}
		if (border->width_bottom > 0.0 && border->color_bottom.a()) {
			SkPath path;
			sweep0 = (C / (C + B)) * 90.0;
			sweep1 = (C / (C + D)) * 90.0;
			if (rC > 0) path.arcTo(rectC,  90.0 - sweep0, sweep0, false);
			else        path.moveTo(_rect.fRight, _rect.fBottom);
			if (rD > 0) path.arcTo(rectD,  90.0, sweep1, false);
			else        path.lineTo(_rect.fLeft, _rect.fBottom);
			if (rectDI.width() > 0 && rectDI.height() > 0)
									path.arcTo(rectDI, 90.0 + sweep1, -sweep1, false);
			else				path.lineTo(_rect_inside.fLeft, _rect_inside.fBottom);
			if (rectCI.width() > 0 && rectCI.height() > 0)
									path.arcTo(rectCI, 90.0, -sweep0, false);
			else				path.lineTo(_rect_inside.fRight, _rect_inside.fBottom);
			path.close();
			auto c4f = SkColor4f::FromColor(border->color_bottom.to_uint32_argb());
			c4f.fA *= _alpha;
			_paint.setColor4f(c4f);
			if (N_ENABLE_DRAW) _canvas->drawPath(path, _paint);
		}
		if (border->width_left > 0.0 && border->color_left.a()) {
			SkPath path;
			sweep0 = (D / (D + C)) * 90.0;
			sweep1 = (D / (D + A)) * 90.0;
			if (rD > 0) path.arcTo(rectD,  180.0 - sweep0, sweep0, false);
			else        path.moveTo(_rect.fLeft, _rect.fBottom);
			if (rA > 0) path.arcTo(rectA,  180.0, sweep1, false);
			else        path.lineTo(_rect.fLeft, _rect.fTop);
			if (rectAI.width() > 0 && rectAI.height() > 0)
									path.arcTo(rectAI, 180.0 + sweep1, -sweep1, false);
			else				path.lineTo(_rect_inside.fLeft, _rect_inside.fTop);
			if (rectDI.width() > 0 && rectDI.height() > 0)
									path.arcTo(rectDI, 180.0, -sweep0, false);
			else				path.lineTo(_rect_inside.fLeft, _rect_inside.fBottom);
			path.close();
			auto c4f = SkColor4f::FromColor(border->color_left.to_uint32_argb());
			c4f.fA *= _alpha;
			_paint.setColor4f(c4f);
			if (N_ENABLE_DRAW) _canvas->drawPath(path, _paint);
		}
	}

	void SkiaRender::solveBorderNoRadius(Box* box) {
		auto border = box->_border;

		if (border->width_top > 0.0 && border->color_top.a()) {
			SkPath path;
			path.moveTo(_rect.fLeft, _rect.fTop);
			path.lineTo(_rect.fRight, _rect.fTop);
			path.lineTo(_rect_inside.fRight, _rect_inside.fTop);
			path.lineTo(_rect_inside.fLeft, _rect_inside.fTop);
			path.close();
			auto c4f = SkColor4f::FromColor(border->color_top.to_uint32_argb());
			c4f.fA *= _alpha;
			_paint.setColor4f(c4f);
			if (N_ENABLE_DRAW) _canvas->drawPath(path, _paint);
		}
		if (border->width_right > 0.0 && border->color_right.a()) {
			SkPath path;
			path.moveTo(_rect.fRight, _rect.fTop);
			path.lineTo(_rect.fRight, _rect.fBottom);
			path.lineTo(_rect_inside.fRight, _rect_inside.fBottom);
			path.lineTo(_rect_inside.fRight, _rect_inside.fTop);
			path.close();
			auto c4f = SkColor4f::FromColor(border->color_right.to_uint32_argb());
			c4f.fA *= _alpha;
			_paint.setColor4f(c4f);
			if (N_ENABLE_DRAW) _canvas->drawPath(path, _paint);
		}
		if (border->width_bottom > 0.0 && border->color_bottom.a()) {
			SkPath path;
			path.moveTo(_rect.fRight, _rect.fBottom);
			path.lineTo(_rect.fLeft, _rect.fBottom);
			path.lineTo(_rect_inside.fLeft, _rect_inside.fBottom);
			path.lineTo(_rect_inside.fRight, _rect_inside.fBottom);
			path.close();
			auto c4f = SkColor4f::FromColor(border->color_bottom.to_uint32_argb());
			c4f.fA *= _alpha;
			_paint.setColor4f(c4f);
			if (N_ENABLE_DRAW) _canvas->drawPath(path, _paint);
		}
		if (border->width_left > 0.0 && border->color_left.a()) {
			SkPath path;
			path.moveTo(_rect.fLeft, _rect.fBottom);
			path.lineTo(_rect.fLeft, _rect.fTop);
			path.lineTo(_rect_inside.fLeft, _rect_inside.fTop);
			path.lineTo(_rect_inside.fLeft, _rect_inside.fBottom);
			path.close();
			auto c4f = SkColor4f::FromColor(border->color_left.to_uint32_argb());
			c4f.fA *= _alpha;
			_paint.setColor4f(c4f);
			if (N_ENABLE_DRAW) _canvas->drawPath(path, _paint);
		}
	}

	void SkiaRender::solveEffect(Box* box, Effect* effect) {
		int clip = 0; // 1 kDifference, 2 kIntersect
		do {
			switch (effect->type()) {
				case Effect::M_SHADOW: {
					if (clip != 1) { // 1 kDifference, 2 kIntersect
						if (clip)
							_canvas->restore(); // cancel reverse clip
						_canvas->save();
						if (N_USE_PATH_DRAW) {
							if (N_ENABLE_CLIP) clipPath(box, SkClipOp::kDifference, true);
						} else
							if (N_ENABLE_CLIP) clipRect(box, SkClipOp::kDifference, true); // exec reverse clip
						clip = 1;
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
						if (N_USE_PATH_DRAW) {
							if (N_ENABLE_DRAW) _canvas->drawPath(_rrect_path, paint);
						} else
							if (N_ENABLE_DRAW) _canvas->drawRRect(_rrect, paint);
					} else {
						if (N_ENABLE_DRAW) _canvas->drawRect(_rect, paint);
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
			if (N_ENABLE_DRAW) _canvas->drawRect(_rect_inside, paint);
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

		auto ori = Vec2(_rect_inside.fLeft, _rect_inside.fTop);
		auto cli = Vec2(_rect_inside.fRight, _rect_inside.fBottom) - ori;
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

		if (_repeat == Repeat::NO_REPEAT) {
			dx = max(x, 0);                 dy = max(y, 0);
			dxm = min(x + w, dw);           dym = min(y + h, dh);
			if (dx < dxm && dy < dym) {
				sx = dx - x;                  sy = dy - y;
				dx1 = min(w - sx + dx, dxm);  dy1 = min(h - sy + dy, dym);
				sx1 = dx1 - dx + sx;          sy1 = dy1 - dy + sy;
				SkRect src{sx*scale_x,sy*scale_y,sx1*scale_x,sy1*scale_y};
				SkRect dest{dx+ori.x(),dy+ori.y(),dx1+ori.x(),dy1+ori.y()};
				if (N_ENABLE_DRAW) _canvas->drawImageRect(img, src, dest, opts, &_paint, SkCanvas::kFast_SrcRectConstraint);
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
						SkRect dest{dx+ori.x(),dy+ori.y(),dx1+ori.x(),dy1+ori.y()};
						if (N_ENABLE_DRAW) _canvas->drawImageRect(img, src, dest, opts, &_paint, SkCanvas::kFast_SrcRectConstraint);
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
		int quadrant = gradient->_quadrant;
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
		float w = _rect_inside.fRight - _rect_inside.fLeft;
		float h = _rect_inside.fBottom - _rect_inside.fTop;
		float a = h / 2;
		float b = w / 2;
		float c = sqrtf(a*a + b*b);
		float A = quadrant & 0x1 ? -atanf(a/b): atanf(a/b);
		float A_ = A - R;
		float d = cosf(A_) * c;
		float p0x = cosf(R) * d;
		float p0y = sinf(R) * d;
		float p1x = -p0x;
		float p1y = -p0y;
		
		float centerX = _rect_inside.fLeft + b;
		float centerY = _rect_inside.fTop + a;
		
		//N_DEBUG("%f, %f, %d", R * N_180_RATIO_PI, gradient->angle(), quadrant);
		
		SkPoint pts[2] = {
			{p0x + centerX, p0y + centerY}, {p1x + centerX, p1y + centerY}
		};
		if (quadrant % 3) { // swap point
			SkPoint t = pts[0];
			pts[0] = pts[1];
			pts[1] = t;
		}

		auto shader = SkGradientShader::MakeLinear
			(pts, colors, pos, gradient->count(), SkTileMode::kDecal, 0, nullptr);
		SkPaint paint = _paint;
		paint.setShader(shader);
		if (N_ENABLE_DRAW) _canvas->drawRect(_rect_inside, paint);
	}

	void SkiaRender::solveFillGradientRadial(Box* box, FillGradientRadial* gradient) {
		const SkColor *colors = gradient->colors_argb_uint32_t().val();
		const SkScalar *pos = gradient->positions().val();
		SkTileMode mode = SkTileMode::kClamp;
		float centerX = (_rect_inside.fLeft + _rect_inside.fRight) / 2;
		SkPoint center = SkPoint::Make(centerX, centerX);
		double r = _rect_inside.fRight - _rect_inside.fLeft;
		double r2 = _rect_inside.fBottom - _rect_inside.fTop;
		auto mat = SkMatrix::Scale(1, r2 / r);
		auto shader = SkGradientShader::MakeRadial
			(center, r / 2, colors, pos, gradient->count(), SkTileMode::kClamp, 0, &mat);
		SkPaint paint = _paint;
		paint.setShader(shader);
		if (N_ENABLE_DRAW) _canvas->drawRect(_rect_inside, paint);
	}

}
