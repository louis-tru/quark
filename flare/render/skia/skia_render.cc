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

#include "./skia_render.h"

F_NAMESPACE_START

// --------------- S k i a . R e n d e r ---------------

SkiaRender::SkiaRender(): _canvas(nullptr), _alpha32(0) {
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

void SkiaRender::solveBox(Box* v) {
	if (v->_fill) {
		_canvas->setMatrix(v->matrix());
		//_fill->draw(box, canvas, true);
	}
	SkiaRender::solveView(v);
}

SkImage* CastSkImage(ImageSource* v);

void SkiaRender::solveImage(Image* v) {
	auto src = source();
	if (src && src->ready()) {
		_canvas->setMatrix(v->matrix());

		auto begin = Vec2(v->_padding_left - v->_transform_origin.x(), v->_padding_top - v->_transform_origin.y());
		auto end = v->layout_content_size() + begin;
		auto img = CastSkImage(src);
		SkRect rect = {begin.x(), begin.y(), end.x(), end.y()};
		SkSamplingOptions opts(SkFilterMode::kLinear, SkMipmapMode::kNearest);

		if (v->is_radius()) {
			// TODO ...
		} else {
			_canvas->drawImageRect(img, rect, opts);
		}
		if (v->_fill) {
			// img->_fill->draw(this, alpha, false);
		}
		SkiaRender::solveView(v);
	} else {
		SkiaRender::solveView(v);
	}
}

void SkiaRender::solveRoot(Root* v) {
	if (v->visible() && v->visible_region() && v->opacity()) {
		auto f = v->fill();
		if (f) {
			_canvas->setMatrix(v->matrix());
			if (f->type() == FillBox::M_COLOR) {
				auto color = static_cast<FillColor*>(f)->color();
				if (color.a()) {
					_canvas->drawColor(color.to_uint32_xrgb());
				} else {
					_canvas->drawColor(SK_ColorBLACK);
				}
				if (f->next()) {
					// f->next()->draw(this, alpha, true);
				}
			} else {
				// f->draw(this, alpha, true);
			}
		} else {
			_canvas->drawColor(SK_ColorBLACK);
		}
		SkiaRender::solveView(v);
	} else {
		_canvas->drawColor(SK_ColorBLACK);
	}
}


// --------------- F i l l . D r a w ---------------

SkRect MakeSkRectFrom(Box *host) {
	auto begin = host->transform_origin(); // begin
	auto end = host->client_size() - begin; // end
	return {-begin.x(), -begin.y(), end.x(), end.y()};
}

void FillColor::draw(Box* host, Canvas* canvas, uint8_t alpha, bool full) {
	if (full && _color.a()) {
		SkPaint paint;
		paint.setColor(_color.to_uint32_argb_from(alpha));
		if (host->is_radius()) {
			// TODO ...
		} else {
			canvas->drawRect(MakeSkRectFrom(host), paint);
		}
	}
	if (_next)
		_next->draw(host, canvas, alpha, full);
}

void FillImage::draw(Box *host, Canvas *canvas, uint8_t alpha, bool full) {
	if (full && source() && source()->ready()) {
		auto src = source();
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
		
		if (solve_size(_size_x, dw, w)) { // ok x
			if (!solve_size(_size_y, dh, h)) { // auto y
				h = w / src_w * src_h;
			}
		} else if (solve_size(_size_y, dh, h)) { // auto x, ok y
			w = h / src_h * src_w;
		} else { // auto x,y
			w = src_w / display()->atom_pixel();
			h = src_h / display()->atom_pixel();
		}
		
		auto scale_x = src_w / w;
		auto scale_y = src_h / h;
		auto min = [] (float a, float b) { return a < b ? a: b; };
		auto max = [] (float a, float b) { return a > b ? a: b; };
		
		x = solve_position(_position_x, dw, w);
		y = solve_position(_position_y, dh, h);

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

	if (_next)
		_next->draw(host, canvas, alpha, full);
}

void FillGradient::draw(Box* host, Canvas* canvas, uint8_t alpha, bool full) {
}

void FillShadow::draw(Box* host, Canvas* canvas, uint8_t alpha, bool full) {
}

void FillBorder::draw(Box* host, Canvas* canvas, uint8_t alpha, bool full) {
}

F_NAMESPACE_END