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

#define isMoreSofterAA 1 // 1 is softer aa, 0 is more radical aa

namespace qk {
	uint32_t massSample(uint32_t n);
	float get_level_font_size(float fontSize);

	extern const Range ZeroRange;
#if isMoreSofterAA
	// Softer:
	//extern const float  aa_fuzz_weight = 0.9; // softer
	//extern const float  aa_fuzz_width = 0.6;
	extern const float  aa_fuzz_weight = 0.9; // medium
	extern const float  aa_fuzz_width = 0.55;
#else
	// More radical:
	extern const float  aa_fuzz_weight = 1; // more radical, hard
	extern const float  aa_fuzz_width = 0.5;
#endif
	extern const float  zDepthNextUnit = 1.0f / 5000000.0f;

	typedef Typeface::TextImage TextImage;

	class GC_Filter {
	public:
		template<typename... Args>
		static GC_Filter* Make(GPUCanvas *host, const Paint &paint, Args... args);
		virtual ~GC_Filter() = default;
	};

	Qk_DEFINE_INLINE_MEMBERS(GPUCanvas, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<GPUCanvas::Inl*>(self)

		inline void zDepthNext() {
			_zDepth += zDepthNextUnit;
		}

		inline void zDepthNextCount(uint32_t count) {
			_zDepth += (zDepthNextUnit * count);
		}

		void computeScale(const Mat& mat) {
			// is translation only matrix
			auto scale = mat.is_translation_matrix() ? 1.0f:
				mat.mul_vec2_no_translate(1.0f).length() * (1.0f / Qk_SQRT_2);
			if (_scale != scale) {
				_scale = scale;
				_allScale = _surfaceScale * scale;
				_phy2Pixel = 2.0f / _allScale;
			}
		}

		void setBlendMode(BlendMode mode) {
			if (_blendMode != mode) {
				_blendMode = mode;
				setBlendModeCmd();
			}
		}

		void clipv(const Path &path, const VertexData &vertex, ClipOp op, bool antiAlias) {
			GC_State::Clip clip{
				.matrix=_state->matrix, /*.path=path,*/ .op=op,
			};
			if (vertex.vertex.val()) { // copy vertex data
				clip.vertex = vertex;
			} else if (path.verbsLen()) {
				clip.vertex = _cache->getPathTriangles(path);
				if (!clip.vertex.vertex.val()) {
					clip.vertex = path.getTriangles();
				}
			}

			// empty clip area, but still need to process stencil buffer,
			// Very important, it may be necessary to buffer template parameters
			// if (clip.vertex.vCount == 0) return;

			if (!_clipState) {
				_clipState = true;
				enableStencilTestCmd(true); // enable stencil test
			}

			if (clip.vertex.vCount && antiAlias && !_DeviceMsaa) {
				clip.aaclip = true;
				clip.aafuzz = _cache->getAAFuzzStrokeTriangle(path,_phy2Pixel*aa_fuzz_width);
				if (!clip.aafuzz.vertex.val() && path.verbsLen()) {
					clip.aafuzz = path.getAAFuzzStrokeTriangle(_phy2Pixel*aa_fuzz_width);
				}
				_state->aaclip++;
			}

			if (clip.op == kDifference_ClipOp) {
				if (_stencilRefDrop == 0) {
					Qk_Warn(" clip stencil ref drop value exceeds limit 0"); return;
				}
				_stencilRefDrop--;
			} else {
				if (_stencilRef == 255) {
					Qk_Warn(" clip stencil ref value exceeds limit 255"); return;
				}
				_stencilRef++;
			}

			drawClipCmd(clip, _stencilRef, false);
			_state->clips.push(std::move(clip));
			zDepthNext();
		}

		void fillPath(const Path &path, const Paint &paint, const PaintStyle &style, bool aa) {
			Qk_ASSERT(path.isNormalized(), "Path must be normalized before filling. Call path.normalize() first.");
			auto &vertex = _cache->getPathTriangles(path);
			if (vertex.vCount) {
				fillv(vertex, paint, style);
				if (aa) {
					drawAAFuzzStroke(path, paint, style, aa_fuzz_weight, aa_fuzz_width);
				}
			}
			zDepthNext();
		}

		void fillv(const VertexData &vertex, const Paint &paint, const PaintStyle &style) {
			if (style.image) {
				drawImageCmd(vertex, style.image, style.color);
			} else if (style.gradient) {
				drawGradientCmd(vertex, style.gradient, style.color);
			} else if (paint.mask) {
				drawImageMaskCmd(vertex, paint.mask, style.color);
			} else {
				drawColorCmd(vertex, style.color);
			}
		}

		void strokePath(const Path &path, const Paint& paint, bool aa) {
			if (aa) {
#if isMoreSofterAA
				const float weight = 0.45f;
#else
				const float weight = 0.5f;
#endif
				auto width = paint.strokeWidth - _phy2Pixel * weight;
				if (width > 0) {
					fillPath(_cache->getStrokePath(path, width, paint.cap, paint.join,0), paint, paint.stroke, true);
				} else {
					width /= (_phy2Pixel * 1.0f - weight); // range: -1 => 0
					width = powf(width*10, 3) * 0.005; // (width*10)^3 * 0.005
					drawAAFuzzStroke(path, paint, paint.stroke, 0.5 / (0.5 - width), 0.5);
					zDepthNext();
				}
			} else {
				fillPath(_cache->getStrokePath(path, paint.strokeWidth, paint.cap, paint.join,0), paint, paint.stroke, false);
			}
		}

		void drawAAFuzzStroke(const Path& path, const Paint &paint, const PaintStyle& style, float aaFuzzWeight, float aaFuzzWidth) {
			//Path newPath(path); newPath.transfrom(Mat(1,0,170,0,1,0));
			// _phy2Pixel*0.6=1.2/_Scale, 2.4px
			auto &vertex = _cache->getAAFuzzStrokeTriangle(path, _phy2Pixel*aaFuzzWidth);
			if (style.image) {
				drawImageCmd(vertex, style.image, style.color.mul_alpha_only(aaFuzzWeight));
			} else if (style.gradient) {
				drawGradientCmd(vertex, style.gradient, style.color.mul_alpha_only(aaFuzzWeight));
			} else if (paint.mask) {
				drawImageMaskCmd(vertex, paint.mask, style.color.mul_alpha_only(aaFuzzWeight));
			} else {
				drawColorCmd(vertex, style.color.mul_alpha_only(aaFuzzWeight));
			}
		}

		float drawTextImage(TextImage &img, float scale, Vec2 origin, const Paint &paint);
		void drawPathv_(const Pathv& path, const Paint& paint);
	};

