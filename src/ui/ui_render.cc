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

#include "./window.h"
#include "./filter.h"
#include "./ui_render.h"
#include "./layout/root.h"
#include "./layout/image.h"
#include "./layout/flex.h"
#include "./layout/float.h"
#include "./layout/flow.h"
#include "./layout/text.h"
#include "./layout/input.h"
#include "./layout/textarea.h"
#include "./layout/label.h"
#include "./layout/transform.h"

namespace qk {

	typedef UIRender::BoxData BoxData;

	UIRender::UIRender(Window *window)
		: _render(window->render()), _canvas(nullptr)
		, _cache(nullptr)
		, _window(window)
		, _opacity(1), _mark_recursive(0), _matrix(nullptr)
	{
		_canvas = _render->getCanvas();
		_cache = _canvas->getPathvCache();
	}

	Rect UIRender::getRect(BoxLayout* box) {
		return {
			_fixOrigin, {box->_client_size[0]-_fixSize,box->_client_size[1]-_fixSize},
		};
	}

	void UIRender::getInsideRectPath(BoxLayout *box, BoxData &out) {
		if (out.inside)
			return;
		if (box->_border) {
			auto rect = getRect(box);
			auto radius = &box->_border_radius_left_top;
			auto border = box->_border->width;
			Hash5381 hash;
			hash.updatefv4(rect.origin.val);
			hash.updatefv4(radius);
			hash.updatefv4(border);

			out.inside = _cache->getRRectPathFromHash(hash.hashCode());
			if (!out.inside) {
				float xy_0_5    = Float32::min(rect.size.x() * 0.5f, rect.size.y() * 0.5f);
				float borderFix[4] = {
					Float32::max(0, border[0]-_fixSize), Float32::max(0, border[1]-_fixSize),
					Float32::max(0, border[2]-_fixSize), Float32::max(0, border[3]-_fixSize),
				};
				rect.origin[0] += borderFix[3]; // left
				rect.origin[1] += borderFix[0]; // top
				rect.size  [0] -= borderFix[3] + borderFix[1]; // left + right
				rect.size  [1] -= borderFix[0] + borderFix[2]; // top + bottom

				//Qk_DEBUG("getInsideRectPath have border");

				if (*reinterpret_cast<const uint64_t*>(radius) == 0 &&
						*reinterpret_cast<const uint64_t*>(radius+2) == 0
				) {
					out.inside = &_cache->setRRectPathFromHash(hash.hashCode(), RectPath::MakeRect(rect));
				} else {
					float lt = Qk_MIN(radius[0],xy_0_5), rt = Qk_MIN(radius[1],xy_0_5);
					float rb = Qk_MIN(radius[2],xy_0_5), lb = Qk_MIN(radius[3],xy_0_5);
					auto border = box->_border->width;
					Path::BorderRadius Br{
						{lt-border[3], lt-border[0]}, {rt-border[1], rt-border[0]},
						{rb-border[1], rb-border[2]}, {lb-border[3], lb-border[2]},
					};
					out.inside = &_cache->setRRectPathFromHash(hash.hashCode(), RectPath::MakeRRect(rect, Br));
				}
			}
		} else {
			out.inside = &_cache->getRRectPath(getRect(box), &box->_border_radius_left_top);
		}
	}

	void UIRender::getOutsideRectPath(BoxLayout *box, BoxData &out) {
		if (!out.outside)
			out.outside = &_cache->getRRectPath(getRect(box), &box->_border_radius_left_top);
	}

	void UIRender::getRRectOutlinePath(BoxLayout *box, BoxData &out) {
		if (!out.outline) {
			auto border = box->_border->width;
			float borderFix[4] = {
				Float32::max(0, border[0]-_fixSize), Float32::max(0, border[1]-_fixSize),
				Float32::max(0, border[2]-_fixSize), Float32::max(0, border[3]-_fixSize),
			};
			out.outline = &_cache->getRRectOutlinePath(getRect(box), borderFix, &box->_border_radius_left_top);
		}
	}

