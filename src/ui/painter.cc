/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./app.h"
#include "./window.h"
#include "./filter.h"
#include "./painter.h"
#include "./view/root.h"
#include "./view/image.h"
#include "./view/flex.h"
#include "./view/flow.h"
#include "./view/text.h"
#include "./view/input.h"
#include "./view/textarea.h"
#include "./view/label.h"
#include "./view/morph.h"
#include "./view/sprite.h"
#include "./view/spine.h"
#include "./geometry.h"

#define _Border(v) auto _border = v->_border.load()
#define _IfBorder(v) _Border(v); if (_border)
#define _IfNotBorder(v) _Border(v); if (!_border) return

namespace qk {
	constexpr BlendMode defaultBlendMode = kSrcOverPre_BlendMode;

	typedef Painter::BoxData BoxData;

	Painter::Painter(Window *window)
		: _render(window->render()), _canvas(nullptr)
		, _cache(nullptr)
		, _window(window)
		, _color(1,1,1,1), _mark_recursive(0), _matrix(nullptr)
		, _delayCmdsAllocator()
		, _delayCmds(nullptr)
	{
		_isMsaa = _window->render()->options().msaaSample;
		_canvas = _render->getCanvas();
		_cache = _canvas->getPathvCache();
		_delayCmdsStack.push(DelayCmdMap(
			std::less<uint32_t>(), STLAllocator<DelayCmdKV>(&_delayCmdsAllocator)
		));
	}

	void Painter::set_matrix(const Mat* mat) {
		_matrix = mat;
		_canvas->setMatrix(*mat);
	}

	void Painter::set_origin(Vec2 origin) {
		_origin = -origin;
		_originAA = _AAShrinkHalf - origin;
	}

	void Painter::set_origin_reverse(Vec2 origin) {
		_origin = origin;
		_originAA = _AAShrinkHalf + origin;
	}

	inline static bool is_not_Zero(const float radius[4]) {
		return *reinterpret_cast<const uint64_t*>(radius) != 0 ||
			*reinterpret_cast<const uint64_t*>(radius+2) != 0;
	}

	Rect Painter::getRect(Box* box) {
		return box->_aa ? Rect{
			_originAA, {box->_client_size[0]-_AAShrink,box->_client_size[1]-_AAShrink},
		}: Rect{
			_origin, {box->_client_size[0],box->_client_size[1]},
		};
	}

	void Painter::getInsideRectPath(Box *box) {
		if (_boxData.inside)
			return;
		auto rect = getRect(box);
		auto radius = &box->_border_radius_left_top;
		_IfBorder(box) {
			auto border = _border->width;
			Hash5381 hash;
			hash.updatefv4(rect.begin.val);
			hash.updatefv4(radius);
			hash.updatefv4(border);

			_boxData.inside = _cache->getRRectPathFromHash(hash.hashCode());
			if (_boxData.inside)
				return;

			auto radiusLimit = Float32::min(rect.size.x() * 0.5f, rect.size.y() * 0.5f);
			auto  borderFix = border;
			float borderFixStore[4];
			if (box->_aa) {
				// TODO: The interior and the border cannot fit completely,
				// causing anti-aliasing to fail at 1 to 2 pixels,
				// so temporarily reduce the interior frame to 0.75
				float AAShrink = _AAShrinkBorder * 0.75;
				borderFixStore[0] = Float32::max(0, border[0]-AAShrink);
				borderFixStore[1] = Float32::max(0, border[1]-AAShrink);
				borderFixStore[2] = Float32::max(0, border[2]-AAShrink);
				borderFixStore[3] = Float32::max(0, border[3]-AAShrink);
				borderFix = borderFixStore;
			}
			rect.begin[0] += borderFix[3]; // left
			rect.begin[1] += borderFix[0]; // top
			rect.size  [0] -= borderFix[3] + borderFix[1]; // left + right
			rect.size  [1] -= borderFix[0] + borderFix[2]; // top + bottom

			if (is_not_Zero(radius)) {
				float leftTop     = Qk_Min(radius[0],radiusLimit), rightTop   = Qk_Min(radius[1],radiusLimit);
				float rightBottom = Qk_Min(radius[2],radiusLimit), leftBottom = Qk_Min(radius[3],radiusLimit);
				Path::BorderRadius br{
					{leftTop-border[3],     leftTop-border[0]},     {rightTop-border[1],  rightTop-border[0]},
					{rightBottom-border[1], rightBottom-border[2]}, {leftBottom-border[3], leftBottom-border[2]},
				};
				_boxData.inside = &_cache->setRRectPathFromHash(hash.hashCode(), RectPath::MakeRRect(rect, br));
			} else {
				_boxData.inside = &_cache->setRRectPathFromHash(hash.hashCode(), RectPath::MakeRect(rect));
			}
		} else if (is_not_Zero(radius)) {
			_boxData.inside = &_cache->getRRectPath(rect, radius);
		} else {
			_boxData.inside = &_cache->getRectPath(rect);
		}
	}

