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

#include "./gpu_canvas.h"
#include "./gpu_canvas_filter.h"
#include "src/util/object.h"

namespace qk {
	float get_level_font_size(float fontSize);
	void setTexUnsafe_SourceImage(ImageSource* img, const TexStat *tex);
	uint32_t upPow2(uint32_t size);

	typedef Typeface::TextImage TextImage;

	//---------------------------------------------------------------------

	Qk_DEFINE_INLINE_MEMBERS(GPUCanvas, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<GPUCanvas::Inl*>(self)

		void computeScale(const Mat& mat) {
			if (mat.is_translation_matrix()) { // is translation matrix only
				if (_scale != Vec2(1.0f)) {
					_scale = 1.0f;
					_scaleAverage = 1.0f;
					_allScaleAverage = _surfaceScaleAverage;
					_allScaleMin = _surfaceScaleAverage;
					_1pxSize = 1.0f / _allScaleMin;
					_aaRadius = _1pxSize * 0.6; // 0.6px
					_aaRadiusRect = _1pxSize * 0.5; // 0.5px
				}
			} else {
				Vec2 scale(Vec2(mat[0], mat[3]).length(), Vec2(mat[1], mat[4]).length());
				if (_scale != scale) {
					_scale = scale;
					_scaleAverage = sqrtf(scale.x() * scale.y());
					_allScaleAverage = _surfaceScaleAverage * _scaleAverage;
					_allScaleMin = _surfaceScaleAverage * F32::min(scale.x(), scale.y());
					_1pxSize = 1.0f / _allScaleMin;
					_aaRadius = _1pxSize * 0.75;
					_aaRadiusRect = _1pxSize * 0.75;
				}
			}
		}

		const VertexData &getVertex(const Path &path, float aaRadius, bool aa) {
			return aa ?
				_cache->getAASideTriangle(path, aaRadius): _cache->getPathTriangles(path);
		}

		float drawTextImage(TextImage &img, float scale, Vec2 origin, const Paint &paint) {
			auto pix = img.image->pixel(0);
			auto scale_1 = 1.0f / scale;
			PaintImage p;
			// Default use baseline align
			Vec2 dst_start(origin.x() - img.left * scale_1, origin.y() - img.top * scale_1);
			Vec2 dst_size(pix->width() * scale_1, pix->height() * scale_1);
			Rect rect{dst_start, dst_size};

			p.setImage(*img.image, rect);
			p.mipmapMode = PaintImage::kLinear_MipmapMode;
			p.filterMode = PaintImage::kLinear_FilterMode;

			Sp<GC_Filter> filter = GC_Filter::Make(this, paint, &rect);

			Vec2 top_right(dst_start.x() + dst_size.x(), dst_start.y()); // top right
			Vec2 left_bottom(dst_start.x(), dst_start.y() + dst_size.y()); // left bottom
			Vec2 right_bottom(dst_start + dst_size); // right bottom
			VertexData vertex{0,6,{
				dst_start,
				top_right, left_bottom, // triangle 0 |/
				top_right,
				right_bottom, left_bottom, // triangle 1 /|
			}};

			if (img.image->type() == kSDF_Unsigned_F32_ColorType) { // SDF text
				auto fillColor = paint.style == Paint::kStroke_Style ? Color4f(0,0,0,0) : paint.fill.color;
				auto strokeWidth = paint.style == Paint::kFill_Style ? 0.0f: paint.strokeWidth;
				drawImageCmd(vertex, &p, fillColor, kSDFMask_DrawKind, paint.stroke.color, strokeWidth * scale);
			} else {
				drawImageCmd(vertex, &p, paint.fill.color, kMask_DrawKind);
			}

			return scale_1;
		}

		void fill(const VertexData& vertex, const Paint &paint, const PaintStyle& style) {
			if (!vertex.vCount) return;
			if (style.image) {
				drawImageCmd(vertex, style.image, style.color);
			} else if (style.gradient) {
				drawGradientCmd(vertex, style.gradient, style.color);
			} else if (paint.mask) {
				drawImageCmd(vertex, paint.mask, style.color, kMask_DrawKind);
			} else {
				drawColorCmd(vertex, style.color);
			}
		}

		void fillPathColor(const Path &path, const Color4f &color, float aaRadius, bool aa) {
			auto &vertex = getVertex(path, aaRadius, aa);
			drawColorCmd(vertex, color);
			// LinearAllocator alloc;
			// AllocatorScope scope(&alloc);
			// Mat pixelMatrix(Vec2(0), _surfaceScale, 0, Vec2(0));
			// pixelMatrix *= _state->matrix;
			// Range clip{{0,0}, _surfaceSize};
			// auto data = buildCGAADrawData(path, &clip, &pixelMatrix, _allScaleAverage * 0.5f);
			// if (data.boundaryTiles.length() || data.uniformTiles.length()) {
			// 	drawCGAAColorCmd(data, color);
			// }
		}

		void strokePath(const Path &path, const Paint& paint, float aaRadius) {
			if (!paint.antiAlias) {
				auto &stroke = _cache->getStrokePath(path, paint.strokeWidth, paint.cap, paint.join, 0);
				auto &vertex = _cache->getPathTriangles(stroke);
				fill(vertex, paint, paint.stroke);
				return;
			}
			auto width = paint.strokeWidth - _1pxSize;
			if (paint.strokeWidth > _1pxSize * 1.8) {
				auto &stroke = _cache->getStrokePath(path, width, paint.cap, paint.join,0);
				auto &vertex = getVertex(stroke, aaRadius, paint.antiAlias);
				fill(vertex, paint, paint.stroke);
			} else {
				aaRadius = _1pxSize * 0.65; // min aa radius for thin strokes
				auto radius = paint.strokeWidth * 0.56f; // bold strokes from 0.5 to 0.65 scale well
				auto stroke = paint.stroke;
				aaRadius = std::max(radius, aaRadius);
				if (radius < aaRadius) {
					auto alpha = radius / aaRadius;
					stroke.color[3] *= alpha; // approximate alpha reduction for smaller radius
				}
				auto &vertex = _cache->getAASideTriangle(path, aaRadius, true);
				_flags |= Qk_FLAG_AASIDE_LINE; // set line AA flag for stroke path
				fill(vertex, paint, stroke);
				_flags &= ~Qk_FLAG_AASIDE_LINE; // clear line AA flag after stroke path
			}
		}

		void drawPath(const Path &path0, const Paint &paint, float aaRadius) {
			auto &path = _cache->getNormalizedPath(path0);
			Sp<GC_Filter> filter = GC_Filter::Make(this, paint, &path);
			auto fillPath = [&]() {
				auto &vertex = getVertex(path, aaRadius, paint.antiAlias);
				fill(vertex, paint, paint.fill);
			};
			// gen stroke path and fill path and polygons
			switch (paint.style) {
				case Paint::kFill_Style:
					fillPath(); break;
				case Paint::kStrokeAndFill_Style:
					fillPath();
				case Paint::kStroke_Style:
					strokePath(path, paint, aaRadius); break;
			}
		}
	};

