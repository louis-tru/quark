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

namespace qk {
	uint32_t alignUp(uint32_t ptr, uint32_t alignment = alignof(void*));

	constexpr bool isAlignUp(uint32_t ptr) {
		constexpr auto alignment = alignof(void*);
		return ((ptr + (alignment - 1)) & ~(alignment - 1)) == ptr;
	}

	/**
	 * @class MemBlockAllocator - A simple memory allocator for GPU buffers, managing memory in blocks.
	 * Allocates memory in fixed-size blocks and provides an interface for allocating memory within those blocks.
	*/
	template<class T> struct MemBlockAllocator {
		struct MemBlock {
			T val; uint32_t begin = 0, end = 0, capacity = 0;
		};
		MemBlockAllocator(uint32_t blockCapacity = 65536)
				: blockCapacity(blockCapacity), blockIndex(0) {
			Qk_ASSERT(isAlignUp(blockCapacity), "Block capacity must be aligned.");
			blocks.push(createBlock(blockCapacity));
			currentBlock = blocks.val();
		}
		~MemBlockAllocator() {
			for (auto &block: blocks)
				deleteBlock(block);
		}
		MemBlock createBlock(uint32_t capacity);
		void deleteBlock(MemBlock &block);
		void clear() {
			currentBlock = blocks.val();
			currentBlock->begin = currentBlock->end = 0;
			blockIndex = 0;
		}
		MemBlock* alloc(uint32_t size, uint32_t reserve) {
			Qk_ASSERT(reserve >= size, "Reserve size must be greater than requested size.");
			Qk_ASSERT(reserve <= blockCapacity, "Requested size exceeds block capacity.");
			size = alignUp(size);
			auto block = currentBlock;
			auto newEnd = block->end + size;
			if (block->end + reserve > block->capacity) {
				if (++blockIndex == blocks.length()) {
					blocks.push(createBlock(blockCapacity));
				}
				block = blocks.val() + blockIndex;
				block->end = 0; // reset block end for new block
				newEnd = size;
				currentBlock = block;
			}
			block->begin = block->end;
			block->end = newEnd;
			return block;
		}
		Qk_DISABLE_COPY(MemBlockAllocator);
		Array<MemBlock> blocks; // memory blocks
		MemBlock        *currentBlock; // current block for alloc
		uint32_t        blockIndex; // current block index
		uint32_t        blockCapacity; // default block capacity, should be aligned
	};

	struct GC_State { // gpu canvas state
		struct Clip: Reference { // clip state
			Range           range; // clip offset for clip path bounds
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

	class GC_Filter;
	class GC_BlurFilter;

