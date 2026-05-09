/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#import "./mtl_render.h"
#import "./mtl_canvas.h"
#import "./mtl_shaders.h"
#import "../pixel.h"
#import "../pathv_cache.h"
#import <Metal/Metal.h>

namespace qk {

	Render* make_metal_render(Render::Options opts) {
		return nil;
	}

	MTLPixelFormat mtl_pixel_format(ColorType type) {
		switch (type) {
			// case kRGB_888_ColorType: // Invalid
			case kAlpha_8_ColorType:
			case kLuminance_8_ColorType:
			case kYUV420P_Y_8_ColorType:
			case kYUV420P_U_8_ColorType:
				return MTLPixelFormatR8Unorm;
			case kLuminance_Alpha_88_ColorType:
			case kYUV420SP_UV_88_ColorType:
				return MTLPixelFormatRG8Unorm;
			case kRGBA_8888_ColorType:
			case kRGB_888X_ColorType:
				// RGBA order, used by opengl and vulkan, but not optimal for metal performance
				return MTLPixelFormatRGBA8Unorm; // RGBA
			case kBGRA_8888_ColorType:
			case kBGR_888X_ColorType:
				// BGRA order, used by windows d3d and macos metal, optimal for metal performance
				return MTLPixelFormatBGRA8Unorm; // BGRA
			case kRGBA_1010102_ColorType:
			case kRGB_101010X_ColorType:
				return MTLPixelFormatRGB10A2Unorm;
			case kSDF_F32_ColorType:
			case kSDF_Unsigned_F32_ColorType:
				return MTLPixelFormatR32Float;
		#if Qk_iOS
			// case kRGB_565_ColorType:
			// 	return MTLPixelFormatB5G6R5Unorm;
			// case kRGBA_4444_ColorType:
			// case kRGB_444X_ColorType:
			// 	return MTLPixelFormatABGR4Unorm;
			// case kRGBA_5551_ColorType:
			// 	return MTLPixelFormatA1BGR5Unorm;
			// Packed 565/4444/5551 formats are intentionally not mapped here.
			// They are awkward across Metal/macOS SDK versions and easy to mismatch in bit layout.
			case kPVRTCI_2BPP_RGB_ColorType:
				return MTLPixelFormatPVRTC_RGB_2BPP;
			case kPVRTCI_2BPP_RGBA_ColorType:
			case kPVRTCII_2BPP_ColorType:
				return MTLPixelFormatPVRTC_RGBA_2BPP;
			case kPVRTCI_4BPP_RGB_ColorType:
				return MTLPixelFormatPVRTC_RGB_4BPP;
			case kPVRTCI_4BPP_RGBA_ColorType:
			case kPVRTCII_4BPP_ColorType:
				return MTLPixelFormatPVRTC_RGBA_4BPP;
		#endif
			default:
				return MTLPixelFormatInvalid;
		}
	}