	//---------------------------------------------------------------------

	GPUCanvas::GPUCanvas(Render *render, Render::Options opts)
		: _state(nullptr), _cache(nullptr), _render(render)
		, _surfaceSize(), _surfaceScale(1)
		, _size(), _scale(1)
		, _surfaceScaleAverage(1), _scaleAverage(1), _allScaleAverage(1)
		, _allScaleMin(1)
		, _1pxSize(1)
		, _aaRadius(0.5), _aaRadiusRect(0.5)
		, _rootMatrix()
		, _blendMode(kInvalid_BlendMode)
		, _clipState(nullptr)
		, _opts(opts)
		, _flags(0)
	{
		auto capacity = opts.maxCapacityForPathvCache ?
			opts.maxCapacityForPathvCache: 128000000/*128mb*/;
		capacity = U32::clamp(capacity, 1024000/*1mb*/, 512000000/*512mb*/);
		_cache = new PathvCache(capacity, render);
		_stateStack.push({ .matrix=Mat() });
		_state = &_stateStack.back();
	}

	GPUCanvas::~GPUCanvas() {
		_texPools.clear();
		Releasep(_cache);
	}

	Sp<ImageSource> GPUCanvas::getTextureFromPool(Vec2 size, ColorType type, bool mipmap) {
		// limit texture size to surface size,
		// to avoid memory waste and performance loss on large render targets
		int w = I32::clamp(upPow2(size.x()), 1, _surfaceSize[0]);
		int h = I32::clamp(upPow2(size.y()), 1, _surfaceSize[1]);
		// limit texture aspect ratio to 4:1,
		// to avoid memory waste and performance loss on large render targets
		if (w > h * 4) h = I32::min(w / 4, _surfaceSize[1]);
		if (h > w * 4) w = I32::min(h / 4, _surfaceSize[0]);

		uint64_t key = (uint64_t(w) << 40) | (uint64_t(h) << 8) | (type << 1) | mipmap;
		auto &pool = _texPools[key];
		for (auto &tex : pool) {
			if (tex->refCount() == 1)
				return tex;
		}
		auto src = ImageSource::Make(PixelInfo{w, h, type, kPremul_AlphaType}, nullptr);
		pool.push(src.get());
		// create texture stat and set texture source
		_render->post_message(Cb([render=_render, src=src.get(), w, h, type, mipmap](auto e) {
			auto stat = render->createTextureStat(Vec2(w, h), type, mipmap);
			src->set_mipmap(mipmap);
			setTexUnsafe_SourceImage(src, &stat);
		}, src.get())); // ref src to ensure texture stat is valid when cb is called
		return src;
	}

