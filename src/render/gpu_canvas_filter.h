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

// @private head

#ifndef __quark_render_gpucanvas_filter__
#define __quark_render_gpucanvas_filter__

#include "./gpu_canvas.h"
#include "src/render/render.h"

namespace qk {

	class GC_Filter {
	public:
		template<typename... Args>
		static GC_Filter* Make(GPUCanvas *host, const Paint &paint, Args... args);
		virtual ~GC_Filter() = default;
	};

	class GC_BlurFilter: public GC_Filter {
	public:
		GC_BlurFilter(GPUCanvas *host, const Paint &paint, const Path *path_)
			: _host(host), _radius(paint.filter->val0)
		{
			// auto &path = host->_cache->getNormalizedPath(*path_);
			_bounds = path_->getBounds(&host->_state->matrix);
			begin(paint);
		}
		GC_BlurFilter(GPUCanvas *host, const Paint &paint, const Rect *rect)
			: _host(host), _radius(paint.filter->val0), _bounds{rect->begin,rect->begin+rect->size}
		{
			if (!host->_state->matrix.is_identity()) { // Not unit matrix
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
			begin(paint);
		}

		int getBlurSampling(float radius, int &imageLod) {
			constexpr int maxN = 13; // 13 samples is enough for 99% blur effect,
			// and more samples will cause performance loss
			const int N[] = { 3,3,3,3,5,5,7,7,9,9,11,11,13,13,15,15,17,17,19,19 };
			radius *= _host->_allScaleAverage;
			float diameter = radius * 2.0f;
			int sample = ceilf(diameter); // sampling rate is diameter
			sample = N[Qk_Min(sample,maxN)]; // sample count for blur filter
			// imageLod is intentionally estimated from radius instead of diameter.
			// Diameter is the theoretical full blur coverage, but using it here makes
			// mip levels increase too aggressively and causes visible jumps when blur
			// radius changes. Using radius keeps mip transitions smoother and avoids
			// excessive downsampling.
			// This is a visual/performance tradeoff: high-frequency images may retain
			// slightly more detail than a diameter-based estimate, but in practice the
			// result is usually smoother and requires fewer downsample passes.
			constexpr float lodRadiusFactor = 1.0f; // maybe 1.5f for high-frequency content
			imageLod = ceilf(F32::max(0, log2f(radius * lodRadiusFactor / sample)));
			return sample;
		}

		~GC_BlurFilter() override {
			if (_host->_capaBuilder) {
				_host->_capaBuilder->flush();
				_host->_capaBuilder->setSurfaceOffset(IVec2(0, 0));
			}
			_host->_flags = _flags; // restore flags before blur filter
			_host->blurFilterEndCmd(_bounds, _rootMatrix, _radius, _clearPad, _sample, _imageLod, *_tmpA, *_tmpB);
		}

	private:
		void begin(const Paint &paint) {
			_radius *= _host->_scaleAverage; // * logical scale
			_sample = getBlurSampling(_radius, _imageLod);
			if (paint.style != Paint::kFill_Style && paint.strokeWidth) {
				// add padding for stroke width, to avoid stroke being cut by blur edge
				auto halfStroke = paint.strokeWidth * _host->_scaleAverage * 0.5f;
				_bounds.begin -= Vec2(halfStroke, halfStroke);
				_bounds.end += Vec2(halfStroke, halfStroke);
			}
			_clearPad = ((1 << _imageLod) + 1.0f) / _host->_allScaleAverage;
			// add padding for blur radius and clear pad
			auto padding = _clearPad + _radius + _radius;
			// expand bounds by padding for blur filter,
			// blur filter will read and write the texture in the area of bounds + padding
			// |r|r|rrrrrr|r|r|
			// |r|r|rrrrrr|r|r|
			// |r|r| body |r|r|
			// |r|r|rrrrrr|r|r|
			// |r|r|rrrrrr|r|r|
			_bounds = {_bounds.begin - padding, _bounds.end + padding};
			// expand bounds to integer values
			_bounds = _bounds.expandToInteger();
			// min begin to 0, to avoid bounds being out of texture range.
			auto begin = _bounds.begin.max(0);
			if (_host->_capaBuilder) {
				_host->_capaBuilder->flush(); // flush current CAPA batch before blur filter
				auto offset = begin * _host->_surfaceScale;
				_host->_capaBuilder->setSurfaceOffset(IVec2(-offset.x(), -offset.y()));
			}
			// save root matrix before blur
			_blurRootMatrix = _rootMatrix = _host->_rootMatrix;
			// adjust root matrix for blur filter, to keep the same visual position after expanding bounds
			_blurRootMatrix.translate(Vec3(-begin, 0));
			// compute texture size for blur filter, limit to surface size
			auto texS = (_bounds.end.min(_host->_size) - begin) * _host->_surfaceScale;
			_tmpA = _host->getTextureFromPool(texS, _host->_opts.colorType, 0, kMipmap_TextureFlags);
			_tmpB = _host->getTextureFromPool(texS, _host->_opts.colorType, 0, kMipmap_TextureFlags);
			// disable clip for blur filter, to avoid blur being cut by clip
			_flags = _host->_flags;
			_host->_flags &= ~Qk_FLAG_CLIP;
			_host->blurFilterBeginCmd(_bounds, _blurRootMatrix, *_tmpA);
		}
		GPUCanvas *_host;
		float  _radius; // blur radius
		float _clearPad; // clear padding for blur edge
		Range _bounds; // bounds for draw path
		Mat4	_rootMatrix, _blurRootMatrix; // root matrix before blur, and root matrix for blur filter
		Sp<ImageSource> _tmpA, _tmpB; // temporary blur textures, retained for async GL commands
		int _sample, _imageLod;
		uint32_t _flags;
	};

	template<typename... Args>
	GC_Filter* GC_Filter::Make(GPUCanvas *host, const Paint &paint, Args... args)
	{
		// switch blend mode and solve matrix
		host->setBlendMode(paint.blendMode);
		if (!paint.filter) {
			return nullptr;
		}
		switch(paint.filter->type) {
			case PaintFilter::kBlur_Type:
				if (host->_allScaleAverage * paint.filter->val0 >= 0.5f) { // limit min blur radius to 0.5 pixel
					return new GC_BlurFilter(host, paint, args...);
				}
				break;
			default: break;
		}
		return nullptr;
	}
}
#endif