	class GC_BlurFilter: public GC_Filter {
	public:
		GC_BlurFilter(GPUCanvas *host, const Paint &paint, const Path *path)
			: _host(host), _radius(paint.filter->val0), _bounds(path->getBounds(&host->_state->matrix))
		{
			begin();
		}
		GC_BlurFilter(GPUCanvas *host, const Paint &paint, const Rect *rect)
			: _host(host), _radius(paint.filter->val0), _bounds{rect->begin,rect->begin+rect->size}
		{
			if (!host->_state->matrix.is_identity_matrix()) { // Not unit matrix
				auto &mat = host->_state->matrix;
				if (mat[0] != 1 || mat[4] != 1) { // rotate or skew
					Vec2 pts[] = {
						_bounds.begin, {_bounds.end.x(),_bounds.begin.y()},
						{_bounds.end.x(),_bounds.end.y()}, {_bounds.begin.x(),_bounds.end.y()}
					};
					_bounds = Path::getBoundsFromPoints(pts, 4, &host->_state->matrix);
				} else {
					Vec2 translate(mat[2],mat[5]); // translate
					_bounds.begin += translate;
					_bounds.end += translate;
				}
			}
			begin();
		}

		int getBlurSampling(float radius, int &imageLod) {
			const int N[] = { 3,3,3,3, 7,7,7,7, 13,13,13,13,13,13, 19,19,19,19,19,19 };
			radius *= _host->_surfaceScale;
			int sample = ceilf(radius); // sampling rate
			sample = N[Qk_Min(sample,19)];
			imageLod = ceilf(Float32::max(0,log2f(radius/sample)));
			return sample;
		}

		~GC_BlurFilter() override {
			_host->blurFilterEndCmd(_bounds, _radius, _clearPad, _sample, _imageLod);
			_inl(_host)->zDepthNextCount(_imageLod + 2);
		}