	void mtl_set_blend(MTLRenderPipelineColorAttachmentDescriptor *ca, BlendMode mode) {
		ca.blendingEnabled = YES;
		ca.rgbBlendOperation = MTLBlendOperationAdd;
		ca.alphaBlendOperation = MTLBlendOperationAdd;

		switch (mode) {
			case kClear_BlendMode: // r = 0
				ca.sourceRGBBlendFactor = MTLBlendFactorZero;
				ca.destinationRGBBlendFactor = MTLBlendFactorZero;
				ca.sourceAlphaBlendFactor = MTLBlendFactorZero;
				ca.destinationAlphaBlendFactor = MTLBlendFactorZero;
				break;

			case kSrc_BlendMode: // r = s
				ca.sourceRGBBlendFactor = MTLBlendFactorOne;
				ca.destinationRGBBlendFactor = MTLBlendFactorZero;
				ca.sourceAlphaBlendFactor = MTLBlendFactorOne;
				ca.destinationAlphaBlendFactor = MTLBlendFactorZero;
				break;

			case kDst_BlendMode: // r = d
				ca.sourceRGBBlendFactor = MTLBlendFactorZero;
				ca.destinationRGBBlendFactor = MTLBlendFactorOne;
				ca.sourceAlphaBlendFactor = MTLBlendFactorZero;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOne;
				break;

			case kSrcOverStraight_BlendMode: // r = sa*s + (1-sa)*d
				ca.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
				ca.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				ca.sourceAlphaBlendFactor = MTLBlendFactorOne;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				break;

			case kSrcOver_BlendMode: // r = s + (1-sa)*d
				ca.sourceRGBBlendFactor = MTLBlendFactorOne;
				ca.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				ca.sourceAlphaBlendFactor = MTLBlendFactorOne;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				break;

			case kDstOver_BlendMode: // r = (1-da)*s + d
				ca.sourceRGBBlendFactor = MTLBlendFactorOneMinusDestinationAlpha;
				ca.destinationRGBBlendFactor = MTLBlendFactorOne;
				ca.sourceAlphaBlendFactor = MTLBlendFactorOneMinusDestinationAlpha;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOne;
				break;

			case kSrcIn_BlendMode: // r = da*s
				ca.sourceRGBBlendFactor = MTLBlendFactorDestinationAlpha;
				ca.destinationRGBBlendFactor = MTLBlendFactorZero;
				ca.sourceAlphaBlendFactor = MTLBlendFactorDestinationAlpha;
				ca.destinationAlphaBlendFactor = MTLBlendFactorZero;
				break;

			case kDstIn_BlendMode: // r = sa*d
				ca.sourceRGBBlendFactor = MTLBlendFactorZero;
				ca.destinationRGBBlendFactor = MTLBlendFactorSourceAlpha;
				ca.sourceAlphaBlendFactor = MTLBlendFactorZero;
				ca.destinationAlphaBlendFactor = MTLBlendFactorSourceAlpha;
				break;

			case kSrcOut_BlendMode: // r = (1-da)*s
				ca.sourceRGBBlendFactor = MTLBlendFactorOneMinusDestinationAlpha;
				ca.destinationRGBBlendFactor = MTLBlendFactorZero;
				ca.sourceAlphaBlendFactor = MTLBlendFactorOneMinusDestinationAlpha;
				ca.destinationAlphaBlendFactor = MTLBlendFactorZero;
				break;

			case kDstOut_BlendMode: // r = (1-sa)*d
				ca.sourceRGBBlendFactor = MTLBlendFactorZero;
				ca.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				ca.sourceAlphaBlendFactor = MTLBlendFactorZero;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				break;

			case kSrcATop_BlendMode: // r = da*s + (1-sa)*d
				ca.sourceRGBBlendFactor = MTLBlendFactorDestinationAlpha;
				ca.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				ca.sourceAlphaBlendFactor = MTLBlendFactorDestinationAlpha;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				break;

			case kDstATop_BlendMode: // r = (1-da)*s + sa*d
				ca.sourceRGBBlendFactor = MTLBlendFactorOneMinusDestinationAlpha;
				ca.destinationRGBBlendFactor = MTLBlendFactorSourceAlpha;
				ca.sourceAlphaBlendFactor = MTLBlendFactorOneMinusDestinationAlpha;
				ca.destinationAlphaBlendFactor = MTLBlendFactorSourceAlpha;
				break;

			case kXor_BlendMode: // r = (1-da)*s + (1-sa)*d
				ca.sourceRGBBlendFactor = MTLBlendFactorOneMinusDestinationAlpha;
				ca.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				ca.sourceAlphaBlendFactor = MTLBlendFactorOneMinusDestinationAlpha;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				break;

			case kPlus_BlendMode: // r = s + d
				ca.sourceRGBBlendFactor = MTLBlendFactorOne;
				ca.destinationRGBBlendFactor = MTLBlendFactorOne;
				ca.sourceAlphaBlendFactor = MTLBlendFactorOne;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOne;
				break;

			case kModulateStraight_BlendMode: // r = s*d
				ca.sourceRGBBlendFactor = MTLBlendFactorZero;
				ca.destinationRGBBlendFactor = MTLBlendFactorSourceColor;
				ca.sourceAlphaBlendFactor = MTLBlendFactorZero;
				ca.destinationAlphaBlendFactor = MTLBlendFactorSourceAlpha;
				break;

			case kScreenStraight_BlendMode: // r = s + (1-s)*d
				ca.sourceRGBBlendFactor = MTLBlendFactorOne;
				ca.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceColor;
				ca.sourceAlphaBlendFactor = MTLBlendFactorOne;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				break;

			case kMultiplyStraight_BlendMode: // r = d*s + (1-sa)*d
				ca.sourceRGBBlendFactor = MTLBlendFactorDestinationColor;
				ca.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				ca.sourceAlphaBlendFactor = MTLBlendFactorDestinationAlpha;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				break;

			case kPlusStraight_BlendMode: // r = sa*s + d
				ca.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
				ca.destinationRGBBlendFactor = MTLBlendFactorOne;
				ca.sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOne;
				break;
		}
	}

