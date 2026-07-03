/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#import "./mtl_canvas.h"
#import "./mtl_render.h"
#import "../source.h"
#import "../pixel.h"

namespace qk {
	MTLPixelFormat mtl_pixel_format(ColorType type);
	MTLTextureID mtl_get_texture(cTexStat *stat);
	MTLTextureID mtl_get_texture_from(const ImageSource* src, MTLTextureID _else = nil);
	void clear_PathvCache(PathvCache *cache, int flags);
	void clearExec_PathvCache(PathvCache *cache);
	uint32_t mtl_get_sampler_key(const PaintImage* paint);

	void setrMatrixFromEnc(MTLEncoder enc, const Mat4 mat[2], Vec2 surfaceScale) {
		MSLColor::RootMatrixBlock rMat {
			.value = mat[0].transpose(), // transpose for shader
			.noScale = mat[1].transpose(), // transpose for shader
			.surfaceScale = surfaceScale
		};
		[enc setVertexBytes:&rMat length: sizeof(rMat) atIndex:1];
	}

	void setvMatrixFromEnc(MTLEncoder enc, const Mat &mat) {
		float vm4x4[16] = {
			mat[0], mat[3], 0.0, 0.0,
			mat[1], mat[4], 0.0, 0.0,
			0.0,    0.0,    1.0, 0.0,
			mat[2], mat[5], 0.0, 1.0
		}; // transpose matrix
		[enc setVertexBytes:vm4x4 length: sizeof(vm4x4) atIndex:2];
	}

	MetalCanvas::MetalCanvas(MetalRender *render, Render::Options opts)
		: GPUCanvas(render, opts)
		, _mtlrender(render)
		, _device(nil), _commandQueue(nil)
		, _cmdPack{}, _cmdPackFront{}
		, _outTex(nil), _outColorTex(nil)
	{
		_opts.colorType = _opts.colorType ? _opts.colorType:
			kBGRA_8888_ColorType; // metal prefers BGRA format, use it as default for better performance
		_device = _mtlrender->_device;
		_commandQueue = _mtlrender->_commandQueue; // share command queue with render
		_shaders = _mtlrender->_resource->shaders(); // copy shader cache reference for render thread use
		_cmdPack.current = [_commandQueue commandBuffer]; // create command buffer for this canvas
		_cmdPack.buffer = new MemBlockAllocator<MTLBufferID>();
		_cmdPackFront.buffer = new MemBlockAllocator<MTLBufferID>();
		_cgaaBuilder = new CGAABuilder(this); // create CGAA builder for anti-aliasing paths
		_capaBuilder = new CAPABuilder(this);
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
		_outColorTex = nil; // Current active color render target texture
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
			if (_cmdPack.pass.colorAttachments[0].texture == _outColorTex &&
					_cmdPack.pass.colorAttachments[0].level == level
			) {
				Qk_Fatal("Same render target should not begin a new pass without ending the previous pass.");
			}
		}
	 #endif
		endPass();
		auto pass = _cmdPack.pass ? _cmdPack.pass: [MTLRenderPassDescriptor new];
		auto recorded = _cmdPack.isRecorded();

		Qk_ASSERT(_outColorTex, "Output color texture should be created before beginning a pass");
		pass.colorAttachments[0].texture = _outColorTex;
		pass.colorAttachments[0].loadAction = recorded && loadColor ? MTLLoadActionLoad : MTLLoadActionDontCare;
		pass.colorAttachments[0].storeAction = MTLStoreActionStore;
		pass.colorAttachments[0].level = level;

