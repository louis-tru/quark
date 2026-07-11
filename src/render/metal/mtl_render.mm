/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#import "./mtl_render.h"
#include "../render.h"
#import "./mtl_canvas.h"
#import "./mtl_shaders.h"
#import "../pixel.h"
#import "../pathv_cache.h"
#import <Metal/Metal.h>

namespace qk {

	MTLTextureID mtl_get_texture(cTexStat *stat) {
		return (__bridge MTLTextureID)stat->ptr();
	}

	MTLTextureID mtl_get_texture_from(const ImageSource* src, MTLTextureID _else) {
		return src ? (__bridge MTLTextureID)src->texture(0)->ptr() : _else;
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
			case kRGB_565_ColorType:
				return MTLPixelFormatB5G6R5Unorm;
			case kRGBA_5551_ColorType:
				return MTLPixelFormatA1BGR5Unorm;
			case kRGBA_4444_ColorType:
			case kRGB_444X_ColorType:
				return MTLPixelFormatABGR4Unorm;
		#if Qk_iOS
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

	ColorType mtl_color_type(MTLPixelFormat format) {
		switch (format) {
			case MTLPixelFormatR8Unorm:
				return kAlpha_8_ColorType; // or kLuminance_8_ColorType, both are single channel 8-bit
			case MTLPixelFormatRG8Unorm:
				return kLuminance_Alpha_88_ColorType; // or kYUV420SP_UV_88_ColorType, both are two channel 8-bit
			case MTLPixelFormatBGRA8Unorm:
				return kBGRA_8888_ColorType; // BGRA order, used by windows d3d and macos metal, optimal for metal performance
			case MTLPixelFormatRGBA8Unorm:
				return kRGBA_8888_ColorType; // RGBA order, used by opengl and vulkan, but not optimal for metal performance
			case MTLPixelFormatRGB10A2Unorm:
				return kRGBA_1010102_ColorType;
			case MTLPixelFormatR32Float:
				return kSDF_F32_ColorType; // or kSDF_Unsigned_F32_ColorType, both are single channel 32-bit float
		#if Qk_iOS
			case MTLPixelFormatPVRTC_RGB_2BPP:
				return kPVRTCI_2BPP_RGB_ColorType;
			case MTLPixelFormatPVRTC_RGBA_2BPP:
				return kPVRTCI_2BPP_RGBA_ColorType; // or kPVRTCII_2BPP_ColorType, both are 2bpp RGBA PVRTC
			case MTLPixelFormatPVRTC_RGB_4BPP:
				return kPVRTCI_4BPP_RGB_ColorType;
			case MTLPixelFormatPVRTC_RGBA_4BPP:
				return kPVRTCI_4BPP_RGBA_ColorType; // or kPVRTCII_4BPP_ColorType, both are 4bpp RGBA PVRTC
		#endif
			default:
				return kInvalid_ColorType;
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
			case kSrcOver_BlendMode: // r = s + (1-sa)*d
				ca.sourceRGBBlendFactor = MTLBlendFactorOne;
				ca.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				ca.sourceAlphaBlendFactor = MTLBlendFactorOne;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				break;
			case kDst_BlendMode: // r = d
				ca.sourceRGBBlendFactor = MTLBlendFactorZero;
				ca.destinationRGBBlendFactor = MTLBlendFactorOne;
				ca.sourceAlphaBlendFactor = MTLBlendFactorZero;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOne;
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
			case kSrcOverLegacy_BlendMode: // r = sa*s + (1-sa)*d
				ca.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
				ca.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				ca.sourceAlphaBlendFactor = MTLBlendFactorOne;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				break;
			case kPlusLegacy_BlendMode: // r = sa*s + d
				ca.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
				ca.destinationRGBBlendFactor = MTLBlendFactorOne;
				ca.sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOne;
				break;
			case kModulateLegacy_BlendMode: // r = s*d
				ca.sourceRGBBlendFactor = MTLBlendFactorZero;
				ca.destinationRGBBlendFactor = MTLBlendFactorSourceColor;
				ca.sourceAlphaBlendFactor = MTLBlendFactorZero;
				ca.destinationAlphaBlendFactor = MTLBlendFactorSourceAlpha;
				break;
			case kScreenLegacy_BlendMode: // r = s + (1-s)*d
				ca.sourceRGBBlendFactor = MTLBlendFactorOne;
				ca.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceColor;
				ca.sourceAlphaBlendFactor = MTLBlendFactorOne;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				break;
			case kMultiplyLegacy_BlendMode: // r = d*s + (1-sa)*d
				ca.sourceRGBBlendFactor = MTLBlendFactorDestinationColor;
				ca.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				ca.sourceAlphaBlendFactor = MTLBlendFactorDestinationAlpha;
				ca.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				break;
			default:
				ca.blendingEnabled = NO;
				break;
		}
	}

	bool mtl_supports_sampler_clamp_to_zero(MTLDeviceID device) {
#if Qk_iOS
		if (@available(iOS 13.0, *))
			return [device supportsFamily:MTLGPUFamilyApple7];
#else
		if (@available(macOS 10.15, *))
			return [device supportsFamily:MTLGPUFamilyApple7] ||
				[device supportsFamily:MTLGPUFamilyMac2];
#endif
		return false;
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
				return MTLSamplerAddressModeClampToZero;
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

	uint32_t mtl_pipeline_key(MSLPipelineKind kind, BlendMode mode, MTLPixelFormat format) {
		// kind: 8 bits, mode: 8 bits, format: 10 bits
		return ((uint32_t)kind << 18) | // 8 bits for pipeline kind
			((uint32_t)mode << 10) | // 8 bits for blend mode
			((uint32_t)format)// 10 bits for output type
		;
	}

	uint32_t mtl_get_sampler_key(const PaintImage* paint) {
		constexpr uint32_t bitfields = (
			// 0 | // src index default zero
			(0b11 << 8)  | // 2 bits for tile mode x
			(0b11 << 10) | // 2 bits for tile mode y
			(0b1  << 12) | // 1 bit for filter mode
			(0b111 << 13)| // 3 bits for mipmap mode
			0
		);
		return bitfields & paint->bitfields;
	}

	MTLTextureID mtl_new_texture(MTLDeviceID device, Vec2 size, MTLPixelFormat format, uint8_t flags) {
		auto desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format
																																		width:size.x()
																																	height:size.y()
																																mipmapped:flags & kMipmap_TextureFlags];
		desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead |
			MTLTextureUsageShaderWrite |
			(flags & kComputeWrite_TextureFlags ? MTLTextureUsageShaderWrite : 0);
		desc.storageMode = /*cpuRead ? MTLStorageModeShared :*/ MTLStorageModePrivate;
		return [device newTextureWithDescriptor:desc];
	}

	cTexStat* mtl_rebuild_texture(MTLDeviceID device, Vec2 size, ColorType type, cTexStat* texStat, TexStat &storeStat, uint8_t flags) {
		auto fmt = mtl_pixel_format(type);
		auto tex = mtl_get_texture(texStat);
		if (fmt == MTLPixelFormatInvalid)
			return nullptr;
		if (!tex || Vec2(tex.width, tex.height) != size.x() || tex.height != size.y() || tex.pixelFormat != fmt ||
				(flags & kMipmap_TextureFlags && tex.mipmapLevelCount <= 1)
		) {
			tex = mtl_new_texture(device, size, fmt, flags);
			if (!tex)
				return nullptr;
			texStat = &storeStat; // update texStat to storeStat
			storeStat.set_ptr(CFBridgingRetain(tex)); // retain new texture and set to texStat
		}
		return texStat;
	}

	MetalRenderResource* getSharedRenderMetalResource() {
		static MetalRenderResource *g_sharedRenderResource = new MetalRenderResource();
		return g_sharedRenderResource;
	}

	RenderResource* getSharedRenderResource() {
		return getSharedRenderMetalResource();
	}

	template<>
	MemBlockAllocator<MTLBufferID>::MemBlock*
	MemBlockAllocator<MTLBufferID>::createBlock(uint32_t capacity) {
		// get MTLDevice from shared render resource to create MTLBuffer for memory block
		auto device = getSharedRenderMetalResource()->device();
		MemBlock* block = new MemBlock([device newBufferWithLength:capacity options:MTLResourceStorageModeShared], capacity);
		Qk_ASSERT(block->val, "Failed to create MTLBuffer with capacity: %u", capacity);
		return block;
	}
	template<> 
	void MemBlockAllocator<MTLBufferID>::deleteBlock(MemBlock *block) {
		block->val = nil;
		delete block;
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
		if (!tex)
			return false;
		auto cmd = [_commandQueue commandBuffer];
		auto blit = [cmd blitCommandEncoder];

		for (int i = 0; i < levels; i++) {
			auto p = pix + i;

			if (!p->val() || p->length() == 0)
				continue;
			auto width = (NSUInteger)p->width();
			auto height = (NSUInteger)p->height();
			auto bytesPerRow = (NSUInteger)p->rowbytes();
			auto uploadSize = (NSUInteger)p->bytes();
			if (!bytesPerRow || !uploadSize || uploadSize > p->length()) {
				[blit endEncoding];
				return false;
			}

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

	void MetalRenderResource::post_message(Cb cb) {
		// directly execute callback for simplicity,
		// since current implementation does not need cross-thread resource access
		cb->resolve();
	}

	TexStat MetalRenderResource::createTextureStat(Vec2 size, ColorType type, uint8_t flags) {
		auto tex = mtl_new_texture(_device, size, mtl_pixel_format(type), flags);
		return TexStat(CFBridgingRetain(tex));
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

	MTLPipeline MetalRenderResource::getPipeline(MSLPipelineKind kind, BlendMode mode, MTLPixelFormat format) {
		ScopeLock lock(_mutex); // protect shader function cache
		auto key = mtl_pipeline_key(kind, mode, format);
		NSObjectID pip = nil;
		if (_pipelines.get(key, pip))
			return (MTLPipeline)pip;
		auto desc = [MTLRenderPipelineDescriptor new];
		desc.vertexFunction = getShaderFunction(kind, true);
		desc.fragmentFunction = getShaderFunction(kind, false);

		// desc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
		// desc.stencilAttachmentPixelFormat = MTLPixelFormatInvalid;
		// desc.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;

		desc.colorAttachments[0].pixelFormat = format;
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
		// desc.sampleCount = 1; // default is 1, set to >1 if using MSAA

		NSError *err = nil;
		pip = [_device newRenderPipelineStateWithDescriptor:desc error:&err];
		Qk_CHECK(pip, "Metal solid pipeline creation failed: %s", err.localizedDescription.UTF8String);
		_pipelines[key] = (NSObjectID)pip;
		return (MTLPipeline)pip;
	}

	MTLComputePipeline MetalRenderResource::getComputePipeline(MSLPipelineKind kind) {
		ScopeLock lock(_mutex);
		auto key = mtl_pipeline_key(kind, (BlendMode)0, (MTLPixelFormat)0);
		NSObjectID pip = nil;
		if (_pipelines.get(key, pip))
			return (MTLComputePipeline)pip;
		Qk_CHECK(kind < kPipelineCount, "Invalid compute pipeline kind: %d", kind);
		auto &src = _shaders.allShaders[kind]->source;
		Qk_CHECK(src.computeSource, "Shader does not contain a compute stage: %s", src.name);
		auto code = src.computeSource();
		NSError *err = nil;
		auto library = [_device newLibraryWithSource:@(code.c_str()) options:nil error:&err];
		Qk_CHECK(library, "Metal compute shader compilation failed: %s", err.localizedDescription.UTF8String);
		auto entry = String::format("%s_comp", src.name);
		auto fn = [library newFunctionWithName:@(entry.c_str())];
		Qk_CHECK(fn, "Metal compute shader entry not found: %s", entry.c_str());
		pip = [_device newComputePipelineStateWithFunction:fn error:&err];
		Qk_CHECK(pip, "Metal compute pipeline creation failed: %s", err.localizedDescription.UTF8String);
		_pipelines[key] = (NSObjectID)pip;
		return (MTLComputePipeline)pip;
	}

	MTLSampler MetalRenderResource::get_sampler(const PaintImage* paint) {
		ScopeLock lock(_mutex); // protect shader function cache
		uint32_t key = mtl_get_sampler_key(paint);
		MTLSampler sampler;
		if (!_texSamplers.get(key, sampler)) {
			auto desc = [MTLSamplerDescriptor new];
			desc.supportArgumentBuffers = YES; // support argument buffers for Metal 2.0 and later
			desc.sAddressMode = mtl_sampler_address_mode(paint->tileModeX);
			desc.tAddressMode = mtl_sampler_address_mode(paint->tileModeY);
			desc.magFilter = mtl_sampler_mag_filter(paint->filterMode);
			mtl_set_sampler_min_mip_filter(desc, paint->mipmapMode);
			sampler = [_device newSamplerStateWithDescriptor:desc];
			_texSamplers.set(key, sampler);
		}
		return sampler;
	}

	MTLSampler MetalRenderResource::get_sampler(PaintImage::FilterMode filter, PaintImage::MipmapMode mipmap) {
		PaintImage img;
		img.tileModeX = PaintImage::kDecal_TileMode;
		img.tileModeY = PaintImage::kDecal_TileMode;
		img.filterMode = filter;
		img.mipmapMode = mipmap;
		return get_sampler(&img);
	}

	MTLPipeline MSLShader::getPipeline(BlendMode mode, MTLPixelFormat format) {
		uint32_t key =
			((uint32_t)mode << 10) | // 8 bits for blend mode
			((uint32_t)format);// 10 bits for output type
		NSObjectID pip;
		if (_pipelines.get(key, pip))
			return (MTLPipeline)pip;
		// get pipeline from render resource by pipeline kind
		pip = ((MetalRenderResource*)getSharedRenderResource())->
			getPipeline(source.kind, mode, format);
		_pipelines[key] = pip;
		return (MTLPipeline)pip;
	}

	MTLComputePipeline MSLShader::getComputePipeline() {
		NSObjectID pip;
		if (_pipelines.get(0, pip))
			return (MTLComputePipeline)pip;
		pip = ((MetalRenderResource*)getSharedRenderResource())->getComputePipeline(source.kind);
		_pipelines[0] = pip;
		return (MTLComputePipeline)pip;
	}

	// ----------------------------------------------------------

	MetalRender::MetalRender(Options opts)
		: RenderBackend(opts)
		, _resource(nil)
		, _mtlcanvas(nil)
		, _device(nil), _commandQueue(nil), _emptyBuffer(nil)
		, _nearestSampler(nil), _linearSampler(nil), _vportCpPipeline(nil)
	{
		_resource = getSharedRenderMetalResource();
		_device = _resource->_device;
		id<MTLCommandQueue> commandQueue = [_device newCommandQueue];
		Qk_CHECK(commandQueue, "Failed to create Metal command queue");
		_commandQueue = commandQueue;
		_mtlcanvas = NewRetain<MetalCanvas>(this, _opts); // new and retain canvas for render backend
		_opts.colorType = _mtlcanvas->opts().colorType; // sync color type
		_canvas = _mtlcanvas; // set default canvas
		// _shaders = _resource->_shaders; // copy shader cache reference for render thread use

		_emptyBuffer = [_device newBufferWithLength:128 options:MTLResourceStorageModeShared];
		// pre-create sampler for nearest filter mode, which is commonly used for non-scaling image rendering
		_nearestSampler = _resource->get_sampler(PaintImage::kNearest_FilterMode, PaintImage::kNearest_MipmapMode);
		_linearSampler = _resource->get_sampler(PaintImage::kLinear_FilterMode, PaintImage::kLinearNearest_MipmapMode);
		_vportCpPipeline = _resource->_shaders.vportCp.getPipeline(kSrc_BlendMode, MTLPixelFormatBGRA8Unorm);
	}

	MetalRender::~MetalRender() {
		Qk_CHECK(_mtlcanvas == nullptr);
	}

	void MetalRender::release() {
		Qk_CHECK(_mtlcanvas->refCount() == 1,
			"MetalCanvas still has reference, ref count: %d", _mtlcanvas->refCount());
		_nearestSampler = nil; // release aa clip sampler reference
		_linearSampler = nil;
		_vportCpPipeline = nil;
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

	TexStat MetalRender::createTextureStat(Vec2 size, ColorType type, uint8_t flags) {
		auto tex = mtl_new_texture(_device, size, mtl_pixel_format(type), flags);
		return TexStat(CFBridgingRetain(tex));
	}

	bool MetalRender::uploadTexture(cPixel *pix, int levels, TexStat *out, bool mipmap) {
		return _resource->MetalRenderResource::uploadTexture(pix, levels, out, mipmap);
	}

	void MetalRender::unloadTexture(TexStat *tex) {
		_resource->MetalRenderResource::unloadTexture(tex);
	}

	bool MetalRender::uploadVertexData(VertexData::ID *vid) {
		if (vid->ptr)
			return true;
		auto &vertex = vid->data->vertex;
		if (!vertex.length())
			return false;
		auto buf = [_device newBufferWithBytes:vertex.val()
																		length:vertex.size()
																		options:MTLResourceStorageModeShared];
		if (!buf)
			return false;
		vid->ptr = (void*)CFBridgingRetain(buf);
		return true;
	}

	void MetalRender::unloadVertexData(VertexData::ID *vid) {
		if (vid->ptr) {
			// (void)(__bridge_transfer id<MTLBuffer>)vid->ptr;
			CFBridgingRelease(vid->ptr); // release buffer for vertex data
			vid->ptr = nullptr;
		}
	}
} // namespace qk
