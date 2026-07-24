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

#include "../mem_allocator.h"
#include "../gpu_canvas.h"
#include "./mtl_shaders.h"

class AppleMetalRender;

namespace qk {
	class MetalRender;
	typedef MemBlockAllocator<MTLBufferID> MTLMemBufferAllocator;
	typedef MTLMemBufferAllocator::MemBlock MTLMemBlock;
	typedef const MTLMemBlock cMTLMemBlock;

	struct MTL_CmdPack {
		inline bool isRecorded() const {
			return recorded || cmds.length();
		}
		template <typename U>
		inline cMTLMemBlock alloc(uint32_t size) {
			return allocator->alloc<U>(size);
		}
		Sp<MTLMemBufferAllocator> allocator = nullptr; // buffers allocator
		Array<MTLCommandBufferID> cmds; // command buffers
		MTLCommandBufferID current = nullptr; // current command buffer for render pass
		MTLPassDesc pass = nullptr; // current render pass descriptor for enc
		MTLEncoder enc = nullptr; // current encoder for render
		MTLPipeline pipeline = nullptr; // current pipeline state for render
		bool recorded = false; // whether current command buffer has recorded commands
		bool beginPass = false; // whether call beginPass, used to determine whether need to create encoder for current pass
	};

	class MetalCanvas: public GPUCanvas {
	public:
		MetalCanvas(MetalRender *render, Render::Options opts);
		~MetalCanvas() override;
		bool swapBuffer() override;
		Array<MTLCommandBufferID> flushBuffer(); // flush front buffer and return mtl command buffers
		bool isRecorded() const { return _cmdPackFront.isRecorded(); }
		void vportCopy(MTLCommandBufferID cmd, MTLDrawableID dst);
	private:
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
		MTLEncoder useTexture0(const PaintImage *paint, int dstSlot, bool* isYuv = nullptr);
		void flushSubcanvasCmd(GPUCanvas *sub) override;
		void setSurfaceCmd(bool changeSize) override;
		void setMatrixCmd() override;
		void setBlendModeCmd() override;
		void drawClipCmd(const VertexData &vertex, GC_State::Clip *lastClip, GC_State::Clip *clip, ClipOp rawOp) override;
		void restoreClipCmd(GC_State::Clip* clip) override;
		void clearColorCmd(const Color4f &color, GC_ClearFlags flags) override;
		void drawImageCmd(const VertexData &vertex, const GC_ImageDrawInfo &info) override;
		void drawGradientCmd(const VertexData &vertex, const PaintGradient *paint, const Color4f &color) override;
		void drawColorCmd(const VertexData &vertex, const Color4f &color) override;
		bool drawCAPACmd(CAPADrawData &data) override;
		void drawRRectBlurColorCmd(const Rect& rect, const float *radius, float blur, const Color4f &color) override;
		void blurFilterBeginCmd(Range bounds, Mat4 &rootMat, ImageSource *tmpA) override;
		void blurFilterEndCmd(Range bounds, Mat4 &recoverRootMat, float radius, float clearPad,
				int sample, int imageLod, ImageSource *tmpA, ImageSource *tmpB) override;
		void drawTrianglesCmd(const Triangles& triangles, const PaintImage *paint, const Color4f &color, bool copyData) override;
		void readImageCmd(const Rect &srcRect, ImageSource* src, ImageSource* dest) override;
		void outputImageBeginCmd(ImageSource* img) override;
		void outputImageEndCmd(ImageSource* exit) override;
		void clearColor(const Color4f &color, const Range *range);
		void copyImage(ImageSource *src, Vec2 srcOffset, Range dst, Vec2 resolution);
		void drawColor(const VertexData &vertex, const Color4f &color, Vec4 offset, uint32_t flags);
		void setSurface(const Mat4& root, Vec2 surfaceSize, Vec2 scale) override;
		const MemBlockAllocator<MTLBufferID>::MemBlock&
			buildGradientBuffer(const PaintGradient *paint, const Color4f &color);
		bool onlyEndEncoderPass(Color4f &color);
	private:
	// fields:
		MetalRender *_mtlrender; // render backend
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
		bool _supportsSamplerClampToZero; // whether device supports sampler clamp to zero.
		MSLShaders _shaders; // shader source and pipeline state cache, for canvas use
		MTLArgumentEncoderID _capaCompositeSet2Encoder, _capaCompositeSet3Encoder;
		Dict<uint32_t, MTLSampler> _texSamplers;
		friend class ::AppleMetalRender;
	};

} // namespace qk

#endif