	uint32_t pipeline_key(MSLPipelineKind kind, BlendMode mode, ColorType outputType, uint32_t sampleCount) {
		// kind: 8 bits, mode: 8 bits, outputType: 8 bits, sampleCount: 4 bits
		return ((uint32_t)kind << 20) | // 8 bits for pipeline kind
			((uint32_t)mode << 12) | // 8 bits for blend mode
			((uint32_t)outputType << 4) | // 8 bits for output type
			(uint32_t)(sampleCount & 0b1111) // 4 bits for sample count, max 16 samples
		;
	}

	RenderResource* getSharedRenderResource() {
		static MetalRenderResource *g_sharedRenderResource = new MetalRenderResource();
		return g_sharedRenderResource;
	}

	MetalRenderResource::MetalRenderResource()
		: _device(nil)
		, _commandQueue(nil)
	{
		id<MTLDevice> device = MTLCreateSystemDefaultDevice();
		Qk_CHECK(device, "Failed to create Metal device");
		id<MTLCommandQueue> commandQueue = [device newCommandQueue];
		Qk_CHECK(commandQueue, "Failed to create Metal command queue");
		_device = device; // retain device for render resource
		_commandQueue = commandQueue; // retain command queue for render resource
		_shaders.buildAll(); // pre-build all shader sources
	}

	MetalRenderResource::~MetalRenderResource() {
		_device = nil;
		_commandQueue = nil;
	}

	bool MetalRenderResource::createTexture(cPixel *pix, int levels, TexStat *&out, bool mipmap) {
		if (!pix || pix->length() == 0)
			return false;

		auto fmt = mtl_pixel_format(pix->type());
		if (fmt == MTLPixelFormatInvalid)
			return false;

		bool genMipmap = levels == 1 && mipmap;
		bool hasMipmaps = levels > 1 || mipmap;

		auto desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:fmt
																																	width:pix->width()
																																	height:pix->height()
																															mipmapped:hasMipmaps];

		desc.storageMode = MTLStorageModePrivate;
		desc.usage = MTLTextureUsageShaderRead;// | MTLTextureUsageBlitDestination;

		if (genMipmap) {
			// generateMipmapsForTexture needs to read existing levels and write generated levels.
			// desc.usage |= MTLTextureUsageBlitSource;
		}

		if (levels > 1) {
			desc.mipmapLevelCount = levels; // only provided levels exist
		}

		id<MTLTexture> tex = [_device newTextureWithDescriptor:desc];
		if (!tex)
			return false;

		id<MTLCommandBuffer> cmd = [_commandQueue commandBuffer];
		id<MTLBlitCommandEncoder> blit = [cmd blitCommandEncoder];

		// NSMutableArray<id<MTLBuffer>> *stagingBuffers = [NSMutableArray array];

		for (int i = 0; i < levels; i++) {
			auto p = pix + i;

			if (!p->val() || p->length() == 0)
				continue;

			NSUInteger width = (NSUInteger)p->width();
			NSUInteger height = (NSUInteger)p->height();
			NSUInteger bpp = (NSUInteger)Pixel::bytes_per_pixel(p->type());
			NSUInteger bytesPerRow = width * bpp;
			NSUInteger uploadSize = bytesPerRow * height;

			id<MTLBuffer> staging = [_device newBufferWithBytes:p->val()
																									length:uploadSize
																									options:MTLResourceStorageModeShared];

			if (!staging) {
				[blit endEncoding];
				return false;
			}

			// Keep staging buffer alive until command buffer completes.
			// [stagingBuffers addObject:staging];

			[blit copyFromBuffer:staging
							sourceOffset:0
				sourceBytesPerRow:bytesPerRow
			sourceBytesPerImage:uploadSize
								sourceSize:MTLSizeMake(width, height, 1)
								toTexture:tex
					destinationSlice:0
					destinationLevel:(NSUInteger)i
				destinationOrigin:MTLOriginMake(0, 0, 0)];
		}