	PathvCache* GPUCanvas::getPathvCache() {
		return _cache;
	}

	Vec2 GPUCanvas::size() {
		return _size;
	}

	bool GPUCanvas::isGpu() {
		return true;
	}

	int GPUCanvas::getSaveCount() const {
		return _stateStack.length() - 1;
	}

	const Mat& GPUCanvas::getMatrix() const {
		return _state->matrix;
	}

	void GPUCanvas::setMatrix(const Mat& mat) {
		_state->matrix = mat;
		_this->computeScale(mat);
		setMatrixCmd();
	}

	void GPUCanvas::translate(Vec2 val) {
		_state->matrix.translate(val);
		setMatrixCmd();
	}

	void GPUCanvas::setTranslate(Vec2 val) {
		_state->matrix.set_translate(val);
		setMatrixCmd();
	}

	void GPUCanvas::scale(Vec2 val) {
		_state->matrix.scale(val);
		_this->computeScale(_state->matrix);
		setMatrixCmd();
	}

	void GPUCanvas::rotate(float z) {
		_state->matrix.rotate(z);
		setMatrixCmd();
	}

	void GPUCanvas::setBlendMode(BlendMode mode) {
		if (_blendMode != mode) {
			_blendMode = mode;
			setBlendModeCmd();
		}
	}

	int GPUCanvas::save() {
		auto &back = _stateStack.back();
		// copy current state to stack, and set state pointer to new state
		_stateStack.push({.matrix=back.matrix, .clip=back.clip, .output=back.output});
		_state = &_stateStack.back();
		return _stateStack.length();
	}

	void GPUCanvas::restore(uint32_t count) {
		if (!count || _stateStack.length() == 1)
			return;
		count = U32::min(count, _stateStack.length() - 1);

		if (count > 0) {
			do {
				auto lastOut = _state->output; // save current output before pop
				_stateStack.pop(); // exit current state
				_state = &_stateStack.back();
				if (lastOut != _state->output) { // restore region draw, only when output changed
					outputImageEndCmd(lastOut.get());
				}
				count--;
			} while (count > 0);
			if (_clipState != _state->clip.get()) {
				restoreClipCmd(_state->clip.get()); // restore clip state if changed
			}
			_clipState = _state->clip.get(); // restore clip state

			// clear clip flag if no clip state
			if (!_clipState) {
				_flags &= ~Qk_FLAG_CLIP;
			}
			setMatrix(_state->matrix); // restore matrix
		}
	}

