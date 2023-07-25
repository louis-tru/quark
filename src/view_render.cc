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

#include "./render/render.h"
#include "./filter.h"
#include "./view_render.h"
#include "./layout/root.h"
#include "./layout/image.h"
#include "./layout/flex.h"
#include "./layout/float.h"
#include "./layout/flow.h"
#include "./layout/text.h"
#include "./layout/button.h"
#include "./layout/input.h"
#include "./layout/textarea.h"
#include "./layout/label.h"

namespace qk {

	Qk_DEFINE_INLINE_MEMBERS(ViewRender, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<ViewRender::Inl*>(self)

		Rect insideRect(Box* box) {
			Rect rect{-box->_origin_value, box->_client_size};
			if (box->_border) {
				// auto fixAA = _render->getAAUnitPixel();
				// auto fixAA_2 = fixAA * 0.5;
				rect.origin[0] += box->_border[3].width;// - fixAA_2; // left
				rect.origin[1] += box->_border[0].width;// - fixAA_2; // top
				rect.size[0] -= (box->_border[3].width + box->_border[1].width/* - fixAA*/); // left + right
				rect.size[1] -= (box->_border[0].width + box->_border[2].width/* - fixAA*/); // top + bottom
			}
			return rect;
		}

		const RectPath* getOutsideRectPath(Box *box, const RectPath *&out) {
			if (!out)
				out = &_render->getRRectPath({-box->_origin_value, box->_client_size}, &box->_radius_left_top);
			return out;
		}

		const RectPath* getInsideRectPath(Box *box, const RectPath *&out) {
			if (!out) {
				if (box->_border) {
					auto rect = insideRect(box);
					const float radius_lessen[4] = {
						box->_border[0].width,box->_border[1].width,
						box->_border[2].width,box->_border[3].width,
					};
					out = &_render->getRRectPath(rect, &box->_radius_left_top, radius_lessen);
				} else {
					out = &_render->getRRectPath({-box->_origin_value, box->_client_size}, &box->_radius_left_top);
				}
			}
			return out;
		}

		void drawBoxColor(Box *box, const RectPath *&inside) {
			auto rect = getInsideRectPath(box, inside);
			_canvas->drawPathvColor(rect->path,
				box->_background_color.to_color4f_alpha(_opacity), kSrcOver_BlendMode
			);
			// Paint paint;
			// paint.antiAlias = false;
			// paint.color = box->_background_color.to_color4f_alpha(_opacity);
			// _canvas->drawPathv(rect->path, paint);
		}

		void drawBoxFill(Box *box, const RectPath *&inside) {
			auto rect = getInsideRectPath(box, inside);
			auto fill = box->_background;
			do {
				switch(fill->type()) {
					case Filter::M_IMAGE:// fill
						drawBoxFillImage(box, static_cast<FillImage*>(fill), rect); break;
					case Filter::M_GRADIENT_Linear: // fill
						drawBoxFillLinear(box, static_cast<FillGradientLinear*>(fill), rect); break;
					case Filter::M_GRADIENT_Radial: // fill
						drawBoxFillRadial(box, static_cast<FillGradientRadial*>(fill), rect); break;
				}
				fill = fill->next();
			} while(fill);
		}

		void drawBoxFillImage(Box *box, FillImage *fill, const RectPath *inside) {
			auto src = fill->source();
			if (!src || !src->load()) return;

			src->mark_as_texture_unsafe(_render); // mark texure

			auto pix = src->pixels().val();
			auto src_w = src->width(), src_h = src->height();
			auto cli = box->_client_size;
			auto dw = cli.x(), dh = cli.y();
			float w, h, x, y;

			if (box->_border) {
				dw -= (box->_border[3].width + box->_border[1].width); // left + right
				dh -= (box->_border[0].width + box->_border[2].width); // top + bottom
			}
			if (FillImage::compute_size(fill->size_x(), dw, w)) { // ok x
				if (!FillImage::compute_size(fill->size_y(), dh, h)) // auto y
					h = w / src_w * src_h;
			} else if (FillImage::compute_size(fill->size_y(), dh, h)) { // auto x, ok y
				w = h / src_h * src_w;
			} else { // auto x,y
				w = src_w / _display->atom_pixel();
				h = src_h / _display->atom_pixel();
			}
			x = FillImage::compute_position(fill->position_x(), dw, w) - box->_origin_value.val[0];
			y = FillImage::compute_position(fill->position_y(), dh, h) - box->_origin_value.val[1];

			if (box->_border) {
				x += box->_border[3].width; // left
				y += box->_border[0].width; // top
			}

			Paint paint;
			paint.setImage(pix, {{x,y}, {w,h}});
			paint.color.set_a(_opacity);

			switch(fill->repeat()) {
				case Repeat::REPEAT:
					paint.tileModeX = Paint::kRepeat_TileMode;
					paint.tileModeY = Paint::kRepeat_TileMode; break;
				case Repeat::REPEAT_X:
					paint.tileModeX = Paint::kRepeat_TileMode;
					paint.tileModeY = Paint::kDecal_TileMode; break;
				case Repeat::REPEAT_Y:
					paint.tileModeX = Paint::kDecal_TileMode;
					paint.tileModeY = Paint::kRepeat_TileMode; break;
				case Repeat::NO_REPEAT:
					paint.tileModeX = Paint::kDecal_TileMode;
					paint.tileModeY = Paint::kDecal_TileMode; break;
			}
			paint.filterMode = Paint::kLinear_FilterMode;
			paint.mipmapMode = Paint::kNearest_MipmapMode;

			_canvas->drawPathv(inside->path, paint);
		}

		void drawBoxFillLinear(Box *box, FillGradientLinear *fill, const RectPath *inside) {
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
			auto _rect_inside = inside->rect;
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
			Gradient g{&fill->colors(), &fill->positions(), pts[0], pts[1]};
			paint.setGradient(Paint::kLinear_GradientType, &g);

			_canvas->drawPathv(inside->path, paint);
		}

		void drawBoxFillRadial(Box *box, FillGradientRadial *fill, const RectPath *inside) {
			auto &colors = fill->colors();
			auto &pos = fill->positions();
			auto _rect_inside = inside->rect;
			Vec2 radius{_rect_inside.size.x() * 0.5f, _rect_inside.size.y() * 0.5f};
			Vec2 center = _rect_inside.origin + radius;
			Paint paint;
			paint.color.set_a(_opacity);
			Gradient g{&fill->colors(), &fill->positions(), center, radius};
			paint.setGradient(Paint::kRadial_GradientType, &g);
			_canvas->drawPathv(inside->path, paint);
		}

		void drawBoxShadow(Box *box, const RectPath *outside = nullptr) {
			auto shadow = box->box_shadow();
			do {
				if (shadow->type() == Filter::M_SHADOW) {
					// TODO ...
				}
				shadow = shadow->next();
			} while(shadow);
		}

		void drawBoxBorder(Box *box) {
			const float border[4] = {
				box->_border[0].width,box->_border[1].width,
				box->_border[2].width,box->_border[3].width,
			};
			auto outline = _render->getRRectOutlinePath(
				{-box->_origin_value, box->_client_size}, border, &box->_radius_left_top, true);

			Paint stroke;
			stroke.style = Paint::kStroke_Style;

			for (int i = 0; i < 4; i++) {
				if (border[i] > 0) { // top
					auto pv = &outline.top + i;
					if (pv->vertex.length()) {
						_canvas->drawPathvColor(*pv, box->_border[i].color.to_color4f_alpha(_opacity), kSrcOver_BlendMode);
					} else { // stroke
						stroke.color = box->_border[i].color.to_color4f_alpha(_opacity);
						stroke.width = border[i];
						_canvas->drawPath(pv->path, stroke);
					}
				}
			}
		}

		void drawBoxEnd(Box *box, const RectPath *inside = nullptr) {
			if (box->_is_clip) {
				if (box->_first) {
					getInsideRectPath(box, inside);
					_canvas->save();
					_canvas->clipPathv(inside->path, Canvas::kIntersect_ClipOp, true); // clip
					ViewRender::visitView(box);
					_canvas->restore(); // cancel clip
				}
			} else {
				ViewRender::visitView(box);
			}
		}

		void drawScrollBar(BaseScroll *v, Box *b) {
			if ( (v->_scrollbar_h || v->_scrollbar_v) && v->_scrollbar_opacity ) {
				auto width = v->_scrollbar_width;
				auto margin = v->_scrollbar_margin;
				auto origin = b->_origin_value;
				auto size = b->_client_size;
				auto color = v->scrollbar_color().to_color4f_alpha(_opacity * v->_scrollbar_opacity);

				if (b->_border) {
					origin += Vec2{b->_border[3].width,b->_border[0].width};
					size[0] -= (b->_border[3].width + b->_border[1].width); // left + right
					size[1] -= (b->_border[0].width + b->_border[2].width); // top + bottom
				}

				if ( v->_scrollbar_h ) { // draw horizontal scrollbar
					auto &rect = _render->getRRectPath({
						{-origin.x(), size.y() - width - margin - origin.y()},
						{v->_scrollbar_position_h.val[1], width}
					}, {
						width,width,width,width
					});
					_canvas->setMatrix(Mat(b->_matrix).translate_x(v->_scrollbar_position_h.val[0]));
					_canvas->drawPathvColor(rect.path, color, kSrcOver_BlendMode);
				}

				if ( v->_scrollbar_v ) { // draw vertical scrollbar
					auto &rect = _render->getRRectPath({
						{size.x() - width - margin - origin.x(), -origin.y()},
						{width, v->_scrollbar_position_v.val[1]}
					}, {
						width,width,width,width
					});
					_canvas->setMatrix(Mat(b->_matrix).translate_y(v->_scrollbar_position_v.val[0]));
					_canvas->drawPathvColor(rect.path, color, kSrcOver_BlendMode);
				}
			}
		}

	};

