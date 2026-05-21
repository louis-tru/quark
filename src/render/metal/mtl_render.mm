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
	uint32_t massSample(uint32_t n);

	MTLTextureID mtl_get_texture(TexStat *stat) {
		return (__bridge MTLTextureID)stat->ptr();
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
			default:
				ca.blendingEnabled = NO;
				break;
		}
	}

	MTLSamplerAddressMode mtl_sampler_address_mode(PaintImage::TileMode mode) {
		switch (mode) {
			case PaintImage::kClamp_TileMode:
				return MTLSamplerAddressModeClampToEdge;
			case PaintImage::kRepeat_TileMode:
				return MTLSamplerAddressModeRepeat;
			case PaintImage::kMirror_TileMode:
				return MTLSamplerAddressModeMirrorRepeat;
			case PaintImage::kDecal_TileMode:
				if (@available(macOS 11.0, iOS 14.0, *)) {
					return MTLSamplerAddressModeClampToZero;
				} else {
					return MTLSamplerAddressModeClampToEdge;
				}
		}
	}

	MTLSamplerMinMagFilter mtl_sampler_mag_filter(PaintImage::FilterMode filter) {
		switch (filter) {
			case PaintImage::kNearest_FilterMode:
				return MTLSamplerMinMagFilterNearest;
			case PaintImage::kLinear_FilterMode:
				return MTLSamplerMinMagFilterLinear;
		}
	}

	void mtl_set_sampler_min_mip_filter(MTLSamplerDescriptor* desc, PaintImage::MipmapMode mode) {
		switch (mode) {
			case PaintImage::kNone_MipmapMode:
				desc.minFilter = MTLSamplerMinMagFilterNearest;
				desc.mipFilter = MTLSamplerMipFilterNotMipmapped;
				break;
			case PaintImage::kNearest_MipmapMode:
				desc.minFilter = MTLSamplerMinMagFilterNearest;
				desc.mipFilter = MTLSamplerMipFilterNearest;
				break;
			case PaintImage::kLinearNearest_MipmapMode:
				desc.minFilter = MTLSamplerMinMagFilterLinear;
				desc.mipFilter = MTLSamplerMipFilterNearest;
				break;
			case PaintImage::kNearestLinear_MipmapMode:
				desc.minFilter = MTLSamplerMinMagFilterNearest;
				desc.mipFilter = MTLSamplerMipFilterLinear;
				break;
			case PaintImage::kLinear_MipmapMode:
				desc.minFilter = MTLSamplerMinMagFilterLinear;
				desc.mipFilter = MTLSamplerMipFilterLinear;
				break;
		}
	}

	uint32_t mtl_pipeline_key(MSLPipelineKind kind, BlendMode mode, ColorType outputType, uint32_t sampleCount) {
		// kind: 8 bits, mode: 8 bits, outputType: 8 bits, sampleCount: 4 bits
		return ((uint32_t)kind << 20) | // 8 bits for pipeline kind
			((uint32_t)mode << 12) | // 8 bits for blend mode
			((uint32_t)outputType << 4) | // 8 bits for output type
			(uint32_t)(sampleCount & 0b1111) // 4 bits for sample count, max 16 samples
		;
	}

	TexStat* mtl_rebuild_texture(MTLDeviceID device, Vec2 size, ColorType type, TexStat* texStat, TexStat &newStat, bool mipmap) {
		auto fmt = mtl_pixel_format(type);
		auto tex = mtl_get_texture(texStat);
		if (fmt == MTLPixelFormatInvalid)
			return nullptr;
		if (!tex || tex.width != size.x() || tex.height != size.y() || tex.pixelFormat != fmt ||
				(mipmap && tex.mipmapLevelCount <= 1)
		) {
			auto desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:fmt
																																		 width:size.x()
																																		height:size.y()
																																mipmapped:mipmap];
			desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
			desc.storageMode = MTLStorageModePrivate;
			tex = [device newTextureWithDescriptor:desc];
			if (!tex)
				return nullptr;
			texStat = &newStat; // update texStat to newStat
			newStat.set_ptr(CFBridgingRetain(tex)); // retain new texture and set to texStat
		}
		return texStat;
	}

	MTLTextureID mtl_new_tex_renderbuffer(MTLDeviceID device, Vec2 size, MTLPixelFormat format, bool read, bool priv, bool mipmap) {
		auto desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format
																																		width:size.x()
																																	height:size.y()
																																mipmapped:mipmap];
		desc.usage = MTLTextureUsageRenderTarget | (read ? MTLTextureUsageShaderRead : 0);
		desc.storageMode = priv ? MTLStorageModePrivate : MTLStorageModeShared;
		return [device newTextureWithDescriptor:desc];
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

	bool MetalRenderResource::uploadTexture(cPixel *pix, int levels, TexStat *out, bool mipmap) {
		Qk_ASSERT_GT(levels, 0, "Levels must be greater than 0");
		if (!pix || pix->length() == 0) {
			return false;
		}

		auto type = pix->type();
		auto fmt = mtl_pixel_format(type);
		if (fmt == MTLPixelFormatInvalid)
			return false;

		bool genMipmap = levels == 1 && mipmap;
		bool hasMipmaps = levels > 1 || mipmap;

		auto desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:fmt
																																	width:pix->width()
																																	height:pix->height()
																															mipmapped:hasMipmaps];
		if (levels > 1) {
			desc.mipmapLevelCount = levels; // only provided levels exist
		}
		desc.usage = MTLTextureUsageShaderRead;
		desc.storageMode = MTLStorageModePrivate;

		auto tex = [_device newTextureWithDescriptor:desc];
		if (!tex) return false;

		auto cmd = [_commandQueue commandBuffer];
		auto blit = [cmd blitCommandEncoder];

		for (int i = 0; i < levels; i++) {
			auto p = pix + i;

			if (!p->val() || p->length() == 0)
				continue;
			auto width = (NSUInteger)p->width();
			auto height = (NSUInteger)p->height();
			auto bytesPerRow = width * Pixel::bytes_per_pixel(type);
			auto uploadSize = bytesPerRow * height;

			auto buff = [_device newBufferWithBytes:p->val()
																			length:uploadSize
																			options:MTLResourceStorageModeShared];
			if (!buff) {
				[blit endEncoding];
				return false;
			}
			[blit copyFromBuffer:buff
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

		if (out->ptr()) {
			CFBridgingRelease(out->ptr()); // release old texture if exists
		}
		out->set_ptr((void*)CFBridgingRetain(tex)); // retain new texture for TexStat

		return true;
	}

	void MetalRenderResource::unloadTexture(TexStat *tex) {
		CFBridgingRelease(tex->ptr()); // release texture
		tex->set_ptr(nullptr);
	}

	bool MetalRenderResource::uploadVertexData(VertexData::ID *vid) {
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
		// vid->ptr = (__bridge_retained void*)buf;
		vid->ptr = (void*)CFBridgingRetain(buf);
		return true;
	}

	void MetalRenderResource::unloadVertexData(VertexData::ID *vid) {
		if (vid->ptr) {
			// (void)(__bridge_transfer id<MTLBuffer>)vid->ptr;
			CFBridgingRelease(vid->ptr); // release buffer for vertex data
			vid->ptr = nullptr;
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

	MTLPipeline MetalRenderResource::getPipeline(MSLPipelineKind kind, BlendMode mode, ColorType outputType, uint32_t sampleCount) {
		ScopeLock lock(_mutex); // protect shader function cache
		auto key = mtl_pipeline_key(kind, mode, outputType, sampleCount);
		MTLPipeline pso = nil;
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
		for (auto &i: shader->attributes) {
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

	MTLPipeline MSLShader::getPipeline(BlendMode mode, ColorType outputType, uint32_t sampleCount) {
		uint32_t key = ((uint32_t)mode << 12) | // 8 bits for blend mode
			((uint32_t)outputType << 4) | // 8 bits for output type
			(uint32_t)sampleCount; // 4 bits for sample count, max 16 samples
		MTLPipeline pso;
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
		, _texStat(new TexStat*[12]{0})
		, _device(nil)
		, _commandQueue(nil)
	{
		_resource = (MetalRenderResource*)getSharedRenderResource();
		_device = _resource->_device;
		id<MTLCommandQueue> commandQueue = [_device newCommandQueue];
		Qk_CHECK(commandQueue, "Failed to create Metal command queue");
		_commandQueue = commandQueue;
		_mtlcanvas = NewRetain<MetalCanvas>(this, _opts); // new and retain canvas for render backend
		_canvas = _mtlcanvas; // set default canvas
		_shaders = _resource->_shaders; // copy shader cache reference for render thread use
		// pre-create sampler for aa clip image
		_aaclipSampler = get_sampler(PaintImage::kNearest_FilterMode, PaintImage::kNearest_MipmapMode);

		// MTLDepthStencilDescriptor *desc = [MTLDepthStencilDescriptor new];
		// desc.depthCompareFunction = MTLCompareFunctionGreater;
		// desc.depthWriteEnabled = YES;
		// id<MTLDepthStencilState> depthOnly = [_device newDepthStencilStateWithDescriptor:desc];
		// desc.frontFaceStencil.stencilCompareFunction = MTLCompareFunctionEqual;
	}

	MetalRender::~MetalRender() {
		Qk_CHECK(_mtlcanvas == nullptr);
	}

	void MetalRender::release() {
		Qk_CHECK(_mtlcanvas->ref_count() == 1,
			"MetalCanvas still has reference, ref count: %d", _mtlcanvas->ref_count());

		for (int i = 0; i < 12; i++) {
			if (_texStat[i])
				CFBridgingRelease(_texStat[i]->ptr); // release texture
		}
		delete[] _texStat;
		_texStat = nullptr;
		_aaclipSampler = nil; // release aa clip sampler reference
		_texSamplers.clear(); // clear sampler cache
		Releasep(_mtlcanvas); // release canvas and set to nullptr
		_canvas = nullptr; // clear canvas reference
		_commandQueue = nil; // release command queue reference
		_device = nil; // release device reference
	}

	void MetalRender::lock() {
	}
	void MetalRender::unlock() {
	}

	void MetalRender::reload() {
		lock();
		_surfaceSize = getSurfaceSize();
		_delegate->onRenderBackendReload(_surfaceSize);
		unlock();
	}

	Canvas* MetalRender::createCanvas(Options opts) {
		return new MetalCanvas(this, opts);
	}

	bool MetalRender::uploadTexture(cPixel *pix, int levels, TexStat *&out, bool mipmap) {
		return _resource->MetalRenderResource::uploadTexture(pix, levels, out, mipmap);
	}

	void MetalRender::unloadTexture(TexStat *tex) {
		_resource->MetalRenderResource::unloadTexture(tex);
	}

	bool MetalRender::uploadVertexData(VertexData::ID *vid) {
		return _resource->MetalRenderResource::uploadVertexData(vid);
	}

	void MetalRender::unloadVertexData(VertexData::ID *vid) {
		_resource->MetalRenderResource::unloadVertexData(vid);
	}

	bool MetalRender::use_texture(MTLRenderEncoder enc, ImageSource *src, int srcSlot, int dstSlot, const PaintImage *paint) {
		auto index = paint->srcIndex + srcSlot;
		Qk_ASSERT_LT(index, 8, "Texture slot index out of range, srcIndex: %d, slot: %d", paint->srcIndex, srcSlot);
		auto tex = src->texture(index);
		if (!tex.ptr()) {
			// mark texture for this render, and try to create texture immediately
			src->markAsTexture(this);
			if (!tex->ptr()) {
				Qk_DLog("Texture not ready for paint image, src index: %d, slot: %d", paint->srcIndex, srcSlot);
				return false; // texture not ready
			}
		}
		set_texture_param(enc, mtl_get_texture(tex), dstSlot, paint);
		return true;
	}

	void MetalRender::set_texture_param(MTLRenderEncoder enc, MTLTextureID tex, int dstSlot, const PaintImage* paint) {
		auto sampler = get_sampler(paint); // get sampler state for paint image
		[enc setFragmentTexture:tex atIndex:dstSlot];
		[enc setFragmentSamplerState:sampler atIndex:dstSlot];
	}

	MTLSampler MetalRender::get_sampler(PaintImage::FilterMode filter, PaintImage::MipmapMode mipmap) {
		PaintImage img;
		img.tileModeX = PaintImage::kDecal_TileMode;
		img.tileModeY = PaintImage::kDecal_TileMode;
		img.filterMode = filter;
		img.mipmapMode = mipmap;
		return get_sampler(&img);
	}

	MTLSampler MetalRender::get_sampler(const PaintImage* paint) {
		constexpr uint32_t bitfields = (
			// 0 | // src index default zero
			(0b11 << 8)  | // 2 bits
			(0b11 << 10) | // 2 bits
			(0b1  << 12) | // 1 bit
			(0b111 << 13)| // 3 bits
			0
		);
		uint32_t key = bitfields & paint->bitfields;
		MTLSampler sampler;
		if (!_texSamplers.get(key, sampler)) {
			auto desc = [MTLSamplerDescriptor new];
			desc.sAddressMode = mtl_sampler_address_mode(paint->tileModeX);
			desc.tAddressMode = mtl_sampler_address_mode(paint->tileModeY);
			desc.magFilter = mtl_sampler_mag_filter(paint->filterMode);
			mtl_set_sampler_min_mip_filter(desc, paint->mipmapMode);
			sampler = [_device newSamplerStateWithDescriptor:desc];
			_texSamplers.set(key, sampler);
		}
		return sampler;
	}

} // namespace qk
