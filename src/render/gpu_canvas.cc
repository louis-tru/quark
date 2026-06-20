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

namespace qk {
	float get_level_font_size(float fontSize);
	uint32_t upPow2(uint32_t size);

	typedef Typeface::TextImage TextImage;

	//---------------------------------------------------------------------

	Qk_DEFINE_INLINE_MEMBERS(GPUCanvas, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<GPUCanvas::Inl*>(self)

		void computeScale(const Mat& mat) {
			if (mat.is_translate_only()) { // is translation matrix only
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

		bool isSDFImage(ImageSource* img) const {
			return img && (img->type() == kSDF_Unsigned_F32_ColorType || img->type() == kSDF_F32_ColorType);
		}

		void commitCAPABatch() {
			if (_capaBuilder)
				_capaBuilder->commit();
		}

		const VertexData &buildVertex(const Path &path, float aaRadius, bool aa) {
			return aa ?
				_cache->getAASideTriangle(path, aaRadius): _cache->getPathTriangles(path);
		}

		cCAPADrawData& buildCAPAA(const Path &path, const Paint &paint, bool stroke) {
			Qk_ASSERT(_capaBuilder, "CAPA builder is null");
			_capaBuilder->commit();
			_capaBuilder->build(stroke ?
				_cache->getStrokePath(path, paint.strokeWidth, paint.cap, paint.join, 0) : path);
			auto &data = _capaBuilder->endBuild();
			makeCAPAAtlasCmd(data);
			return data;
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

			_this->commitCAPABatch(); // commit current CAPA batch

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

			if (isSDFImage(img.image.get())) { // SDF text
				auto fillColor = paint.style == Paint::kStroke_Style ? Color4f(0,0,0,0) : paint.fill.color;
				auto strokeWidth = paint.style == Paint::kFill_Style ? 0.0f: paint.strokeWidth;
				drawImageCmd(vertex, { &p, fillColor, kSDFMask_DrawKind, paint.stroke.color, strokeWidth * scale});
			} else {
				drawImageCmd(vertex, { &p, paint.fill.color, kMask_DrawKind});
			}

			return scale_1;
		}

		void fillPathAASide(const VertexData& vertex, const Paint &paint, const PaintStyle& style) {
			if (!vertex.vCount)
				return;
			commitCAPABatch(); // commit current CAPA batch before fill path
			if (style.image) {
				auto isSDF = isSDFImage(style.image->image);
				drawImageCmd(vertex, { style.image, style.color, isSDF ? kSDFMask_DrawKind : kImage_DrawKind });
			} else if (style.gradient) {
				drawGradientCmd(vertex, style.gradient, style.color);
			} else if (paint.mask) {
				drawImageCmd(vertex, { paint.mask, style.color, kMask_DrawKind });
			} else {
				drawColorCmd(vertex, style.color);
			}
		}

		bool fillPathCAPA(const Path &path, const Paint &paint, const PaintStyle& style, bool stroke) {
			if (!_capaBuilder)
				return false;
			if (style.image) {
				auto isSDF = isSDFImage(style.image->image);
				if (!isSDF && !style.image->_isCanvas && style.image->image->type() == kYUV420P_Y_8_ColorType)
					return false; // fallback to non-CAPA path for YUV420P image
				drawCAPAImageCmd(buildCAPAA(path, paint, stroke), {
					style.image, style.color, isSDF ? kSDFMask_DrawKind : kImage_DrawKind
				});
			} else if (style.gradient) {
				drawCAPAGradientCmd(buildCAPAA(path, paint, stroke), style.gradient, style.color);
			} else if (paint.mask) {
				drawCAPAImageCmd(buildCAPAA(path, paint, stroke), { paint.mask, style.color, kMask_DrawKind });
			} else { // color fill/stroke with CAPA
				_capaBuilder->color = style.color;
				_capaBuilder->build(stroke ?
					_cache->getStrokePath(path, paint.strokeWidth, paint.cap, paint.join, 0) : path);
				return true;
			}
			_capaBuilder->reset();
			return true;
		}

		void fillPathColor(const Path &path, const Color4f &color, float aaRadius, bool aa) {
			if (_capaBuilder && aa) {
				_capaBuilder->color = color;
				_capaBuilder->build(path); // build CAPA data for path
			} else {
				commitCAPABatch();
				drawColorCmd(buildVertex(path, aaRadius, aa), color);
			}
		}

		void strokePath(const Path &path, const Paint& paint, float aaRadius) {
			if (!paint.antiAlias) {
				auto &stroke = _cache->getStrokePath(path, paint.strokeWidth, paint.cap, paint.join, 0);
				auto &vertex = _cache->getPathTriangles(stroke);
				fillPathAASide(vertex, paint, paint.stroke);
				return;
			}
			auto width = paint.strokeWidth - _1pxSize;
			if (paint.strokeWidth > _1pxSize * 1.8) {
				// Stroke is currently implemented using aaside
				// because CAPA doesn't work well for wireframes with small lines,
				// but we can't rule out the possibility that future algorithm improvements
				// will allow wireframes to also be implemented using CAPA.
				// if (fillPathCAPA(path, paint, paint.stroke, true)) return;
				auto &stroke = _cache->getStrokePath(path, width, paint.cap, paint.join,0);
				auto &vertex = buildVertex(stroke, aaRadius, paint.antiAlias);
				fillPathAASide(vertex, paint, paint.stroke);
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
				fillPathAASide(vertex, paint, stroke);
				_flags &= ~Qk_FLAG_AASIDE_LINE; // clear line AA flag after stroke path
			}
		}

		void drawPath(const Path &path, const Paint &paint, float aaRadius) {
			Sp<GC_Filter> filter = GC_Filter::Make(this, paint, &path);
			auto fillPath = [&]() {
				if (!paint.antiAlias || !fillPathCAPA(path, paint, paint.fill, false)) {
					auto &vertex = buildVertex(path, aaRadius, paint.antiAlias);
					fillPathAASide(vertex, paint, paint.fill);
				}
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
		, _flags(0)
		, _blendMode(kInvalid_BlendMode)
		, _clipState(nullptr)
		, _opts(opts)
		, _capaBuilder(nullptr)
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

	Sp<ImageSource> GPUCanvas::getTextureFromPool(Vec2 size, ColorType type, Vec2 limit, uint8_t flags) {
		if (limit.is_zero_axis()) {
			limit = _surfaceSize; // default limit to surface size if not provided
		}
		// limit texture size to surface size,
		// to avoid memory waste and performance loss on large render targets
		int w = I32::clamp(upPow2(size.x()), 1, int(limit.x()));
		int h = I32::clamp(upPow2(size.y()), 1, int(limit.y()));
		// limit texture aspect ratio to 4:1,
		// to avoid memory waste and performance loss on large render targets
		if (w > h * 4) h = I32::min(w / 4, limit[1]);
		if (h > w * 4) w = I32::min(h / 4, limit[0]);
		uint64_t key =
			(uint64_t(w) << 32) | // bit16
			(uint64_t(h) << 16) | // bit16
			(type << 8) | // bit8
			flags; // bit8
		auto &pool = _texPools[key];
		for (auto &tex : pool) {
			if (tex->refCount() == 1)
				return tex;
		}
		auto src = _render->createTexture(Vec2(w,h), type, flags);
		pool.push(src.get());
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
			if (_capaBuilder) {
				_capaBuilder->setBlendMode(mode); // commit current CAPA batch if blend mode changed
			}
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
			_this->commitCAPABatch(); // commit current CAPA batch before restore
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
		const Vec2 pad = 1.0f; // 1 pixel pad for anti-aliasing
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
		_this->commitCAPABatch();
		clip->mask = getTextureFromPool(range.end - range.begin, kLuminance_8_ColorType);
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
		if (_capaBuilder)
			_capaBuilder->reset();
		clearColorCmd(color, _stateStack.length() == 1 ? kClearAll_ClearFlags : kOnlyColor_ClearFlags);
	}

	void GPUCanvas::drawColor(const Color4f &color, BlendMode mode) {
		_this->setBlendMode(mode); // switch blend mode
		_this->commitCAPABatch();
		drawColorCmd({0,6, {
			{0,0,0}, {_size[0],0,0}, {_size[0],_size[1],0}, // triangle 1
			{_size[0],_size[1],0}, {0,_size[1],0}, { 0,0,0 } // triangle 2
		}}, color);
	}

	void GPUCanvas::drawRRectBlurColor(const Rect& rect,
		const float radius[4], float blur, const Color4f &color, BlendMode mode)
	{
		if (rect.size.is_zero_axis())
			return;
		_this->setBlendMode(mode); // switch blend mode
		_this->commitCAPABatch();
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
		auto needSDF = paint.style != Paint::kFill_Style;

		if (blob->img.fontSize != fixedFSize || !blob->img.image ||
			(needSDF ? !_this->isSDFImage(blob->img.image.get()): false)
		) { // fill text bolb
			Array<Vec2> offset;
			if (blob->offset.length() >= blob->glyphs.length()) {
				offset = blob->offset;
				for (auto &o: offset) o *= scale;
			}
			blob->img = needSDF ?
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
		_this->commitCAPABatch(); // commit current CAPA batch before read image
		drawTrianglesCmd(triangles, paint.fill.image, paint.fill.color, copyData);
	}

	Sp<ImageSource> GPUCanvas::readImage(const Rect &src, Vec2 dst, ColorType type, BlendMode mode, bool mipmap) {
		_this->setBlendMode(mode); // switch blend mode
		_this->commitCAPABatch();
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
		_this->commitCAPABatch();
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
		_rootMatrixNoScale = root;
		_rootMatrixNoScale.scale_x(1.0f/surfaceScale.x());
		_rootMatrixNoScale.scale_y(1.0f/surfaceScale.y());
		_texPools.clear(); // clear texture pool when surface size changed
		if (_capaBuilder)
			_capaBuilder->reset(true);

		Qk_DLog("setSurface: %f, %f", _surfaceSize.x(), _surfaceSize.y());

		setSurfaceCmd(chSize); // set buffers
	}
}