	// --------------------------------------------------------------------------

	ViewRender::ViewRender(Display *display)
		: _render(nullptr), _canvas(nullptr)
		, _display(display)
		, _opacity(1), _mark_recursive(0)
	{
	}

	void ViewRender::set_render(Render *render) {
		_render = render;
		_canvas = render->getCanvas();
	}

	void ViewRender::visitView(View* view) {
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

	void ViewRender::visitBox(Box* box) {
		_canvas->setMatrix(box->matrix());
		const RectPath *inside = nullptr;
		if (box->_background_color.a())
			_this->drawBoxColor(box, inside);
		if (box->_background)
			_this->drawBoxFill(box, inside);
		if (box->_box_shadow)
			_this->drawBoxShadow(box);
		if (box->_border)
			_this->drawBoxBorder(box);
		_this->drawBoxEnd(box, inside);
	}

	void ViewRender::visitImage(Image* v) {
		_canvas->setMatrix(v->matrix());

		const RectPath *inside = nullptr;
		if (v->_background_color.a())
			_this->drawBoxColor(v, inside);
		if (v->_background)
			_this->drawBoxFill(v, inside);

		auto src = v->source();
		if (src && src->load()) {
			src->mark_as_texture_unsafe(_render);
			_this->getInsideRectPath(v, inside);
			auto origin = inside->rect.origin - v->_origin_value;
			if (v->_border) {
				origin += Vec2{v->_border[3].width,v->_border[0].width};
			}
			Paint paint;
			paint.color.set_a(_opacity);
			paint.tileModeX = Paint::kDecal_TileMode;
			paint.tileModeY = Paint::kDecal_TileMode;
			paint.filterMode = Paint::kLinear_FilterMode;
			paint.mipmapMode = Paint::kNearest_MipmapMode;
			paint.setImage(src->pixels().val(), { origin, inside->rect.size });
			_canvas->drawPathv(inside->path, paint);
		}
		if (v->_box_shadow)
			_this->drawBoxShadow(v);
		if (v->_border)
			_this->drawBoxBorder(v);
		_this->drawBoxEnd(v, inside);
	}

	void ViewRender::visitVideo(Video* video) {
		//ViewRender::visitBox(video);
	}

	void ViewRender::visitScroll(Scroll* v) {
		ViewRender::visitBox(v);
		_this->drawScrollBar(v,v);
	}

	void ViewRender::visitInput(Input* v) {
		_canvas->setMatrix(v->matrix());
		const RectPath *inside = nullptr;
		if (v->_background_color.a())
			_this->drawBoxColor(v, inside);
		if (v->_background)
			_this->drawBoxFill(v, inside);
		if (v->_box_shadow)
			_this->drawBoxShadow(v);
		if (v->_border)
			_this->drawBoxBorder(v);

		auto lines = *v->_lines;
		auto offset = v->input_text_offset() + Vec2(v->_padding_left, v->_padding_top);
		auto twinkle = v->_editing && v->_cursor_twinkle_status;
		auto visible = v->_blob_visible.length() > 0;
		auto clip = v->_is_clip && (visible || twinkle);

		if (clip) {
			_this->getInsideRectPath(v, inside);
			_canvas->save();
			_canvas->clipPathv(inside->path, Canvas::kIntersect_ClipOp, true); // clip
		}

		if (visible) {
			auto draw_background = [&](TextBlob &blob, const Color4f &color) {
				auto &line = lines->line(blob.line);
				auto x = offset.x() + line.origin + blob.origin;
				auto y = offset.y() + line.baseline - blob.ascent;
				auto offset_x = blob.core.offset.front().x();
				auto &rect = _render->getRectPath({{x + offset_x, y},{blob.core.offset.back().x(), blob.height}});
				_canvas->drawPathvColor(rect.path, color, kSrcOver_BlendMode);
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

			auto color = v->_text_value_u4.length() ? v->text_color().value: v->placeholder_color();
			if (color.a()) {
				auto size = v->text_size().value;
				// TODO draw text shadow..
				Paint paint;
				paint.color = v->text_color().value.to_color4f_alpha(_opacity);
				for (auto i: v->_blob_visible) {
					auto &blob = v->_blob[i];
					auto &line = lines->line(blob.line);
					_canvas->drawTextBlob(&blob.core, {line.origin + blob.origin, line.baseline}, size, paint);
				}
			} // if (color.a())
		}

		// draw cursor
		if (twinkle) {
			auto &line = lines->line(v->_cursor_linenum);
			auto x = offset.x() + v->_cursor_x - 1;
			auto y = offset.y() + line.baseline - v->_text_ascent - 1;
			auto &rect = _render->getRectPath({{x, y},{2,v->_text_height+2}});
			_canvas->drawPathvColor(rect.path, v->cursor_color().to_color4f_alpha(_opacity), kSrcOver_BlendMode);
		}

		if (clip) {
			_canvas->restore(); // cancel clip
		}

		if ( v->is_multiline() ) {
			_this->drawScrollBar(static_cast<Textarea*>(v), v);
		}
	}

	void ViewRender::visitLabel(Label* v) {
		if (v->_blob_visible.length()) {
			_canvas->setMatrix(v->matrix());

			auto lines = *v->_lines;
			auto size = v->text_size().value;

			// draw text  background
			if (v->text_background_color().value.a()) {
				auto color = v->text_background_color().value.to_color4f_alpha(_opacity);
				for (auto i: v->_blob_visible) {
					auto &blob = v->_blob[i];
					auto &line = lines->line(blob.line);
					auto offset_x = blob.core.offset.front().x();
					auto &rect = _render->getRectPath({
						{line.origin + blob.origin + offset_x, line.baseline - blob.ascent},
						{blob.core.offset.back().x()-offset_x, blob.height},
					});
					_canvas->drawPathvColor(rect.path, color, kSrcOver_BlendMode);
				}
			}
			
			if (v->text_color().value.a()) {
				// TODO draw text shadow..
				Paint paint;
				paint.color = v->text_color().value.to_color4f_alpha(_opacity);
				for (auto i: v->_blob_visible) {
					auto &blob = v->_blob[i];
					auto &line = lines->line(blob.line);
					_canvas->drawTextBlob(&blob.core, {line.origin + blob.origin, line.baseline}, size, paint);
				}
			} // if (v->text_color().value.a())
		}
		
		ViewRender::visitView(v);
	}

	void ViewRender::visitTextarea(Textarea* textarea) {
		ViewRender::visitInput(textarea);
	}

	void ViewRender::visitButton(Button* btn) {
		ViewRender::visitBox(btn);
	}

	void ViewRender::visitTextLayout(TextLayout* text) {
		ViewRender::visitBox(text);
	}

	void ViewRender::visitFloatLayout(FloatLayout* box) {
		ViewRender::visitBox(box);
	}

	void ViewRender::visitFlowLayout(FlowLayout* flow) {
		ViewRender::visitBox(flow);
	}

	void ViewRender::visitFlexLayout(FlexLayout* flex) {
		ViewRender::visitBox(flex);
	}

	void ViewRender::visitRoot(Root* v) {
		if (v->_visible) {
			uint32_t mark = v->layout_mark();
			if (mark) {
				v->solve_marks(mark);
				_mark_recursive = mark & Layout::kRecursive_Mark;
			}
			if (v->_visible_region && v->_opacity != 0) {
				_canvas->setMatrix(v->matrix());
				_canvas->clearColor(v->_background_color.to_color4f());
				const RectPath *inside = nullptr;
				if (v->_background)
					_this->drawBoxFill(v, inside);
				if (v->_box_shadow)
					_this->drawBoxShadow(v, inside);
				if (v->_border)
					_this->drawBoxBorder(v);
				_this->drawBoxEnd(v);
			} else {
				_canvas->clearColor(Color4f(0,0,0,0));
			}
			_mark_recursive = 0;
		}
	}

}
