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

void SkiaRender::solveView(View* view) {
	// visit child
	auto v = view->first();
	while(v) {
		if (v->_visible & v->_visible_region) {
			uint8_t a = (uint16_t(alpha) * v->_opacity) >> 8;
			if (a)
				v->accept(this);
				// v->accept(canvas, a);
		}
		v = v->next();
	}
}

void SkiaRender::solveBox(Box* box/*Canvas* canvas, uint8_t alpha*/) {
	if (_fill) {
		canvas->setMatrix(matrix());
		//_fill->draw(box, canvas, alpha, true);
	}
	SkiaRender::solveView(box/*canvas, alpha*/);
}

SkImage* CastSkImage(ImageSource* img);

void SkiaRender::solveImage(Image* img/*, uint8_t alpha*/) {
	auto src = source();
	if (src && src->ready()) {
		canvas->setMatrix(matrix());

		auto begin = Vec2(_padding_left - _transform_origin.x(), _padding_top - _transform_origin.y());
		auto end = layout_content_size() + begin;
		auto img = CastSkImage(src);
		SkRect rect = {begin.x(), begin.y(), end.x(), end.y()};
		SkSamplingOptions opts(SkFilterMode::kLinear, SkMipmapMode::kNearest);

		if (is_radius()) {
			// TODO ...
		} else {
			canvas->drawImageRect(img, rect, opts);
		}
		if (_fill) {
			_fill->draw(this, canvas, alpha, false);
		}
		View::draw(canvas, alpha);
	} else {
		Box::draw(canvas, alpha);
	}
}

void SkiaRender::solveRoot(Root* root/*, uint8_t alpha*/) {
	if (visible() && visible_region()) {
		uint8_t alpha = this->opacity();
		if (!alpha) return;

		auto f = fill();
		if (f) {
			canvas->setMatrix(matrix());
			if (f->type() == FillBox::M_COLOR) {
				auto color = static_cast<FillColor*>(f)->color();
				if (color.a()) {
					canvas->drawColor(color.to_uint32_xrgb());
				} else {
					canvas->drawColor(SK_ColorBLACK);
				}
				if (f->next()) {
					f->next()->draw(this, canvas, alpha, true);
				}
			} else {
				f->draw(this, canvas, alpha, true);
			}
		} else {
			canvas->drawColor(SK_ColorBLACK);
		}
		View::draw(canvas, this->opacity());
	}
}


F_NAMESPACE_END