	void Painter::getOutsideRectPath(Box *v) {
		if (!_boxData.outside) {
			auto rect = getRect(v);
			auto radius = &v->_border_radius_left_top;
			if (is_not_Zero(radius)) {
				_boxData.outside = &_cache->getRRectPath(rect, radius);
			} else {
				_boxData.outside = &_cache->getRectPath(rect);
			}
		}
	}

	void Painter::getRRectOutlinePath(Box *v) {
		if (!_boxData.outline) {
			_Border(v);
			// if border is zero, outline is null
			if (is_not_Zero(_border->width)) {
				auto rect = getRect(v);
				auto border = _border->width;
				auto radius = &v->_border_radius_left_top;

				Hash5381 hash;
				hash.updatefv4(rect.begin.val);
				hash.updatefv4(border);
				hash.updatefv4(radius);

				_boxData.outline = _cache->getRRectOutlinePathFromHash(hash.hashCode());
				if (_boxData.outline)
					return;

				auto  borderFix = border;
				float borderFixStore[4];
				if (v->_aa) {
					float AAShrink = _AAShrinkBorder;
					borderFixStore[0] = Float32::max(0, border[0]-AAShrink);
					borderFixStore[1] = Float32::max(0, border[1]-AAShrink);
					borderFixStore[2] = Float32::max(0, border[2]-AAShrink);
					borderFixStore[3] = Float32::max(0, border[3]-AAShrink);
					borderFix = borderFixStore;
				}
				if (is_not_Zero(radius)) {
					float radiusLimit = Float32::min(rect.size.x() * 0.5f, rect.size.y() * 0.5f);
					Path::BorderRadius br{
						{Qk_Min(radius[0],radiusLimit)}, {Qk_Min(radius[1],radiusLimit)},
						{Qk_Min(radius[2],radiusLimit)}, {Qk_Min(radius[3],radiusLimit)},
					};
					_boxData.outline = &_cache->setRRectOutlinePathFromHash(hash.hashCode(),
							RectOutlinePath::MakeRRectOutline(rect, borderFix, br));
				} else {
					_boxData.outline = &_cache->setRRectOutlinePathFromHash(hash.hashCode(),
							RectOutlinePath::MakeRectOutline(rect, borderFix));
				}
			}
		}
	}

	void Painter::drawBoxBasic(Box *v) {
		if (!v->_color.a())
			return;
		drawBoxShadow(v);

		_IfNotBorder(v) ({
			drawBoxColor(v);
			drawBoxFill(v);
		});

		auto addBatch = [](PathvBatchs &out, Color color, const qk::Pathv *pv) {
			auto key = reinterpret_cast<uint32_t&>(color);
			int i = 0;
			for (; i < out.total; i++) {
				auto &it = out.indexed[i];
				if (it.key == key) {
					it.pathv[it.count++] = pv;
					return;
				}
			}
			auto &it = out.indexed[i];
			it.key = key;
			it.color = color;
			it.pathv[it.count++] = pv;
			out.total++;
		};

		if (v->background()) {
			drawBoxColor(v);
			drawBoxFill(v);
		}
		else if (v->_background_color.a()) {
			getInsideRectPath(v);
			addBatch(_pathvs, v->_background_color, _boxData.inside);
		}

		getRRectOutlinePath(v);

		if (_boxData.outline) {
			Paint paint;
			paint.antiAlias = v->_aa;
			paint.style = Paint::kStroke_Style;
			for (int i = 0; i < 4; i++) {
				if (_border->width[i] && _border->color[i].a()) {
					auto pv = &_boxData.outline->top + i;
					if (pv->vCount) {
						addBatch(_pathvs, _border->color[i], pv);
					} else { // too thin, draw only a little stroke
						paint.stroke.color = _border->color[i].premul_alpha().mul(_color);
						paint.strokeWidth = _border->width[i];
						_canvas->drawPath(pv->path, paint);
					}
				}
			}
		}

		if (_pathvs.total) {
			for (int i = 0; i < _pathvs.total; i++) {
				auto & it = _pathvs.indexed[i];
				_canvas->drawPathvColors(it.pathv, it.count,
					it.color.premul_alpha().mul(_color), defaultBlendMode, v->_aa);
			}
			_pathvs = {0}; // reset batch
		}
	}