	private:
		void begin() {
			_radius *= _host->_scale; // * logical scale
			_bounds = {_bounds.begin - _radius, _bounds.end + _radius};
			_sample = getBlurSampling(_radius, _imageLod);
			_clearPad = ((1 << _imageLod) + 1.0f) / _host->_allScale;
			_host->blurFilterBeginCmd(_bounds, _radius, _clearPad);
			_inl(_host)->zDepthNext();
		}
		GPUCanvas *_host;
		float  _radius; // blur radius
		float _clearPad; // clear padding for blur edge
		Range _bounds; // bounds for draw path
		int _sample, _imageLod;
	};

	template<typename... Args>
	GC_Filter* GC_Filter::Make(GPUCanvas *host, const Paint &paint, Args... args)
	{
		// switch blend mode and solve matrix
		_inl(host)->setBlendMode(paint.blendMode);
		if (!paint.filter) {
			return nullptr;
		}
		switch(paint.filter->type) {
			case PaintFilter::kBlur_Type:
				if (host->_allScale * paint.filter->val0 >= 1.0) {
					return new GC_BlurFilter(host, paint, args...);
				}
				break;
			default: break;
		}
		return nullptr;
	}

	GPUCanvas::GPUCanvas(Render *render, Render::Options opts)
		: _state(nullptr), _cache(nullptr)
		, _stencilRef(127), _stencilRefDrop(127)
		, _zDepth(0)
		, _surfaceScale(1), _scale(1), _allScale(1), _phy2Pixel(1)
		, _size(), _surfaceSize()
		, _rootMatrix()
		, _blendMode(kInvalid_BlendMode)
		, _DeviceMsaa(0)
		, _clipState(false)
		, _opts(opts)
	{
		_opts.msaaSample = massSample(_opts.msaaSample);
		_DeviceMsaa = _opts.msaaSample > 1 ? _opts.msaaSample: 0;
		auto capacity = opts.maxCapacityForPathvCache ?
			opts.maxCapacityForPathvCache: 128000000/*128mb*/;
		capacity = Uint32::clamp(capacity, 1024000/*1mb*/, 512000000/*512mb*/);
		_cache = new PathvCache(capacity, render);
		_stateStack.push({ .matrix=Mat(), .aaclip=0 });
		_state = &_stateStack.back();
	}

