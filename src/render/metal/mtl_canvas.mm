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
	MTLTextureID mtl_get_texture_from(ImageSource* src, MTLTextureID _else = nil);
	void clear_PathvCache(PathvCache *cache, int flags);
	void clearExec_PathvCache(PathvCache *cache);
	uint32_t mtl_get_sampler_key(const PaintImage* paint);

	void setrMatrixFromEnc(MTLEncoder enc, const Mat4 &mat) {
		auto matrix = mat.transpose();
		[enc setVertexBytes:matrix.val length: sizeof(Mat4) atIndex:1];
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
		, _render(render)
		, _device(nil), _commandQueue(nil)
		, _cmdPack{}, _cmdPackFront{}
		, _outTex(nil), _outColorTex(nil), _outDepthTex(nil)
	{
		_opts.colorType = _opts.colorType ? _opts.colorType:
			kBGRA_8888_ColorType; // metal prefers BGRA format, use it as default for better performance
		_device = _render->_device;
		_commandQueue = _render->_commandQueue; // share command queue with render
		_cmdPack.current = [_commandQueue commandBuffer]; // create command buffer for this canvas
		_shaders = _render->_resource->shaders(); // copy shader cache reference for render thread use
		_cmdPack.buffer = new MemBlockAllocator<MTLBufferID>();
		_cmdPackFront.buffer = new MemBlockAllocator<MTLBufferID>();
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
		_outDepthTex = nil; // Depth stencil buffer object of texture
		_commandQueue = nil; // Metal command queue
		_device = nil;
		_mutex.unlock();
	}

	void MetalCanvas::endPass() {
		if (_cmdPack.pass && !_cmdPack.enc) {
			_cmdPack.enc = [_cmdPack.current renderCommandEncoderWithDescriptor:_cmdPack.pass];
		}
		if (_cmdPack.enc) {
			[_cmdPack.enc endEncoding]; // end current pass
			_cmdPack.recorded = true;
			_cmdPack.enc = nil;
		}
		_cmdPack.pass = nil;
		_cmdPack.pipeline = nil;
	}

	MTLPassDesc MetalCanvas::beginPass(int level, bool isLoadColor) {
	 #if DEBUG
		if (_cmdPack.pass) {
			if (_cmdPack.pass.colorAttachments[0].texture == _outColorTex &&
					_cmdPack.pass.colorAttachments[0].level == level
			) {
				Qk_Fatal("Same render target should not begin a new pass without ending the previous pass.");
			}
		}
	 #endif
		endPass();
		// new pass descriptor
		auto pass = [MTLRenderPassDescriptor new];
		auto recorded = _cmdPack.isRecorded();

		Qk_ASSERT(_outColorTex, "Output color texture should be created before beginning a pass");
		pass.colorAttachments[0].texture = _outColorTex;
		pass.colorAttachments[0].loadAction = recorded && isLoadColor ? MTLLoadActionLoad : MTLLoadActionDontCare;
		pass.colorAttachments[0].storeAction = MTLStoreActionStore;
		pass.colorAttachments[0].level = level;

		Qk_ASSERT(_outDepthTex, "Output depth texture should be created before beginning a pass");
		pass.depthAttachment.texture = _outDepthTex;
		pass.depthAttachment.loadAction = recorded ? MTLLoadActionLoad: MTLLoadActionDontCare;
		pass.depthAttachment.storeAction = MTLStoreActionStore;

		// pass.stencilAttachment.texture = _outDepthTex;
		// pass.stencilAttachment.loadAction = loadAction;
		// pass.stencilAttachment.storeAction = MTLStoreActionStore;

		_cmdPack.pass = pass;
		return pass;
	}

	MTLEncoder MetalCanvas::getEncoder() {
		if (_cmdPack.enc)
			return _cmdPack.enc;
		if (!_cmdPack.pass)
			beginPass();
		_cmdPack.enc = [_cmdPack.current renderCommandEncoderWithDescriptor:_cmdPack.pass];
		Qk_ASSERT(_cmdPack.enc, "Failed to create render command encoder for new pass");
		// set root matrix for new encoder
		setrMatrixFromEnc(_cmdPack.enc, _rootMatrix);
		// set view matrix for new encoder
		setvMatrixFromEnc(_cmdPack.enc, _state->matrix);
		// set clip texture for new encoder if clip state exists
		if (_clipState) {
			MSLColor::ClipStatBlock clipStat = { *((Vec4*)_clipState->range.begin.val), _clipState->op };
			[_cmdPack.enc setFragmentBytes:&clipStat length:sizeof(clipStat) atIndex:3];
			[_cmdPack.enc setFragmentTexture:mtl_get_texture_from(*_clipState->mask) atIndex:0];
			[_cmdPack.enc setFragmentSamplerState:_render->_nearestSampler atIndex:0];
		} else {
			[_cmdPack.enc setFragmentBuffer:_render->_emptyBuffer offset:0 atIndex:3];
		}
		[_cmdPack.enc setViewport: {0, 0, _surfaceSize.x(), _surfaceSize.y(), 0, 1}];
		[_cmdPack.enc setDepthStencilState:_render->_depthOnly];
		return _cmdPack.enc;
	}

	void MetalCanvas::setPipeline(MTLEncoder enc, MTLPipeline pipeline) {
		if (_cmdPack.pipeline != pipeline) {
			[enc setRenderPipelineState:pipeline]; // set pipeline state for shader
			_cmdPack.pipeline = pipeline;
		}
	}

	void MetalCanvas::setPipeline(MTLEncoder enc, MSLShader& shader) {
		return setPipeline(enc, shader.getPipeline(_blendMode, _outColorTex.pixelFormat, _opts.msaaSample));
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
		endPass(); // end current pass to ensure all commands are encoded before swap
		_mutex.lock();
		bool canSwap = _cmdPackFront.current == nil;
		// only swap if there are recorded commands and front cmd pack is empty
		if (canSwap && _cmdPack.isRecorded()) {
			std::swap(_cmdPackFront, _cmdPack); // swap cmd buffer and pass descriptor to front
			clear_PathvCache(_cache, 0); // tag: clear mark
			Qk_ASSERT_EQ(_cmdPack.current, nil, "MetalCanvas: cmd buffer should be nil after swap");
			_cmdPack.current = [_commandQueue commandBuffer]; // create new cmd buffer for next frame
			_cmdPack.buffer->clear(); // clear vertex/index buffers allocator for next frame
		}
		_mutex.unlock();
		return canSwap;
	}

	Array<MTLCommandBuffer> MetalCanvas::flushBuffer() {
		_mutex.lock();
		auto cmds = std::move(_cmdPackFront.cmds); // get command buffers for flush
		if (_cmdPackFront.recorded) {
			// add command buffer to cmds for flush if it has recorded commands
			cmds.push(_cmdPackFront.current);
		}
		_cmdPackFront = {_cmdPackFront.buffer}; // reset front cmd pack
		_mutex.unlock();
		clearExec_PathvCache(_cache); // clear @clear marked cache after flush
		Qk_ReturnLocal(cmds); // return command buffers for flush
	}

	void MetalCanvas::flushSubcanvas(MetalCanvas *sub) {
		Qk_ASSERT_EQ(sub->_render, _render, "Subcanvas should belong to the same render for flush");
		auto cmds = sub->flushBuffer(); // flush subcanvas to get command buffers
		if (cmds.is_null())
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

	void MetalCanvas::vportCopy(MTLCommandBuffer cmd, MTLDrawableID dst) {
		auto tex = dst.texture;
		auto &cp = _shaders.vportCp;
		auto pass = [MTLRenderPassDescriptor new];
		pass.colorAttachments[0].texture = tex;
		pass.colorAttachments[0].loadAction = MTLLoadActionDontCare;
		pass.colorAttachments[0].storeAction = MTLStoreActionStore;
		pass.colorAttachments[0].level = 0;
		pass.depthAttachment.texture = _outDepthTex;
		pass.depthAttachment.loadAction = MTLLoadActionDontCare;
		pass.depthAttachment.storeAction = MTLStoreActionDontCare;
		auto enc = [cmd renderCommandEncoderWithDescriptor:pass];
		setrMatrixFromEnc(enc, Mat4());
		setvMatrixFromEnc(enc, Mat());
		[enc setViewport: {0, 0, (float)tex.width, (float)tex.height, 0, 1}];
		[enc setRenderPipelineState:_render->_vportCpPipeline];
		auto sampler = _surfaceSize == Vec2(tex.width, tex.height)? _render->_nearestSampler : _render->_linearSampler;
		[enc setFragmentTexture:_outTex atIndex:cp.fragment.image];
		[enc setFragmentSamplerState:sampler atIndex:cp.fragment.image];
		[enc setVertexBuffer:_render->_emptyBuffer offset:0 atIndex:cp.bufferIndex];
		[enc setFragmentBuffer:_render->_emptyBuffer offset:0 atIndex:3];
		[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
		[enc endEncoding];
		[cmd presentDrawable:dst];
	}

	bool MetalCanvas::use_texture(MTLEncoder enc, ImageSource *src, int srcSlot, int dstSlot, const PaintImage *paint) {
		auto index = paint->srcIndex + srcSlot;
		Qk_ASSERT_LT(index, 8, "Texture slot index out of range, srcIndex: %d, slot: %d", paint->srcIndex, srcSlot);
		auto tex = src->texture(index);
		if (!tex->ptr()) {
			// mark texture for this render, and try to create texture immediately
			src->markAsTexture(_render);
			if (!tex->ptr()) {
				Qk_DLog("Texture not ready for paint image, src index: %d, slot: %d", paint->srcIndex, srcSlot);
				return false; // texture not ready
			}
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
			sampler = _render->_resource->get_sampler(paint);
			_texSamplers.set(key, sampler);
		}
		return sampler;
	}

} // namespace qk