	void GPUCanvas::clipPath(const Path& path, ClipOp rawOp, bool antiAlias) {
		if (rawOp > kReplace_ClipOp) {
			Qk_DLog("Invalid ClipOp: %d, expected kIntersect_ClipOp, kDifference_ClipOp, or kReplace_ClipOp", rawOp);
			return;
		}
		const Vec2 pad = _surfaceScale; // 1 pixel pad for anti-aliasing
		auto clip = new GC_State::Clip;
		auto range = path.getBounds(&_state->matrix);
		auto lastClip = _clipState;

		if (range.begin.x() >= range.end.x() || range.begin.y() >= range.end.y()) {
			return; // skip empty clip
		}

		// for replace, we can directly use the new clip region,
		// so skip combining with last clip state
		if (rawOp == kReplace_ClipOp) {
			rawOp = kIntersect_ClipOp; // treat replace as intersect
			lastClip = nullptr; // ignore last clip state
		}
		// last clip state operation, default as intersect
		auto lastOp = lastClip ? lastClip->op : kIntersect_ClipOp;
		// last clip state range, default as surface size
		auto lastRange = lastClip ? lastClip->range : Range{{0},_surfaceSize};
		// apply surface scale and padding
		range.begin = (range.begin * _surfaceScale - pad).floor();
		range.end = (range.end * _surfaceScale + pad).ceil();
		// rawOp: requested operation for this clip command.
		// clip->op: how the resulting mask should be interpreted by fragment shader.
		clip->op = rawOp;

		// combine with last clip state
		if (rawOp == kIntersect_ClipOp) {
			if (lastOp == kIntersect_ClipOp) {
				range.begin = range.begin.max(lastRange.begin);
				range.end = range.end.min(lastRange.end);
			} else { // if (lastOp == kDifference_ClipOp)
				range.begin = range.begin.max(0);
				range.end = range.end.min(_surfaceSize);
			}
		} else { // if (rawOp == kDifference_ClipOp)
			if (!lastClip) {
				// Difference with no previous clip means full surface minus incoming.
				// Store only incoming bounds as the restricted mask area.
				range.begin = range.begin.max(0);
				range.end = range.end.min(_surfaceSize);
			} else if (lastOp == kDifference_ClipOp) {
				// expand to a larger restricted area image
				range.begin = range.begin.min(lastRange.begin).max(0);
				range.end = range.end.max(lastRange.end).min(_surfaceSize);
			} else { // if (lastOp == kIntersect_ClipOp)
				// difference with intersect is still intersect,
				// but keep the larger range for shader to do difference clipping
				clip->op = kIntersect_ClipOp;
				// keep last clip range for shader to do difference clipping
				range = lastRange;
			}
		}
		clip->mask = getTextureFromPool(range.end - range.begin, kLuminance_8_ColorType, false);
		clip->range = range;
		// adjust range to actual allocated texture size
		clip->range.end = clip->range.begin + clip->mask->size();
		if (antiAlias) {
			drawClipCmd(_cache->getAASideTriangle(path,_aaRadius), lastClip, clip, rawOp);
		} else {
			drawClipCmd(_cache->getPathTriangles(path), lastClip, clip, rawOp);
		}
		_state->clip = clip;
		_clipState = clip; // set current clip state
		_flags |= Qk_FLAG_CLIP; // set clip flag
	}

	void GPUCanvas::clipRect(const Rect& rect, ClipOp op, bool antiAlias) {
		_this->clipPath(_cache->getRectPath(rect), op, antiAlias);
	}

	void GPUCanvas::clearColor(const Color4f& color) {
		clearColorCmd(color, _stateStack.length() == 1 ? kClearAll_ClearFlags : kOnlyColor_ClearFlags);
	}

