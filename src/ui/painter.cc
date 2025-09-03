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
#include "./painter.h"
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
#include "./view/sprite.h"
#include "./view/spine.h"

#define _Border(v) auto _border = v->_border.load()
#define _IfBorder(v) _Border(v); if (_border)
#define _IfNotBorder(v) _Border(v); if (!_border) return

namespace qk {

	typedef Painter::BoxData BoxData;

	Painter::Painter(Window *window)
		: _render(window->render()), _canvas(nullptr)
		, _cache(nullptr)
		, _window(window)
		, _opacity(1), _mark_recursive(0), _matrix(nullptr)
	{
		_canvas = _render->getCanvas();
		_cache = _canvas->getPathvCache();
	}

	Rect Painter::getRect(Box* box) {
		return {
			_origin, {box->_client_size[0]-_AAShrink,box->_client_size[1]-_AAShrink},
		};
	}

	void Painter::getInsideRectPath(Box *box, BoxData &out) {
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
				// TODO: The interior and the border cannot fit completely,
				// causing anti-aliasing to fail at 1 to 2 pixels,
				// so temporarily reduce the interior frame to 0.75
				float AAShrink = _AAShrink * 0.75;
				float borderFix[4] = {
					Float32::max(0, border[0]-AAShrink), Float32::max(0, border[1]-AAShrink),
					Float32::max(0, border[2]-AAShrink), Float32::max(0, border[3]-AAShrink),
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

	void Painter::getOutsideRectPath(Box *box, BoxData &out) {
		if (!out.outside)
			out.outside = &_cache->getRRectPath(getRect(box), &box->_border_radius_left_top);
	}

	void Painter::getRRectOutlinePath(Box *box, BoxData &out) {
		if (!out.outline) {
			_Border(box);
			auto border = _border->width;
			if (*reinterpret_cast<const uint64_t*>(border) != 0 ||
					*reinterpret_cast<const uint64_t*>(border+2) != 0
			) {
				float borderFix[4] = {
					Float32::max(0, border[0]-_AAShrink), Float32::max(0, border[1]-_AAShrink),
					Float32::max(0, border[2]-_AAShrink), Float32::max(0, border[3]-_AAShrink),
				};
				out.outline = &_cache->getRRectOutlinePath(getRect(box), borderFix, &box->_border_radius_left_top);
			}
		}
	}

	void Painter::drawBoxBasic(Box *box, BoxData &data) {
		drawBoxShadow(box, data);

		if (_opacity == 1) {
			drawBoxColor(box, data);
			drawBoxFill(box, data);
			drawBoxBorder(box, data);
			return;
		}

		struct PathvItems {
			int total = 0;
			int items[5];
			struct Item {
				const Pathv* pathv[5];
				Color color;
				int count = 0;
			} indexed[5];
		} pathvs;

		auto addItem = [](PathvItems &out, Color color, const qk::Pathv *pv) {
			auto key = *reinterpret_cast<uint32_t*>(&color) % 5;
			auto &it = out.indexed[key];
			if (it.count == 0) {
				it.color = color;
				out.items[out.total++] = key;
			}
			it.pathv[it.count++] = pv;
		};

		if (box->_background_color.a()) {
			getInsideRectPath(box, data);
			addItem(pathvs, box->_background_color, data.inside);
		}

		_IfBorder(box) {
			getRRectOutlinePath(box, data);
			Paint stroke;
			stroke.style = Paint::kStroke_Style;
			if (data.outline) {
				for (int i = 0; i < 4; i++) {
					if (_border->width[i]) { // top
						auto pv = &data.outline->top + i;
						if (pv->vCount) {
							addItem(pathvs, _border->color[i], pv);
						} else { // stroke
							stroke.color = _border->color[i].to_color4f_alpha(_opacity);
							stroke.width = _border->width[i];
							_canvas->drawPath(pv->path, stroke);
						}
					}
				}
			}
		}

		if (pathvs.total) {
			for (int i = 0; i < pathvs.total; i++) {
				auto & it = pathvs.indexed[pathvs.items[i]];
				_canvas->drawPathvColors(it.pathv, it.count,
					it.color.to_color4f_alpha(_opacity), kSrcOver_BlendMode);
			}
		}

		drawBoxFill(box, data);
	}

	void Painter::drawBoxFill(Box *box, BoxData &data) {
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

	void Painter::drawBoxFillImage(Box *box, FillImage *fill, BoxData &data) {
		auto src = fill->source();
		if (!src || !src->load())
			return;

		src->markAsTexture();

		auto src_w = src->width(), src_h = src->height();
		auto cli = box->_client_size;
		auto h_w = cli.x(), h_h = cli.y();
		float w, h, x, y;

		_IfBorder(box) {
			h_w -= (_border->width[3] + _border->width[1]); // left + right
			h_h -= (_border->width[0] + _border->width[2]); // top + bottom
		}
		if (FillImage::compute_size(fill->width(), h_w, w)) { // ok x
			if (!FillImage::compute_size(fill->height(), h_h, h)) // auto y
				h = w / src_w * src_h;
		} else if (FillImage::compute_size(fill->height(), h_h, h)) { // auto x, ok y
			w = h / src_h * src_w;
		} else { // auto x,y
			w = src_w * _window->atomPixel();
			h = src_h * _window->atomPixel();
		}
		x = FillImage::compute_position(fill->x(), h_w, w) + _origin[0];
		y = FillImage::compute_position(fill->y(), h_h, h) + _origin[1];

		if (_border) {
			x += _border->width[3]; // left
			y += _border->width[0]; // top
		}

		Paint paint;
		ImagePaint img;
		paint.type = Paint::kBitmap_Type;
		paint.image = &img;
		paint.color.set_a(_opacity);

		switch(fill->repeat()) {
			case Repeat::Repeat:
				img.tileModeX = ImagePaint::kRepeat_TileMode;
				img.tileModeY = ImagePaint::kRepeat_TileMode; break;
			case Repeat::RepeatX:
				img.tileModeX = ImagePaint::kRepeat_TileMode;
				img.tileModeY = ImagePaint::kDecal_TileMode; break;
			case Repeat::RepeatY:
				img.tileModeX = ImagePaint::kDecal_TileMode;
				img.tileModeY = ImagePaint::kRepeat_TileMode; break;
			case Repeat::RepeatNo:
				img.tileModeX = ImagePaint::kDecal_TileMode;
				img.tileModeY = ImagePaint::kDecal_TileMode; break;
		}
		img.filterMode = ImagePaint::kLinear_FilterMode;
		img.mipmapMode = ImagePaint::kNearest_MipmapMode;

		img.setImage(src.get(), {{x,y}, {w,h}});

		_canvas->drawPathv(*data.inside, paint);
	}

	void Painter::drawBoxFillLinear(Box *box, FillGradientLinear *fill, BoxData &data) {
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
			GradientPaint::kLinear_Type, pts[0], pts[1],
			(uint32_t)fill->colors().length(),
			fill->colors().val(), fill->positions().val()
		};
		paint.gradient = &g;
		paint.type = Paint::kGradient_Type;

		_canvas->drawPathv(*data.inside, paint);
	}

	void Painter::drawBoxFillRadial(Box *box, FillGradientRadial *fill, BoxData &data) {
		auto &colors = fill->colors();
		auto &pos = fill->positions();
		auto _rect_inside = data.inside->rect;
		Vec2 radius{_rect_inside.size.x() * 0.5f, _rect_inside.size.y() * 0.5f};
		Vec2 center = _rect_inside.origin + radius;
		Paint paint;
		paint.color.set_a(_opacity);
		GradientPaint g{
			GradientPaint::kRadial_Type, center, radius,
			(uint32_t)fill->colors().length(),
			fill->colors().val(), fill->positions().val()
		};
		paint.gradient = &g;
		paint.type = Paint::kGradient_Type;
		_canvas->drawPathv(*data.inside, paint);
	}

	void Painter::drawBoxShadow(Box *box, BoxData &data) {
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

	void Painter::drawBoxColor(Box *box, BoxData &data) {
		if (!box->_background_color.a())
			return;
		getInsideRectPath(box, data);
		_canvas->drawPathvColor(*data.inside,
			box->_background_color.to_color4f_alpha(_opacity), kSrcOver_BlendMode
		);
		// Paint paint;
		// paint.antiAlias = false;
		// paint.color = box->_background_color.to_color4f_alpha(_opacity);
		// _canvas->drawPathv(rect->path, paint);
	}

	void Painter::drawBoxBorder(Box *box, BoxData &data) {
		_IfNotBorder(box);
		getRRectOutlinePath(box, data);
		if (data.outline) {
			Paint stroke;
			stroke.style = Paint::kStroke_Style;
			for (int i = 0; i < 4; i++) {
				if (_border->width[i]) { // top
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
	}

	void Painter::drawBoxEnd(Box *box, BoxData &data) {
		if (box->_clip) {
			if (box->_first.load()) {
				getInsideRectPath(box, data);
				_window->clipRegion(screen_region_from_convex_quadrilateral(box->_vertex));
				_canvas->save();
				_canvas->clipPathv(*data.inside, Canvas::kIntersect_ClipOp, true); // clip
				visitView(box);
				_canvas->restore(); // cancel clip
				_window->clipRestore();
			}
		} else {
			visitView(box);
		}
	}

	void Painter::drawScrollBar(ScrollView *v) {
		if ( (v->_scrollbar_h || v->_scrollbar_v) && v->_scrollbar_opacity ) {
			auto b = v->host();
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

	void Painter::drawTextBlob(TextOptions *v, Vec2 inOffset,
		TextLines *lines, Array<TextBlob> &_blob, Array<uint32_t> &blob_visible) 
	{
		auto size = v->text_size().value;
		auto shadow = v->text_shadow().value;

		// draw text  background
		if (v->text_background_color().value.a()) {
			auto color = v->text_background_color().value.to_color4f_alpha(_opacity);
			for (auto i: blob_visible) {
				auto &blob = _blob[i];
				auto &line = lines->line(blob.line);
				auto offset_x = blob.blob.offset.front().x() + inOffset.x();
				auto &rect = _cache->getRectPath({
					{
						line.origin + blob.origin + offset_x,
						line.baseline - blob.ascent + inOffset.y()
					},
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
			shadow.x += inOffset.x();
			shadow.y += inOffset.y();
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
				_canvas->drawTextBlob(&blob.blob, {
					line.origin + blob.origin + inOffset.x(),
					line.baseline + inOffset.y()
				}, size, paint);
			}
			// Qk_DLog("Color: %f, %f, %f", paint.color.r(), paint.color.g(), paint.color.b());
		} // if (v->text_color().value.a())
	}

	//////////////////////////////////////////////////////////
	void Painter::visitView(View *view) {
		auto v = view->_first.load();
		if (v) {
			auto opacity = _opacity;
			auto markCurr = _mark_recursive;
			do {
				if (v->_visible) {
					uint32_t mark = markCurr | v->mark_value(); // inherit recursive
					if (mark) {
						v->solve_marks(*_matrix, view, mark);
						_mark_recursive = mark & View::kRecursive_Mark;
					}
					if (v->_visible_region && v->_opacity) {
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

	void Painter::visitView(View* v, const Mat &mat) {
		if (v->_first.load()) {
			auto matrix = _matrix;
			_matrix = &mat;
			_canvas->setMatrix(mat);
			visitView(v);
			_matrix = matrix;
			_canvas->setMatrix(*matrix);
		}
	}

	void View::draw(Painter *draw) {
		draw->visitView(this);
	}

	void Box::draw(Painter *draw) {
		BoxData data;
		draw->canvas()->setTranslate(position());
		draw->drawBoxBasic(this, data);
		draw->drawBoxEnd(this, data);
	}

	void Image::draw(Painter *draw) {
		BoxData data;
		draw->canvas()->setTranslate(position());
		draw->drawBoxBasic(this, data);

		auto src = source();
		if (src && src->load()) {
			draw->getInsideRectPath(this, data);
			Paint paint;
			ImagePaint img;
			// paint.antiAlias = false;
			paint.type = Paint::kBitmap_Type;
			paint.image = &img;
			paint.color.set_a(draw->opacity());
			//img.tileModeX = ImagePaint::kDecal_TileMode;
			//img.tileModeY = ImagePaint::kDecal_TileMode;
			img.filterMode = ImagePaint::kLinear_FilterMode;
			img.mipmapMode = ImagePaint::kNearest_MipmapMode;
			img.setImage(src.get(), data.inside->rect);
			draw->canvas()->drawPathv(*data.inside, paint);
		}
		draw->drawBoxEnd(this, data);
	}

	void Scroll::draw(Painter *draw) {
		Box::draw(draw);
		draw->drawScrollBar(this);
	}

	void Text::draw(Painter *draw) {
		BoxData data;
		auto canvas = draw->canvas();
		canvas->setTranslate(position());
		draw->drawBoxBasic(this, data);

		Vec2 offset(padding_left(), padding_top());

		_IfBorder(this) {
			offset[0] += _border->width[3];
			offset[1] += _border->width[0];
		}

		if (clip()) {
			if (first() || _blob_visible.length()) {
				draw->getInsideRectPath(this, data);
				window()->clipRegion(screen_region_from_convex_quadrilateral(_vertex));
				canvas->save();
				canvas->clipPathv(*data.inside, Canvas::kIntersect_ClipOp, true); // clip
				if (_blob_visible.length()) {
					draw->drawTextBlob(this, offset, *_lines, _blob, _blob_visible);
				}
				draw->visitView(this);
				canvas->restore(); // cancel clip
				window()->clipRestore();
			}
		} else {
			if (_blob_visible.length()) {
				draw->drawTextBlob(this, offset, *_lines, _blob, _blob_visible);
			}
			draw->visitView(this);
		}
	}

	void Input::draw(Painter *draw) {
		BoxData data;
		auto canvas = draw->canvas();
		canvas->setTranslate(position());
		draw->drawBoxBasic(this, data);

		auto lines = *_lines;
		auto offset = input_text_offset() + Vec2(padding_left(), padding_top());
		auto twinkle = _editing && _cursor_twinkle_status;
		auto visible = _blob_visible.length();
		auto clip = this->clip() && (visible || twinkle);

		if (clip) {
			draw->getInsideRectPath(this, data);
			canvas->save();
			canvas->clipPathv(*data.inside, Canvas::kIntersect_ClipOp, true); // clip
		}

		if (visible) {
			auto draw_background = [&](TextBlob &blob, const Color4f &color) {
				auto &line = lines->line(blob.line);
				auto x = offset.x() + line.origin + blob.origin;
				auto y = offset.y() + line.baseline - blob.ascent;
				auto offset_x = blob.blob.offset.front().x();
				auto width = blob.blob.offset.back().x();
				auto &rect = draw->cache()->getRectPath({{x + offset_x, y},{width, blob.height}});
				canvas->drawPathvColor(rect, color, kSrcOver_BlendMode);
			};
			auto size = text_size().value;
			auto shadow = text_shadow().value;

			// draw text background
			if (text_length() && text_background_color().value.a()) {
				auto color = text_background_color().value.to_color4f_alpha(draw->opacity());
				for (auto i: _blob_visible)
					draw_background(_blob[i], color);
			}
			
			// draw text marked
			auto begin = _marked_blob_begin;
			if (begin < _marked_blob_end && _marked_color.a()) {
				auto color = _marked_color.to_color4f_alpha(draw->opacity());
				do
					draw_background(_blob[begin++], color);
				while(begin < _marked_blob_end);
			}

			// draw text shadow
			if (shadow.color.a()) {
				Paint paint;
				PaintFilter filter;
				paint.color = shadow.color.to_color4f_alpha(draw->opacity());
				if (shadow.size) {
					filter.type = PaintFilter::kBlur_Type;
					filter.val0 = shadow.size;
					paint.filter = &filter;
				}
				for (auto i: _blob_visible) {
					auto &blob = _blob[i];
					auto &line = lines->line(blob.line);
					canvas->drawTextBlob(&blob.blob, {
						line.origin + shadow.x + offset.x() + blob.origin,
						line.baseline + shadow.y + offset.y()
					}, size, paint);
				}
			}

			// draw text blob
			auto color = _value_u4.length() ? text_color().value: _placeholder_color;
			if (color.a()) {
				Paint paint;
				paint.color = color.to_color4f_alpha(draw->opacity());
				for (auto i: _blob_visible) {
					auto &blob = _blob[i];
					auto &line = lines->line(blob.line);
					canvas->drawTextBlob(&blob.blob, {
						line.origin + blob.origin + offset.x(), line.baseline + offset.y()
					}, size, paint);
				}
			} // if (color.a())
		}

		// draw cursor
		if (twinkle) {
			auto &line = lines->line(_cursor_line);
			auto x = Float32::max(0, offset.x() + _cursor_x - 1.0f);
			//auto y = offset.y() + line.baseline - _text_ascent;
			auto y = offset.y() + (line.end_y + line.start_y - _cursor_height) * 0.5f;
			auto &rect = draw->cache()->getRectPath({{x, y},{2.0,_cursor_height}});
			canvas->drawPathvColor(rect, _cursor_color.to_color4f_alpha(draw->opacity()), kSrcOver_BlendMode);
		}

		if (clip) {
			canvas->restore(); // cancel clip
		}

		if ( is_multiline() ) {
			draw->drawScrollBar(static_cast<Textarea*>(this));
		}
	}

	void Label::draw(Painter *draw) {
		if (_blob_visible.length()) {
			draw->canvas()->setTranslate(position()); // set position in matrix
			draw->drawTextBlob(this, {}, *_lines, _blob, _blob_visible);
		}
		draw->visitView(this);
	}

	void Matrix::draw(Painter *draw) {
		auto canvas = draw->_canvas;
		auto matrix = draw->_matrix;
		auto origin = draw->_origin;
		draw->_origin = Vec2(draw->_AAShrink * 0.5) - _origin_value;
		draw->_matrix = &this->matrix();
		canvas->setMatrix(*draw->_matrix);
		BoxData data;
		draw->drawBoxBasic(this, data);
		draw->_origin = origin;
		draw->drawBoxEnd(this, data);
		draw->_matrix = matrix;
		canvas->setMatrix(*matrix); // restore previous matrix
	}

	void Root::draw(Painter *draw) {
		if (draw->_canvas && _visible) {
			auto canvas = draw->_canvas;
			auto mark = mark_value();
			if (mark) {
				solve_marks(Mat(), nullptr, mark);
				draw->_mark_recursive = mark & View::kRecursive_Mark;
			}
			if (_visible_region && opacity() != 0) {
				BoxData data;
				// Fix rect aa stroke width
				bool isMsaa = _window->render()->options().msaaSample;
				auto origin = isMsaa ? 0: 0.45f / _window->scale(); // fix aa stroke width
				//auto origin = isMsaa ? 0: 0.5f / _window->scale(); // fix aa stroke width
				draw->_AAShrink = origin + origin;
				draw->_origin = Vec2(origin) - origin_value();
				draw->_matrix = &matrix();
				canvas->setMatrix(matrix());
				canvas->clearColor(background_color().to_color4f());
				draw->drawBoxColor(this, data);
				draw->drawBoxBorder(this, data);
				draw->_origin = origin;
				draw->drawBoxEnd(this, data);
			} else {
				canvas->clearColor(Color4f(0,0,0,0));
			}
			draw->_mark_recursive = 0;
		}
	}
}
