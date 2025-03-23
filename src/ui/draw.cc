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
#include "./draw.h"
#include "./view/root.h"
#include "./view/image.h"
#include "./view/flex.h"
#include "./view/free.h"
#include "./view/flow.h"
#include "./view/text.h"
#include "./view/input.h"
#include "./view/textarea.h"
#include "./view/label.h"
#include "./view/matrix.h"

#define _Border(v) auto _border = v->_border.load()
#define _IfBorder(v) _Border(v); if (_border)
#define _IfNotBorder(v) _Border(v); if (!_border) return

namespace qk {

	typedef UIDraw::BoxData BoxData;

	UIDraw::UIDraw(Window *window)
		: _render(window->render()), _canvas(nullptr)
		, _cache(nullptr)
		, _window(window)
		, _opacity(1), _mark_recursive(0), _matrix(nullptr)
	{
		_canvas = _render->getCanvas();
		_cache = _canvas->getPathvCache();
	}

	Rect UIDraw::getRect(Box* box) {
		return {
			_origin, {box->_client_size[0]-_fixSize,box->_client_size[1]-_fixSize},
		};
	}

	void UIDraw::getInsideRectPath(Box *box, BoxData &out) {
		if (out.inside)
			return;
		_IfBorder(box) {
			auto rect = getRect(box);
			auto radius = &box->_border_radius_left_top;
			auto border = _border->width;
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

				//Qk_DLog("getInsideRectPath have border");

				if (*reinterpret_cast<const uint64_t*>(radius) == 0 &&
						*reinterpret_cast<const uint64_t*>(radius+2) == 0
				) {
					out.inside = &_cache->setRRectPathFromHash(hash.hashCode(), RectPath::MakeRect(rect));
				} else {
					float lt = Qk_Min(radius[0],xy_0_5), rt = Qk_Min(radius[1],xy_0_5);
					float rb = Qk_Min(radius[2],xy_0_5), lb = Qk_Min(radius[3],xy_0_5);
					auto border = _border->width;
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

	void UIDraw::getOutsideRectPath(Box *box, BoxData &out) {
		if (!out.outside)
			out.outside = &_cache->getRRectPath(getRect(box), &box->_border_radius_left_top);
	}

	void UIDraw::getRRectOutlinePath(Box *box, BoxData &out) {
		if (!out.outline) {
			_Border(box);
			auto border = _border->width;
			float borderFix[4] = {
				Float32::max(0, border[0]-_fixSize), Float32::max(0, border[1]-_fixSize),
				Float32::max(0, border[2]-_fixSize), Float32::max(0, border[3]-_fixSize),
			};
			out.outline = &_cache->getRRectOutlinePath(getRect(box), borderFix, &box->_border_radius_left_top);
		}
	}

	void UIDraw::drawBoxColor(Box *box, BoxData &data) {
		if (!box->_background_color.a()) return;
		getInsideRectPath(box, data);
		_canvas->drawPathvColor(*data.inside,
			box->_background_color.to_color4f_alpha(_opacity), kSrcOver_BlendMode
		);
		// Paint paint;
		// paint.antiAlias = false;
		// paint.color = box->_background_color.to_color4f_alpha(_opacity);
		// _canvas->drawPathv(rect->path, paint);
	}

	void UIDraw::drawBoxFill(Box *box, BoxData &data) {
		auto filter = box->background();
		if (!filter) return;
		getInsideRectPath(box, data);
		do {
			switch(filter->type()) {
				case BoxFilter::kImage:// fill
					drawBoxFillImage(box, static_cast<FillImage*>(filter), data); break;
				case BoxFilter::kGradientLinear: // fill
					drawBoxFillLinear(box, static_cast<FillGradientLinear*>(filter), data); break;
				case BoxFilter::kGradientRadial: // fill
					drawBoxFillRadial(box, static_cast<FillGradientRadial*>(filter), data); break;
				default: break;
			}
			filter = filter->next();
		} while(filter);
	}

	void UIDraw::drawBoxFillImage(Box *box, FillImage *fill, BoxData &data) {
		auto src = fill->source();
		if (!src || !src->load()) return;

		auto src_w = src->width(), src_h = src->height();
		auto cli = box->_client_size;
		auto dw = cli.x(), dh = cli.y();
		float w, h, x, y;

		_IfBorder(box) {
			dw -= (_border->width[3] + _border->width[1]); // left + right
			dh -= (_border->width[0] + _border->width[2]); // top + bottom
		}
		if (FillImage::compute_size(fill->width(), dw, w)) { // ok x
			if (!FillImage::compute_size(fill->height(), dh, h)) // auto y
				h = w / src_w * src_h;
		} else if (FillImage::compute_size(fill->height(), dh, h)) { // auto x, ok y
			w = h / src_h * src_w;
		} else { // auto x,y
			w = src_w / _window->atomPixel();
			h = src_h / _window->atomPixel();
		}
		x = FillImage::compute_position(fill->x(), dw, w);
		y = FillImage::compute_position(fill->y(), dh, h);

		if (_border) {
			x += _border->width[3]; // left
			y += _border->width[0]; // top
		}

		Paint paint0;
		ImagePaint paint;
		paint.setImage(src.get(), {{x,y}, {w,h}});
		paint0.image = &paint;
		paint0.color.set_a(_opacity);

		switch(fill->repeat()) {
			case Repeat::Repeat:
				paint.tileModeX = ImagePaint::kRepeat_TileMode;
				paint.tileModeY = ImagePaint::kRepeat_TileMode; break;
			case Repeat::RepeatX:
				paint.tileModeX = ImagePaint::kRepeat_TileMode;
				paint.tileModeY = ImagePaint::kDecal_TileMode; break;
			case Repeat::RepeatY:
				paint.tileModeX = ImagePaint::kDecal_TileMode;
				paint.tileModeY = ImagePaint::kRepeat_TileMode; break;
			case Repeat::RepeatNo:
				paint.tileModeX = ImagePaint::kDecal_TileMode;
				paint.tileModeY = ImagePaint::kDecal_TileMode; break;
		}
		paint.filterMode = ImagePaint::kLinear_FilterMode;
		paint.mipmapMode = ImagePaint::kNearest_MipmapMode;

		_canvas->drawPathv(*data.inside, paint0);
	}

	void UIDraw::drawBoxFillLinear(Box *box, FillGradientLinear *fill, BoxData &data) {
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

		//Qk_DLog("%f, %f, %d", R * Qk_180_RATIO_PI, gradient->angle(), quadrant);

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

	void UIDraw::drawBoxFillRadial(Box *box, FillGradientRadial *fill, BoxData &data) {
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

	void UIDraw::drawBoxShadow(Box *box, BoxData &data) {
		auto shadow = box->box_shadow();
		if (!shadow) return;
		getOutsideRectPath(box, data);
		_canvas->save();
		_canvas->clipPathv(*data.outside, Canvas::kDifference_ClipOp, false);
		do {
			if (shadow->type() != BoxFilter::kShadow)
				break;
			auto s = shadow->value();
			auto &o = data.outside->rect.origin;
			_canvas->drawRRectBlurColor({
				{o.x()+s.x, o.y()+s.y}, data.outside->rect.size,
			},&box->_border_radius_left_top, s.size, s.color.to_color4f_alpha(_opacity), kSrcOver_BlendMode);
			shadow = static_cast<BoxShadow*>(shadow->next());
		} while(shadow);
		_canvas->restore();
	}

	void UIDraw::drawBoxBorder(Box *box, BoxData &data) {
		_IfNotBorder(box);

		getRRectOutlinePath(box, data);
		Paint stroke;
		stroke.style = Paint::kStroke_Style;

		for (int i = 0; i < 4; i++) {
			if (_border->width[i] > 0) { // top
				auto pv = &data.outline->top + i;
				if (pv->vCount) {
					_canvas->drawPathvColor(*pv, _border->color[i].to_color4f_alpha(_opacity), kSrcOver_BlendMode);
				} else { // stroke
					stroke.color = _border->color[i].to_color4f_alpha(_opacity);
					stroke.width = _border->width[i];
					_canvas->drawPath(pv->path, stroke);
				}
			}
		}
	}

	void UIDraw::drawBoxEnd(Box *box, BoxData &data) {
		if (box->_clip) {
			if (box->_first.load()) {
				getInsideRectPath(box, data);
				_window->clipRegion(screen_region_from_convex_quadrilateral(box->_vertex));
				_canvas->save();
				_canvas->clipPathv(*data.inside, Canvas::kIntersect_ClipOp, true); // clip
				UIDraw::visitView(box);
				_canvas->restore(); // cancel clip
				_window->clipRestore();
			}
		} else {
			UIDraw::visitView(box);
		}
	}

	void UIDraw::drawScrollBar(Box *b, ScrollBase *v) {
		if ( (v->_scrollbar_h || v->_scrollbar_v) && v->_scrollbar_opacity ) {
			auto width = v->_scrollbar_width;
			auto margin = v->_scrollbar_margin;
			auto origin = Vec2();// Vec2{b->margin_left(),b->margin_top()};
			auto size = b->_client_size;
			auto color = v->scrollbar_color().to_color4f_alpha(_opacity * v->_scrollbar_opacity);
			auto _border = b->_border.load();

			if ( v->_scrollbar_h ) { // draw horizontal scrollbar
				float radius[] = {width,width,width,width};
				Rect rect = {
					{v->_scrollbar_position_h[0], size[1] - width - margin},
					{v->_scrollbar_position_h[1], width}
				};
				if (_border) {
					rect.origin += {_border->width[3], -_border->width[0]};
				}
				_canvas->drawPathvColor(_cache->getRRectPath(rect, radius), color, kSrcOver_BlendMode);
			}

			if ( v->_scrollbar_v ) { // draw vertical scrollbar
				float radius[] = {width,width,width,width};
				Rect rect = {
					{size[0] - width - margin, v->_scrollbar_position_v[0]},
					{width,                    v->_scrollbar_position_v[1]}
				};
				if (_border) {
					rect.origin += {-_border->width[3], _border->width[0]};
				}
				_canvas->drawPathvColor(_cache->getRRectPath(rect, radius), color, kSrcOver_BlendMode);
			}
		}
	}

	void UIDraw::drawTextBlob(TextOptions *v, TextLines *lines, Array<TextBlob> &_blob, Array<uint32_t> &blob_visible) {
		auto size = v->text_size().value;
		auto shadow = v->text_shadow().value;

		// draw text  background
		if (v->text_background_color().value.a()) {
			auto color = v->text_background_color().value.to_color4f_alpha(_opacity);
			for (auto i: blob_visible) {
				auto &blob = _blob[i];
				auto &line = lines->line(blob.line);
				auto offset_x = blob.blob.offset.front().x();
				auto &rect = _cache->getRectPath({
					{line.origin + blob.origin + offset_x, line.baseline - blob.ascent},
					{blob.blob.offset.back().x()-offset_x, blob.height},
				});
				_canvas->drawPathvColor(rect, color, kSrcOver_BlendMode);
			}
		}

		// draw text shadow
		if (shadow.color.a()) {
			Paint paint;
			PaintFilter filter;
			paint.color = shadow.color.to_color4f_alpha(_opacity);
			if (shadow.size) {
				filter.type = PaintFilter::kBlur_Type;
				filter.val0 = shadow.size;
				paint.filter = &filter;
			}
			for (auto i: blob_visible) {
				auto &blob = _blob[i];
				auto &line = lines->line(blob.line);
				_canvas->drawTextBlob(&blob.blob, {
					line.origin + blob.origin + shadow.x, line.baseline + shadow.y
				}, size, paint);
			}
		}

		// draw text blob
		if (v->text_color().value.a()) {
			Paint paint;
			paint.color = v->text_color().value.to_color4f_alpha(_opacity);
			for (auto i: blob_visible) {
				auto &blob = _blob[i];
				auto &line = lines->line(blob.line);
				_canvas->drawTextBlob(&blob.blob, {line.origin + blob.origin, line.baseline}, size, paint);
			}
		} // if (v->text_color().value.a())
	}

	void UIDraw::visitView(View *view) {
		// visit child
		auto v = view->_first.load();
		if (v) {
			auto opacity = _opacity;
			auto markCurr = _mark_recursive;
			do {
				if (v->_visible) {
					uint32_t mark = markCurr | v->mark_value(); // inherit recursive
					if (mark) {
						v->solve_marks(*_matrix, mark);
						_mark_recursive = mark & View::kRecursive_Mark;
					}
					if (v->_opacity) {
						_opacity = opacity * v->_opacity;
						v->draw(this);
					}
				}
				v = v->_next.load();
			} while(v);
			_opacity = opacity;
			_mark_recursive = markCurr;
		}
	}

	void UIDraw::visitBox(Box* box) {
		BoxData data;
		_canvas->setTranslate(box->position());
		drawBoxShadow(box, data);
		drawBoxColor(box, data);
		drawBoxFill(box, data);
		drawBoxBorder(box, data);
		drawBoxEnd(box, data);
	}

	void UIDraw::visitImage(Image* v) {
		BoxData data;
		_canvas->setTranslate(v->position());
		drawBoxShadow(v, data);
		drawBoxColor(v, data);
		drawBoxFill(v, data);

		auto src = v->source();
		if (src && src->load()) {
			getInsideRectPath(v, data);
			//auto cli = v->client_size();
			//Qk_DLog("--- w %f, h %f, s: %f", cli.x(), cli.y(), cli.x()/cli.y());
			Paint p0;
			ImagePaint paint;
			// p0.antiAlias = false;
			p0.type = Paint::kBitmap_Type;
			p0.image = &paint;
			p0.color.set_a(_opacity);
			//paint.tileModeX = ImagePaint::kDecal_TileMode;
			//paint.tileModeY = ImagePaint::kDecal_TileMode;
			paint.filterMode = ImagePaint::kLinear_FilterMode;
			paint.mipmapMode = ImagePaint::kNearest_MipmapMode;
			paint.setImage(src.get(), data.inside->rect);
			_canvas->drawPathv(*data.inside, p0);
		}
		drawBoxBorder(v, data);
		drawBoxEnd(v, data);
	}

	void UIDraw::visitScroll(Scroll* v) {
		UIDraw::visitBox(v);
		drawScrollBar(v,v);
	}

	void UIDraw::visitText(Text* v) {
		BoxData data;
		_canvas->setTranslate(v->position());
		drawBoxShadow(v, data);
		drawBoxColor(v, data);
		drawBoxFill(v, data);
		drawBoxBorder(v, data);

		if (v->_clip) {
			if (v->_first.load() || v->_blob_visible.length()) {
				getInsideRectPath(v, data);
				_window->clipRegion(screen_region_from_convex_quadrilateral(v->_vertex));
				_canvas->save();
				_canvas->clipPathv(*data.inside, Canvas::kIntersect_ClipOp, true); // clip
				if (v->_blob_visible.length()) {
					drawTextBlob(v, *v->_lines, v->_blob, v->_blob_visible);
				}
				UIDraw::visitView(v);
				_canvas->restore(); // cancel clip
				_window->clipRestore();
			}
		} else {
			if (v->_blob_visible.length()) {
				drawTextBlob(v, *v->_lines, v->_blob, v->_blob_visible);
			}
			UIDraw::visitView(v);
		}
	}

	void UIDraw::visitInput(Input* v) {
		BoxData data;
		_canvas->setTranslate(v->position());
		drawBoxShadow(v, data);
		drawBoxColor(v, data);
		drawBoxFill(v, data);
		drawBoxBorder(v, data);

		auto lines = *v->_lines;
		auto offset = v->input_text_offset() + Vec2(v->_padding_left, v->_padding_top);
		auto twinkle = v->_editing && v->_cursor_twinkle_status;
		auto visible = v->_blob_visible.length();
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
				auto offset_x = blob.blob.offset.front().x();
				auto width = blob.blob.offset.back().x();
				auto &rect = _cache->getRectPath({{x + offset_x, y},{width, blob.height}});
				_canvas->drawPathvColor(rect, color, kSrcOver_BlendMode);
			};
			auto size = v->text_size().value;
			auto shadow = v->text_shadow().value;

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

			// draw text shadow
			if (shadow.color.a()) {
				Paint paint;
				PaintFilter filter;
				paint.color = shadow.color.to_color4f_alpha(_opacity);
				if (shadow.size) {
					filter.type = PaintFilter::kBlur_Type;
					filter.val0 = shadow.size;
					paint.filter = &filter;
				}
				for (auto i: v->_blob_visible) {
					auto &blob = v->_blob[i];
					auto &line = lines->line(blob.line);
					_canvas->drawTextBlob(&blob.blob, {
						line.origin + shadow.x + offset.x() + blob.origin,
						line.baseline + shadow.y + offset.y()
					}, size, paint);
				}
			}

			// draw text blob
			auto color = v->_value_u4.length() ? v->text_color().value: v->placeholder_color();
			if (color.a()) {
				Paint paint;
				paint.color = color.to_color4f_alpha(_opacity);
				for (auto i: v->_blob_visible) {
					auto &blob = v->_blob[i];
					auto &line = lines->line(blob.line);
					_canvas->drawTextBlob(&blob.blob, {
						line.origin + blob.origin + offset.x(), line.baseline + offset.y()
					}, size, paint);
				}
			} // if (color.a())
		}

		// draw cursor
		if (twinkle) {
			auto &line = lines->line(v->_cursor_line);
			auto x = Float32::max(0, offset.x() + v->_cursor_x - 1.0f);
			//auto y = offset.y() + line.baseline - v->_text_ascent;
			auto y = offset.y() + (line.end_y + line.start_y - v->_cursor_height) * 0.5f;
			auto &rect = _cache->getRectPath({{x, y},{2.0,v->_cursor_height}});
			_canvas->drawPathvColor(rect, v->cursor_color().to_color4f_alpha(_opacity), kSrcOver_BlendMode);
		}

		if (clip) {
			_canvas->restore(); // cancel clip
		}

		if ( v->is_multiline() ) {
			drawScrollBar(v, static_cast<Textarea*>(v));
		}
	}

	void UIDraw::visitLabel(Label* v) {
		if (v->_blob_visible.length()) {
			_canvas->setTranslate(v->position());
			drawTextBlob(v, *v->_lines, v->_blob, v->_blob_visible);
		}
		UIDraw::visitView(v);
	}

	void UIDraw::visitMatrix(Matrix* box) {
		auto matrix = _matrix;
		auto origin = _origin;
		_origin -= box->_origin_value;
		_matrix = &box->mat();
		_canvas->setMatrix(*_matrix);
		// draw box
		BoxData data;
		_canvas->setTranslate(box->position());

		drawBoxShadow(box, data);
		drawBoxColor(box, data);
		drawBoxFill(box, data);
		drawBoxBorder(box, data);
		_origin = origin;
		drawBoxEnd(box, data);
		// draw box end
		_matrix = matrix;
		_canvas->setMatrix(*_matrix);
	}

	void UIDraw::visitRoot(Root* v) {
		if (_canvas && v->_visible) {
			uint32_t mark = v->mark_value();
			if (mark) {
				v->solve_marks(Mat(), mark);
				_mark_recursive = mark & View::kRecursive_Mark;
			}
			if (v->_visible_region && v->_opacity != 0) {
				BoxData data;
				// Fix rect aa stroke width
				auto origin = 2.0f * 0.225f / _window->scale(); // fix aa stroke width
				_fixSize = origin + origin;
				_origin = Vec2(origin) - v->_origin_value;
				_matrix = &v->mat();
				_canvas->setMatrix(Mat(*_matrix).set_translate(v->position()));
				_canvas->clearColor(v->_background_color.to_color4f());
				// _canvas->clearColor(Color4f(1,0,0,1));

				drawBoxShadow(v, data);
				drawBoxFill(v, data);
				drawBoxBorder(v, data);
				_origin = origin;
				drawBoxEnd(v, data);
			} else {
				_canvas->clearColor(Color4f(0,0,0,0));
			}
			_mark_recursive = 0;
		}
	}

	void View::draw(UIDraw *draw) {
		if (_visible_region) {
			draw->visitView(this);
		}
	}

	void Box::draw(UIDraw *draw) {
		if (_visible_region) {
			draw->visitBox(this);
		}
	}

	void Image::draw(UIDraw *draw) {
		if (_visible_region) {
			draw->visitImage(this);
		}
	}

	void Scroll::draw(UIDraw *draw) {
		if (_visible_region) {
			draw->visitScroll(this);
		}
	}

	void Text::draw(UIDraw *draw) {
		if (_visible_region) {
			draw->visitText(this);
		}
	}

	void Input::draw(UIDraw *draw) {
		if (_visible_region) {
			draw->visitInput(this);
		}
	}

	void Label::draw(UIDraw *draw) {
		draw->visitLabel(this);
	}

	void Matrix::draw(UIDraw *draw) {
		if (_visible_region) {
			draw->visitMatrix(this);
		}
	}

	void Root::draw(UIDraw *draw) {
		draw->visitRoot(this);
	}
}