	void Painter::drawBoxBorder(Box *v) {
		_IfNotBorder(v);
		getRRectOutlinePath(v);
		if (_boxData.outline) {
			Paint paint;
			paint.style = Paint::kStroke_Style;
			for (int i = 0; i < 4; i++) {
				if (_border->width[i] && _border->color[i].a()) { // top
					auto pv = &_boxData.outline->top + i;
					if (pv->vCount) {
						_canvas->drawPathvColor(*pv, _border->color[i].premul_alpha().mul(_color), defaultBlendMode, v->_aa);
					} else { // stroke
						paint.stroke.color = _border->color[i].premul_alpha().mul(_color);
						paint.strokeWidth = _border->width[i];
						_canvas->drawPath(pv->path, paint);
					}
				}
			}
		}
	}

	void Painter::drawBoxFill(Box *v) {
		auto filter = v->background();
		if (!filter)
			return;
		getInsideRectPath(v);
		do {
			switch(filter->type()) {
				case BoxFilter::kImage:// fill
					drawBoxFillImage(v, static_cast<FillImage*>(filter)); break;
				case BoxFilter::kGradientLinear: // fill
					drawBoxFillLinear(v, static_cast<FillGradientLinear*>(filter)); break;
				case BoxFilter::kGradientRadial: // fill
					drawBoxFillRadial(v, static_cast<FillGradientRadial*>(filter)); break;
				default: break;
			}
			filter = filter->next();
		} while(filter);
	}