		_cmdPack.pass = pass;
		_cmdPack.beginPass = true;
		return pass;
	}

	MTLEncoder MetalCanvas::getEncoder() {
		if (_cmdPack.enc)
			return _cmdPack.enc;
		if (!_cmdPack.beginPass)
			beginPass();
		_cmdPack.enc = [_cmdPack.current renderCommandEncoderWithDescriptor:_cmdPack.pass];
		Qk_ASSERT(_cmdPack.enc, "Failed to create render command encoder for new pass");
		// set root matrix for new encoder
		setrMatrixFromEnc(_cmdPack.enc, &_rootMatrix, _surfaceScale);
		// set view matrix for new encoder
		setvMatrixFromEnc(_cmdPack.enc, _state->matrix);
		// set clip texture for new encoder if clip state exists
		if (_clipState) {
			MSLColor::ClipStatBlock clipStat = { *((Vec4*)_clipState->range.begin.val), _clipState->op };
			[_cmdPack.enc setFragmentBytes:&clipStat length:sizeof(clipStat) atIndex:3];
			[_cmdPack.enc setFragmentTexture:mtl_get_texture_from(*_clipState->mask) atIndex:0];
			[_cmdPack.enc setFragmentSamplerState:_mtlrender->_nearestSampler atIndex:0];
		} else {
			[_cmdPack.enc setFragmentBuffer:_mtlrender->_emptyBuffer offset:0 atIndex:3];
		}
		[_cmdPack.enc setViewport: {0, 0, _surfaceSize.x(), _surfaceSize.y(), 0, 1}];
		return _cmdPack.enc;
	}

	void MetalCanvas::setPipeline(MTLEncoder enc, MTLPipeline pipeline) {
		if (_cmdPack.pipeline != pipeline) {
			[enc setRenderPipelineState:pipeline]; // set pipeline state for shader
			_cmdPack.pipeline = pipeline;
		}
	}

	void MetalCanvas::setPipeline(MTLEncoder enc, MSLShader& shader) {
		return setPipeline(enc, shader.getPipeline(_blendMode, _outColorTex.pixelFormat));
	}

	// usePipeline with vertex data ensures vertex data is valid and set for draw call,
	// if vertex data is invalid, return nil and skip draw call
	MTLEncoder MetalCanvas::usePipeline(MSLShader& shader, const VertexData &vertex, MTLEncoder enc) {
		setPipeline(enc, shader); // set pipeline state for shader
		if (Render::useVertexData(vertex.id)) {
			auto buf = (__bridge id<MTLBuffer>)vertex.id->ptr; // get vertex buffer from vertex data id
			[enc setVertexBuffer:buf offset:0 atIndex:shader.bufferIndex];
		} else if (vertex.vertex.val()) {
			Qk_ASSERT_EQ(vertex.vertex.length(), vertex.vCount, "Vertex data length should match vertex count");
			[enc setVertexBytes:vertex.vertex.val() length:vertex.vertex.size() atIndex:shader.bufferIndex];
		} else {
			return nil; // invalid vertex data, skip draw call
		}
		return enc;
	}

	bool MetalCanvas::swapBuffer() {
		if (_capaBuilder)
			_capaBuilder->commit(); // commit CAPA data for current frame before swap
		if (_cgaaBuilder)
			_cgaaBuilder->commit(); // commit CGAA data for current frame before swap
		endPass(); // end current pass to ensure all commands are encoded before swap
		_mutex.lock();
		bool canSwap = _cmdPackFront.current == nil;
		// only swap if there are recorded commands and front cmd pack is empty
		if (canSwap && _cmdPack.isRecorded()) {
			std::swap(_cmdPackFront, _cmdPack); // swap cmd buffer and pass descriptor to front
		}
		clear_PathvCache(_cache, 0); // tag: clear mark
		// reset cmd pack for next frame
		_cmdPack = {
			.buffer = std::move(_cmdPack.buffer), // move buffer allocator to new cmd pack
			.current = [_commandQueue commandBuffer], // create new command buffer for next frame
		};
		_cmdPack.buffer->reset(); // reset buffer allocator for new frame
		_mutex.unlock();
		return canSwap;
	}

	Array<MTLCommandBufferID> MetalCanvas::flushBuffer() {
		_mutex.lock();
		Qk_ASSERT(!_cmdPackFront.beginPass, "Cannot flush buffer while a pass is still active, end the pass first");
		auto cmds = std::move(_cmdPackFront.cmds); // get command buffers for flush
		if (_cmdPackFront.recorded) {
			// add command buffer to cmds for flush if it has recorded commands
			cmds.push(_cmdPackFront.current);
		}
		_cmdPackFront.recorded = false;
		[_cmdPackFront.current addCompletedHandler:^(MTLCommandBufferID buffer) {
			// clear front command pack after flush is completed
			_cmdPackFront.current = nil;
		}];
		_mutex.unlock();
		clearExec_PathvCache(_cache); // clear @clear marked cache after flush
		Qk_ReturnLocal(cmds); // return command buffers for flush
	}

	void MetalCanvas::flushSubcanvasCmd(GPUCanvas *sub) {
		if (sub == this)
			return; // only flush subcanvas if it is not the same as current canvas
		if (sub->render() != _render)
			return; // only flush subcanvas if it is from the same render
		auto cmds = static_cast<MetalCanvas*>(sub)->flushBuffer(); // flush subcanvas to get command buffers
		if (cmds.isNull())
			return; // if no command buffers, skip

		endPass(); // end current pass to ensure all commands are encoded before flush

		if (_cmdPack.recorded) {
			// add current command buffer to cmds for flush if it has recorded commands
			_cmdPack.cmds.push(_cmdPack.current);
		}
		_cmdPack.current = cmds.back(); // get a command buffer from cmds for next pass
		cmds.pop();
		// add remaining command buffers to cmd pack for flush
		_cmdPack.cmds.concat(cmds);
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
		setrMatrixFromEnc(enc, &_rootMatrix, _surfaceScale);
		setvMatrixFromEnc(enc, Mat());
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