	void UIRender::drawBoxColor(BoxLayout *box, BoxData &data) {
		getInsideRectPath(box, data);
		_canvas->drawPathvColor(*data.inside,
			box->_background_color.to_color4f_alpha(_opacity), kSrcOver_BlendMode
		);
		// Paint paint;
		// paint.antiAlias = false;
		// paint.color = box->_background_color.to_color4f_alpha(_opacity);
		// _canvas->drawPathv(rect->path, paint);
	}

	void UIRender::drawBoxFill(BoxLayout *box, BoxData &data) {
		getInsideRectPath(box, data);
		auto fill = box->_background;
		do {
			switch(fill->type()) {
				case BoxFilter::kImage:// fill
					drawBoxFillImage(box, static_cast<FillImage*>(fill), data); break;
				case BoxFilter::kGradientLinear: // fill
					drawBoxFillLinear(box, static_cast<FillGradientLinear*>(fill), data); break;
				case BoxFilter::kGradientRadial: // fill
					drawBoxFillRadial(box, static_cast<FillGradientRadial*>(fill), data); break;
				default: break;
			}
			fill = fill->next();
		} while(fill);
	}

	void UIRender::drawBoxFillImage(BoxLayout *box, FillImage *fill, BoxData &data) {
		auto src = fill->source();
		if (!src || !src->load()) return;

		src->markAsTexture(_render); // mark texure

		//auto pix = src->pixels().val();
		auto src_w = src->width(), src_h = src->height();
		auto cli = box->_client_size;
		auto dw = cli.x(), dh = cli.y();
		float w, h, x, y;

		if (box->_border) {
			dw -= (box->_border->width[3] + box->_border->width[1]); // left + right
			dh -= (box->_border->width[0] + box->_border->width[2]); // top + bottom
		}
		if (FillImage::compute_size(fill->size_x(), dw, w)) { // ok x
			if (!FillImage::compute_size(fill->size_y(), dh, h)) // auto y
				h = w / src_w * src_h;
		} else if (FillImage::compute_size(fill->size_y(), dh, h)) { // auto x, ok y
			w = h / src_h * src_w;
		} else { // auto x,y
			w = src_w / _window->atomPixel();
			h = src_h / _window->atomPixel();
		}
		x = FillImage::compute_position(fill->position_x(), dw, w);
		y = FillImage::compute_position(fill->position_y(), dh, h);

		if (box->_border) {
			x += box->_border->width[3]; // left
			y += box->_border->width[0]; // top
		}

		Paint paint0;
		ImagePaint paint;
		paint.setImage(src, {{x,y}, {w,h}});
		paint0.image = &paint;
		paint0.color.set_a(_opacity);

		switch(fill->repeat()) {
			case Repeat::kRepeat:
				paint.tileModeX = ImagePaint::kRepeat_TileMode;
				paint.tileModeY = ImagePaint::kRepeat_TileMode; break;
			case Repeat::kRepeatX:
				paint.tileModeX = ImagePaint::kRepeat_TileMode;
				paint.tileModeY = ImagePaint::kDecal_TileMode; break;
			case Repeat::kRepeatY:
				paint.tileModeX = ImagePaint::kDecal_TileMode;
				paint.tileModeY = ImagePaint::kRepeat_TileMode; break;
			case Repeat::kRepeatNo:
				paint.tileModeX = ImagePaint::kDecal_TileMode;
				paint.tileModeY = ImagePaint::kDecal_TileMode; break;
		}
		paint.filterMode = ImagePaint::kLinear_FilterMode;
		paint.mipmapMode = ImagePaint::kNearest_MipmapMode;

		_canvas->drawPathv(*data.inside, paint0);
	}

