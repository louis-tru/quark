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

#define Qk_FLAG_AACLIP (1u << 0)
#define Qk_AACLIP(clip) (clip ? Qk_FLAG_AACLIP: 0)
// use pipeline and set vertex buffer for vertex data, if invalid vertex data, return and skip draw call
#define Qk_usePipeline(shader, ...) auto enc = usePipeline(shader,##__VA_ARGS__); if (!enc) return
// set texture for slot 0 and return encoder, if texture not ready, return nil and skip draw call
#define Qk_useTextureSlot0(paint, dstSlot, ...) bool isYuv = false; \
	auto enc = useTextureSlot0(paint, dstSlot, &isYuv); if (!enc) return __VA_ARGS__

namespace qk {
	extern const float  zDepthNextUnit;
	MTLPixelFormat mtl_pixel_format(ColorType type);
	MTLTextureID mtl_new_tex_renderbuffer(MTLDeviceID device, Vec2 size, MTLPixelFormat format, bool read, bool priv, bool mipmap);
	MTLTextureID mtl_get_texture(cTexStat *stat);
	MTLTextureID mtl_get_texture_from(ImageSource* src, MTLTextureID _else = nil);
	void setTex_SourceImage(ImageSource* s, cPixelInfo &i, cTexStat *tex);
	void clear_PathvCache(PathvCache *cache, int flags);
	void clearExec_PathvCache(PathvCache *cache);
	cTexStat* mtl_rebuild_texture(MTLDeviceID device, Vec2 size, ColorType type, cTexStat* texStat, TexStat &newStat, bool mipmap);

	void setvMatrixFromEnc(MTLRenderEncoder enc, const Mat &mat) {
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
	}

	MetalCanvas::~MetalCanvas() {
		_mutex.lock();
		_device = nil;
		_commandQueue = nil; // Metal command queue
		_cmdPack = {}; // clear
		_cmdPackFront = {}; // clear
		_outTex = nil; // Color render buffer object of texture
		_outColorTex = nil; // Current active color render target texture
		_outDepthTex = nil; // Depth stencil buffer object of texture
		_mutex.unlock();
	}

	void MetalCanvas::endPass() {
		if (_cmdPack.pass && !_cmdPack.enc) {
			_cmdPack.enc = [_cmdPack.current renderCommandEncoderWithDescriptor:_cmdPack.pass];
			_cmdPack.pass = nil;
		}
		if (_cmdPack.enc) {
			[_cmdPack.enc endEncoding]; // end current pass
			_cmdPack.recorded = true;
			_cmdPack.enc = nil;
		}
		_cmdPack.pipeline = nil;
	}

	MTLPassDesc MetalCanvas::beginPass() {
		endPass();

		auto loadAction = _cmdPack.isRecorded() ? MTLLoadActionLoad: MTLLoadActionDontCare;
		auto pass = [MTLRenderPassDescriptor new];

		Qk_ASSERT(_outColorTex, "Output color texture should be created before beginning a pass");
		pass.colorAttachments[0].texture = _outColorTex;
		pass.colorAttachments[0].loadAction = loadAction;
		pass.colorAttachments[0].storeAction = MTLStoreActionStore;

		Qk_ASSERT(_outDepthTex, "Output depth texture should be created before beginning a pass");
		pass.depthAttachment.texture = _outDepthTex;
		pass.depthAttachment.loadAction = loadAction;
		pass.depthAttachment.storeAction = MTLStoreActionStore;

		// pass.stencilAttachment.texture = _outDepthTex;
		// pass.stencilAttachment.loadAction = loadAction;
		// pass.stencilAttachment.storeAction = MTLStoreActionStore;

		_cmdPack.pass = pass;
		return pass;
	}

	MTLPassDesc MetalCanvas::beginPassFrom(int level, bool loadColor, bool disableDepth) {
		beginPass();
		_cmdPack.pass.colorAttachments[0].loadAction = loadColor ? MTLLoadActionLoad: MTLLoadActionDontCare;
		_cmdPack.pass.colorAttachments[0].level = level;
		if (disableDepth) {
			_cmdPack.pass.depthAttachment.texture = nil;
		}
		return _cmdPack.pass;
	}

