/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
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

#include <math.h>
#include "../util/loop.h"
#include "../util/codec.h"
#include "../app.h"
#include "../display.h"
#include "./render.h"
#include "./gl/gl_render.h"
#include "../layout/root.h"
#include "../filter.h"
#include "../layout/image.h"
#include "../layout/flex.h"
#include "../layout/float.h"
#include "../layout/flow.h"
#include "../layout/text.h"
#include "../layout/button.h"
#include "../layout/input.h"
#include "../layout/textarea.h"

namespace qk {

	static uint32_t integerExp(uint32_t n) {
		return (uint32_t) powf(2, floor(log2(n)));
	}

	static uint32_t massSample(uint32_t n) {
		n = integerExp(n);
		return Qk_MIN(n, 8);
	}

	Qk_DEFINE_INLINE_MEMBERS(RenderBackend, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<RenderBackend::Inl*>(self)

		Rect insideRect(Box* box) {
			Rect rect{-box->_origin_value, box->_client_size};
			if (box->_border) {
				rect.origin[0] += box->_border[3].width; // left
				rect.origin[1] += box->_border[0].width; // top
				rect.size[0] -= (box->_border[3].width + box->_border[1].width); // left + right
				rect.size[1] -= (box->_border[0].width + box->_border[2].width); // top + bottom
			}
			return rect;
		}

		const RectPath* makeOutsideRectPath(Box *box, const RectPath *&out) {
			if (!out)
				out = &getRRectPath({-box->_origin_value, box->_client_size}, &box->_radius_left_top);
			return out;
		}

		const RectPath* makeInsideRectPath(Box *box, const RectPath *&out) {
			if (!out) {
				if (box->_border) {
					auto rect = insideRect(box);
					const float border[4] = {
						box->_border[0].width,box->_border[1].width,
						box->_border[2].width,box->_border[3].width,
					};
					out = &getInsideRRectPath(rect, &box->_radius_left_top, border);
				} else {
					out = &getRRectPath({-box->_origin_value, box->_client_size}, &box->_radius_left_top);
				}
			}
			return out;
		}

		void drawBoxColor(Box *box, const RectPath *&outside) {
			auto rect = makeOutsideRectPath(box, outside);
			_canvas->drawRectPathColor(*rect,
				box->_background_color.to_color4f_alpha(_opacity), kSrcOver_BlendMode
			);
		}

		void drawBoxFill(Box *box, const RectPath *&outside) {
			auto rect = makeOutsideRectPath(box, outside);
			auto fill = box->background();
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

		void drawBoxFillImage(Box *box, FillImage *fill, const RectPath *rect) {
			auto src = fill->source();
			if (!src || !src->load()) return;

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
				w = src_w / shared_app()->display()->atom_pixel();
				h = src_h / shared_app()->display()->atom_pixel();
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

			_canvas->drawRectPath(*rect, paint);
		}

		void drawBoxFillLinear(Box *box, FillGradientLinear *fill, const RectPath *rect) {
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
			auto _rect_inside = insideRect(box);
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

			_canvas->drawRectPath(*rect, paint);
		}

		void drawBoxFillRadial(Box *box, FillGradientRadial *fill, const RectPath *rect) {
			auto &colors = fill->colors();
			auto &pos = fill->positions();
			auto _rect_inside = insideRect(box);
			Vec2 radius{_rect_inside.size.x() * 0.5f, _rect_inside.size.y() * 0.5f};
			Vec2 center = _rect_inside.origin + radius;
			Paint paint;
			paint.color.set_a(_opacity);
			Gradient g{&fill->colors(), &fill->positions(), center, radius};
			paint.setGradient(Paint::kLinear_GradientType, &g);
			_canvas->drawRectPath(*rect, paint);
		}

		void drawBoxShadow(Box *box, const RectPath *&outside) {
			auto shadow = box->box_shadow();
			do {
				if (shadow->type() == Filter::M_SHADOW) {
				}
				shadow = shadow->next();
			} while(shadow);
		}

		void drawBoxBorder(Box *box) {
			// TODO: RectOutlinePath
		}

		void drawBoxEnd(Box *box, const RectPath *inside = nullptr) {
			if (box->_is_clip) {
				if (box->_first) {
					_this->makeInsideRectPath(box, inside);
					_canvas->clipRectPath(*inside, Canvas::kIntersect_ClipOp, true); // clip
					Render::visitView(box);
					_canvas->restore(); // cancel clip
				}
			} else {
				Render::visitView(box);
			}
		}

	};

	RenderBackend::RenderBackend(Options opts)
		: _opts(opts)
		, _canvas(nullptr)
		, _delegate(nullptr)
		, _default_scale(1)
		, _opacity(1), _mark_recursive(0)
	{
		_opts.colorType = _opts.colorType ? _opts.colorType: kColor_Type_RGBA_8888;//kColor_Type_BGRA_8888;
		_opts.msaaSampleCnt = massSample(_opts.msaaSampleCnt);
	}

	RenderBackend::~RenderBackend() {}

	void RenderBackend::activate(bool isActive) {
	}

	const Array<Vec2>& RenderBackend::getPathTriangles(const Path &path) {
		auto hash = path.hashCode();
		const Array<Vec2> *out;
		if (_PathTrianglesCache.get(hash, out)) return *out;
		if (_PathTrianglesCache.length() >= 1024)
			_PathTrianglesCache.clear();
		return _PathTrianglesCache.set(hash, path.getTriangles(1));
	}

	const Path& RenderBackend::getStrokePath(
		const Path &path, float width, Path::Cap cap, Path::Join join, float miterLimit) 
	{
		auto hash = path.hashCode();
		auto hash_part = ((*(int64_t*)&width) << 32) | *(int32_t*)&miterLimit;
		hash += (hash << 5) + hash_part + ((cap << 2) | join);
		const Path *out;
		if (_StrokePathCache.get(hash, out)) return *out;
		if (_StrokePathCache.length() >= 1024)
			_StrokePathCache.clear();
		auto stroke = path.strokePath(width,cap,join,miterLimit);
		return _StrokePathCache.set(hash, stroke.isNormalized() ? std::move(stroke): stroke.normalizedPath(1));
	}

	const Array<Vec3>& RenderBackend::getSDFStrokeTriangleStrip(const Path &path, float width) {
		auto hash = path.hashCode();
		hash += (hash << 5) + *(int32_t*)&width;
		const Array<Vec3> *out;
		if (_SDFStrokeTriangleStripCache.get(hash, out)) return *out;
		if (_SDFStrokeTriangleStripCache.length() >= 1024)
			_SDFStrokeTriangleStripCache.clear();
		return _SDFStrokeTriangleStripCache.set(hash, path.getSDFStrokeTriangleStrip(width, 1));
	}

	const Path& RenderBackend::getNormalizedPath(const Path &path) {
		if (path.isNormalized()) return path;
		auto hash = path.hashCode();
		const Path *out;
		if (_NormalizedPathCache.get(hash, out)) return *out;
		if (_NormalizedPathCache.length() >= 1024)
			_NormalizedPathCache.clear();
		return _NormalizedPathCache.set(hash, path.normalizedPath(1));
	}

	const RectPath& RenderBackend::getRectPath(const Rect &rect) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		const RectPath *out;
		if (_RectPathCache.get(hash.hashCode(), out)) return *out;
		if (_RectPathCache.length() >= 1024)
			_RectPathCache.clear();
		return _RectPathCache.set(hash.hashCode(), RectPath::MakeRect(rect));
	}