		if (genMipmap) {
			[blit generateMipmapsForTexture:tex];
		}

		[blit endEncoding];

		// Capture stagingBuffers so ARC keeps upload buffers alive until GPU copy finishes.
		// [cmd addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
		// 	(void)stagingBuffers;
		// }];

		[cmd commit];

		if (!out) {
			out = new TexStat{.ptr = nullptr};
		}

		if (out->ptr) {
			CFBridgingRelease(out->ptr); // release old texture if exists
		}
		out->ptr = (void*)CFBridgingRetain(tex); // retain new texture for TexStat

		return true;
	}

	void MetalRenderResource::deleteTexture(TexStat *tex) {
		CFBridgingRelease(tex->ptr); // release texture
		tex->ptr = nullptr;
		delete tex;
	}

	bool MetalRenderResource::createVertexData(VertexData::ID *vid) {
		if (vid->c)
			return true;
		auto &vertex = vid->data->vertex;
		if (!vertex.length())
			return false;
		// create id<MTLBuffer>
		auto buf = [_device newBufferWithBytes:vertex.val()
																		length:vertex.size()
																		options:MTLResourceStorageModeShared];
		if (!buf) return false;
		// vid->c = (__bridge_retained void*)buf;
		vid->c = (void*)CFBridgingRetain(buf);
		return true;
	}

	void MetalRenderResource::deleteVertexData(VertexData::ID *vid) {
		if (vid->c) {
			// (void)(__bridge_transfer id<MTLBuffer>)vid->c;
			CFBridgingRelease(vid->c); // release buffer for vertex data
			vid->c = nullptr;
		}
	}

	void MetalRenderResource::post_message(Cb cb) {
		// directly execute callback for simplicity,
		// since current implementation does not need cross-thread resource access
		cb->resolve();
	}

	MTLFunctionID MetalRenderResource::getShaderFunction(MSLPipelineKind kind, bool vertex) {
		auto key = (kind << 1) | (vertex ? 1 : 0);
		MTLFunctionID fn = nil;
		if (_functions.get(key, fn))
			return fn;
		Qk_CHECK(kind < kPipelineCount, "Invalid pipeline kind: %d", kind);
		auto &src = _shaders.allShaders[kind]->source; // get shader source by pipeline kind
		auto code = vertex ? src.vertexSource(): src.fragmentSource();
		NSError *err = nil;
		auto library = [_device newLibraryWithSource:@(code.c_str()) options:nil error:&err];
		Qk_CHECK(library, "Metal shader compilation failed: %s", err.localizedDescription.UTF8String);
		auto entry = String::format("%s_%s", src.name, vertex ? "vert": "frag");
		fn = [library newFunctionWithName:@(entry.c_str())];
		Qk_CHECK(fn, "Metal shader entry not found: %s.%s", src.name, entry.c_str());
		return _functions[key] = fn;
	}

	MTLRenderPipelineStateID MetalRenderResource::getPipeline(MSLPipelineKind kind, BlendMode mode, ColorType outputType, uint32_t sampleCount) {
		ScopeLock lock(_mutex); // protect shader function cache
		auto key = pipeline_key(kind, mode, outputType, sampleCount);
		MTLRenderPipelineStateID pso = nil;
		if (_pipelines.get(key, pso))
			return pso;
		auto desc = [MTLRenderPipelineDescriptor new];
		desc.vertexFunction = getShaderFunction(kind, true);
		desc.fragmentFunction = getShaderFunction(kind, false);

		desc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
		desc.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;

		desc.colorAttachments[0].pixelFormat = mtl_pixel_format(outputType);
		mtl_set_blend(desc.colorAttachments[0], mode);

		uint32_t offset = 0,
						attrIndex = 0;
		auto shader = _shaders.allShaders[kind]; // get shader by pipeline kind
		desc.vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
		for (auto &i: shader->vertexAttrs) {
			desc.vertexDescriptor.attributes[attrIndex].format = (MTLVertexFormat)i.format;
			desc.vertexDescriptor.attributes[attrIndex].offset = offset;
			desc.vertexDescriptor.attributes[attrIndex].bufferIndex = i.bufferIndex;
			offset += i.sizeOf;
			attrIndex++;
		}
		desc.vertexDescriptor.layouts[shader->bufferIndex].stride = offset;
		desc.sampleCount = sampleCount; // msaa

		NSError *err = nil;
		pso = [_device newRenderPipelineStateWithDescriptor:desc error:&err];
		Qk_CHECK(pso, "Metal solid pipeline creation failed: %s", err.localizedDescription.UTF8String);
		_pipelines[key] = pso;
		return pso;
	}

	MTLRenderPipelineStateID MSLShader::getPipeline(BlendMode mode, ColorType outputType, uint32_t sampleCount) {
		uint32_t key = ((uint32_t)mode << 12) | // 8 bits for blend mode
			((uint32_t)outputType << 4) | // 8 bits for output type
			(uint32_t)sampleCount; // 4 bits for sample count, max 16 samples
		MTLRenderPipelineStateID pso;
		if (_pipelines.get(key, pso))
			return pso;
		// get pipeline from render resource by pipeline kind
		pso = ((MetalRenderResource*)getSharedRenderResource())->
			getPipeline(source.kind, mode, outputType, sampleCount);
		return _pipelines[key] = pso;
	}

	// ----------------------------------------------------------

	MetalRender::MetalRender(Options opts)
		: RenderBackend(opts)
		, _resource(nil)
		, _mtlcanvas(nil)
		, _device(nil)
		, _commandQueue(nil)
	{
		_opts.colorType = _opts.colorType ? _opts.colorType:
			kBGRA_8888_ColorType; // metal prefers BGRA format, use it as default for better performance
		_resource = (MetalRenderResource*)getSharedRenderResource();
		_device = _resource->_device;
		id<MTLCommandQueue> commandQueue = [_device newCommandQueue];
		Qk_CHECK(commandQueue, "Failed to create Metal command queue");
		_commandQueue = commandQueue;
		_mtlcanvas = NewRetain<MetalCanvas>(this, _opts); // new and retain canvas for render backend
		_canvas = _mtlcanvas; // set default canvas
		_shaders = _resource->_shaders; // copy shader cache reference for render thread use

		// auto samplerDesc = [MTLSamplerDescriptor new];
		// samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
		// samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
		// samplerDesc.mipFilter = MTLSamplerMipFilterLinear;
		// samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
		// samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
		// _sampler = [_device newSamplerStateWithDescriptor:samplerDesc];
	}

	MetalRender::~MetalRender() {
		Qk_CHECK(_mtlcanvas == nullptr);
	}

	void MetalRender::release() {
		Qk_CHECK(_mtlcanvas->ref_count() == 1,
			"MetalCanvas still has reference, ref count: %d", _mtlcanvas->ref_count());
		Releasep(_mtlcanvas); // release canvas and set to nullptr
		_canvas = nullptr;
		// _sampler = nil;
		_commandQueue = nil;
		_device = nil;
	}

	void MetalRender::lock() {}
	void MetalRender::unlock() {}

	void MetalRender::reload() {
		lock();
		_surfaceSize = getSurfaceSize();
		_delegate->onRenderBackendReload(_surfaceSize);
		unlock();
	}

	Canvas* MetalRender::createCanvas(Options opts) {
		opts.colorType = opts.colorType ? opts.colorType:
			kBGRA_8888_ColorType; // default to BGRA for better performance on Metal
		return new MetalCanvas(this, opts);
	}

	bool MetalRender::createTexture(cPixel *pix, int levels, TexStat *&out, bool mipmap) {
		return _resource->MetalRenderResource::createTexture(pix, levels, out, mipmap);
	}

	void MetalRender::deleteTexture(TexStat *tex) {
		_resource->MetalRenderResource::deleteTexture(tex);
	}

	bool MetalRender::createVertexData(VertexData::ID *vid) {
		return _resource->MetalRenderResource::createVertexData(vid);
	}

	void MetalRender::deleteVertexData(VertexData::ID *vid) {
		_resource->MetalRenderResource::deleteVertexData(vid);
	}
} // namespace qk