	void UIRender::drawBoxFillLinear(BoxLayout *box, FillGradientLinear *fill, BoxData &data) {
		auto &colors = fill->colors();
		auto &pos = fill->positions();
		auto R = fill->radian();
		auto quadrant = fill->quadrant();
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
		auto _rect_inside = data.inside->rect;
		float w = _rect_inside.size.x();
		float h = _rect_inside.size.y();
		float a = h * 0.5;
		float b = w * 0.5;
		float c = sqrtf(a*a + b*b);
		float A = quadrant & 0x1 ? -atanf(a/b): atanf(a/b);
		float A_ = A - R;
		float d = cosf(A_) * c;
		float p0x = cosf(R) * d;
		float p0y = sinf(R) * d;
		float p1x = -p0x;
		float p1y = -p0y;
		float centerX = _rect_inside.origin.x() + b;
		float centerY = _rect_inside.origin.y() + a;

		//Qk_DEBUG("%f, %f, %d", R * Qk_180_RATIO_PI, gradient->angle(), quadrant);

		Vec2 pts[2] = {
			{p0x + centerX, p0y + centerY}, // origin
			{p1x + centerX, p1y + centerY} // end
		};
		if (quadrant % 3) { // swap vec
			auto t = pts[0];
			pts[0] = pts[1];
			pts[1] = t;
		}
		Paint paint;
		paint.color.set_a(_opacity);
		GradientPaint g{
			GradientPaint::kLinear_Type, pts[0], pts[1], fill->colors().length(),
			fill->colors().val(), fill->positions().val()
		};
		paint.gradient = &g;
		paint.type = Paint::kGradient_Type;

		_canvas->drawPathv(*data.inside, paint);
	}

	void UIRender::drawBoxFillRadial(BoxLayout *box, FillGradientRadial *fill, BoxData &data) {
		auto &colors = fill->colors();
		auto &pos = fill->positions();
		auto _rect_inside = data.inside->rect;
		Vec2 radius{_rect_inside.size.x() * 0.5f, _rect_inside.size.y() * 0.5f};
		Vec2 center = _rect_inside.origin + radius;
		Paint paint;
		paint.color.set_a(_opacity);
		GradientPaint g{
			GradientPaint::kRadial_Type, center, radius, fill->colors().length(),
			fill->colors().val(), fill->positions().val()
		};
		paint.gradient = &g;
		paint.type = Paint::kGradient_Type;
		_canvas->drawPathv(*data.inside, paint);
	}

	void UIRender::drawBoxShadow(BoxLayout *box, BoxData &data) {
		getOutsideRectPath(box, data);
		_canvas->save();
		_canvas->clipPathv(*data.outside, Canvas::kDifference_ClipOp, false);
		auto shadow = box->box_shadow();
		do {
			if (shadow->type() != BoxFilter::kShadow)
				break;
			auto s = shadow->value();
			auto &o = data.outside->rect.origin;
			_canvas->drawRRectBlurColor({
				{o.x()+s.offset_x, o.y()+s.offset_y}, data.outside->rect.size,
			},&box->_border_radius_left_top, s.size, s.color.to_color4f_alpha(_opacity), kSrcOver_BlendMode);
			shadow = static_cast<BoxShadow*>(shadow->next());
		} while(shadow);
		_canvas->restore();
	}

	void UIRender::drawBoxBorder(BoxLayout *box, BoxData &data) {
		getRRectOutlinePath(box, data);
		auto border = box->_border;
		Paint stroke;
		stroke.style = Paint::kStroke_Style;

		for (int i = 0; i < 4; i++) {
			if (border->width[i] > 0) { // top
				auto pv = &data.outline->top + i;
				if (pv->vCount) {
					_canvas->drawPathvColor(*pv, border->color[i].to_color4f_alpha(_opacity), kSrcOver_BlendMode);
				} else { // stroke
					stroke.color = border->color[i].to_color4f_alpha(_opacity);
					stroke.width = border->width[i];
					_canvas->drawPath(pv->path, stroke);
				}
			}
		}
	}

	void UIRender::drawBoxEnd(BoxLayout *box, BoxData &data) {
		if (box->_clip) {
			if (box->_first) {
				getInsideRectPath(box, data);
				_canvas->save();
				_canvas->clipPathv(*data.inside, Canvas::kIntersect_ClipOp, true); // clip
				UIRender::visitLayout(box);
				_canvas->restore(); // cancel clip
			}
		} else {
			UIRender::visitLayout(box);
		}
	}

