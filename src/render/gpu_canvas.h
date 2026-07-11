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

#ifndef __quark_render_gpucanvas__
#define __quark_render_gpucanvas__

#include "./render.h"
#include "./canvas.h"
#include "./capa.h"

#define Qk_CLIP(clip) (clip ? Qk_FLAG_CLIP: 0)
// global flags from 1u << 0 to 1u << 15, 0x0000FFFF
#define Qk_FLAG_CLIP (1u << 0)
#define Qk_FLAG_PMA (1u << 1)
#define Qk_FLAG_AASIDE_LINE (1u << 2)
#define Qk_FLAG_CGAA (1u << 3)
// shader private flags from 1u << 16 to 1u << 31, 0xFFFF0000
// gradient shader
#define Qk_FLAG_GRADIENT_COUNT2 (1u << 16)
#define Qk_FLAG_RADIAL_GRADIENT (1u << 17)
// color shader
#define Qk_FLAG_AASIDE_Inverted (1u << 16)
// image shader
#define Qk_FLAG_IMAGE_MASK (1u << 16)
#define Qk_FLAG_IMAGE_SDF_MASK (1u << 17)
#define Qk_FLAG_IMAGE_CLAMP_TO_ZERO_X (1u << 18)
#define Qk_FLAG_IMAGE_CLAMP_TO_ZERO_Y (1u << 19)
// triangles shader
#define Qk_FLAGS_DARK_COLOR (1u << 16)

namespace qk {

	struct GC_State { // gpu canvas state
		struct Clip: Reference { // clip state
			Range           bounds; // clip bounds for clip texture
			Range           range; // clip the extent of external influence
			Sp<ImageSource> mask; // clip mask texture
			Canvas::ClipOp  op; // clip op
		};
		Mat             matrix; // current state matrix
		Sp<Clip>        clip; // clip state, null for no clip
		Sp<ImageSource> output; // output dest texture, null for to main color buffer
	};

	enum GC_ClearFlags {
		kOnlyColor_ClearFlags, // only clear color
		kClearAll_ClearFlags, // clear color and reset render target state
	};

	enum GC_ImageDrawKind {
		kImage_DrawKind,
		kMask_DrawKind,
		kSDFMask_DrawKind,
	};

	struct GC_ImageDrawInfo {
		const PaintImage *paint;
		Color4f color;
		GC_ImageDrawKind kind = kImage_DrawKind;
		Color4f strokeColor;
		float stroke = 0;
	};

	class GC_Filter;
	class GC_BlurFilter;

