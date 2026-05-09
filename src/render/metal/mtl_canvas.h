/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __quark_render_metal_mtlcanvas__
#define __quark_render_metal_mtlcanvas__

#include "../canvas.h"
#include "../pathv_cache.h"
#include "./mtl_render.h"

#ifdef __OBJC__
# import <Metal/Metal.h>
#endif

namespace qk {

	class MetalRender;

	class MetalCanvas: public Canvas, public PathvCache::ClearSync {
	public:
		MetalCanvas(MetalRender *render, Render::Options opts);
		~MetalCanvas() override;

		int save() override;
		void restore(uint32_t count) override;
		int getSaveCount() const override;
		const Mat& getMatrix() const override;
		void setMatrix(const Mat& mat) override;
		void setTranslate(Vec2 val) override;
		void translate(Vec2 val) override;
		void scale(Vec2 val) override;
		void rotate(float z) override;

		void clipPath(const Path& path, ClipOp op, bool antiAlias) override;
		void clipPathv(const Pathv& path, ClipOp op, bool antiAlias) override;
		void clipRect(const Rect& rect, ClipOp op, bool antiAlias) override;

		void clearColor(const Color4f& color) override;
		void drawColor(const Color4f& color, BlendMode mode) override;
		void drawPath(const Path& path, const Paint& paint) override;
		void drawPathv(const Pathv& path, const Paint& paint) override;
		void drawPathvColor(const Pathv &path, const Color4f &color, BlendMode mode, bool antiAlias) override;
		void drawPathvColors(const Pathv* path[], int count, const Color4f &color, BlendMode mode, bool antiAlias) override;
		void drawRRectBlurColor(const Rect& rect, const float radius[4], float blur, const Color4f &color, BlendMode mode) override;
		void drawRect(const Rect& rect, const Paint& paint) override;
		void drawRRect(const Rect& rect, const Path::BorderRadius &radius, const Paint& paint) override;
		float drawGlyphs(const FontGlyphs &glyphs, Vec2 origin, const Array<Vec2> *offset, const Paint &paint) override;
		void drawTextBlob(TextBlob *blob, Vec2 origin, float fontSize, const Paint &paint) override;
		void drawTriangles(const Triangles& triangles, const Paint &paint) override;

		bool readPixels(uint32_t srcX, uint32_t srcY, Pixel* dst) override;
		Sp<ImageSource> readImage(const Rect &src, Vec2 dest, ColorType type, BlendMode mode, bool isMipmap) override;
		Sp<ImageSource> outputImage(ImageSource* dest, bool isMipmap) override;

		void swapBuffer() override;
		void flushBuffer();
		PathvCache* getPathvCache() override;
		void setSurface(const Mat4& root, Vec2 surfaceSize, Vec2 scale) override;
		Vec2 size() override;
		bool isGpu() override;
		void lock() override;
		void unlock() override;

#ifdef __OBJC__
		id<MTLTexture> colorTexture() const { return _colorTex; }
#endif

	private:
		struct State {
			Mat matrix;
		};

		enum OpType: uint8_t {
			kClear_Op,
			kSolid_Op,
			kImage_Op,
			kImageMask_Op,
			kSdfMask_Op,
		};

		struct DrawOp {
			OpType type;
			Mat matrix;
			VertexData vertex;
			Color4f color;
			Color4f strokeColor;
			PaintImage image;
			Sp<ImageSource> imageSource;
			BlendMode blendMode;
			float strokeWidth;
			int alphaIndex;
			bool fullClear;
		};

		void recordSolid(const VertexData &vertex, const Color4f &color, BlendMode mode);
		void recordImage(OpType type, const VertexData &vertex, const PaintImage *image,
			const Color4f &color, BlendMode mode, int alphaIndex = 3,
			const Color4f &strokeColor = Color4f(), float strokeWidth = 0);
		void drawPaintPathv(const Pathv &path, const Paint &paint);
		float drawTextImage(Typeface::TextImage &img, float scale, Vec2 origin, const Paint &paint);
		void ensureTarget();

		Array<State> _stateStack;
		State *_state;
		Array<DrawOp> _ops;
		Array<DrawOp> _frontOps;
		MetalRender *_render;
		PathvCache *_cache;
		CondMutex _mutex;
		Render::Options _opts;
		Mat4 _rootMatrix;
		Vec2 _surfaceSize;
		Vec2 _size;
		Vec2 _scale;
		float _surfaceScale;

#ifdef __OBJC__
		id<MTLTexture> _colorTex;
#endif
	};

} // namespace qk

#endif