	void UIRender::drawScrollBar(BoxLayout *b, ScrollLayoutBase *v) {
		if ( (v->_scrollbar_h || v->_scrollbar_v) && v->_scrollbar_opacity ) {
			auto width = v->_scrollbar_width;
			auto margin = v->_scrollbar_margin;
			auto origin = Vec2();
			auto size = b->_client_size;
			auto color = v->scrollbar_color().to_color4f_alpha(_opacity * v->_scrollbar_opacity);

			if (b->_border) {
				origin = Vec2{b->_border->width[3],b->_border->width[0]};
				size[0] -= (b->_border->width[3] + b->_border->width[1]); // left + right
				size[1] -= (b->_border->width[0] + b->_border->width[2]); // top + bottom
			}

			if ( v->_scrollbar_h ) { // draw horizontal scrollbar
				float radius[] = {width,width,width,width};
				auto &rect = _cache->getRRectPath({
					{-origin.x(), size.y() - width - margin - origin.y()},
					{v->_scrollbar_position_h.val[1], width}
				}, radius);
				auto mat = Mat(*_matrix).set_translate(b->position());
				_canvas->setMatrix(mat.translate_x(v->_scrollbar_position_h.val[0]));
				_canvas->drawPathvColor(rect, color, kSrcOver_BlendMode);
			}

			if ( v->_scrollbar_v ) { // draw vertical scrollbar
				float radius[] = {width,width,width,width};
				auto &rect = _cache->getRRectPath({
					{size.x() - width - margin - origin.x(), -origin.y()},
					{width, v->_scrollbar_position_v.val[1]}
				}, radius);
				auto mat = Mat(*_matrix).set_translate(b->position());
				_canvas->setMatrix(mat.translate_y(v->_scrollbar_position_v.val[0]));
				_canvas->drawPathvColor(rect, color, kSrcOver_BlendMode);
			}
		}
	}

	void UIRender::visitLayout(Layout *view) {
		// visit child
		auto v = view->_first;
		if (v) {
			auto opacityCurr = _opacity;
			auto markCurr = _mark_recursive;
			do {
				if (v->_visible) {
					uint32_t mark = markCurr | v->mark_value(); // inherit recursive
					if (mark) {
						v->solve_marks(*_matrix, mark);
						_mark_recursive = mark & Layout::kRecursive_Mark;
					}
					if (v->_visible_region && v->_opacity) {
						_opacity = opacityCurr * v->_opacity;
						v->draw(this);
					}
				}
				v = v->_next;
			} while(v);
			_opacity = opacityCurr;
			_mark_recursive = markCurr;
		}
	}

	void UIRender::visitBox(BoxLayout* box) {
		BoxData data;
		_canvas->setTranslate(box->position());
		if (box->_box_shadow)
			drawBoxShadow(box, data);
		if (box->_background_color.a())
			drawBoxColor(box, data);
		if (box->_background)
			drawBoxFill(box, data);
		if (box->_border)
			drawBoxBorder(box, data);
		drawBoxEnd(box, data);
	}

	void UIRender::visitImage(ImageLayout* v) {
		BoxData data;
		_canvas->setTranslate(v->position());
		if (v->_box_shadow)
			drawBoxShadow(v, data);
		if (v->_background_color.a())
			drawBoxColor(v, data);
		if (v->_background)
			drawBoxFill(v, data);

		auto src = v->source();
		if (src && src->load()) {
			src->markAsTexture(_render);
			getInsideRectPath(v, data);
			//auto cli = v->client_size();
			//Qk_DEBUG("--- w %f, h %f, s: %f", cli.x(), cli.y(), cli.x()/cli.y());
			Paint p0;
			ImagePaint paint;
			p0.image = &paint;
			p0.color.set_a(_opacity);
			paint.tileModeX = ImagePaint::kDecal_TileMode;
			paint.tileModeY = ImagePaint::kDecal_TileMode;
			paint.filterMode = ImagePaint::kLinear_FilterMode;
			paint.mipmapMode = ImagePaint::kNearest_MipmapMode;
			paint.setImage(src, data.inside->rect);
			_canvas->drawPathv(*data.inside, p0);
		}
		if (v->_border)
			drawBoxBorder(v, data);
		drawBoxEnd(v, data);
	}

	void UIRender::visitScroll(ScrollLayout* v) {
		UIRender::visitBox(v);
		drawScrollBar(v,v);
	}