	/**
	 * @class GPUCanvas - A Canvas implementation that renders using GPU acceleration.
	 */
	class GPUCanvas: public Canvas {
	public:
		GPUCanvas(Render *render, Render::Options opts);
		~GPUCanvas() override;
		Vec2 size() const override; // _size = surfaceSize / scale
		bool isGpu() const override;
		int  save() override;
		void restore(uint32_t count) override;
		int  getSaveCount() const override;
		const Mat& getMatrix() const override;
		void setMatrix(const Mat& mat) override;
		void setTranslate(Vec2 val) override;
		void translate(Vec2 val) override;
		void scale(Vec2 val) override;
		void rotate(float z) override;
		void clipPath(const Path& path, ClipOp op, bool antiAlias) override;
		void clipRect(const Rect& rect, ClipOp op, bool antiAlias) override;
		void clearColor(const Color4f& color) override;
		void drawColor(const Color4f& color, BlendMode mode) override;
		void drawPath(const Path& path, const Paint& paint) override;
		void drawPathColor(const Path &path, const Color4f &color, BlendMode mode, bool antiAlias) override;
		void drawPathColors(const Path* path[4], int count, const Color4f &color, BlendMode mode, bool antiAlias) override;
		void drawRRectBlurColor(const Rect& rect, const float radius[4], float blur, const Color4f &color, BlendMode mode) override;
		void drawRect(const Rect& rect, const Paint& paint) override;
		void drawRRect(const Rect& rect, const Path::BorderRadius &radius, const Paint& paint) override;
		void drawRectPath(const RectPath& path, const Paint& paint) override;
		void drawRectOutlinePath(const RectOutlinePath& path, const Color4f color[4], const Paint& paint) override;
		float drawGlyphs(const FontGlyphs &glyphs, Vec2 origin, const Array<Vec2> *offset, const Paint &paint) override;
		void drawTextBlob(TextBlob *blob, Vec2 origin, float fontSize, const Paint &paint) override;
		void drawTriangles(const Triangles& triangles, const Paint &paint, bool copyData) override;
		Sp<ImageSource> readImage(const Rect &src, Vec2 dst, ColorType type, BlendMode mode, bool mipmap) override;
		Sp<ImageSource> outputImage(ImageSource* dst, bool mipmap) override;
		PathvCache* getPathvCache() override;
		void setSurface(const Mat4& root, Vec2 surfaceSize, Vec2 surfaceScale) override;
		Vec2 surfaceSize() const override;
		const Render::Options& opts() const { return _opts; }
		Render* render() { return _render; }
		uint32_t capaMaxImageCount() const { return _capaMaxImageCount; }
		inline Color4f premul_alpha(const Color4f &color) const { return color.premul_alpha(); }
	protected:
		void setCAPAMaxImageCount(uint32_t count);
		virtual void setSurfaceCmd(bool changeSize) = 0;
		virtual void setMatrixCmd() = 0;
		virtual void setBlendModeCmd() = 0;
		virtual void drawClipCmd(const VertexData &vertex, GC_State::Clip *lastClip, GC_State::Clip *clip, ClipOp rawOp) = 0;
		virtual void clearColorCmd(const Color4f &color, GC_ClearFlags flags) = 0;
		virtual void drawImageCmd(const VertexData &vertex, const GC_ImageDrawInfo &info) = 0;
		virtual void drawGradientCmd(const VertexData &vertex, const PaintGradient *paint, const Color4f &color) = 0;
		virtual void drawColorCmd(const VertexData &vertex, const Color4f &color) = 0;
		virtual bool drawCAPACmd(CAPADrawData &data) = 0;
		virtual void drawRRectBlurColorCmd(const Rect& rect, const float *radius, float blur, const Color4f &color) = 0;
		virtual void blurFilterBeginCmd(Range bounds, Mat4 &rootMat, ImageSource *tmpA) = 0;
		virtual void blurFilterEndCmd(Range bounds, Mat4 &recoverRootMat, float radius, float clearPad,
				int sample, int imageLod, ImageSource *tmpA, ImageSource *tmpB) = 0;
		virtual void drawTrianglesCmd(const Triangles& triangles, const PaintImage *paint,
				const Color4f &color, bool copyData) = 0;
		virtual void readImageCmd(const Rect &srcRect, ImageSource* src, ImageSource* dst) = 0;
		virtual void outputImageBeginCmd(ImageSource* dst) = 0;
		virtual void outputImageEndCmd(ImageSource* exit) = 0;
		virtual void restoreClipCmd(GC_State::Clip* clip) = 0;
		virtual void flushSubcanvasCmd(GPUCanvas* canvas) = 0;
		// get texture count from pool and add ref count, limit texture size to surface size
		// flags can be kMipmap_TextureFlags, kComputeWrite_TextureFlags, etc
		Sp<ImageSource> getTextureFromPool(Vec2 size, ColorType type, Vec2 limit = Vec2(), uint8_t flags = 0);
		void setBlendMode(BlendMode mode);
	// fields:
		Array<GC_State> _stateStack; // state
		GC_State    *_state; // state pointer
		PathvCache *_cache;
		Render 	 *_render; // render backend
		Vec2  _surfaceSize, _surfaceScale; // surface scale and size, surfaceSize = surfaceScale * size
		Vec2   _size, _scale; // size=surfaceSize/surfaceScale, _scale = matrix scale extracted
		float  _surfaceScaleAverage, _scaleAverage, _allScaleAverage; // average of x/y scale
		float  _allScaleMin; // _surfaceScaleAverage * min(scale)
		float  _1pxSize; // _1pxSize = 1 / _allScaleMin
		float  _aaRadius, _aaRadiusRect; // anti-aliasing side radius, for path and rect respectively
		Mat4   _rootMatrix, _rootMatrixNoScale; // root matrix and root matrix with scale removed
		uint32_t _flags; // flags for current state, such as anti-aliasing, etc
		BlendMode _blendMode; // blend mode state
		uint32_t _capaMaxImageCount; // backend CAPA image/sampler table size
		GC_State::Clip  *_clipState; // clip state
		Render::Options _opts;
		Mutex _mutex; // submit swap mutex
		// texture pool, key(w << 40 | h << 8 | colorType << 1 | mipmap),
		// value is texture handle and ref count
		Dict<uint64_t, Array<Sp<ImageSource>>> _texPools;
		Sp<CAPABuilder> _capaBuilder; // compute shader batch builder for CAPA
		friend class GC_Filter;
		friend class GC_BlurFilter;
		friend class CAPABuilder;
		Qk_DEFINE_INLINE_CLASS(Inl);
	};

}
#endif