	void GPUCanvas::drawColor(const Color4f &color, BlendMode mode) {
		_this->setBlendMode(mode); // switch blend mode
		drawColorCmd({0,6, {
			{0,0,0}, {_size[0],0,0}, {_size[0],_size[1],0}, // triangle 1
			{_size[0],_size[1],0}, {0,_size[1],0}, { 0,0,0 } // triangle 2
		}}, color);
	}

	void GPUCanvas::drawRRectBlurColor(const Rect& rect,
		const float radius[4], float blur, const Color4f &color, BlendMode mode)
	{
		if (rect.size.is_zero_axis()) return;
		_this->setBlendMode(mode); // switch blend mode
		drawRRectBlurColorCmd(rect, radius, blur, color);
	}

	void GPUCanvas::drawPathColor(const Path& path, const Color4f &color, BlendMode mode, bool antiAlias) {
		_this->setBlendMode(mode); // switch blend mode
		_this->fillPathColor(path, color, _aaRadius, antiAlias);
	}

	void GPUCanvas::drawPath(const Path &path, const Paint &paint) {
		_this->drawPath(path, paint, _aaRadius);
	}

	void GPUCanvas::drawRect(const Rect& rect, const Paint& paint) {
		_this->drawPath(_cache->getRectPath(rect), paint, _aaRadiusRect);
	}

	void GPUCanvas::drawRRect(const Rect& rect, const Path::BorderRadius &radius, const Paint& paint) {
		_this->drawPath(_cache->getRRectPath(rect,radius), paint, _aaRadiusRect);
	}

	void GPUCanvas::drawRectPath(const RectPath& path, const Paint& paint) {
		_this->drawPath(path, paint, _aaRadiusRect);
	}

	void GPUCanvas::drawPathColors(const Path* paths[], int count, const Color4f &color, BlendMode mode, bool antiAlias) {
		_this->setBlendMode(mode); // switch blend mode
		for (int i = 0; i < count; i++) {
			_this->fillPathColor(*paths[i], color, _aaRadius, antiAlias);
		}
	}

	void GPUCanvas::drawRectOutlinePath(const RectOutlinePath& rect, const Color4f color[4], const Paint& paint) {
		auto newPaint = paint;
		auto baseColor = paint.fill.color;
		newPaint.style = Paint::kFill_Style; // only fill for outline, no stroke
		for (int i = 0; i < 4; i++) {
			if (rect.flags & (1 << i)) { // if this edge is visible
				newPaint.fill.color = baseColor.mul(color[i]);
				_this->drawPath((&rect.top)[i], newPaint, _aaRadiusRect);
			}
		}
	}

	float GPUCanvas::drawGlyphs(const FontGlyphs &glyphs, Vec2 origin, cArray<Vec2> *offsetIn, const Paint &paint) {
		Array<Vec2> offset, *offsetP = nullptr;
		if (offsetIn) {
			offset = *offsetIn;
			offsetP = &offset;
			for (auto &o: offset) o *= _allScaleAverage;
		}
		auto isSDF = paint.style != Paint::kFill_Style;
		auto tf = glyphs.typeface();
		auto img = isSDF ?
			tf->getSDFImage(glyphs.glyphs(), glyphs.fontSize() * _allScaleAverage, offsetP, false):
			tf->getImage(glyphs.glyphs(), glyphs.fontSize() * _allScaleAverage, offsetP);
		img.image->set_mipmap(false); // disable mipmap for text
		auto scale = _this->drawTextImage(img, _allScaleAverage, origin, paint);
		return scale * img.width;
	}

