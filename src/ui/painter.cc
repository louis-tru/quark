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
		, _color(1,1,1,1), _mark_recursive(0), _matrix(nullptr)
		, _is_translation_matrix(false)
	{
		_isMsaa = _window->render()->options().msaaSample;
		_canvas = _render->getCanvas();
		_cache = _canvas->getPathvCache();
	}

	void Painter::set_matrix(const Mat* mat) {
		_matrix = mat;
		_is_translation_matrix = mat->is_translation_matrix();
		_canvas->setMatrix(*mat);
	}

	void Painter::set_origin(Vec2 origin) {
		_origin = -origin;
		_originAA = Vec2(_AAShrink * 0.5) - origin;
	}

	void Painter::set_origin_reverse(Vec2 origin) {
		_origin = origin;
		_originAA = Vec2(_AAShrink * 0.5) + origin;
	}

	inline static bool is_not_Zero(const float radius[4]) {
		return *reinterpret_cast<const uint64_t*>(radius) != 0 ||
			*reinterpret_cast<const uint64_t*>(radius+2) != 0;
	}

	Rect Painter::getRect(Box* box, BoxData &data) {
		if (!data.isInit) {
			auto radius = &box->_border_radius_left_top;
			data.isRadius = is_not_Zero(radius);
			data.antiAlias = data.isRadius || !_is_translation_matrix;
			//data.antiAlias = false; // disable antiAlias for rect
			data.isInit = true;
		}
		return data.antiAlias ? Rect{
			_originAA, {box->_client_size[0]-_AAShrink,box->_client_size[1]-_AAShrink},
		}: Rect{
			_origin, {box->_client_size[0],box->_client_size[1]},
		};
	}

	void Painter::getInsideRectPath(Box *box, BoxData &out) {
		if (out.inside)
			return;
		auto rect = getRect(box, out);
		auto radius = &box->_border_radius_left_top;
		_IfBorder(box) {
			auto border = _border->width;
			Hash5381 hash;
			hash.updatefv4(rect.origin.val);
			hash.updatefv4(radius);
			hash.updatefv4(border);

			out.inside = _cache->getRRectPathFromHash(hash.hashCode());
			if (!out.inside) {
				if (out.antiAlias) {
					float radiusLimit = Float32::min(rect.size.x() * 0.5f, rect.size.y() * 0.5f);
					// TODO: The interior and the border cannot fit completely,
					// causing anti-aliasing to fail at 1 to 2 pixels,
					// so temporarily reduce the interior frame to 0.75
					float AAShrink = _AAShrinkBorder * 0.75;
					float borderFix[4] = {
						Float32::max(0, border[0]-AAShrink), Float32::max(0, border[1]-AAShrink),
						Float32::max(0, border[2]-AAShrink), Float32::max(0, border[3]-AAShrink),
					};
					rect.origin[0] += borderFix[3]; // left
					rect.origin[1] += borderFix[0]; // top
					rect.size  [0] -= borderFix[3] + borderFix[1]; // left + right
					rect.size  [1] -= borderFix[0] + borderFix[2]; // top + bottom

					if (out.isRadius) {
						float leftTop = Qk_Min(radius[0],radiusLimit), rightTop = Qk_Min(radius[1],radiusLimit);
						float rightBottom = Qk_Min(radius[2],radiusLimit), leftBottom = Qk_Min(radius[3],radiusLimit);
						Path::BorderRadius br{
							{leftTop-border[3], leftTop-border[0]}, {rightTop-border[1], leftTop-border[0]},
							{rightBottom-border[1], rightBottom-border[2]}, {leftBottom-border[3], rightBottom-border[2]},
						};
						out.inside = &_cache->setRRectPathFromHash(hash.hashCode(), RectPath::MakeRRect(rect, br));
					} else {
						out.inside = &_cache->setRRectPathFromHash(hash.hashCode(), RectPath::MakeRect(rect));
					}
				} else {
					rect.origin[0] += border[3]; // left
					rect.origin[1] += border[0]; // top
					rect.size  [0] -= border[3] + border[1]; // left + right
					rect.size  [1] -= border[0] + border[2]; // top + bottom
					out.inside = &_cache->setRRectPathFromHash(hash.hashCode(), RectPath::MakeRect(rect));
				}
			}
		} else if (out.isRadius) {
			out.inside = &_cache->getRRectPath(rect, radius);
		} else {
			out.inside = &_cache->getRectPath(rect);
		}
	}

	void Painter::getOutsideRectPath(Box *box, BoxData &out) {
		if (!out.outside) {
			auto rect = getRect(box, out);
			if (out.isRadius) {
				out.outside = &_cache->getRRectPath(rect, &box->_border_radius_left_top);
			} else {
				out.outside = &_cache->getRectPath(rect);
			}
		}
	}

	void Painter::getRRectOutlinePath(Box *box, BoxData &out) {
		if (!out.outline) {
			_Border(box);
			// if border is zero, outline is null
			if (is_not_Zero(_border->width)) {
				auto rect = getRect(box, out);
				auto border = _border->width;
				auto radius = &box->_border_radius_left_top;
				if (out.isRadius) { // radius is not zero
					Hash5381 hash;
					hash.updatefv4(rect.origin.val);
					hash.updatefv4(border);
					hash.updatefv4(radius);

					out.outline = _cache->getRRectOutlinePathFromHash(hash.hashCode());
					if (!out.outline) {
						float radiusLimit = Float32::min(rect.size.x() * 0.5f, rect.size.y() * 0.5f);
						float AAShrink = _AAShrinkBorder;
						float borderFix[4] = {
							Float32::max(0, border[0]-AAShrink), Float32::max(0, border[1]-AAShrink),
							Float32::max(0, border[2]-AAShrink), Float32::max(0, border[3]-AAShrink),
						};
						Path::BorderRadius br{
							{Qk_Min(radius[0],radiusLimit)}, {Qk_Min(radius[1],radiusLimit)},
							{Qk_Min(radius[2],radiusLimit)}, {Qk_Min(radius[3],radiusLimit)},
						};
						out.outline = &_cache->setRRectOutlinePathFromHash(hash.hashCode(),
								RectOutlinePath::MakeRRectOutline(rect, borderFix, br));
					}
				} else {
					out.outline = &_cache->getRectOutlinePath(rect, border);
				}
			}
		}
	}

	template<CascadeColor parentCascadeColor>
	inline void Painter::visitView_(View *view, View *v) {
		auto lastColor = _color;
		auto lastMarkRecursive = _mark_recursive;
		do {
			if (v->_visible) {
				uint32_t mark = lastMarkRecursive | v->mark_value(); // inherit recursive
				if (mark) {
					v->solve_marks(*_matrix, view, mark);
					_mark_recursive = mark & View::kRecursive_Mark;
				}
				if (v->_visible_region && v->_color.a()) {
					switch (parentCascadeColor) {
						case CascadeColor::None:
							_color = v->_color.to_color4f(); break;
						case CascadeColor::Alpha:
							_color = v->_color.mul_alpha_only(lastColor.a()); break;
						case CascadeColor::Color:
							_color = v->_color.mul_rgb_only(lastColor); break;
						case CascadeColor::Both:
							_color = v->_color.mul_color4f(lastColor); break;
					}
					v->draw(this);
				}
			}
			v = v->_next.load(std::memory_order_acquire);
		} while(v);
		_color = lastColor;
		_mark_recursive = lastMarkRecursive;
	}

	void Painter::visitView(View *view) {
		auto v = view->_first.load(std::memory_order_acquire);
		if (v) {
			switch (view->_cascade_color) {
				case CascadeColor::None:
					visitView_<CascadeColor::None>(view, v); break;
				case CascadeColor::Alpha:
					visitView_<CascadeColor::Alpha>(view, v); break;
				case CascadeColor::Color:
					visitView_<CascadeColor::Color>(view, v); break;
				case CascadeColor::Both:
					visitView_<CascadeColor::Both>(view, v); break;
			}
		}
	}

	void Painter::visitView(View* v, cMat* mat) {
		if (v->_first.load(std::memory_order_relaxed)) {
			auto lastMatrix = _matrix;
			set_matrix(mat);
			visitView(v);
			set_matrix(lastMatrix);
		}
	}

	void Painter::drawBoxBasic(Box *box, BoxData &data) {
		drawBoxShadow(box, data);

		if (_color.a() == 1.0f) {
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
			stroke.antiAlias = data.antiAlias;
			stroke.style = Paint::kStroke_Style;
			if (data.outline) {
				for (int i = 0; i < 4; i++) {
					if (_border->width[i]) { // top
						auto pv = &data.outline->top + i;
						if (pv->vCount) {
							addItem(pathvs, _border->color[i], pv);
						} else { // too thin, draw only a little stroke
							stroke.color = _border->color[i].mul_color4f(_color);
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
					it.color.mul_color4f(_color), kSrcOver_BlendMode, data.antiAlias);
			}
		}

		drawBoxFill(box, data);
	}

	void Painter::drawBoxFill(Box *box, BoxData &data) {
		auto filter = box->background();
		if (!filter)
			return;
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
		// auto h_w = data.inside->rect.size[0],
		// 		 h_h = data.inside->rect.size[1];
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
		x = FillImage::compute_position(fill->x(), h_w, w) + _origin[0]; // _originAA[0]
		y = FillImage::compute_position(fill->y(), h_h, h) + _origin[1]; // _originAA[1]

		if (_border) {
			x += _border->width[3]; // left
			y += _border->width[0]; // top
		}

		Paint paint;
		ImagePaint img;
		paint.antiAlias = data.antiAlias;
		paint.type = Paint::kBitmap_Type;
		paint.image = &img;
		paint.color = _color;

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
			case Repeat::NoRepeat:
				img.tileModeX = ImagePaint::kDecal_TileMode;
				img.tileModeY = ImagePaint::kDecal_TileMode; break;
		}
		img.filterMode = default_FilterMode;
		img.mipmapMode = default_MipmapMode;

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
		paint.antiAlias = data.antiAlias;
		paint.color = _color;
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
		paint.antiAlias = data.antiAlias;
		paint.color = _color;
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
		if (!shadow)
			return;
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
			},&box->_border_radius_left_top, s.size, s.color.mul_color4f(_color), kSrcOver_BlendMode);
			shadow = static_cast<BoxShadow*>(shadow->next());
		} while(shadow);
		_canvas->restore();
	}

	void Painter::drawBoxColor(Box *box, BoxData &data) {
		if (!box->_background_color.a())
			return;
		getInsideRectPath(box, data);
		_canvas->drawPathvColor(*data.inside,
			box->_background_color.mul_color4f(_color), kSrcOver_BlendMode, data.antiAlias
		);
		//Paint paint;
		//paint.antiAlias = true;
		//paint.color = box->_background_color.mul_color4f(_color);
		//_canvas->drawPathv(*data.inside, paint);
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
						_canvas->drawPathvColor(*pv, _border->color[i].mul_color4f(_color), kSrcOver_BlendMode, data.antiAlias);
					} else { // stroke
						stroke.color = _border->color[i].mul_color4f(_color);
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
				_window->clipRegion(screen_region_from_convex_quadrilateral(box->_bounds));
				_canvas->save();
				_canvas->clipPathv(*data.inside, Canvas::kIntersect_ClipOp, data.antiAlias); // clip
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
			auto color = v->scrollbar_color().mul_color4f(_color);
			color[3] *= v->_scrollbar_opacity;
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
				_canvas->drawPathvColor(_cache->getRRectPath(rect, radius), color, kSrcOver_BlendMode, true);
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
				_canvas->drawPathvColor(_cache->getRRectPath(rect, radius), color, kSrcOver_BlendMode, true);
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
			auto color = v->text_background_color().value.mul_color4f(_color);
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
				_canvas->drawPathvColor(rect, color, kSrcOver_BlendMode, true);
			}
		}

		// draw text shadow
		if (shadow.color.a()) {
			Paint paint;
			PaintFilter filter;
			paint.color = shadow.color.mul_color4f(_color);
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
			paint.color = v->text_color().value.mul_color4f(_color);
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
			paint.antiAlias = data.antiAlias;
			paint.type = Paint::kBitmap_Type;
			paint.image = &img;
			paint.color = draw->color();
			//img.tileModeX = ImagePaint::kDecal_TileMode;
			//img.tileModeY = ImagePaint::kDecal_TileMode;
			img.filterMode = default_FilterMode;
			img.mipmapMode = default_MipmapMode;
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
				window()->clipRegion(screen_region_from_convex_quadrilateral(_bounds));
				canvas->save();
				canvas->clipPathv(*data.inside, Canvas::kIntersect_ClipOp, data.antiAlias); // clip
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
			canvas->clipPathv(*data.inside, Canvas::kIntersect_ClipOp, data.antiAlias); // clip
		}

		if (visible) {
			auto draw_background = [&](TextBlob &blob, const Color4f &color) {
				auto &line = lines->line(blob.line);
				auto x = offset.x() + line.origin + blob.origin;
				auto y = offset.y() + line.baseline - blob.ascent;
				auto offset_x = blob.blob.offset.front().x();
				auto width = blob.blob.offset.back().x();
				auto &rect = draw->cache()->getRectPath({{x + offset_x, y},{width, blob.height}});
				canvas->drawPathvColor(rect, color, kSrcOver_BlendMode, true);
			};
			auto size = text_size().value;
			auto shadow = text_shadow().value;

			// draw text background
			if (text_length() && text_background_color().value.a()) {
				auto color = text_background_color().value.mul_color4f(draw->color());
				for (auto i: _blob_visible)
					draw_background(_blob[i], color);
			}
			
			// draw text marked
			auto begin = _marked_blob_begin;
			if (begin < _marked_blob_end && _marked_color.a()) {
				auto color = _marked_color.mul_color4f(draw->color());
				do
					draw_background(_blob[begin++], color);
				while(begin < _marked_blob_end);
			}

			// draw text shadow
			if (shadow.color.a()) {
				Paint paint;
				PaintFilter filter;
				paint.color = shadow.color.mul_color4f(draw->color());
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
				paint.color = color.mul_color4f(draw->color());
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
			canvas->drawPathvColor(rect, _cursor_color.mul_color4f(draw->color()), kSrcOver_BlendMode, true);
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

	void Matrix::draw(Painter *painter) {
		BoxData data;
		auto lastMatrix = painter->matrix();
		auto lastOrigin = painter->origin();
		painter->set_origin(_origin_value);
		painter->set_matrix(&matrix());
		painter->drawBoxBasic(this, data);
		painter->set_origin_reverse(lastOrigin); // restore previous origin
		painter->drawBoxEnd(this, data);
		painter->set_matrix(lastMatrix); // restore previous matrix
	}

	void Root::draw(Painter *painter) {
		if (painter->_canvas && _visible) {
			auto canvas = painter->_canvas;
			auto mark = mark_value();
			if (mark) {
				solve_marks(Mat(), nullptr, mark);
				painter->_mark_recursive = mark & View::kRecursive_Mark;
			}
			if (_visible_region && color().a() != 0) {
				painter->_tempAllocator[0].reset();
				painter->_tempAllocator[1].reset();
				BoxData data;
				// Fix rect aa stroke width
				auto AAShrink_half = painter->_isMsaa ? 0: 0.43f / _window->scale(); // fix aa stroke width, 0.4-0.5
				//auto AAShrink_half = painter->_isMsaa ? 0: 0.5f / _window->scale();
				auto AAShrink = AAShrink_half + AAShrink_half;
				auto AAShrinkBorder_half = painter->_isMsaa ? 0: 0.02f / _window->scale(); // plus 0.02 to avoid aa gap
				painter->_AAShrink = AAShrink;
				painter->_AAShrinkBorder = AAShrink + AAShrinkBorder_half + AAShrinkBorder_half;
				painter->_color = color().to_color4f();
				painter->set_origin(origin_value());
				painter->set_matrix(&matrix());
				canvas->clearColor(background_color().mul_color4f(painter->_color));
				painter->drawBoxFill(this, data);
				painter->drawBoxBorder(this, data);
				painter->set_origin({}); // reset origin
				painter->drawBoxEnd(this, data);
			} else {
				canvas->clearColor(Color4f(0,0,0,0));
			}
			painter->_mark_recursive = 0;
		}
	}
}