	void UIRender::visitInput(InputLayout* v) {
		BoxData data;
		_canvas->setTranslate(v->position());
		if (v->_box_shadow)
			drawBoxShadow(v, data);
		if (v->_background_color.a())
			drawBoxColor(v, data);
		if (v->_background)
			drawBoxFill(v, data);
		if (v->_border)
			drawBoxBorder(v, data);

		auto lines = *v->_lines;
		auto offset = v->input_text_offset() + Vec2(v->_padding_left, v->_padding_top);
		auto twinkle = v->_editing && v->_cursor_twinkle_status;
		auto visible = v->_blob_visible.length() > 0;
		auto clip = v->_clip && (visible || twinkle);

		if (clip) {
			getInsideRectPath(v, data);
			_canvas->save();
			_canvas->clipPathv(*data.inside, Canvas::kIntersect_ClipOp, true); // clip
		}

		if (visible) {
			auto draw_background = [&](TextBlob &blob, const Color4f &color) {
				auto &line = lines->line(blob.line);
				auto x = offset.x() + line.origin + blob.origin;
				auto y = offset.y() + line.baseline - blob.ascent;
				auto offset_x = blob.core.offset.front().x();
				auto width = blob.core.offset.back().x();
				auto &rect = _cache->getRectPath({{x + offset_x, y},{width, blob.height}});
				_canvas->drawPathvColor(rect, color, kSrcOver_BlendMode);
			};

			// draw text background
			if (v->text_length() && v->text_background_color().value.a()) {
				auto color = v->text_background_color().value.to_color4f_alpha(_opacity);
				for (auto i: v->_blob_visible)
					draw_background(v->_blob[i], color);
			}

			// draw text marked
			auto begin = v->_marked_blob_begin;
			if (begin < v->_marked_blob_end && v->_marked_color.a()) {
				auto color = v->_marked_color.to_color4f_alpha(_opacity);
				do
					draw_background(v->_blob[begin++], color);
				while(begin < v->_marked_blob_end);
			}

			auto color = v->_value_u4.length() ? v->text_color().value: v->placeholder_color();
			if (color.a()) {
				// draw text shadow
				auto size = v->text_size().value;
				auto shadow = v->text_shadow().value;
				if (shadow.size) {
					Paint paint;
					PaintFilter filter;
					filter.type = PaintFilter::kBlur_Type;
					filter.val0 = shadow.size;
					paint.filter = &filter;
					paint.color = shadow.color.to_color4f_alpha(_opacity * color.a());
					for (auto i: v->_blob_visible) {
						auto &blob = v->_blob[i];
						auto &line = lines->line(blob.line);
						_canvas->drawTextBlob(&blob.core, {
							line.origin + blob.origin + shadow.offset_x + offset.x(),
							line.baseline + shadow.offset_y + offset.y()
						}, size, paint);
					}
				}

				Paint paint;
				paint.color = v->text_color().value.to_color4f_alpha(_opacity);
				for (auto i: v->_blob_visible) {
					auto &blob = v->_blob[i];
					auto &line = lines->line(blob.line);
					_canvas->drawTextBlob(&blob.core, {
						line.origin + blob.origin + offset.x(), line.baseline + offset.y()
					}, size, paint);
				}
			} // if (color.a())
		}

		// draw cursor
		if (twinkle) {
			auto &line = lines->line(v->_cursor_linenum);
			auto x = offset.x() + v->_cursor_x - 1;
			auto y = offset.y() + line.baseline - v->_text_ascent - 1;
			auto &rect = _cache->getRectPath({{x, y},{2,v->_text_height+2}});
			_canvas->drawPathvColor(rect, v->cursor_color().to_color4f_alpha(_opacity), kSrcOver_BlendMode);
		}

		if (clip) {
			_canvas->restore(); // cancel clip
		}

		if ( v->is_multiline() ) {
			drawScrollBar(v, static_cast<TextareaLayout*>(v));
		}
	}