	MTLRenderEncoder MetalCanvas::getEncoder() {
		if (_cmdPack.enc)
			return _cmdPack.enc;
		if (!_cmdPack.pass)
			beginPass();
		_cmdPack.enc = [_cmdPack.current renderCommandEncoderWithDescriptor:_cmdPack.pass];
		Qk_ASSERT(_cmdPack.enc, "Failed to create render command encoder for new pass");
		// set root matrix for new encoder
		[_cmdPack.enc setVertexBytes:_rootMatrix.val length: sizeof(Mat4) atIndex:1];
		// set view matrix for new encoder
		setvMatrixFromEnc(_cmdPack.enc, _state->matrix);
		// set clip texture for new encoder if clip state exists
		if (_clipState) {
			MSLColor::ClipStatBlock clipStat = { *((Vec4*)_clipState->range.begin.val), _clipState->op };
			[_cmdPack.enc setFragmentBytes:&clipStat length:sizeof(clipStat) atIndex:3];
			[_cmdPack.enc setFragmentTexture:mtl_get_texture_from(_clipState->mask) atIndex:0];
			[_cmdPack.enc setFragmentSamplerState:_render->_clipSampler atIndex:0];
		}
		_cmdPack.enc.viewport = MTLViewportMake(0, 0, _surfaceSize.x(), _surfaceSize.y(), 0, 1);
		return _cmdPack.enc;
	}

	inline void MetalCanvas::setPipeline(MTLRenderEncoder enc, MSLShader& shader) {
		auto pipeline = getPipeline(shader);
		if (_cmdPack.pipeline != pipeline) {
			[enc setRenderPipelineState:pipeline]; // set pipeline state for shader
			_cmdPack.pipeline = pipeline;
		}
	}

	// usePipeline with vertex data ensures vertex data is valid and set for draw call,
	// if vertex data is invalid, return nil and skip draw call
	MTLRenderEncoder MetalCanvas::usePipeline(MSLShader& shader, const VertexData &vertex, MTLRenderEncoder enc) {
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

	// usePipeline ensures vertex data is valid and set for draw call,
	// if vertex data is invalid, return nil and skip draw call
	inline MTLRenderEncoder MetalCanvas::usePipeline(MSLShader& shader, const VertexData &vertex) {
		return usePipeline(shader, vertex, getEncoder());
	}

	// usePipeline without vertex data just sets the pipeline state for shader and returns encoder for draw call
	inline MTLRenderEncoder MetalCanvas::usePipeline(MSLShader& shader) {
		auto enc = getEncoder();
		setPipeline(enc, shader);
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
		_cmdPackFront = {}; // reset front cmd pack
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
		_cmdPack.cmds.concat(cmds); // add remaining command buffers to cmd pack for flush
	}

	// --------------------------------------------------------------------

	void MetalCanvas::setSurfaceCmd(bool changeSize) {
		if (changeSize) {
			_outTex = mtl_new_tex_renderbuffer(
				_device, _surfaceSize, mtl_pixel_format(_opts.colorType), true, true, false);
			_outDepthTex = mtl_new_tex_renderbuffer(
				_device, _surfaceSize, MTLPixelFormatDepth32Float_Stencil8, false, true, false);
		}
		_outColorTex = _outTex; // set to main texture by default

		// start a new pass with new buffers
		auto pass = beginPass();
		pass.colorAttachments[0].loadAction = MTLLoadActionClear;
		pass.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 0); // clear to transparent
		// clear depth to default values
		pass.depthAttachment.loadAction = MTLLoadActionClear;
		pass.depthAttachment.clearDepth = 0; // default depth value

		// Root/view matrices are uploaded when the encoder is lazily created by getEncoder().
	}

	void MetalCanvas::setMatrixCmd() {
		if (_cmdPack.enc) {
			// set matrix bytes for current encoder if it exists, so that it can be used by subsequent draw calls
			setvMatrixFromEnc(_cmdPack.enc, _state->matrix);
		}
	}

	void MetalCanvas::setBlendModeCmd() {
		// no need to set blend mode for Metal, it will be set in pipeline state when encoding draw calls
	}