	void GPUCanvas::drawTextBlob(TextBlob *blob, Vec2 origin, float fontSize, const Paint &paint) {
		auto fixedFSize = get_level_font_size(_scaleAverage * fontSize) * _surfaceScaleAverage;
		// auto fixedFSize = fontSize * _allScaleAverage;
		if (fixedFSize == 0.0)
			return;
		auto scale = fixedFSize / fontSize; // scale from original font size to fixed font size
		auto isSDF = paint.style != Paint::kFill_Style;

		if (blob->img.fontSize != fixedFSize || !blob->img.image ||
			(isSDF ? blob->img.image->type() != kSDF_Unsigned_F32_ColorType: false)
		) { // fill text bolb
			Array<Vec2> offset;
			if (blob->offset.length() >= blob->glyphs.length()) {
				offset = blob->offset;
				for (auto &o: offset) o *= scale;
			}
			blob->img = isSDF ?
				blob->typeface->getSDFImage(blob->glyphs, fixedFSize, &offset, false):
				blob->typeface->getImage(blob->glyphs, fixedFSize, &offset);
			blob->img.image->set_mipmap(false); // disable mipmap for text
			blob->img.scale *= scale;
		}
		auto img = blob->img.image.get();
		if (img->width() && img->height()) {
			Qk_ASSERT(img->count(), "GLCanvas::drawTextBlob img->count()");
			_this->drawTextImage(blob->img, scale, origin, paint);
		}
	}

	void GPUCanvas::drawTriangles(const Triangles& triangles, const Paint &paint, bool copyData) {
		_this->setBlendMode(paint.blendMode); // switch blend mode
		drawTrianglesCmd(triangles, paint.fill.image, paint.fill.color, copyData);
	}

	Sp<ImageSource> GPUCanvas::readImage(const Rect &src, Vec2 dst, ColorType type, BlendMode mode, bool mipmap) {
		_this->setBlendMode(mode); // switch blend mode
		auto o = src.begin;
		auto s = Vec2{
			F32::min(o.x()+src.size.x(), _size.x()) - o.x(),
			F32::min(o.y()+src.size.y(), _size.y()) - o.y()
		};
		if (s[0] > 0 && s[1] > 0 && dst[0] > 0 && dst[1] > 0) {
			type = type ? type: _opts.colorType; // default to surface color type
			auto destImg = ImageSource::Make(PixelInfo{
				int(Qk_Min(dst.x(),_surfaceSize.x())), // limit max read image size to surface size
				int(Qk_Min(dst.y(),_surfaceSize.y())), type, kPremul_AlphaType
			});
			destImg->set_mipmap(mipmap);
			readImageCmd({o*_surfaceScale,s*_surfaceScale}, _state->output.get(), destImg.get());
			return destImg;
		}
		return nullptr;
	}

	Sp<ImageSource> GPUCanvas::outputImage(ImageSource* dst, bool mipmap) {
		Sp<ImageSource> img(dst);
		if (!dst) {
			img = ImageSource::Make(PixelInfo{
				int(_surfaceSize[0]), int(_surfaceSize[1]), _opts.colorType, kPremul_AlphaType
			});
		}
		if (img == _state->output)
			return img; // same image, no need to switch
		_state->output = img;
		img->set_mipmap(mipmap);
		outputImageBeginCmd(img.get());
		Qk_ReturnLocal(img);
	}

	void GPUCanvas::setSurface(const Mat4& root, Vec2 surfaceSize, Vec2 surfaceScale) {
		Qk_ASSERT_GT(surfaceSize.x(), 0, "Invalid surface size width");
		Qk_ASSERT_GT(surfaceSize.y(), 0, "Invalid surface size height");
		auto chSize = surfaceSize != _surfaceSize;
		// clear all state
		_stateStack.clear();
		_stateStack.push({ .matrix=Mat() });
		_state = &_stateStack.back(); // reset state
		_clipState = _state->clip.get(); // current clip state
		// set surface data
		_surfaceSize = surfaceSize;
		_surfaceScale = surfaceScale;
		_size = surfaceSize / surfaceScale;
		_surfaceScaleAverage = sqrtf(surfaceScale.x() * surfaceScale.y());
		_scale = 0; // force computeScale()
		_this->computeScale(_state->matrix);
		_rootMatrix = root;
		_texPools.clear(); // clear texture pool when surface size changed

		Qk_DLog("setSurface: %f, %f", _surfaceSize.x(), _surfaceSize.y());

		setSurfaceCmd(chSize); // set buffers
	}
}