	/**
	 * @class GPUCanvas - A Canvas implementation that renders using GPU acceleration.
	 */
	class GPUCanvas: public Canvas {
	public:
		GPUCanvas(Render *render, Render::Options opts);
		~GPUCanvas() override;
		Vec2 size() override; // _size = surfaceSize / scale
		bool isGpu() override;
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
		void clipPathv(const Pathv& path, ClipOp op, bool antiAlias) override;
		void clipRect(const Rect& rect, ClipOp op, bool antiAlias) override;
		void clearColor(const Color4f& color) override;
		void drawColor(const Color4f& color, BlendMode mode) override;
		void drawPath(const Path& path, const Paint& paint) override;
		void drawPathv(const Pathv& path, const Paint& paint) override;
		void drawPathvColor(const Pathv &path, const Color4f &color, BlendMode mode, bool antiAlias) override;
		void drawPathvColors(const Pathv* path[], int count, const Color4f &color, BlendMode mode, bool antiAlias) override;
		void drawRRectBlurColor(const Rect& rect,
			const float radius[4], float blur, const Color4f &color, BlendMode mode) override;
		void drawRect(const Rect& rect, const Paint& paint) override;
		void drawRRect(const Rect& rect, const Path::BorderRadius &radius, const Paint& paint) override;
		float drawGlyphs(const FontGlyphs &glyphs, Vec2 origin, const Array<Vec2> *offset, const Paint &paint) override;
		void drawTextBlob(TextBlob *blob, Vec2 origin, float fontSize, const Paint &paint) override;
		void drawTriangles(const Triangles& triangles, const Paint &paint, bool copyData) override;
		Sp<ImageSource> readImage(const Rect &src, Vec2 dst, ColorType type, BlendMode mode, bool mipmap) override;
		Sp<ImageSource> outputImage(ImageSource* dst, bool mipmap) override;
		PathvCache* getPathvCache() override;
		void setSurface(const Mat4& root, Vec2 surfaceSize, Vec2 surfaceScale) override;
		Vec2 surfaceSize() { return _surfaceSize; }
		const Render::Options& opts() const { return _opts; }
	protected:
		virtual void setSurfaceCmd(bool changeSize) = 0;
		virtual void setMatrixCmd() = 0;
		virtual void setBlendModeCmd() = 0;
		virtual void drawClipCmd(const VertexData &vertex, const VertexData &aaSide,
				GC_State::Clip *lastClip, GC_State::Clip *clip, ClipOp rawOp) = 0;
		virtual void clearColorCmd(const Color4f &color, GC_ClearFlags flags) = 0;
		virtual void drawImageCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color) = 0;
		virtual void drawGradientCmd(const VertexData &vertex, const PaintGradient *paint, const Color4f &color) = 0;
		virtual void drawImageMaskCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color) = 0;
		virtual void drawColorCmd(const VertexData &vertex, const Color4f &color) = 0;
		virtual void drawRRectBlurColorCmd(const Rect& rect, const float *radius, float blur, const Color4f &color) = 0;
		virtual void drawSDFImageMaskCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color,
				const Color4f &strokeColor, float stroke) = 0;
		virtual void blurFilterBeginCmd(Range bounds, Mat4 &rootMat, ImageSource *tmpA) = 0;
		virtual void blurFilterEndCmd(Range bounds, Mat4 &recoverRootMat, float radius, float clearPad,
				int sample, int imageLod, ImageSource *tmpA, ImageSource *tmpB) = 0;
		virtual void drawTrianglesCmd(const Triangles& triangles, const PaintImage *paint,
				const Color4f &color, bool copyData) = 0;
		virtual void readImageCmd(const Rect &srcRect, ImageSource* src, ImageSource* dst) = 0;
		virtual void outputImageBeginCmd(ImageSource* dst) = 0;
		virtual void outputImageEndCmd(ImageSource* exit) = 0;
		virtual void restoreClipCmd(GC_State::Clip* clip) = 0;
		// get texture count from pool and add ref count
		Sp<ImageSource> getTextureFromPool(Vec2 size, ColorType type, bool mipmap);
	// fields:
		Array<GC_State> _stateStack; // state
		GC_State    *_state; // state pointer
		PathvCache *_cache;
		Render 	 *_render; // render backend
		Vec2  _surfaceSize, _surfaceScale; // surface scale and size, surfaceSize = surfaceScale * size
		Vec2   _size, _scale; // size=surfaceSize/surfaceScale, _scale = matrix scale extracted
		float  _surfaceScaleAverage, _scaleAverage, _allScaleAverage; // average of x/y scale
		float  _allScaleMin; // _surfaceScaleAverage * min(scale)
		float  _phy2Pixel; // _phy2Pixel = 2 / _allScaleMin
		Mat4   _rootMatrix;
		BlendMode _blendMode; // blend mode state
		uint8_t  _DeviceMsaa; // device anti alias, msaa
		GC_State::Clip  *_clipState; // clip state
		Render::Options _opts;
		Mutex _mutex; // submit swap mutex
		// texture pool, key(w << 40 | h << 8 | colorType << 1 | mipmap),
		// value is texture handle and ref count
		Dict<uint64_t, Array<Sp<ImageSource>>> _texPools;
		friend class GC_Filter;
		friend class GC_BlurFilter;
		Qk_DEFINE_INLINE_CLASS(Inl);
	};

}
#endif