	void MetalCanvas::drawClipCmd(const VertexData &vertex, const VertexData &aafuzz,
			GC_State::Clip *last, GC_State::Clip *clip, ClipOp rawOp) {
		auto begin = clip->range.begin,
					end = clip->range.end, size = end - begin;
		auto depth = _zDepth;
		auto blend = _blendMode; // save current blend mode
		auto colorTex = _outColorTex; // save current color texture
		// switch blend mode to src
		_blendMode = kSrc_BlendMode;
		// output to clip mask texture
		_outColorTex = mtl_get_texture_from(clip->mask);
		beginPass(); // start a new pass for clip drawing

		auto drawClip = [&](bool black, bool clip) {
			depth += zDepthNextUnit;
			auto scale = 1 / _surfaceScale;
			Vec4 surface = {-begin.x(), -begin.y(), scale, scale};
			// Difference clip cannot directly render solid black with AA,
			// otherwise edge blending becomes incorrect.
			// Instead, invert the aafuzz alpha curve:
			//   normal:   alpha = 1 - abs(aafuzz)
			//   inverted: alpha = abs(aafuzz)
			// This produces a smooth subtractive mask edge.
			int flags = black ? 1u << 2 : 0; // Qk_FLAG_AAFUZZ_Inverted
			flags |= Qk_CLIP(clip); // set clip flag if have clip
			drawColor(vertex, {1,1,1,1}, surface, depth, flags);
			if (aafuzz.vCount) { // draw aa fuzz if have
				drawColor(aafuzz, {1,1,1,1}, surface, depth, flags);
			}
		};
		if (rawOp == Canvas::kIntersect_ClipOp || !last) {
			// clear clipTex with black color
			clearColor({0,0,0,0}, {0,size}, depth);
			// draw clip shape to clipTex with white color
			drawClip(false, last);
		} else { // if (rawOp == Canvas::kDifference_ClipOp)
			// copy last clip color to clipTex as the clear color
			copyImage(last->mask.get(), begin - last->range.begin, {0,size}, size, depth);
			// draw clip shape to clipTex with white color if last op equal difference,
			// or black color if last op equal intersect
			auto black = last->op == Canvas::kIntersect_ClipOp;
			// draw clip shape to clipTex with color
			drawClip(black, last);
		}
		endPass();
		// restore framebuffer and blend mode
		_blendMode = blend;
		_outColorTex = colorTex;
		restoreClipCmd(clip); // set clip data
	}

	void MetalCanvas::restoreClipCmd(GC_State::Clip* clip) {
		_clipState = clip; // update current clip state
		if (_cmdPack.enc == nil) return;
		if (clip) {
			MSLColor::ClipStatBlock clipStat = { *((Vec4*)clip->range.begin.val), clip->op };
			[_cmdPack.enc setFragmentBytes:&clipStat length:sizeof(clipStat) atIndex:3];
			[_cmdPack.enc setFragmentTexture:mtl_get_texture_from(clip->mask) atIndex:0];
			[_cmdPack.enc setFragmentSamplerState:_render->_clipSampler atIndex:0];
		} else {
			[_cmdPack.enc setFragmentTexture:nil atIndex:0];
		}
	}

