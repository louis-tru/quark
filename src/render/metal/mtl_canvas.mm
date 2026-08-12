/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#import "./mtl_canvas.h"
#include <cstdint>
#import "./mtl_render.h"
#import "../source.h"
#import "../pixel.h"
#import "../arguments.h"

namespace qk {
	MTLPixelFormat mtl_pixel_format(ColorType type);
	MTLTextureID mtl_get_texture(cTexStat *stat);
	MTLTextureID mtl_get_texture_from(const ImageSource* src, MTLTextureID _else = nil);
	void clear_PathvCache(PathvCache *cache, int flags);
	uint32_t mtl_get_sampler_key(const PaintImage* paint);
	bool mtl_supports_sampler_clamp_to_zero(MTLDeviceID device);

	static uint32_t mtl_capa_max_image_count(MTLDeviceID device) {
		uint32_t count = 64;
	#if Qk_iOS
		uint32_t maxSamplerCount = 256;
	#else
		uint32_t maxSamplerCount = 512;
	#endif
		if (@available(macOS 10.14, iOS 12.0, *)) {
			count = U32::min((uint32_t)device.maxArgumentBufferSamplerCount, maxSamplerCount);
		}
		return count;
	}

	static bool mtl_capa_supported(MTLDeviceID device) {
		if (runArguments && runArguments->options.has("aaside"))
			return false; // disable CAPA for AASide test
		if (@available(macOS 10.13, iOS 11.0, *))
			return device.argumentBuffersSupport == MTLArgumentBuffersTier2;
		return false;
	}

	void setRootMatrixFromEnc(MTLEncoder enc, const Mat4 mat[2], Vec2 surfaceScale, uint32_t index = 1) {
		MSLColor::RootMatrixBlock rMat {
			.value = mat[0].transpose(), // transpose for shader
			.noScale = mat[1].transpose(), // transpose for shader
			.surfaceScale = surfaceScale
		};
		[enc setVertexBytes:&rMat length: sizeof(rMat) atIndex:index];
	}

	void setViewMatrixFromEnc(MTLEncoder enc, const Mat &mat, uint32_t index = 2) {
		float vm2x2[4] = {
			mat[0], mat[3],
			mat[1], mat[4],
		}; // transpose matrix
		[enc setVertexBytes:vm2x2 length: sizeof(vm2x2) atIndex:index];
	}

	MetalCanvas::MetalCanvas(MetalRender *render, Render::Options opts)
		: GPUCanvas(render, opts)
		, _mtlrender(render)
		, _device(nil), _commandQueue(nil)
		, _cmdPack{}, _cmdPackFront{}
		, _outTex(nil), _target(nil)
		, _supportsSamplerClampToZero(false)
		, _capaCompositeSet2Encoder(nil)
		, _capaCompositeSet3Encoder(nil)
	{
		_opts.colorType = _opts.colorType ? _opts.colorType:
			kBGRA_8888_ColorType; // metal prefers BGRA format, use it as default for better performance
		_device = _mtlrender->_device;
		_supportsSamplerClampToZero = mtl_supports_sampler_clamp_to_zero(_device);
		_commandQueue = _mtlrender->_commandQueue; // share command queue with render
		_shaders = _mtlrender->_resource->shaders(); // copy shader cache reference for render thread use
		_cmdPack.current = [_commandQueue commandBuffer]; // create command buffer for this canvas
		_cmdPack.allocator = new MemBlockAllocator<MTLBufferID>();
		_cmdPackFront.allocator = new MemBlockAllocator<MTLBufferID>();

		if (opts.enableCAPA && mtl_capa_supported(_device))
			_capaBuilder = new CAPABuilder(this);

		if (_capaBuilder) {
			_capaMaxImageCount = mtl_capa_max_image_count(_device);
			_capaEnabled = true;
			// create argument encoder for capa composite shader to bind images and samplers
			auto &shader = _shaders.capaComposite;
			auto imagesDesc = [MTLArgumentDescriptor argumentDescriptor];
			imagesDesc.dataType = MTLDataTypeTexture;
			imagesDesc.index = shader.compute.set2.images.id;
			imagesDesc.arrayLength = _capaMaxImageCount;
			imagesDesc.access = MTLBindingAccessReadOnly;
			imagesDesc.textureType = MTLTextureType2D;
			_capaCompositeSet2Encoder = [_device newArgumentEncoderWithArguments:@[imagesDesc]];
			auto samplersDesc = [MTLArgumentDescriptor argumentDescriptor];
			samplersDesc.dataType = MTLDataTypeSampler;
			samplersDesc.index = shader.compute.set3.samplers.id;
			samplersDesc.arrayLength = _capaMaxImageCount;
			samplersDesc.access = MTLBindingAccessReadOnly;
			_capaCompositeSet3Encoder = [_device newArgumentEncoderWithArguments:@[samplersDesc]];
		}
	}

