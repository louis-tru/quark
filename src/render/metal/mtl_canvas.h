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

	struct MTL_CmdPack {
		inline bool isRecorded() const {
			return recorded || cmds.length();
		}
		Array<MTLCommandBuffer> cmds; // command buffers
		MTLCommandBuffer current = nullptr; // current command buffer for render pass
		MTLPassDesc pass = nullptr; // current render pass descriptor for enc
		MTLRenderEncoder enc = nullptr; // current encoder for render
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
	private:
		inline Color4f premul_alpha(const Color4f &color) const {
			return color.premul_alpha();
		}
		inline MTLPipeline getPipeline(MSLShader& shader) {
			return shader.getPipeline(_blendMode, _opts.colorType, _opts.msaaSample);
		}
		void setPipeline(MTLRenderEncoder enc, MSLShader& shader);
		MTLPassDesc beginPass();
		MTLPassDesc beginPassFrom(int level, bool loadColor = true, bool disableDepth = false);
		MTLRenderEncoder getEncoder();
		void endPass();
		// set enc pipeline state for shader
		// get encoder and set pipeline state for shader, also ensure vertex data is valid and set for draw call
		// if vertex data is invalid, return nullptr and skip draw call
		MTLRenderEncoder usePipeline(MSLShader& shader, const VertexData &vertex);
		MTLRenderEncoder usePipeline(MSLShader& shader, const VertexData &vertex, MTLRenderEncoder enc);
		MTLRenderEncoder usePipeline(MSLShader& shader);
		MTLRenderEncoder useTextureSlot0(const PaintImage *paint, int dstSlot, bool* isYuv = nullptr);
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
		void copyImage(ImageSource *src, Vec2 srcOffset, Range dst, Vec2 resolution, float depth);
		void drawColor(const VertexData &vertex, const Color4f &color, Vec4 surfaceOffset, float depth, int flags);
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
	};

} // namespace qk

#endif
