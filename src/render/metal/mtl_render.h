/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

// @private head

#ifndef __quark_render_metal_mtlrender__
#define __quark_render_metal_mtlrender__

#include "../render.h"
#include "../blend.h"
#include "./mtl_canvas.h"

namespace qk {

	// Global render resource,
	// used for texture/vertex data creation, shader function and pipeline state caching
	class MetalRenderResource: public RenderResource {
	public:
		~MetalRenderResource();
		void post_message(Cb cb) override;
		bool uploadTexture(cPixel *pix, int levels, TexStat *out, bool mipmap) override;
		bool uploadVertexData(VertexData::ID *id) override;
		void unloadTexture(TexStat *tex) override;
		void unloadVertexData(VertexData::ID *id) override;
		MTLPipeline getPipeline(MSLPipelineKind kind, BlendMode mode, MTLPixelFormat format);
		MTLSampler get_sampler(const PaintImage* paint);
		MTLSampler get_sampler(PaintImage::FilterMode filter, PaintImage::MipmapMode mipmap);
		MSLShaders& shaders() { return _shaders; }
		MTLDeviceID device() { return _device; }
	private:
		explicit MetalRenderResource();
		MTLFunctionID getShaderFunction(MSLPipelineKind kind, bool vertex);
	// fields:
		Mutex _mutex; // protect shared resource
		MSLShaders _shaders; // shader source and pipeline state cache
		MTLDeviceID _device;
		MTLCommandQueueID _commandQueue;
		Dict<uint32_t, MTLFunctionID> _functions; // key = (kind << 1) | vertex, value = MTLFunctionID
		// key = (kind << 16) | (blendMode << 8) | (outputType << 4) | sampleCount, value = MTLPipeline
		Dict<uint32_t, MTLPipeline> _pipelines;
		Dict<uint32_t, MTLSampler> _texSamplers; // PaintImage => Sampler, indexed by PaintImage bitfields (tile/filter/mipmap modes)
		friend MetalRenderResource* getSharedRenderMetalResource();
		friend class MetalRender;
	};

	// Metal render backend implementation for iOS and macOS
	class MetalRender: public RenderBackend {
	public:
		~MetalRender() override;
		void release() override;
		void reload() override;
		Canvas* createCanvas(Options opts) override;
		TexStat createTextureStat(Vec2 size, ColorType type, bool mipmap) override;
		bool uploadTexture(cPixel *pix, int levels, TexStat *out, bool mipmap) override;
		bool uploadVertexData(VertexData::ID *id) override;
		void unloadTexture(TexStat *tex) override;
		void unloadVertexData(VertexData::ID *id) override;
		virtual void lock(); // lock render
		virtual void unlock(); // unlock render
	protected:
		explicit MetalRender(Options opts);
	// fields:
		MetalRenderResource* _resource; // shared render resource, used for texture/vertex data creation
		MetalCanvas *_mtlcanvas; // current main canvas, owned by this backend
		MTLDeviceID _device; // Metal device
		MTLCommandQueueID _commandQueue; // Metal command queue
		MTLBufferID _emptyBuffer; // empty vertex buffer 128 bytes
		MTLSampler _nearestSampler; // sampler state for nearest filter mode
		MTLSampler _linearSampler; // sampler state for linear filter mode, kLinear_FilterMode and kLinearNearest_MipmapMode
		MTLPipeline _vportCpPipeline; // pipeline for viewport copy
		friend class MetalCanvas;
	};

} // namespace qk

#endif