	MetalCanvas::~MetalCanvas() {
		_mutex.lock();
		_texSamplers.clear(); // clear sampler cache
		// end encoding if still in a pass, to ensure all resources can be released properly
		if (_cmdPack.enc) [_cmdPack.enc endEncoding];
		if (_cmdPackFront.enc) [_cmdPackFront.enc endEncoding];
		_cmdPack = {};
		_cmdPackFront = {}; // clear cmd packs
		_outTex = nil; // Color render buffer object of texture
		_target = nil; // Current active color render target texture
		_capaCompositeSet2Encoder = nil;
		_capaCompositeSet3Encoder = nil;
		_commandQueue = nil; // Metal command queue
		_device = nil;
		_mutex.unlock();
	}

	void MetalCanvas::endPass() {
		if (_cmdPack.beginPass && !_cmdPack.enc) {
			_cmdPack.enc = [_cmdPack.current renderCommandEncoderWithDescriptor:_cmdPack.pass];
		}
		if (_cmdPack.enc) {
			[_cmdPack.enc endEncoding]; // end current pass
			_cmdPack.enc = nil;
			_cmdPack.recorded = true;
			_cmdPack.beginPass = false;
		}
		_cmdPack.pipeline = nil;
	}

	MTLPassDesc MetalCanvas::beginPass(int level, bool loadColor) {
	 #if DEBUG
		if (_cmdPack.beginPass) {
			if (_cmdPack.pass.colorAttachments[0].texture == _target &&
					_cmdPack.pass.colorAttachments[0].level == level
			) {
				Qk_Fatal("Same render target should not begin a new pass without ending the previous pass.");
			}
		}
	 #endif
		endPass();
		auto pass = _cmdPack.pass ? _cmdPack.pass: [MTLRenderPassDescriptor new];
		auto recorded = _cmdPack.isRecorded();

		Qk_ASSERT(_target, "Output color texture should be created before beginning a pass");
		pass.colorAttachments[0].texture = _target;
		pass.colorAttachments[0].loadAction = recorded && loadColor ? MTLLoadActionLoad : MTLLoadActionDontCare;
		pass.colorAttachments[0].storeAction = MTLStoreActionStore;
		pass.colorAttachments[0].level = level;

		_cmdPack.pass = pass;
		_cmdPack.beginPass = true;
		return pass;
	}

	bool MetalCanvas::onlyEndEncoderPass(Color4f &color) {
		if (_cmdPack.beginPass && !_cmdPack.enc) {
			if (_cmdPack.pass.colorAttachments[0].loadAction == MTLLoadActionClear) {
				auto clr = _cmdPack.pass.colorAttachments[0].clearColor;
				color = Color4f(clr.red, clr.green, clr.blue, clr.alpha);
				_cmdPack.beginPass = false;
				return true;
			}
		}
		endPass();
		return false;
	}

