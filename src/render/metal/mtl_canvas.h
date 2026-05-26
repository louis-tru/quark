/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

// @private head

#ifndef __quark_render_metal_mtlcanvas__
#define __quark_render_metal_mtlcanvas__

#include "../gpu_canvas.h"
#include "./mtl_shaders.h"

namespace qk {
	class MetalRender;

	// Specialized Objective-C objects for Metal rendering.
	template<> struct IsPointer<MTLCommandBuffer> {
		static constexpr bool value = false;
	};

	struct MTL_CmdPack {
		inline bool isRecorded() const {
			return recorded || cmds.length();
		}
		Sp<MemBlockAllocator<MTLBufferID>> buffer = nullptr; // vertex/index buffers allocator
		Array<MTLCommandBuffer> cmds; // command buffers
		MTLCommandBuffer current = nullptr; // current command buffer for render pass
		MTLPassDesc pass = nullptr; // current render pass descriptor for enc
		MTLEncoder enc = nullptr; // current encoder for render
		MTLPipeline pipeline = nullptr; // current pipeline state for render
		bool recorded = false; // whether current command buffer has recorded commands
	};

	class MetalCanvas: public GPUCanvas {
	public:
		MetalCanvas(MetalRender *render, Render::Options opts);
		~MetalCanvas() override;
		bool swapBuffer() override;
		Array<MTLCommandBuffer> flushBuffer(); // flush front buffer and return mtl command buffers
		void flushSubcanvas(MetalCanvas *sub); // flush subcanvas to current canvas
		// inline MTLTextureID outTex() { return _outTex; }
		bool isRecorded() const { return _cmdPackFront.isRecorded(); }
		void vportCopy(MTLCommandBuffer cmd, MTLDrawableID dst);
	private:
		inline Color4f premul_alpha(const Color4f &color) const {
			return color.premul_alpha();
		}
		bool use_texture(MTLEncoder enc, ImageSource *src, int srcSlot, int dstSlot, const PaintImage *paint);
		void set_texture_param(MTLEncoder enc, MTLTextureID tex, int dstSlot, const PaintImage* paint);
		MTLSampler get_sampler(const PaintImage* paint);
		MTLSampler get_sampler(PaintImage::FilterMode filter, PaintImage::MipmapMode mipmap);
		MTLPassDesc beginPass(int level = 0, bool loadColor = true);
		MTLEncoder getEncoder();
		void endPass(); // end current render pass
		void setPipeline(MTLEncoder enc, MTLPipeline pipeline);
		void setPipeline(MTLEncoder enc, MSLShader& shader);
		// set enc pipeline state for shader
		// get encoder and set pipeline state for shader, also ensure vertex data is valid and set for draw call
		// if vertex data is invalid, return nullptr and skip draw call
		inline MTLEncoder usePipeline(MSLShader& shader, const VertexData &vertex) {
			return usePipeline(shader, vertex, getEncoder());
		}
		inline MTLEncoder usePipeline(MSLShader& shader) {
			auto enc = getEncoder();
			return setPipeline(enc, shader), enc;
		}
		MTLEncoder usePipeline(MSLShader& shader, const VertexData &vertex, MTLEncoder enc);
		MTLEncoder useTextureSlot0(const PaintImage *paint, int dstSlot, bool* isYuv = nullptr);
		void setSurfaceCmd(bool changeSize) override;
		void setMatrixCmd() override;
		void setBlendModeCmd() override;
		void drawClipCmd(const VertexData &vertex, const VertexData &aafuzz, GC_State::Clip *lastClip,
				GC_State::Clip *clip, ClipOp rawOp) override;
		void restoreClipCmd(GC_State::Clip* clip) override;
		void clearColorCmd(const Color4f &color, GC_ClearFlags flags) override;
		void drawImageCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color) override;
		void drawGradientCmd(const VertexData &vertex, const PaintGradient *paint, const Color4f &color) override;
		void drawImageMaskCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color) override;
		void drawColorCmd(const VertexData &vertex, const Color4f &color) override;
		void drawRRectBlurColorCmd(const Rect& rect, const float *radius, float blur, const Color4f &color) override;
		void drawSDFImageMaskCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color,
				const Color4f &strokeColor, float stroke) override;
		void blurFilterBeginCmd(Range bounds, Mat4 &rootMat, ImageSource *tmpA) override;
		void blurFilterEndCmd(Range bounds, Mat4 &recoverRootMat, float radius, float clearPad,
				int sample, int imageLod, ImageSource *tmpA, ImageSource *tmpB) override;
		void drawTrianglesCmd(const Triangles& triangles, const PaintImage *paint, const Color4f &color, bool copyData) override;
		void readImageCmd(const Rect &srcRect, ImageSource* src, ImageSource* dest) override;
		void outputImageBeginCmd(ImageSource* img) override;
		void outputImageEndCmd(ImageSource* exit) override;
		void clearColor(const Color4f &color, const Range *range);
		void copyImage(ImageSource *src, Vec2 srcOffset, Range dst, Vec2 resolution, float depth);
		void drawColor(const VertexData &vertex, const Color4f &color, Vec4 surfaceOffset, float depth, uint32_t flags);
		void setSurface(const Mat4& root, Vec2 surfaceSize, Vec2 scale) override;
	private:
	// fields:
		MetalRender *_render; // render backend
		MTLDeviceID  _device; // Metal device
		MTLCommandQueueID _commandQueue; // Metal command queue
		MTL_CmdPack  _cmdPack, _cmdPackFront; // current and front command pack for render
		MTLTextureID _outTex; // main window/surface color render target texture
		// current active color render target texture.
		// may point to:
		//   _outTex               : default surface render target
		//   _state->output        : rendering directly into output image
		//   tmpA / tmpB           : temporary ping-pong render targets
		// actual render passes always write into _outColorTex.
		MTLTextureID _outColorTex; //
		MTLTextureID _outDepthTex; // Depth stencil buffer object of texture
		MSLShaders _shaders; // shader source and pipeline state cache, for canvas use
		Dict<uint32_t, MTLSampler> _texSamplers;
		MTLBufferID _gradientBuf; // temp buffer for gradient pos and color data
	};

} // namespace qk

#endif
