/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __quark_render_metal_mtlrender__
#define __quark_render_metal_mtlrender__

#include "../render.h"
#include "../blend.h"
#include "../canvas.h"
#include "./mtl_shaders.h"

namespace qk {
	class MetalCanvas;

	// Global render resource,
	// used for texture/vertex data creation, shader function and pipeline state caching
	class MetalRenderResource: public RenderResource {
	public:
		~MetalRenderResource();
		void post_message(Cb cb) override;
		bool createTexture(cPixel *pix, int levels, TexStat *&out, bool mipmap) override;
		void deleteTexture(TexStat *tex) override;
		bool createVertexData(VertexData::ID *id) override;
		void deleteVertexData(VertexData::ID *id) override;
		MTLRenderPipelineStateID
		getPipeline(MSLPipelineKind kind, BlendMode mode, ColorType outputType, uint32_t sampleCount);
	private:
		explicit MetalRenderResource();
		MTLFunctionID getShaderFunction(MSLPipelineKind kind, bool vertex);
	// fields:
		Mutex _mutex; // protect shared resource
		MSLShaders _shaders; // shader source and pipeline state cache
		MTLDeviceID _device;
		MTLCommandQueueID _commandQueue;
		Dict<uint32_t, MTLFunctionID> _functions; // key = (kind << 1) | vertex, value = MTLFunctionID
		// key = (kind << 16) | (blendMode << 8) | (outputType << 4) | sampleCount, value = MTLRenderPipelineStateID
		Dict<uint32_t, MTLRenderPipelineStateID> _pipelines;
		friend RenderResource* getSharedRenderResource();
		friend class MetalRender;
	};

	// Metal render backend implementation for iOS and macOS
	class MetalRender: public RenderBackend {
	public:
		~MetalRender() override;
		void release() override;
		void reload() override;
		Canvas* createCanvas(Options opts) override;
		bool createTexture(cPixel *pix, int levels, TexStat *&out, bool mipmap) override;
		bool createVertexData(VertexData::ID *id) override;
		void deleteTexture(TexStat *tex) override;
		void deleteVertexData(VertexData::ID *id) override;
		virtual void lock(); // lock render
		virtual void unlock(); // unlock render
	protected:
		MetalRender(Options opts);
	// fields:
		MetalRenderResource* _resource; // shared render resource, used for texture/vertex data creation
		MetalCanvas *_mtlcanvas;
		MSLShaders _shaders; // shader source and pipeline state cache, for render thread use
		MTLDeviceID _device;
		MTLCommandQueueID _commandQueue;
		friend class MetalCanvas;
	};

} // namespace qk

#endif