	MTLEncoder MetalCanvas::getEncoder() {
		if (_cmdPack.enc)
			return _cmdPack.enc;
		if (!_cmdPack.beginPass)
			beginPass();
		_cmdPack.enc = [_cmdPack.current renderCommandEncoderWithDescriptor:_cmdPack.pass];
		Qk_ASSERT(_cmdPack.enc, "Failed to create render command encoder for new pass");
		// set root matrix for new encoder
		setRootMatrixFromEnc(_cmdPack.enc, &_rootMatrix, _surfaceScale);
		// set view matrix for new encoder
		setViewMatrixFromEnc(_cmdPack.enc, _state->matrix);
		// set clip texture for new encoder if clip state exists
		if (_clipState) {
			MSLColor::ClipStatBlock clipStat = { _clipState->bounds.iVec4(), _clipState->op };
			[_cmdPack.enc setFragmentBytes:&clipStat length:sizeof(clipStat) atIndex:3];
			[_cmdPack.enc setFragmentTexture:mtl_get_texture_from(*_clipState->mask) atIndex:0];
			[_cmdPack.enc setFragmentSamplerState:_mtlrender->_nearestSampler atIndex:0];
		} else {
			[_cmdPack.enc setFragmentBuffer:_mtlrender->_emptyBuffer offset:0 atIndex:3];
		}
		[_cmdPack.enc setViewport: {0, 0, _surfaceSize.x(), _surfaceSize.y(), 0, 1}];
		return _cmdPack.enc;
	}

	void MetalCanvas::setPipeline(MTLEncoder enc, MSLShader& shader) {
		auto pipeline = shader.getPipeline(_blendMode, _target.pixelFormat);
		if (_cmdPack.pipeline != pipeline) {
			[enc setRenderPipelineState:pipeline]; // set pipeline state for shader
			_cmdPack.pipeline = pipeline;
		}
	}

	// usePipeline with vertex data ensures vertex data is valid and set for draw call,
	// if vertex data is invalid, return nil and skip draw call
	MTLEncoder MetalCanvas::usePipeline(MSLShader& shader, const VertexData &vertex, MTLEncoder enc) {
		setPipeline(enc, shader); // set pipeline state for shader
		if (Render::useVertexData(vertex.id)) {
			auto buf = (__bridge id<MTLBuffer>)vertex.id->ptr; // get vertex buffer from vertex data id
			[enc setVertexBuffer:buf offset:0 atIndex:shader.bufferIndex];
		} else {
			Qk_ASSERT(vertex.vertex.val(), "Vertex data should not be null for draw call");
			Qk_ASSERT_EQ(vertex.vertex.length(), vertex.vCount, "Vertex data length should match vertex count");
			auto &block = makeBufferT(_cmdPack, vertex.vertex.val(), vertex.vertex.length());
			[enc setVertexBuffer:block.val offset:block.begin atIndex:shader.bufferIndex];
			// [enc setVertexBytes:vertex.vertex.val() length:vertex.vertex.size() atIndex:shader.bufferIndex];
		}
		return enc;
	}

	bool MetalCanvas::swapBuffer() {
		flushCAPABatch(); // flush CAPA data
		_alloc.reset();
		endPass(); // end current pass to ensure all commands are encoded before swap
		ScopeLock lock(_mutex);
		bool canSwap = _cmdPackFront.current == nil;
		// only swap if there are recorded commands and front cmd pack is empty
		if (canSwap && _cmdPack.isRecorded()) {
			std::swap(_cmdPackFront, _cmdPack); // swap cmd buffer and pass descriptor to front
		}
		clear_PathvCache(_cache, 0); // tag: clear mark
		// reset cmd pack for next frame
		_cmdPack = {
			.allocator = std::move(_cmdPack.allocator), // move buffer allocator to new cmd pack
			.current = [_commandQueue commandBuffer], // create new command buffer for next frame
		};
		_cmdPack.allocator->reset(); // reset buffer allocator for new frame
		return canSwap;
	}

	Array<MTLCommandBufferID> MetalCanvas::flushBuffer() {
		ScopeLock lock(_mutex); // ensure mutex is unlocked when function exits
		Qk_ASSERT(!_cmdPackFront.beginPass, "Cannot flush buffer while a pass is still active, end the pass first");
		if (_cmdPackFront.recorded) {
			// add command buffer to cmds for flush if it has recorded commands
			_cmdPackFront.commands.push(_cmdPackFront.current);
		}
		_cmdPackFront.recorded = false;
		[_cmdPackFront.current addCompletedHandler:^(MTLCommandBufferID buffer) {
			// clear front command pack after flush is completed
			_cmdPackFront.current = nil;
		}];
		return std::move(_cmdPackFront.commands);
	}