	void MetalCanvas::copyImage(ImageSource *src, Vec2 srcOffset, Range dst, Vec2 resolution, float depth) {
		float x1 = dst.begin.x(), y1 = dst.begin.y();
		float x2 = dst.end.x(), y2 = dst.end.y();
		float vertex[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0, };
		auto &cp = _render->_shaders.cp;
		auto scale = resolution / src->size();
		auto offset = (srcOffset - dst.begin) / src->size();
		auto coord = Vec4(offset.x(), offset.y(), scale.x(), scale.y());
		auto enc = usePipeline(cp);
		MSLCp::PcArgs pc{ resolution, resolution, coord, 0, depth, 0 };
		[enc setVertexBytes:vertex length:sizeof(vertex) atIndex:cp.bufferIndex];
		[enc setVertexBytes:&pc length:sizeof(pc) atIndex:0];
		[enc setFragmentBytes:&pc length:sizeof(pc) atIndex:0];
		[enc setFragmentTexture:mtl_get_texture_from(src) atIndex:cp.fragment.image];
		[enc setFragmentSamplerState:_render->_clipSampler atIndex:cp.fragment.image];
		[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
	}

	void MetalCanvas::drawColor(const VertexData &vertex, const Color4f &color, Vec4 surfaceOffset, float depth, int flags) {
		Qk_usePipeline(_render->_shaders.color, vertex); // use shader and set vertex buffer for vertex data
		// set color and other args for shader push constants
		MSLColor::PcArgs pc{ color, surfaceOffset, _zDepth, flags };
		// set vertex bytes
		[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
		// set fragment bytes
		[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
		// draw a full-screen triangle for clear
		[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vertex.vCount];
	}

	void MetalCanvas::clearColor(const Color4f &color, const Range &range, float depth) {
		float x1 = range.begin.x(), y1 = range.begin.y();
		float x2 = range.end.x(), y2 = range.end.y();
		float vertex[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
		auto &clear = _render->_shaders.clear;
		auto enc = usePipeline(clear); // use pipeline state for clear shader
		// set color and other args for shader push constants
		MSLClear::PcArgs pc{ color, _zDepth, 0 };
		[enc setVertexBytes:vertex length:sizeof(vertex) atIndex:clear.bufferIndex];
		[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
		[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
		[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
	}

	void MetalCanvas::drawColorCmd(const VertexData &vertex, const Color4f &color) {
		drawColor(vertex, premul_alpha(color), {0,0,1,1}, _zDepth, Qk_AACLIP(_clipState));
	}

	void MetalCanvas::clearColorCmd(const Color4f &color, GC_ClearFlags flags) {
		if (flags) {
			auto pass = beginPass(); // start a new pass for clear
			if (flags == kClearAll_ClearFlags) {
				pass.depthAttachment.loadAction = MTLLoadActionClear;
				pass.depthAttachment.clearDepth = 0;
			}
			pass.colorAttachments[0].loadAction = MTLLoadActionClear;
			pass.colorAttachments[0].clearColor = MTLClearColorMake(color.r(), color.g(), color.b(), color.a());
		} else {
			clearColor(premul_alpha(color), {0, _surfaceSize}, _zDepth); // clear full screen with shader
		}
	}

	MTLRenderEncoder MetalCanvas::useTextureSlot0(const PaintImage *paint, int dstSlot, bool* isYuv) {
		if (paint->_isCanvas) { // flush canvas to current canvas
			auto srcC = static_cast<MetalCanvas*>(paint->canvas);
			if (srcC != this && srcC->isGpu()) { // now only supported gpu
				if (srcC->_render == _render) {
					if (srcC->_outTex) {
						flushSubcanvas(srcC); // flush subcanvas to current canvas
						auto enc = getEncoder();
						_render->set_texture_param(enc, srcC->_outTex, dstSlot, paint);
						return enc;
					}
				}
			}
			return nil;
		} else {
			auto enc = getEncoder();
			if (kYUV420P_Y_8_ColorType == paint->image->type()) { // yuv420p or yuv420sp
				if (isYuv) *isYuv = true;
			}
			if (_render->use_texture(enc, paint->image, 0, dstSlot, paint)) {
				return enc;
			}
			return nil;
		}
	}

	void MetalCanvas::drawImageCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color) {
		auto &shader = _render->_shaders.image;
		// set texture for slot 0 and return encoder, if texture not ready, return nil and skip draw call
		Qk_useTextureSlot0(paint, shader.fragment.image); // slot 0 default match dst slot to 1
		if (isYuv) { // yuv420p or yuv420sp
			auto src = paint->image;
			auto &yuv = _render->_shaders.imageYuv;
			Qk_ASSERT_EQ(shader.fragment.image, yuv.fragment.image,
				"YUV shader should use the same texture slot for image as non-YUV shader");
			enc = usePipeline(yuv, vertex, enc);
			if (!enc) return;
			if (!_render->use_texture(enc, src, 1, yuv.fragment.image_uv, paint))
				return; // u or uv
			int format = 0; // default to yuv420sp
			if (src->pixel(1)->type() == kYUV420P_U_8_ColorType) {
				if (!_render->use_texture(enc, src, 2, yuv.fragment.image_v, paint))
					return; // v
				format = 1; // yuv420p
			}
			MSLImageYuv::PcArgs pc{
				*((Vec4*)paint->coord.begin.val),
				premul_alpha(color),
				format,
				_allScale,
				_zDepth,
				Qk_AACLIP(_clipState)
			};
			[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
		} else {
			enc = usePipeline(shader, vertex, enc);
			if (!enc) return;
			// set color and other args for shader push constants
			MSLImage::PcArgs pc{
				*((Vec4*)paint->coord.begin.val),
				premul_alpha(color),
				_allScale,
				_zDepth,
				Qk_AACLIP(_clipState)
			};
			[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
		}
		[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vertex.vCount];
	}

	void MetalCanvas::drawImageMaskCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color) {
		auto &shader = _render->_shaders.imageMask;
		Qk_useTextureSlot0(paint, shader.fragment.image);
		enc = usePipeline(shader, vertex, enc);
		if (!enc) return;
		auto type = paint->_isCanvas ? kRGBA_8888_ColorType: paint->image->type();
		MSLImageMask::PcArgs pc{
			*((Vec4*)paint->coord.begin.val),
			premul_alpha(color),
			type == kAlpha_8_ColorType ? 0 : type == kLuminance_Alpha_88_ColorType ? 1 : 3,
			_allScale,
			_zDepth,
			Qk_AACLIP(_clipState)
		};
		[enc setVertexBytes:&pc length:sizeof(pc) atIndex:0];
		[enc setFragmentBytes:&pc length:sizeof(pc) atIndex:0];
		[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vertex.vCount];
	}

	void MetalCanvas::drawSDFImageMaskCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color,
			const Color4f &strokeColor, float stroke) {
		auto &shader = _render->_shaders.imageSdfMask;
		Qk_useTextureSlot0(paint, shader.fragment.image);
		enc = usePipeline(shader, vertex, enc);
		if (!enc) return;
		MSLImageSdfMask::PcArgs pc{
			*((Vec4*)paint->coord.begin.val),
			premul_alpha(color),
			premul_alpha(strokeColor),
			stroke,
			_allScale,
			_zDepth,
			Qk_AACLIP(_clipState)
		};
		[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
		[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
		[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vertex.vCount];
	}

	void MetalCanvas::drawGradientCmd(const VertexData &vertex, const PaintGradient *paint, const Color4f &color) {
		static_assert(sizeof(MSLColorRadial::PcArgs) == sizeof(MSLColorLinear::PcArgs),
				"MSLColorRadial::PcArgs and MSLColorLinear::PcArgs must have identical size");
		static_assert(sizeof(MSLColorRadial) == sizeof(MSLColorLinear),
				"MSLColorRadial and MSLColorLinear must have identical size");
		int count = Qk_Min(64, paint->count);
		auto &shader = paint->type == PaintGradient::kRadial_Type ?
			(MSLColorLinear&)_render->_shaders.colorRadial : _render->_shaders.colorLinear;
		Qk_usePipeline(shader, vertex);
		Array<Color4f> colors(count);
		for (int i = 0; i < count; i++) {
			colors[i] = premul_alpha(paint->colors[i]);
		}
		MSLColorRadial::PcArgs pc{
			*((Vec4*)paint->origin.val),
			premul_alpha(color),
			count,
			_zDepth,
			Qk_AACLIP(_clipState) | (count == 2 ? (1u << 1): 0)
		};
		[enc setVertexBytes:&pc length:sizeof(pc) atIndex:0];
		[enc setFragmentBytes:&pc length:sizeof(pc) atIndex:0];
		[enc setFragmentBytes:colors.val() length:sizeof(Color4f) * count atIndex:shader.fragment.colors];
		[enc setFragmentBytes:paint->positions length:sizeof(float) * count atIndex:shader.fragment.positions];
		[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vertex.vCount];
	}

	void MetalCanvas::drawRRectBlurColorCmd(const Rect& rect, const float *radius, float blur, const Color4f &color) {
		blur = Qk_Max(blur, 0.5);
		float s1 = blur * 1.15, s2 = blur * 2.0;
		float min_edge = Qk_Min(rect.size[0], rect.size[1]);
		float rmax = 0.5 * min_edge;
		Vec2 size = rect.size * 0.5;
		Vec2 c = rect.begin + size;
		float w = size[0] + blur, h = size[1] + blur;
		float x1 = c[0] - w, x2 = c[0] + w;
		float y1 = c[1] - h, y2 = c[1] + h;
		Vec2 horns[] = { {x1,y1}, {x2,y1}, {x2,y2}, {x1,y2} };
		auto& sh = _render->_shaders.colorRrectBlur;
		auto enc = usePipeline(sh);
		int flags = Qk_AACLIP(_clipState);
		float s_inv = 1.0f / blur;

		for (int i = 0; i < 4; i++) {
			auto horn = horns[i];
			float v[] = { c[0],c[1],horn[0],c[1],horn[0],horn[1],c[0],horn[1] };
			float r0 = Float32::min(Vec2(radius[i], s1).length(), rmax);
			float r1 = Float32::min(Vec2(radius[i], s2).length(), rmax);
			float n = 2.0 * r1 / r0;
			MSLColorRrectBlur::PcArgs pc{
				horn,
				color,
				{ Vec3(r1, n, 1.0 / n), 0 },
				min_edge,
				s_inv,
				_zDepth,
				uint32_t(flags),
			};
			[enc setVertexBytes:v length:sizeof(v) atIndex:sh.bufferIndex];
			[enc setVertexBytes:&pc length:sizeof(pc) atIndex:0];
			[enc setFragmentBytes:&pc length:sizeof(pc) atIndex:0];
			[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
		}
	}

	void MetalCanvas::drawTrianglesCmd(const Triangles& triangles, const PaintImage *paint, const Color4f &color, bool copyData) {
		if (!triangles.verts || !triangles.indices || !triangles.vertCount || !triangles.indexCount)
			return;
		Qk_ASSERT_EQ(triangles.indexCount % 3, 0, "Triangle index count should be a multiple of 3");
		auto &shader = _render->_shaders.triangles;
		Qk_useTextureSlot0(paint, shader.fragment.image);
		setPipeline(enc, shader);

		auto vbuf = [_device newBufferWithBytes:triangles.verts
																		 length:sizeof(V3F_T2F_C4B_C4B) * triangles.vertCount
																		options:MTLResourceStorageModeShared];
		auto ibuf = [_device newBufferWithBytes:triangles.indices
																		 length:sizeof(uint16_t) * triangles.indexCount
																		options:MTLResourceStorageModeShared];
		if (!vbuf || !ibuf)
			return;

		MSLTriangles::PcArgs pc{
			color,
			_zDepth,
			Qk_AACLIP(_clipState) | (triangles.isDarkColor ? (1u << 1): 0)
		};
		[enc setVertexBuffer:vbuf offset:0 atIndex:shader.bufferIndex];
		[enc setVertexBytes:&pc length:sizeof(pc) atIndex:0];
		[enc setFragmentBytes:&pc length:sizeof(pc) atIndex:0];
		[enc drawIndexedPrimitives:MTLPrimitiveTypeTriangle
											 indexCount:triangles.indexCount
												indexType:MTLIndexTypeUInt16
											 indexBuffer:ibuf
									 indexBufferOffset:0];
	}

	void MetalCanvas::blurFilterBeginCmd(Range bounds, Mat4 &blurRootMat, ImageSource *tmpA) {
		auto outTexA = mtl_get_texture_from(tmpA);
		Qk_ASSERT(outTexA, "blurFilterBeginCmd tmpA texture is null");
		auto blend = _blendMode; // save current blend mode
		_rootMatrix = blurRootMat;
		_blendMode = kSrc_BlendMode;
		_outColorTex = outTexA; // output to texture A for blur filter then do post processing
		// begin a new pass for blur filter with texture A as render target
		auto pass = beginPassFrom(0, false);
		pass.colorAttachments[0].loadAction = MTLLoadActionClear;
		pass.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 0);

		// clear color
		// ..

		_blendMode = blend; // restore blend mode
	}

	/**
		* Fast gaussian blur reference https://www.shadertoy.com/view/ctjcWR
		* Relevant information reference:
		*   https://www.shadertoy.com/view/WtKfD3
		*   https://blog.ivank.net/fastest-gaussian-blur.html
		*   https://elynxsdk.free.fr/ext-docs/Blur/Fast_box_blur.pdf
		*   https://www.peterkovesi.com/papers/FastGaussianSmoothing.pdf
		*/
	void MetalCanvas::blurFilterEndCmd(Range bounds, Mat4 &recoverRootMat, float radius, float clearPad,
			int sample, int imageLod, ImageSource *tmpA, ImageSource *tmpB) {
		auto texA = mtl_get_texture_from(tmpA);
		auto texB = mtl_get_texture_from(tmpB);
		Qk_ASSERT(texA && texB, "blurFilterEndCmd temp texture is null");
		auto offset = bounds.begin.max(0);
		auto begin = bounds.begin, end = bounds.end;
		float x1 = begin.x() - offset.x(), y1 = begin.y() - offset.y();
		float x2 = end.x() - offset.x(), y2 = end.y() - offset.y();
		float vertex[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
		float radius2 = radius * _allScale; // radius in pixel unit
		Vec2 iR = tmpA->size(); // input resolution
		int oRw = iR.x(), oRh = iR.y();
		float depth = _zDepth;

		auto blend = _blendMode; // save current blend mode
		_blendMode = kSrc_BlendMode;
		_rootMatrix = recoverRootMat; // recover root matrix
		// get sampler state for paint image
		auto sampler = _render->get_sampler(PaintImage::kNearest_FilterMode,
			PaintImage::kLinearNearest_MipmapMode);
		auto &cp = _render->_shaders.cp;
		// Choosing the right blur shader
		auto blur = &_render->_shaders.blur;

		if (imageLod) {
			if (oRw >> imageLod == 0 || oRh >> imageLod == 0) {
				_outColorTex = mtl_get_texture_from(_state->output, _outTex);
				_blendMode = blend;
				beginPass();
				return;
			}
			// |r|r|rrrrrr|r|r|
			// |r|r|rrrrrr|r|r|
			// |r|r| body |r|r|
			// |r|r|rrrrrr|r|r|
			// |r|r|rrrrrr|r|r|
			int level = 0;
			do { // Copy the image to smaller texture for next level
				oRw >>= 1; oRh >>= 1;
				MSLCp::PcArgs pcArgs{ iR, Vec2(oRw, oRh), { 0, 0, 1, 1 }, float(level++), depth, 0};
				beginPassFrom(level, false); // begin new pass for next level
				auto enc = usePipeline(cp);
				enc.viewport = MTLViewportMake(0, 0, oRw, oRh, 0, 1); // set viewport for current level
				[enc setVertexBytes:vertex length:sizeof(vertex) atIndex:cp.bufferIndex];
				[enc setVertexBytes:&pcArgs length: sizeof(pcArgs) atIndex:0];
				[enc setFragmentBytes:&pcArgs length: sizeof(pcArgs) atIndex:0];
				[enc setFragmentTexture:_outTexA atIndex:cp.fragment.image];
				[enc setFragmentSamplerState:sampler atIndex:cp.fragment.image];
				[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
				depth += zDepthNextUnit;
			} while (level < imageLod);
		}
		{
			// The blur regions for x-axis
			// |r|rrrrrr|r|
			// |r|rrrrrr|r|
			// |r| body |r|
			// |r|rrrrrr|r|
			// |r|rrrrrr|r|
			_outColorTex = _outTexB; // output to texture B
			// begin new pass for blur filter with texture B as render target
			beginPassFrom(imageLod, false);
			// Making blur of the x-axis direction
			float vertex_x[] = { x1+radius,y1,0, x2-radius,y1,0, x1+radius,y2,0, x2-radius,y2,0 };
			MSLBlur::PcArgs pc{ iR, Vec2(oRw, oRh), Vec2(radius2 / iR.x(), 0), {0,0},
				1.0f / (sample - 1), float(imageLod), depth, 0 };
			auto enc = usePipeline(*blur);
			[enc setVertexBytes:vertex_x length:sizeof(vertex_x) atIndex:blur->bufferIndex];
			[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentTexture:_outTexA atIndex:blur->fragment.image];
			[enc setFragmentSamplerState:sampler atIndex:blur->fragment.image];
			// draw blur to texture B
			[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
			depth += zDepthNextUnit;
		}
		{
			// The blur regions for y-axis
			// |r|rrrrrr|r|
			// |r| body |r|
			// |r|rrrrrr|r|
			float padding = radius + clearPad;
			x1 = begin.x() + padding;
			y1 = begin.y() + padding;
			x2 = end.x() - padding;
			y2 = end.y() - padding;
			float vertex_y[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
			auto uvOffset = -offset * _surfaceScale / iR;
			_outColorTex = mtl_get_texture_from(_state->output, _outTex);
			_blendMode = blend;
			beginPass(); // begin new pass for main render target
			MSLBlur::PcArgs pc = { iR, Vec2(oRw, oRh), Vec2(0, radius2 / iR.y()), uvOffset,
				1.0f / (sample - 1), float(imageLod), depth, 0 };
			auto enc = usePipeline(*blur);
			[enc setVertexBytes:vertex_y length:sizeof(vertex_y) atIndex:blur->bufferIndex];
			[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentTexture:_outTexB atIndex:blur->fragment.image];
			[enc setFragmentSamplerState:sampler atIndex:blur->fragment.image];
			[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
		}
	}

	void MetalCanvas::readImageCmd(const Rect &srcRect, ImageSource* src, ImageSource* dst) {
		auto w = dst->width(), h = dst->height();
		auto dstSize = Vec2(w, h);
		auto srcTex = mtl_get_texture_from(src, _outTex);
		Qk_ASSERT(srcTex, "readImageCmd source texture is null");

		TexStat storeStat;
		auto tex = mtl_rebuild_texture(_device, dstSize, dst->type(), dst->texture(0), storeStat, dst->mipmap());
		if (!tex) return;
		auto dstTex = mtl_get_texture(tex);

		endPass(); // end current pass

		id<MTLBlitCommandEncoder> blit = nil;
		if (srcRect.size == dstSize && srcTex.pixelFormat == dstTex.pixelFormat) {
			blit = [_cmdPack.current blitCommandEncoder];
			[blit copyFromTexture:srcTex
								sourceSlice:0
								sourceLevel:0
							sourceOrigin:MTLOriginMake(srcRect.begin.x(), srcRect.begin.y(), 0)
								sourceSize:MTLSizeMake(w, h, 1)
									toTexture:dstTex
					destinationSlice:0
					destinationLevel:0
					destinationOrigin:MTLOriginMake(0, 0, 0)];
		} else {
			auto &cp = _render->_shaders.cp;
			// new pass for viewport copy shader
			auto pass = [MTLRenderPassDescriptor new];
			pass.colorAttachments[0].texture = dstTex;
			pass.colorAttachments[0].loadAction = MTLLoadActionDontCare;
			pass.colorAttachments[0].storeAction = MTLStoreActionStore;
			// set pipeline state for viewport copy shader and set texture for shader
			auto enc = [_cmdPack.current renderCommandEncoderWithDescriptor:pass];
			enc.viewport = MTLViewportMake(0, 0, w, h, 0, 1);
			[enc setVertexBytes:_rootMatrix.val length:sizeof(Mat4) atIndex:1];
			enc.renderPipelineState = cp.getPipeline(_blendMode, dst->type(), 1);
			// set texture and sampler for shader
			PaintImage sampler;
			sampler.filterMode = srcRect.size == dstSize ?
					PaintImage::kNearest_FilterMode : PaintImage::kLinear_FilterMode;
			sampler.mipmapMode = PaintImage::kNone_MipmapMode;
			_render->set_texture_param(enc, srcTex, cp.fragment.image, &sampler);

			auto size = dstSize / srcRect.size, // coord size   0-1
					 begin = srcRect.begin * size; // coord begin   0-1 < size
			MSLCp::PcArgs pc{
				dstSize, // iResolution
				dstSize, // oResolution
				{ begin.x(), begin.y(), size.x(), size.y() }, // coord
				0, // imageLod
				_zDepth,
				0
			};
			float vertex[] = { 0,0,0, w,0,0, 0,h,0, w,h,0 };
			[enc setVertexBytes:vertex length:sizeof(vertex) atIndex:cp.bufferIndex];
			[enc setVertexBytes:&pc length:sizeof(pc) atIndex:0];
			[enc setFragmentBytes:&pc length:sizeof(pc) atIndex:0];
			[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
			[enc endEncoding];
		}

		if (dst->mipmap()) {
			if (!blit)
				blit = [_cmdPack.current blitCommandEncoder];
			[blit generateMipmapsForTexture:dstTex];
		}
		if (blit) {
			[blit endEncoding];
		}
		_cmdPack.recorded = true; // mark cmd pack as recorded after encoding commands

		setTex_SourceImage(dst, dst->info(), tex);
	}

	void MetalCanvas::outputImageBeginCmd(ImageSource* dst) {
		endPass(); // end pass, change outTex for next pass
		auto s = _surfaceSize; // surface size
		TexStat storeStat;
		auto tex = mtl_rebuild_texture(_device, s, dst->type(), dst->texture(0), storeStat, dst->mipmap());
		if (!tex) {
			// texture rebuild failed after current pass was ended.
			// upper layer currently does not track this failure state,
			// so subsequent rendering commands may fail due to missing
			// output render target texture.
			return;
		}
		_outColorTex = mtl_get_texture(tex); // set new output color texture for next pass
		setTex_SourceImage(dst, {int(s[0]),int(s[1]),dst->type(),dst->info().alphaType()}, tex);
	}

	void MetalCanvas::outputImageEndCmd(ImageSource* exit) {
		endPass(); // end current pass, change outTex back to canvas's own texture for next pass
		// restore output color texture for next pass
		_outColorTex = mtl_get_texture_from(_state->output, _outTex);
		if (exit->mipmap()) {
			auto tex = mtl_get_texture(exit->texture(0));
			Qk_ASSERT(tex, "outputImageEndCmd exit texture is null");
			auto blit = [_cmdPack.current blitCommandEncoder];
			[blit generateMipmapsForTexture:tex];
			[blit endEncoding];
			_cmdPack.recorded = true; // mark cmd pack as recorded after encoding commands
		}
	}

} // namespace qk
