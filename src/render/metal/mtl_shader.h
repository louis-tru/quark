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

#ifndef __quark_render_metal_mtl_shader__
#define __quark_render_metal_mtl_shader__

#include "../../util/dict.h"
#include "../math.h"
#include "../pixel.h"
#include "../blend.h"

#ifdef __OBJC__
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#endif

namespace qk {
#ifdef __OBJC__
	typedef id<NSObject> NSObjectID;
	typedef id<MTLRenderPipelineState> MTLPipeline;
	typedef id<MTLComputePipelineState> MTLComputePipeline;
	typedef id<MTLFunction> MTLFunctionID;
	typedef id<MTLDevice> MTLDeviceID;
	typedef id<MTLCommandQueue> MTLCommandQueueID;
	typedef id<MTLSamplerState> MTLSampler;
	typedef id<MTLRenderCommandEncoder> MTLEncoder;
	typedef id<MTLTexture> MTLTextureID;
	typedef id<MTLCommandBuffer> MTLCommandBufferID;
	typedef MTLRenderPassDescriptor* MTLPassDesc;
	typedef id<CAMetalDrawable> MTLDrawable;
	typedef id<MTLBuffer> MTLBufferID;
	typedef id<CAMetalDrawable> MTLDrawableID;
#else
	typedef void* NSObjectID;
	typedef void* MTLPipeline;
	typedef void* MTLComputePipeline;
	typedef void* MTLFunctionID;
	typedef void* MTLDeviceID;
	typedef void* MTLCommandQueueID;
	typedef void* MTLSampler;
	typedef void* MTLEncoder;
	typedef void* MTLTextureID;
	typedef void* MTLCommandBufferID;
	typedef void* MTLPassDesc;
	typedef void* MTLDrawable;
	typedef void* MTLBufferID;
	typedef int MTLPixelFormat;
	typedef void* MTLDrawableID;
#endif

	struct Vec3Padding {
		Vec3 value;
		float padding;
	};

	struct IVec3Padding {
		IVec3 value;
		int32_t padding;
	};

	enum MSLPipelineKind: uint8_t;

	struct MSLShaderSource {
		const char *name;
		MSLPipelineKind kind;
		String (*vertexSource)();
		String (*fragmentSource)();
		String (*computeSource)();
	};

	struct MSLShaderAttr {
		uint32_t bufferIndex; // vertex buffer index
		uint32_t size; // glsl attribute size, for vec4 colors[8]; size = 4*8 = 32
		uint32_t format; // MTLVertexFormat
		uint32_t sizeOf; // for example: sizeof(float)*4 for vec4
	};

	struct MSLShaders;

	struct MSLShader {
		MSLShaderSource source; // shader source
		Array<MSLShaderAttr> attributes; // vertex attributes format
		uint32_t bufferIndex; // vertex buffer index
		MTLPipeline getPipeline(BlendMode mode, MTLPixelFormat format);
		MTLComputePipeline getComputePipeline();
	protected:
		Dict<uint32_t, NSObjectID> _pipelines;
		friend class MSLShaders;
	};
}

#endif