	void MetalCanvas::flushSubcanvasCmd(GPUCanvas *sub) {
		if (sub == this)
			return; // only flush subcanvas if it is not the same as current canvas
		if (sub->render() != _render)
			return; // only flush subcanvas if it is from the same render

		auto commands = static_cast<MetalCanvas*>(sub)->flushBuffer();
		if (commands.isNull())
			return; // if no command buffers, skip

		endPass(); // end current pass to ensure all commands are encoded before flush

		if (_cmdPack.recorded)
			_cmdPack.commands.push(_cmdPack.current);
		_cmdPack.current = commands.back(); // get a command buffer from cmds for next pass
		commands.pop();
		// add remaining command buffers to cmd pack for flush
		_cmdPack.commands.concat(commands);
		_cmdPack.recorded = true;
	}

	void MetalCanvas::vportCopy(MTLCommandBufferID cmd, MTLDrawableID dst) {
		auto tex = dst.texture;
		auto &cp = _shaders.vportCp;
		auto pass = [MTLRenderPassDescriptor new];
		pass.colorAttachments[0].texture = tex;
		pass.colorAttachments[0].loadAction = MTLLoadActionDontCare;
		pass.colorAttachments[0].storeAction = MTLStoreActionStore;
		pass.colorAttachments[0].level = 0;
		auto enc = [cmd renderCommandEncoderWithDescriptor:pass];
		enc.label = @"Viewport Copy Pass";
		setRootMatrixFromEnc(enc, &_rootMatrix, _surfaceScale);
		setViewMatrixFromEnc(enc, Mat());
		[enc setViewport: {0, 0, (float)tex.width, (float)tex.height, 0, 1}];
		[enc setRenderPipelineState:_mtlrender->_vportCpPipeline];
		auto sampler = _surfaceSize == Vec2(tex.width, tex.height)? _mtlrender->_nearestSampler : _mtlrender->_linearSampler;
		[enc setFragmentTexture:_outTex atIndex:cp.fragment.image];
		[enc setFragmentSamplerState:sampler atIndex:cp.fragment.image];
		[enc setVertexBuffer:_mtlrender->_emptyBuffer offset:0 atIndex:cp.bufferIndex];
		[enc setFragmentBuffer:_mtlrender->_emptyBuffer offset:0 atIndex:3];
		[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
		[enc endEncoding];
		[cmd presentDrawable:dst];
	}

	bool MetalCanvas::use_texture(MTLEncoder enc, ImageSource *src, int srcSlot, int dstSlot, const PaintImage *paint) {
		auto index = paint->srcIndex + srcSlot;
		Qk_ASSERT_LT(index, 8, "Texture slot index out of range, srcIndex: %d, slot: %d", paint->srcIndex, srcSlot);
		auto tex = src->texture(index);
		// mark texture for this render, and try to create texture immediately
		src->markAsTexture();
		if (!tex->ptr()) {
			Qk_DLog("Texture not ready for paint image, src index: %d, slot: %d", paint->srcIndex, srcSlot);
			return false; // texture not ready
		}
		set_texture_param(enc, mtl_get_texture(tex), dstSlot, paint);
		return true;
	}

	void MetalCanvas::set_texture_param(MTLEncoder enc, MTLTextureID tex, int dstSlot, const PaintImage* paint) {
		auto sampler = get_sampler(paint); // get sampler state for paint image
		[enc setFragmentTexture:tex atIndex:dstSlot];
		[enc setFragmentSamplerState:sampler atIndex:dstSlot];
	}

	MTLSampler MetalCanvas::get_sampler(PaintImage::FilterMode filter, PaintImage::MipmapMode mipmap) {
		PaintImage img;
		img.tileModeX = PaintImage::kDecal_TileMode;
		img.tileModeY = PaintImage::kDecal_TileMode;
		img.filterMode = filter;
		img.mipmapMode = mipmap;
		return get_sampler(&img);
	}

	MTLSampler MetalCanvas::get_sampler(const PaintImage* paint) {
		uint32_t key = mtl_get_sampler_key(paint);
		MTLSampler sampler;
		if (!_texSamplers.get(key, sampler)) {
			sampler = _mtlrender->_resource->get_sampler(paint);
			_texSamplers.set(key, sampler);
		}
		return sampler;
	}

} // namespace qk