	void UIRender::visitLabel(LabelLayout* v) {
		if (v->_blob_visible.length()) {
			_canvas->setTranslate(v->position());

			auto lines = *v->_lines;
			auto size = v->text_size().value;

			// draw text  background
			if (v->text_background_color().value.a()) {
				auto color = v->text_background_color().value.to_color4f_alpha(_opacity);
				for (auto i: v->_blob_visible) {
					auto &blob = v->_blob[i];
					auto &line = lines->line(blob.line);
					auto offset_x = blob.core.offset.front().x();
					auto &rect = _cache->getRectPath({
						{line.origin + blob.origin + offset_x, line.baseline - blob.ascent},
						{blob.core.offset.back().x()-offset_x, blob.height},
					});
					_canvas->drawPathvColor(rect, color, kSrcOver_BlendMode);
				}
			}

			if (v->text_color().value.a()) {
				// draw text shadow
				auto shadow = v->text_shadow().value;
				if (shadow.size) {
					Paint paint;
					PaintFilter filter;
					filter.type = PaintFilter::kBlur_Type;
					filter.val0 = shadow.size;
					paint.filter = &filter;
					paint.color = shadow.color.to_color4f_alpha(_opacity * v->text_color().value.a());
					for (auto i: v->_blob_visible) {
						auto &blob = v->_blob[i];
						auto &line = lines->line(blob.line);
						_canvas->drawTextBlob(&blob.core, {
							line.origin + blob.origin + shadow.offset_x, line.baseline + shadow.offset_y
						}, size, paint);
					}
				}

				Paint paint;
				paint.color = v->text_color().value.to_color4f_alpha(_opacity);
				for (auto i: v->_blob_visible) {
					auto &blob = v->_blob[i];
					auto &line = lines->line(blob.line);
					_canvas->drawTextBlob(&blob.core, {line.origin + blob.origin, line.baseline}, size, paint);
				}
			} // if (v->text_color().value.a())
		}
		
		UIRender::visitLayout(v);
	}

	void UIRender::visitTransform(TransformLayout* box) {
		auto matrix = _matrix;
		auto fixOrigin = _fixOrigin;
		_fixOrigin -= box->_origin_value;
		_matrix = &box->matrix();
		_canvas->setMatrix(*_matrix);
		UIRender::visitBox(box);
		_fixOrigin = fixOrigin;
		_matrix = matrix;
	}

	void UIRender::visitRoot(RootLayout* v) {
		if (_canvas && v->_visible) {
			uint32_t mark = v->mark_value();
			if (mark) {
				v->solve_marks(Mat(), mark);
				_mark_recursive = mark & Layout::kRecursive_Mark;
			}
			if (v->_visible_region && v->_opacity != 0) {
				// Fix rect aa stroke width
				// _fix = _render->getUnitPixel() * 0.225f; // fix aa stroke width
				_fixOrigin = 2.0f * 0.225f / _window->scale(); // fix aa stroke width
				_fixSize = _fixOrigin[0] + _fixOrigin[0];
				BoxData data;
				_matrix = &v->matrix();
				_canvas->setMatrix(Mat(*_matrix).set_translate(v->position()));
				_canvas->clearColor(v->_background_color.to_color4f());
				if (v->_box_shadow)
					drawBoxShadow(v, data);
				if (v->_background)
					drawBoxFill(v, data);
				if (v->_border)
					drawBoxBorder(v, data);
				drawBoxEnd(v, data);
			} else {
				_canvas->clearColor(Color4f(0,0,0,0));
			}
			_mark_recursive = 0;
		}
	}

	void Layout::draw(UIRender *render) {
		render->visitLayout(this);
	}

	void BoxLayout::draw(UIRender *render) {
		render->visitBox(this);
	}

	void ImageLayout::draw(UIRender *render) {
		render->visitImage(this);
	}

	void ScrollLayout::draw(UIRender *render) {
		render->visitScroll(this);
	}

	void InputLayout::draw(UIRender *render) {
		render->visitInput(this);
	}

	void LabelLayout::draw(UIRender *render) {
		render->visitLabel(this);
	}

	void RootLayout::draw(UIRender *render) {
		render->visitRoot(this);
	}

	void TransformLayout::draw(UIRender *render) {
		render->visitTransform(this);
	}
	
}