	GPUCanvas::~GPUCanvas() {
		Releasep(_cache);
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

	int GPUCanvas::save() {
		auto &state = _stateStack.back();
		_stateStack.push({ .matrix=state.matrix,.aaclip=state.aaclip,.output=state.output});
		_state = &_stateStack.back();
		return _stateStack.length();
	}

	void GPUCanvas::restore(uint32_t count) {
		if (!count || _stateStack.length() == 1)
			return;
		count = Uint32::min(count, _stateStack.length() - 1);

		if (count > 0) {
			do {
				for (int i = Qk_Minus(_state->clips.length(), 1); i >= 0; i--) {
					auto &clip = _state->clips[i];
					if (clip.op == kDifference_ClipOp) {
						_stencilRefDrop++;
					} else {
						_stencilRef--;
					}
					setMatrix(clip.matrix);
					drawClipCmd(clip, _stencilRef, true);
					_this->zDepthNext();

					if (clip.aaclip) {
						_state->aaclip--;
					}
				}
				auto exit = _state->output; // save output image before pop state
				_stateStack.pop();
				_state = &_stateStack.back();
				if (exit != _state->output) { // restore region draw, only when output changed
					outputImageEndCmd(exit.get());
				}
				count--;
			} while (count > 0);

			if (_clipState && _stencilRef == _stencilRefDrop) { // not stencil test
				_clipState = false;
				enableStencilTestCmd(false); // disable stencil test
			}
			setMatrix(_state->matrix);
		}
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

	void GPUCanvas::clipPath(const Path& path, ClipOp op, bool antiAlias) {
		_this->clipv(path, _cache->getPathTriangles(path), op, antiAlias);
	}

	void GPUCanvas::clipPathv(const Pathv& path, ClipOp op, bool antiAlias) {
		_this->clipv(path.path, path, op, antiAlias);
	}

	void GPUCanvas::clipRect(const Rect& rect, ClipOp op, bool antiAlias) {
		auto &path = _cache->getRectPath(rect);
		_this->clipv(path.path, path, op, antiAlias);
	}

	void GPUCanvas::clearColor(const Color4f& color) {
		auto clearDepth = _stateStack.length() == 1;
		if (clearDepth)
			_zDepth = 0; // set z depth state
		// 2: clear color/clip/depth, 1: clear color only
		clearColorCmd(color, clearDepth ? kClearAll_ClearFlags : kOnlyColor_ClearFlags);
	}

	void GPUCanvas::drawColor(const Color4f &color, BlendMode mode) {
		auto needBlend = mode != kSrc_BlendMode;// || color.a() != 1;
		if (needBlend) { // draw color
			_this->setBlendMode(mode); // switch blend mode
			clearColorCmd(color, kBlend_ClearFlags); // 0: clear color and blending color
			_this->zDepthNext();
		} else { // clear color
			clearColor(color);
		}
	}

	void GPUCanvas::drawPathvColor(const Pathv& path, const Color4f &color, BlendMode mode, bool antiAlias) {
		_this->setBlendMode(mode); // switch blend mode
		drawColorCmd(path, color);
		if (!_DeviceMsaa && antiAlias) { // Anti-aliasing using software
			auto &vertex = _cache->getAAFuzzStrokeTriangle(path.path, _phy2Pixel*aa_fuzz_width);
			drawColorCmd(vertex, color.mul_alpha_only(aa_fuzz_weight));
		}
		_this->zDepthNext();
	}

	void GPUCanvas::drawPathvColors(const Pathv* paths[], int count, const Color4f &color, 
		BlendMode mode, bool antiAlias) 
	{
		_this->setBlendMode(mode); // switch blend mode
		for (int i = 0; i < count; i++) {
			drawColorCmd(*paths[i], color);
		}
		if (!_DeviceMsaa && antiAlias) { // Anti-aliasing using software
			auto c2 = color.mul_alpha_only(aa_fuzz_weight);
			for (int i = 0; i < count; i++) {
				auto &vertex = _cache->getAAFuzzStrokeTriangle(paths[i]->path, _phy2Pixel*aa_fuzz_width);
				drawColorCmd(vertex, c2);
			}
		}
		_this->zDepthNext();
	}

	void GPUCanvas::drawRRectBlurColor(const Rect& rect,
		const float radius[4], float blur, const Color4f &color, BlendMode mode) 
	{
		if (!rect.size.is_zero_axis()) {
			_this->setBlendMode(mode); // switch blend mode
			drawRRectBlurColorCmd(rect, radius, blur, color);
			_this->zDepthNext();
		}
	}

	void GPUCanvas::drawPath(const Path &path0, const Paint &paint) {
		auto &path = _cache->getNormalizedPath(path0);
		bool aa = paint.antiAlias && !_DeviceMsaa; // Anti-aliasing using software
		Sp<GC_Filter> filter = GC_Filter::Make(this, paint, &path);

		// gen stroke path and fill path and polygons
		switch (paint.style) {
			case Paint::kFill_Style:
				_this->fillPath(path, paint, paint.fill, aa); break;
			case Paint::kStrokeAndFill_Style:
				_this->fillPath(path, paint, paint.fill, aa);
			case Paint::kStroke_Style:
				_this->strokePath(path, paint, aa); break;
		}
	}

	void GPUCanvas::Inl::drawPathv_(const Pathv& path, const Paint& paint) {
		bool aa = paint.antiAlias && !_DeviceMsaa; // Anti-aliasing using software
		Sp<GC_Filter> filter = GC_Filter::Make(this, paint, &path.path);

		auto fillPathv = [](Inl* self, const Pathv &path, const Paint &paint, bool aa) {
			if (path.vCount) {
				Qk_ASSERT(path.path.isNormalized());
				self->fillv(path, paint, paint.fill);
				if (aa) {
					self->drawAAFuzzStroke(path.path, paint, paint.fill, aa_fuzz_weight, aa_fuzz_width);
				}
				self->zDepthNext();
			}
		};
		// gen stroke path and fill path and polygons
		switch (paint.style) {
			case Paint::kFill_Style:
				fillPathv(this, path, paint, aa); break;
			case Paint::kStrokeAndFill_Style:
				fillPathv(this, path, paint, aa);
			case Paint::kStroke_Style:
				_this->strokePath(path.path, paint, aa); break;
		}
	}

	void GPUCanvas::drawRect(const Rect& rect, const Paint& paint) {
		_this->drawPathv_(_cache->getRectPath(rect), paint);
	}

	void GPUCanvas::drawRRect(const Rect& rect, const Path::BorderRadius &radius, const Paint& paint) {
		_this->drawPathv_(_cache->getRRectPath(rect,radius), paint);
	}

	void GPUCanvas::drawPathv(const Pathv& path, const Paint& paint) {
		_this->drawPathv_(path, paint);
	}

	float GPUCanvas::Inl::drawTextImage(TextImage &img, float scale, Vec2 origin, const Paint &paint) {
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
			auto strokeWidth = paint.style == Paint::kFill_Style ?
					0.0f: paint.strokeWidth;
			drawSDFImageMaskCmd(vertex, &p, paint.fill.color,
					paint.stroke.color, strokeWidth * scale);
		} else {
			drawImageMaskCmd(vertex, &p, paint.fill.color);
		}

		zDepthNext();
		return scale_1;
	}

	float GPUCanvas::drawGlyphs(const FontGlyphs &glyphs, Vec2 origin, cArray<Vec2> *offsetIn, const Paint &paint) {
		Array<Vec2> offset, *offsetP = nullptr;
		if (offsetIn) {
			offset = *offsetIn;
			offsetP = &offset;
			for (auto &o: offset) o *= _allScale;
		}
		auto isSDF = paint.style != Paint::kFill_Style;
		auto tf = glyphs.typeface();
		auto img = isSDF ?
			tf->getSDFImage(glyphs.glyphs(), glyphs.fontSize() * _allScale, offsetP, false):
			tf->getImage(glyphs.glyphs(), glyphs.fontSize() * _allScale, offsetP);
		img.image->set_mipmap(false); // disable mipmap for text
		auto scale = _this->drawTextImage(img, _allScale, origin, paint);
		return scale * img.width;
	}

	void GPUCanvas::drawTextBlob(TextBlob *blob, Vec2 origin, float fontSize, const Paint &paint) {
		auto fixedFSize = get_level_font_size(_scale * fontSize) * _surfaceScale;
		if (fixedFSize == 0.0)
			return;
		auto scale = fixedFSize / fontSize;
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
		if (triangles.zDepthTotal) {
			_zDepth += triangles.zDepthTotal;
		} else {
			_this->zDepthNext();
		}
	}

	Sp<ImageSource> GPUCanvas::readImage(const Rect &src, Vec2 dst, ColorType type, BlendMode mode, bool mipmap) {
		_this->setBlendMode(mode); // switch blend mode
		auto o = src.begin;
		auto s = Vec2{
			Float32::min(o.x()+src.size.x(), _size.x()) - o.x(),
			Float32::min(o.y()+src.size.y(), _size.y()) - o.y()
		};
		if (s[0] > 0 && s[1] > 0 && dst[0] > 0 && dst[1] > 0) {
			type = type ? type: _opts.colorType; // default to surface color type
			auto destImg = ImageSource::Make(PixelInfo{
				int(Qk_Min(dst.x(),_surfaceSize.x())), // limit max read image size to surface size
				int(Qk_Min(dst.y(),_surfaceSize.y())), type, kPremul_AlphaType
			});
			destImg->set_mipmap(mipmap);
			readImageCmd({o*_surfaceScale,s*_surfaceScale}, _state->output.get(), destImg.get());
			_this->zDepthNext();
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

	void GPUCanvas::setSurface(const Mat4& root, Vec2 surfaceSize, Vec2 scale) {
		if (_DeviceMsaa) {
			auto msaa = ceilf(sqrtf(_DeviceMsaa));
			surfaceSize *= msaa;
			scale *= msaa;
		}
		auto chSize = surfaceSize != _surfaceSize;
		// clear all state
		_stateStack.clear();
		_stateStack.push({ .matrix=Mat(), .aaclip=0 });
		_state = &_stateStack.back(); // reset state
		_stencilRef = _stencilRefDrop = 127; // reset stencil ref
		// set surface scale
		_surfaceSize = surfaceSize;
		_size = surfaceSize / scale;
		_surfaceScale = (scale[0] + scale[1]) * 0.5;
		_scale = _state->matrix.mul_vec2_no_translate(1).length() / Qk_SQRT_2;
		_allScale = _surfaceScale * _scale;
		_phy2Pixel = 2 / _allScale;
		_rootMatrix = root.transpose(); // transpose matrix
		_zDepth = 0;
		_clipState = false; // clear clip state
		setSurfaceCmd(chSize); // set buffers
	}
}