	const RectPath& RenderBackend::getRRectPath(const Rect &rect, const float radius[4]) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		hash.updatefv4(radius);
		const RectPath *out;
		if (_RectPathCache.get(hash.hashCode(), out)) return *out;
		if (_RectPathCache.length() >= 1024)
			_RectPathCache.clear();

		if (radius[0] == 0 && radius[0] == radius[1] &&
			radius[1] == radius[2] && radius[2] == radius[3])
		{
			return _RectPathCache.set(hash.hashCode(), RectPath::MakeRect(rect));
		} else {
			return _RectPathCache.set(hash.hashCode(), RectPath::MakeRRect(rect, {
				radius[0],radius[1],radius[2],radius[3]
			}));
		}
	}

	const RectPath& RenderBackend::getInsideRRectPath(const Rect &rect, const float radius[4], const float border[4]) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		hash.updatefv4(radius);
		hash.updatefv4(border);
		const RectPath *out;
		if (_RectPathCache.get(hash.hashCode(), out)) return *out;
		if (_RectPathCache.length() >= 1024)
			_RectPathCache.clear();
		return _RectPathCache.set(hash.hashCode(), RectPath::MakeRRect(rect, {
			{Float::max(radius[0]-border[3],0), Float::max(radius[0]-border[0],0)},
			{Float::max(radius[1]-border[1],0), Float::max(radius[1]-border[0],0)},
			{Float::max(radius[2]-border[1],0), Float::max(radius[2]-border[2],0)},
			{Float::max(radius[3]-border[3],0), Float::max(radius[3]-border[2],0)},
		}));
	}

	const RectPath& RenderBackend::getRRectPath(const Rect &rect, const Path::BorderRadius &radius) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		hash.updatefv4(radius.leftTop.val);
		hash.updatefv4(radius.rightBottom.val);
		const RectPath *out;
		if (_RectPathCache.get(hash.hashCode(), out)) return *out;
		if (_RectPathCache.length() >= 1024)
			_RectPathCache.clear();
		return _RectPathCache.set(hash.hashCode(), RectPath::MakeRRect(rect, radius));
	}

	// --------------------------------------------------------------------------

	void RenderBackend::visitView(View* view) {
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

	void RenderBackend::visitBox(Box* box) {
		_canvas->setMatrix(box->matrix());
		const RectPath *outside = nullptr;
		if (box->_background_color.a())
			_this->drawBoxColor(box, outside);
		if (box->background())
			_this->drawBoxFill(box, outside);
		if (box->box_shadow())
			_this->drawBoxShadow(box, outside);
		if (box->_border)
			_this->drawBoxBorder(box);
		_this->drawBoxEnd(box);
	}

	void RenderBackend::visitImage(Image* v) {
		_canvas->setMatrix(v->matrix());

		const RectPath *outside = nullptr;
		const RectPath *inside = nullptr;
		if (v->_background_color.a())
			_this->drawBoxColor(v, outside);
		if (v->background())
			_this->drawBoxFill(v, outside);

		auto src = v->source();
		if (src && src->load()) {
			_this->makeInsideRectPath(v, inside);
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
			_canvas->drawRectPath(*inside, paint);
		}
		if (v->box_shadow())
			_this->drawBoxShadow(v, outside);
		if (v->_border)
			_this->drawBoxBorder(v);
		_this->drawBoxEnd(v, inside);
	}

	void RenderBackend::visitVideo(Video* video) {
		//RenderBackend::visitBox(video);
	}

	void RenderBackend::visitScroll(Scroll* scroll) {
		// TODO ...
	}

	void RenderBackend::visitInput(Input* input) {
		// TODO ...
	}

	void RenderBackend::visitLabel(Label* label) {
		// TODO ...
	}

	void RenderBackend::visitTextarea(Textarea* textarea) {
		RenderBackend::visitInput(textarea);
	}

	void RenderBackend::visitButton(Button* btn) {
		RenderBackend::visitBox(btn);
	}

	void RenderBackend::visitTextLayout(TextLayout* text) {
		RenderBackend::visitBox(text);
	}

	void RenderBackend::visitFloatLayout(FloatLayout* box) {
		RenderBackend::visitBox(box);
	}

	void RenderBackend::visitFlowLayout(FlowLayout* flow) {
		RenderBackend::visitBox(flow);
	}

	void RenderBackend::visitFlexLayout(FlexLayout* flex) {
		RenderBackend::visitBox(flex);
	}

	void RenderBackend::visitRoot(Root* v) {
		if (v->_visible) {
			uint32_t mark = v->layout_mark();
			if (mark) {
				v->solve_marks(mark);
				_mark_recursive = mark & Layout::kRecursive_Mark;
			}
			if (v->_visible_region && v->_opacity != 0) {
				_canvas->setMatrix(v->matrix());
				_canvas->clearColor(v->_background_color.to_color4f());
				const RectPath *outside = nullptr;
				if (v->background())
					_this->drawBoxFill(v, outside);
				if (v->box_shadow())
					_this->drawBoxShadow(v, outside);
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
