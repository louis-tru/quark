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
		Array<MTLCommandBuffer> cmds; // command buffers
		MTLCommandBuffer current = nullptr; // current command buffer for render pass
		MTLPassDescriptor pass = nullptr; // current render pass descriptor for enc
		MTLRenderEncoder enc = nullptr; // current encoder for render
		MTLPipeline pipeline = nullptr; // current pipeline state for render
		bool recorded = false; // whether current command buffer has recorded commands
		inline bool isRecorded() const {
			return recorded || cmds.length();
		}
	};

	class MetalCanvas: public GPUCanvas {
	public:
		MetalCanvas(MetalRender *render, Render::Options opts);
		~MetalCanvas() override;
		bool swapBuffer() override;
		Array<MTLCommandBuffer> flushBuffer(); // flush front buffer and return mtl command buffers
		void flushSubcanvas(MetalCanvas *sub); // flush subcanvas to current canvas
	private:
		inline MTLPipeline getPipeline(MSLShader& shader) {
			return shader.getPipeline(_blendMode, _opts.colorType, _opts.msaaSample);
		}
		inline void setPipeline(MTLRenderEncoder enc, MSLShader& shader);
		virtual void setBuffers(Vec2 size);
		MTLPassDescriptor beginPass();
		MTLPassDescriptor beginPass(int level, bool loadColor);
		MTLRenderEncoder getEncoder();
		void endPass();
		// set enc pipeline state for shader
		// get encoder and set pipeline state for shader, also ensure vertex data is valid and set for draw call
		// if vertex data is invalid, return nullptr and skip draw call
		MTLRenderEncoder useShader(MSLShader& shader, const VertexData &vertex);
		MTLRenderEncoder useShader(MTLRenderEncoder enc, MSLShader& shader, const VertexData &vertex);
		MTLRenderEncoder useTextureSlot0(const PaintImage *paint, bool* isYuv = nullptr);
		void setSurfaceCmd(bool changeSize) override;
		void setMatrixCmd() override;
		void setBlendModeCmd() override;
		void enableStencilTestCmd(bool enable) override;
		void drawClipCmd(const GC_State::Clip &clip, uint32_t ref, bool revoke) override;
		void clearColorCmd(const Color4f &color, GC_ClearFlags flags) override;
		void drawImageCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color) override;
		void drawGradientCmd(const VertexData &vertex, const PaintGradient *paint, const Color4f &color) override;
		void drawImageMaskCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color) override;
		void drawColorCmd(const VertexData &vertex, const Color4f &color) override;
		void drawRRectBlurColorCmd(const Rect& rect, const float *radius, float blur, const Color4f &color) override;
		void drawSDFImageMaskCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color,
				const Color4f &strokeColor, float stroke) override;
		void blurFilterBeginCmd(Range bounds, float radius, float clearPad) override;
		void blurFilterEndCmd(Range bounds, float radius, float clearPad, int sample, int imageLod) override;
		void drawTrianglesCmd(const Triangles& triangles, const PaintImage *paint, const Color4f &color, bool copyData) override;
		void readImageCmd(const Rect &srcRect, ImageSource* src, ImageSource* dest) override;
		void outputImageBeginCmd(ImageSource* img) override;
		void outputImageEndCmd(ImageSource* exit) override;
		void drawRegion(const Color4f &color, const Range &region, float depth);
		void clearRegion(const Range &region, float scale, float offsetY, float depth);
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
		//   _outTexA / _outTexB   : temporary ping-pong render targets
		// actual render passes always write into _outColorTex.
		MTLTextureID _outColorTex; //
		MTLTextureID _outDepthTex; // Depth stencil buffer object of texture
		MTLTextureID _outAaclipTex; // Aaclip buffer object of texture
		MTLTextureID _outTexA, _outTexB; // temp texture for render
		bool _enableStencilTest; // whether stencil test enabled
	};

} // namespace qk

#endif