	void Painter::drawBoxFillImage(Box *v, FillImage *fill) {
		auto src = fill->source();
		if (!src || !src->load())
			return;

		src->markAsTexture();

		auto src_w = src->width(), src_h = src->height();
		auto cli = v->_client_size;
		auto h_w = cli.x(), h_h = cli.y();
		float w, h, x, y;

		_IfBorder(v) {
			h_w -= (_border->width[3] + _border->width[1]); // left + right
			h_h -= (_border->width[0] + _border->width[2]); // top + bottom
		}
		if (FillImage::compute_size(fill->width(), h_w, w)) { // ok x
			if (!FillImage::compute_size(fill->height(), h_h, h)) // auto y
				h = fabs(w / src_w * src_h);
		} else if (FillImage::compute_size(fill->height(), h_h, h)) { // auto x, ok y
			w = fabs(h / src_h * src_w);
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
		PaintImage img;
		paint.fill.color = _color;
		paint.fill.image = &img;
		paint.antiAlias = v->_aa;

		if (!src->premultipliedAlpha()) {
			paint.blendMode = kSrcOver_BlendMode;
			paint.fill.color = _color.recover_unpremul_alpha();
		}

		auto inside = _boxData.inside;
		auto rect = inside->rect;

		auto clip = [](Painter *self, Box *v, Vec2 a, Vec2 &b) {
			auto a0 = a.x(), a1 = a.x() + a.y();
			if (a.y() < 0)
				std::swap(a0, a1);
			if (v->_aa) {
				a0 += self->_AAShrink * 0.5f;
				a1 -= self->_AAShrink;
			}
			// clip rect
			auto max = std::min(a1, b.x() + b.y());
			auto min = std::max(a0, b.x());
			if (min < max) { // ok
				b = {min, max - min};
				return false; // not failed
			} else {
				return true; // failed
			}
		};

		switch(fill->repeat()) {
			case Repeat::Repeat:
				img.tileModeX = PaintImage::kRepeat_TileMode;
				img.tileModeY = PaintImage::kRepeat_TileMode; break;
			case Repeat::MirrorRepeat:
				img.tileModeX = PaintImage::kMirror_TileMode;
				img.tileModeY = PaintImage::kMirror_TileMode; break;
			case Repeat::MirrorRepeatX:
				img.tileModeX = PaintImage::kMirror_TileMode;
				img.tileModeY = PaintImage::kDecal_TileMode; goto try_clipY;
			case Repeat::MirrorRepeatY:
				img.tileModeX = PaintImage::kDecal_TileMode;
				img.tileModeY = PaintImage::kMirror_TileMode; goto try_clipX;
			case Repeat::RepeatX:
				img.tileModeX = PaintImage::kRepeat_TileMode;
				img.tileModeY = PaintImage::kDecal_TileMode;
			try_clipY:
				if (!inside->rrectMask) { // no need clip if rrectMask
					Vec2 out{rect.begin.y(), rect.size.y()};
					if (clip(this, v, {y,h}, out)) // clip y
						return;
					inside = &_cache->getRectPath({{rect.begin.x(), out.x()}, {rect.size.x(), out.y()}});
				}
				break;
			case Repeat::RepeatY:
				img.tileModeX = PaintImage::kDecal_TileMode;
				img.tileModeY = PaintImage::kRepeat_TileMode;
			try_clipX:
				if (!inside->rrectMask) {
					Vec2 out{rect.begin.x(), rect.size.x()};
					if (clip(this, v, {y,h}, out)) // clip x
						return;
					inside = &_cache->getRectPath({{out.x(), rect.begin.y()}, {out.y(), rect.size.y()}});
				}
				break;
			case Repeat::NoRepeat:
				img.tileModeX = PaintImage::kDecal_TileMode;
				img.tileModeY = PaintImage::kDecal_TileMode;
				if (!inside->rrectMask) {
					Vec2 outX{rect.begin.x(), rect.size.x()};
					Vec2 outY{rect.begin.y(), rect.size.y()};
					if (clip(this, v, {x,w}, outX) || clip(this, v, {y,h}, outY))
						return;
					inside = &_cache->getRectPath({{outX.x(), outY.x()}, {outX.y(), outY.y()}});
				}
				break;
		}

		img.filterMode = default_FilterMode;
		img.mipmapMode = default_MipmapMode;
		// img.filterMode = PaintImage::kNearest_FilterMode;
		// img.mipmapMode = PaintImage::kNone_MipmapMode;
		img.setImage(src.get(), {{x,y}, {w,h}});

		_canvas->drawPathv(*inside, paint);
	}

	void Painter::drawBoxFillLinear(Box *v, FillGradientLinear *fill) {
		auto &colors = fill->premul_colors();
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
		auto _rect_inside = _boxData.inside->rect;
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
		float centerX = _rect_inside.begin.x() + b;
		float centerY = _rect_inside.begin.y() + a;

		Vec2 pts[2] = {
			{p0x + centerX, p0y + centerY}, // origin
			{p1x + centerX, p1y + centerY} // end
		};
		if (quadrant % 3) { // swap vec
			auto t = pts[0];
			pts[0] = pts[1];
			pts[1] = t;
		}
		PaintGradient g{
			PaintGradient::kLinear_Type, pts[0], pts[1],
			fill->premul_colors().length(),
			fill->premul_colors().val(), fill->positions().val()
		};
		Paint paint;
		paint.antiAlias = v->_aa;
		paint.fill.color = _color;
		paint.fill.gradient = &g;

		_canvas->drawPathv(*_boxData.inside, paint);
	}

	void Painter::drawBoxFillRadial(Box *v, FillGradientRadial *fill) {
		auto &colors = fill->premul_colors();
		auto &pos = fill->positions();
		auto rect_inside = _boxData.inside->rect;
		Vec2 radius{rect_inside.size.x() * 0.5f, rect_inside.size.y() * 0.5f};
		Vec2 center = rect_inside.begin + radius;
		PaintGradient g{
			PaintGradient::kRadial_Type, center, radius,
			fill->premul_colors().length(),
			fill->premul_colors().val(), fill->positions().val()
		};
		Paint paint;
		paint.antiAlias = v->_aa;
		paint.fill.color = _color;
		paint.fill.gradient = &g;
		_canvas->drawPathv(*_boxData.inside, paint);
	}

	void Painter::drawBoxShadow(Box *v) {
		auto shadow = v->box_shadow();
		if (!shadow)
			return;
		getOutsideRectPath(v);
		_canvas->save();
		_canvas->clipPathv(*_boxData.outside, Canvas::kDifference_ClipOp, false);
		do {
			if (shadow->type() != BoxFilter::kShadow)
				break;
			auto s = shadow->value();
			auto &o = _boxData.outside->rect.begin;
			_canvas->drawRRectBlurColor({
				{o.x()+s.x, o.y()+s.y}, _boxData.outside->rect.size,
			},&v->_border_radius_left_top, s.size, s.color.premul_alpha().mul(_color), kSrcOverPre_BlendMode);
			shadow = static_cast<BoxShadow*>(shadow->next());
		} while(shadow);
		_canvas->restore();
	}

	void Painter::drawBoxColor(Box *v) {
		if (!v->_background_color.a())
			return;
		getInsideRectPath(v);
		_canvas->drawPathvColor(*_boxData.inside,
			v->_background_color.premul_alpha().mul(_color), kSrcOverPre_BlendMode, v->_aa
		);
		//Paint paint;
		//paint.antiAlias = true;
		//paint.color = box->_background_color.premul_alpha().mul(_color);
		//_canvas->drawPathv(*data.inside, paint);
	}

	void Painter::drawScrollBar(ScrollView *v) {
		if ( (v->_scrollbar_h || v->_scrollbar_v) && v->_scrollbar_opacity ) {
			auto b = v->host();
			auto width = v->_scrollbar_width;
			auto margin = v->_scrollbar_margin;
			auto origin = Vec2();// Vec2{b->margin_left(),b->margin_top()};
			auto size = b->_client_size;
			auto color = v->scrollbar_color().mul_alpha_only(v->_scrollbar_opacity)
					.premul_alpha().mul(_color);
			auto _border = b->_border.load();

			if ( v->_scrollbar_h ) { // draw horizontal scrollbar
				float radius[] = {width,width,width,width};
				Rect rect = {
					{v->_scrollbar_position_h[0], size[1] - width - margin},
					{v->_scrollbar_position_h[1], width}
				};
				if (_border) {
					rect.begin += {_border->width[3], -_border->width[0]};
				}
				_canvas->drawPathvColor(_cache->getRRectPath(rect, radius), color, defaultBlendMode, true);
			}

			if ( v->_scrollbar_v ) { // draw vertical scrollbar
				float radius[] = {width,width,width,width};
				Rect rect = {
					{size[0] - width - margin, v->_scrollbar_position_v[0]},
					{width,                    v->_scrollbar_position_v[1]}
				};
				if (_border) {
					rect.begin += {-_border->width[3], _border->width[0]};
				}
				_canvas->drawPathvColor(_cache->getRRectPath(rect, radius), color, defaultBlendMode, true);
			}
		}
	}

	void Painter::drawTextBlob(TextOptions *v, Vec2 inOffset,
		TextLinesCore *lines, Array<TextBlob> &blobs, Array<uint32_t> &blob_visible) 
	{
		if (!_color.a())
			return;
		auto size = v->text_size().value;
		auto shadow = v->text_shadow().value;

		// draw text  background
		if (v->text_background_color().value.a()) {
			auto color = v->text_background_color().value.premul_alpha().mul(_color);
			for (auto i: blob_visible) {
				auto &blob = blobs[i];
				auto &line = lines->line(blob.line);
				auto offset_x = blob.blob.offset.front().x() + inOffset.x();
				auto &rect = _cache->getRectPath({
					{
						line.origin + blob.origin + offset_x,
						line.baseline - blob.ascent + inOffset.y()
					},
					{blob.blob.offset.back().x()-offset_x, blob.height},
				});
				_canvas->drawPathvColor(rect, color, defaultBlendMode, true);
			}
		}

		// draw text shadow
		if (shadow.color.a()) {
			Paint paint;
			PaintFilter filter;
			paint.fill.color = shadow.color.premul_alpha().mul(_color);
			auto stroke = v->text_stroke();
			if (stroke.value.width) {
				paint.style = Paint::kStrokeAndFill_Style;
				paint.strokeWidth = stroke.value.width;
				paint.stroke.color = paint.fill.color;
			}
			if (shadow.size) { // no need blur if size is zero, only offset
				filter.type = PaintFilter::kBlur_Type;
				filter.val0 = shadow.size;
				paint.filter = &filter;
			}
			shadow.x += inOffset.x();
			shadow.y += inOffset.y();
			for (auto i: blob_visible) {
				auto &blob = blobs[i];
				auto &line = lines->line(blob.line);
				_canvas->drawTextBlob(&blob.blob, {
					line.origin + blob.origin + shadow.x, line.baseline + shadow.y
				}, size, paint);
			}
		}

		// draw text blob
		if (v->text_color().value.a()) {
			Paint paint;
			paint.fill.color = v->text_color().value.premul_alpha().mul(_color);
			auto stroke = v->text_stroke();
			if (stroke.value.width) {
				paint.style = Paint::kStrokeAndFill_Style;
				paint.stroke.color = stroke.value.color.premul_alpha().mul(_color);
				paint.strokeWidth = stroke.value.width;
			}
			for (auto i: blob_visible) {
				auto &blob = blobs[i];
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
		auto v = view->_first.load(std::memory_order_acquire);
		if (!v) return;
		auto lastColor = _color; // save parent color
		auto lastMarkRecursive = _mark_recursive; // save parent recursive mark
		do {
			if (v->_visible) {
				uint32_t mark = lastMarkRecursive | v->mark_value(); // inherit recursive
				if (mark) {
					v->solve_marks(*_matrix, view, mark);
					_mark_recursive = mark & View::kRecursive_Mark;
				}
				if (v->_visible_area) {
					// Use premultiplied alpha color
					switch (v->_cascade_color) {
						case CascadeColor::None:
							_color = v->_color.premul_alpha(); break;
						case CascadeColor::Alpha:
							_color = v->_color.premul_alpha().mul_alpha_only(lastColor.a()); break;
						case CascadeColor::Color:
							_color = v->_color.premul_alpha().mul_rgb_only(lastColor); break;
						case CascadeColor::Both:
							_color = v->_color.premul_alpha().mul(lastColor); break;
					}
					if (Qk_LIKELY(v->_z_index == 0)) {
						v->draw(this); // draw immediately
					} else {
						// commit delay draw command
						_delayCmds->insert({v->_z_index, { v, _matrix, _color, _mark_recursive }});
					}
				}
			}
			v = v->_next.load(std::memory_order_acquire);
		} while(v);
		_color = lastColor; // restore parent color
		_mark_recursive = lastMarkRecursive; // restore parent recursive mark
	}

	void Painter::visitView(View* v, cMat* mat) {
		if (v->_first.load(std::memory_order_relaxed)) {
			auto lastMatrix = _matrix;
			set_matrix(mat);
			visitView(v);
			set_matrix(lastMatrix);
		}
	}

	typedef void (*DrawCallback)(Painter *drawer, Box *v);

	void Painter::visitAndClipBox(Box *v, DrawCallback cb) {
		getInsideRectPath(v);
		_canvas->save();
		_canvas->clipPathv(*_boxData.inside, Canvas::kIntersect_ClipOp, v->_aa); // clip
		_window->clipRange(region_aabb_from_convex_quadrilateral(v->_boxBounds));
		_delayCmds = &_delayCmdsStack.push(DelayCmdMap(
			std::less<uint32_t>(), STLAllocator<DelayCmdKV>(&_delayCmdsAllocator)
		));
		if (cb)
			cb(this, v);
		visitView(v); // draw children views
		flushDelayDrawCommands();
		_delayCmdsStack.pop();
		_delayCmds = &_delayCmdsStack.back();
		_window->clipRestore();
		_canvas->restore(); // cancel clip
	}

	void Painter::visitBox(Box *v) {
		if (v->_clip) {
			if (v->_first.load())
				visitAndClipBox(v, nullptr);
		} else {
			visitView(v);
		}
	}

	void Painter::flushDelayDrawCommands() {
		if (_delayCmds->size() == 0)
			return;
		auto lastMatrix = _matrix; // save matrix
		auto lastColor = _color; // save color
		auto lastMarkRecursive = _mark_recursive; // save recursive mark
		do {
			// move cmds to avoid re-entrance issue
			DelayCmdMap cmds(std::move(*_delayCmds));
			for (auto &it: cmds) {
				auto &cmd = it.second;
				set_matrix(cmd.matrix);
				_color = cmd.color;
				_mark_recursive = cmd.mark_recursive;
				cmd.view->draw(this);
			}
		} while(_delayCmds->size());
		_mark_recursive = lastMarkRecursive; // restore last recursive mark
		_color = lastColor; // restore last color
		_matrix = lastMatrix; // restore last matrix
	}

	//////////////////////////////////////////////////////////

	void View::draw(Painter *draw) {
		draw->visitView(this);
	}

	void Box::draw(Painter *draw) {
		draw->resetBoxData(); // reset box data
		draw->canvas()->setTranslate(position());
		draw->drawBoxBasic(this);
		draw->visitBox(this);
	}

	void Image::draw(Painter *draw) {
		draw->resetBoxData();
		draw->canvas()->setTranslate(position());
		draw->drawBoxBasic(this);

		auto src = source();
		if (src && src->load()) {
			draw->getInsideRectPath(this);
			Paint paint;
			PaintImage img;
			paint.antiAlias = aa();
			paint.fill.image = &img;
			paint.fill.color = draw->color();
			if (!src->premultipliedAlpha()) {
				paint.blendMode = kSrcOver_BlendMode;
				paint.fill.color = paint.fill.color.recover_unpremul_alpha();
			}
			//img.tileModeX = PaintImage::kDecal_TileMode;
			//img.tileModeY = PaintImage::kDecal_TileMode;
			img.filterMode = default_FilterMode;
			img.mipmapMode = default_MipmapMode;
			img.setImage(src.get(), draw->boxData().inside->rect);
			draw->canvas()->drawPathv(*draw->boxData().inside, paint);
		}
		draw->visitBox(this);
	}

	void Scroll::draw(Painter *draw) {
		Box::draw(draw);
		draw->canvas()->setTranslate(position());
		draw->drawScrollBar(this);
	}

	void Text::draw(Painter *painter) {
		painter->resetBoxData(); // reset box data
		painter->canvas()->setTranslate(position());
		painter->drawBoxBasic(this);

		struct Fn {
			static Vec2 getOffset(Text *self) {
				Vec2 offset(self->padding_left(), self->padding_top());
				_IfBorder(self) {
					offset[0] += _border->width[3];
					offset[1] += _border->width[0];
				}
				return offset;
			};
		};
		auto drawText = _blob_visible.length() != 0;

		if (clip()) {
			if (drawText || first()) {
				painter->visitAndClipBox(this, drawText ? [](Painter *painter, Box *v) {
					auto self = static_cast<Text*>(v);
					painter->drawTextBlob(self,
						Fn::getOffset(self), *self->_lines, self->_blob, self->_blob_visible);
				}: DrawCallback(nullptr));
			}
		} else {
			if (drawText)
				painter->drawTextBlob(this, Fn::getOffset(this), *_lines, _blob, _blob_visible);
			painter->visitView(this);
		}
	}

	void Input::draw(Painter *draw) {
		draw->resetBoxData();
		auto canvas = draw->canvas();
		canvas->setTranslate(position());
		draw->drawBoxBasic(this);

		auto lines = *_lines;
		auto offset = input_text_offset() + Vec2(padding_left(), padding_top());
		auto twinkle = _editing && _cursor_twinkle_status;
		auto visible = _blob_visible.length();
		auto clip = this->clip() && (visible || twinkle);

		if (clip) {
			draw->getInsideRectPath(this);
			canvas->save();
			canvas->clipPathv(*draw->boxData().inside, Canvas::kIntersect_ClipOp, aa()); // clip
		}

		if (visible) {
			auto draw_background = [&](TextBlob &blob, const Color4f &color) {
				auto &line = lines->line(blob.line);
				auto x = offset.x() + line.origin + blob.origin;
				auto y = offset.y() + line.baseline - blob.ascent;
				auto offset_x = blob.blob.offset.front().x();
				auto width = blob.blob.offset.back().x();
				auto &rect = draw->cache()->getRectPath({{x + offset_x, y},{width, blob.height}});
				canvas->drawPathvColor(rect, color, defaultBlendMode, true);
			};
			auto size = text_size().value;
			auto shadow = text_shadow().value;

			// draw text background
			if (text_length() && text_background_color().value.a()) {
				auto color = text_background_color().value.premul_alpha().mul(draw->color());
				for (auto i: _blob_visible)
					draw_background(_blob[i], color);
			}
			
			// draw text marked
			auto begin = _marked_blob_begin;
			if (begin < _marked_blob_end && _marked_color.a()) {
				auto color = _marked_color.premul_alpha().mul(draw->color());
				do
					draw_background(_blob[begin++], color);
				while(begin < _marked_blob_end);
			}

			// draw text shadow
			if (shadow.color.a()) {
				Paint paint;
				PaintFilter filter;
				paint.fill.color = shadow.color.premul_alpha().mul(draw->color());
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
				paint.fill.color = color.premul_alpha().mul(draw->color());
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
			canvas->drawPathvColor(rect, _cursor_color.premul_alpha().mul(draw->color()), defaultBlendMode, true);
		}

		if (clip) {
			canvas->restore(); // cancel clip
		}

		if ( is_multiline() ) {
			draw->drawScrollBar(static_cast<Textarea*>(this));
		}
	}

	void Label::draw(Painter *painter) {
		if (_blob_visible.length()) {
			painter->canvas()->setTranslate(position()); // set position in matrix
			painter->drawTextBlob(this, {}, *_lines, _blob, _blob_visible);
		}
		painter->visitView(this);
	}

	void Morph::draw(Painter *painter) {
		painter->resetBoxData();
		auto lastMatrix = painter->matrix();
		auto lastOrigin = painter->origin();
		painter->set_matrix(&matrix());
		painter->set_origin(_origin_value);
		painter->drawBoxBasic(this);
		if (clip())
			painter->getInsideRectPath(this);
		painter->set_origin_reverse(lastOrigin); // restore origin
		painter->visitBox(this); // clip box
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
			if (_visible_area && color().a() != 0) {
				painter->_delayCmds = &painter->_delayCmdsStack.back();
				painter->_tempAllocator[0].reset(); // reset temp allocator
				painter->_tempAllocator[1].reset(); // reset temp allocator
				painter->_delayCmdsAllocator.reset(); // reset delay cmds allocator
				painter->resetBoxData();
				// Fix rect aa stroke width
				auto AAShrink_half = painter->_isMsaa ? 0: 0.43f / _window->scale(); // fix aa stroke width, 0.4-0.5
				//auto AAShrink_half = painter->_isMsaa ? 0: 0.5f / _window->scale();
				auto AAShrink = AAShrink_half + AAShrink_half;
				auto AAShrinkBorder_half = painter->_isMsaa ? 0: 0.02f / _window->scale(); // plus 0.02 to avoid aa gap
				painter->_AAShrink = AAShrink;
				painter->_AAShrinkHalf = AAShrink_half;
				painter->_AAShrinkBorder = AAShrink + AAShrinkBorder_half + AAShrinkBorder_half;
				painter->_color = color().premul_alpha();
				painter->set_origin(origin_value());
				painter->set_matrix(&matrix());
				// The root background is not pre-multiplied, the color is drawn directly.
				canvas->clearColor(background_color().mul_color4f(color().to_color4f()));
				painter->drawBoxFill(this);
				painter->drawBoxBorder(this);
				painter->set_origin({}); // reset origin
				painter->visitView(this);
				painter->flushDelayDrawCommands(); // flush delay draw commands
			} else {
				canvas->clearColor(Color4f(0,0,0,0));
			}
			painter->_mark_recursive = 0;
		}
	}

}
