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
// use shader and set vertex buffer for vertex data, if invalid vertex data, return and skip draw call
#define Qk_useShader(a, b, ...) auto enc = useShader(a,b,##__VA_ARGS__); if (!enc) return
// set texture for slot 0 and return encoder, if texture not ready, return nil and skip draw call
#define Qk_useTextureSlot0(paint, dstSlot, ...) bool isYuv = false; \
	auto enc = useTextureSlot0(paint, dstSlot, &isYuv); if (!enc) return __VA_ARGS__

namespace qk {
	extern const float  zDepthNextUnit;
	MTLPixelFormat mtl_pixel_format(ColorType type);
	MTLTextureID mtl_new_tex_renderbuffer(MTLDeviceID device, Vec2 size, MTLPixelFormat format, bool read, bool priv, bool mipmap);
	MTLTextureID mtl_get_texture(TexStat *stat);
	void setTex_SourceImage(ImageSource* s, cPixelInfo &i, const TexStat *tex);
	void clear_PathvCache(PathvCache *cache, int flags);
	void clearExec_PathvCache(PathvCache *cache);
	TexStat* mtl_rebuild_texture(MTLDeviceID device, Vec2 size, ColorType type, TexStat* texStat, TexStat &newStat, bool mipmap);

	void setvMatrixFromEnc(MTLRenderEncoder enc, const Matrix &mat) {
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
		, _outTex(nil), _outColorTex(nil), _outDepthTex(nil), _outAaclipTex(nil), _outTexA(nil), _outTexB(nil)
		, _enableStencilTest(false)
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
		_outAaclipTex = nil; // Aaclip buffer object of texture
		_outTexA = nil;
		_outTexB = nil;
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

	MTLPassDescriptor MetalCanvas::beginPass() {
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

		pass.stencilAttachment.texture = _outDepthTex;
		pass.stencilAttachment.loadAction = loadAction;
		pass.stencilAttachment.storeAction = MTLStoreActionStore;

		_cmdPack.pass = pass;
		return pass;
	}

	MTLPassDescriptor MetalCanvas::beginPass(int level, bool loadColor) {
		beginPass();
		if (!loadColor)
			_cmdPack.pass.colorAttachments[0].loadAction = MTLLoadActionDontCare;
		_cmdPack.pass.colorAttachments[0].level = level;
		return _cmdPack.pass;
	}

	MTLRenderEncoder MetalCanvas::getEncoder() {
		if (!_cmdPack.enc) {
			if (!_cmdPack.pass)
				beginPass();
			_cmdPack.enc = [_cmdPack.current renderCommandEncoderWithDescriptor:_cmdPack.pass];
			Qk_ASSERT(_cmdPack.enc, "Failed to create render command encoder for new pass");
			// set root matrix for new encoder
			[_cmdPack.enc setVertexBytes:_rootMatrix.val length: sizeof(Mat4) atIndex:1];
			// set view matrix for new encoder
			setvMatrixFromEnc(_cmdPack.enc, _state->matrix);
			// set aaclip texture if needed
			if (_aaclipTex) {
				[_cmdPack.enc setFragmentTexture:_aaclipTex atIndex:3];
				[_cmdPack.enc setFragmentSamplerState:_render->_aaclipSampler atIndex:3];
			}
			_cmdPack.enc.viewport = MTLViewportMake(0, 0, _surfaceSize.x(), _surfaceSize.y(), 0, 1);
		}
		return _cmdPack.enc;
	}

	inline void MetalCanvas::setPipeline(MTLRenderEncoder enc, MSLShader& shader) {
		auto pipeline = getPipeline(shader);
		if (_cmdPack.pipeline != pipeline) {
			enc.renderPipelineState = pipeline; // set pipeline state for shader
			_cmdPack.pipeline = pipeline;
		}
	}

	// useShader with vertex data ensures vertex data is valid and set for draw call,
	// if vertex data is invalid, return nil and skip draw call
	MTLRenderEncoder MetalCanvas::useShader(MTLRenderEncoder enc, MSLShader& shader, const VertexData &vertex) {
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

	// useShaderWithVertex ensures vertex data is valid and set for draw call,
	// if vertex data is invalid, return nil and skip draw call
	MTLRenderEncoder MetalCanvas::useShader(MSLShader& shader, const VertexData &vertex) {
		return useShader(getEncoder(), shader, vertex);
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
		_cmdPack.current = cmds.pop(); // get a command buffer from cmds for next pass
		_cmdPack.cmds.concat(cmds); // add remaining command buffers to cmd pack for flush
	}

	// --------------------------------------------------------------------

	void MetalCanvas::setSurfaceCmd(bool changeSize) {
		int w = _surfaceSize.x(), h = _surfaceSize.y();

		Qk_ASSERT(w, "Invalid viewport surface size width");
		Qk_ASSERT(h, "Invalid viewport surface size height");

		if (changeSize) {
			_outTex = mtl_new_tex_renderbuffer(
				_device, _surfaceSize, mtl_pixel_format(_opts.colorType), true, true, false);
			_outDepthTex = mtl_new_tex_renderbuffer(
				_device, _surfaceSize, MTLPixelFormatDepth32Float_Stencil8, false, true, false);
		}
		_outColorTex = _outTex; // set to main texture by default
		// release old aa clip texture if size changed, it will be recreated when needed
		_outAaclipTex = nil;
		_outTexA = nil; // release old
		_outTexB = nil; // release old

		Qk_DLog("setSurfaceCmd: %f, %f", w, h);

		// start a new pass with new buffers
		auto pass = beginPass();
		// clear depth and stencil to default values
		pass.depthAttachment.loadAction = MTLLoadActionClear;
		pass.depthAttachment.clearDepth = 0; // default depth value
		pass.stencilAttachment.loadAction = MTLLoadActionClear;
		pass.stencilAttachment.clearStencil = 127; // default stencil ref value
		pass.colorAttachments[0].loadAction = MTLLoadActionClear;
		pass.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 0); // clear to transparent

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

	void MetalCanvas::enableStencilTestCmd(bool enable) {
		// no need to enable stencil test for Metal, it will be set in pipeline state when encoding draw calls
		_enableStencilTest = enable;
		if (_cmdPack.enc) {
			// enc.depthStencilState = enable;
		}
	}

	void MetalCanvas::drawClipCmd(const GC_State::Clip &clip, uint32_t ref, bool revoke) {
		// TODO
	}

	void MetalCanvas::clearColorCmd(const Color4f &color, GC_ClearFlags flags) {
		if (flags) {
			auto pass = beginPass(); // start a new pass for clear
			if (flags == kClearAll_ClearFlags) {
				pass.depthAttachment.loadAction = MTLLoadActionClear;
				pass.depthAttachment.clearDepth = 0;
				pass.stencilAttachment.loadAction = MTLLoadActionClear;
				pass.stencilAttachment.clearStencil = 127;
			}
			pass.colorAttachments[0].loadAction = MTLLoadActionClear;
			pass.colorAttachments[0].clearColor = MTLClearColorMake(color.r(), color.g(), color.b(), color.a());
		} else {
			auto &shader = _render->_shaders.clear;
			auto enc = getEncoder();
			setPipeline(enc, shader); // set pipeline state for clear shader

			MSLClear::PcArgs pc{ color, _zDepth, 0 };

			// set vertex bytes
			[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
			// set fragment bytes
			[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
			// draw a full-screen triangle for clear
			[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
		}
	}

	MTLRenderEncoder MetalCanvas::useTextureSlot0(const PaintImage *paint, int dstSlot, bool* isYuv) {
		if (paint->_isCanvas) { // flush canvas to current canvas
			auto srcC = static_cast<MetalCanvas*>(paint->canvas);
			if (srcC != _canvas && srcC->isGpu()) { // now only supported gpu
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
		auto &shader = paint->_render->_shaders.image;
		// set texture for slot 0 and return encoder, if texture not ready, return nil and skip draw call
		Qk_useTextureSlot0(paint, shader.fragment.image); // slot 0 default match dst slot to 4
		if (isYuv) { // yuv420p or yuv420sp
			auto src = paint->image;
			auto &yuv = _render->_shaders.imageYuv;
			Qk_ASSERT_EQ(shader.fragment.image, yuv.fragment.image,
				"YUV shader should use the same texture slot for image as non-YUV shader");
			Qk_useShader(enc, yuv, vertex);
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
				color,
				format,
				_allScale,
				_zDepth,
				Qk_AACLIP(_state->clip.aaclip)
			};
			[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
		} else {
			Qk_useShader(enc, shader, vertex);
			// set color and other args for shader push constants
			MSLImage::PcArgs pc{
				*((Vec4*)paint->coord.begin.val),
				color,
				_allScale,
				_zDepth,
				Qk_AACLIP(_state->clip.aaclip)
			};
			[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
		}
		[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vertex.vCount];
	}

	void MetalCanvas::drawImageMaskCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color) {
		auto &shader = _render->_shaders.imageMask;
		Qk_useTextureSlot0(paint, shader.fragment.image);
		Qk_useShader(enc, shader, vertex);
		auto type = paint->_isCanvas ? kRGBA_8888_ColorType: paint->image->type();
		MSLImageMask::PcArgs pc{
			*((Vec4*)paint->coord.begin.val),
			color,
			type == kAlpha_8_ColorType ? 0 : type == kLuminance_Alpha_88_ColorType ? 1 : 3,
			_allScale,
			_zDepth,
			Qk_AACLIP(_state->clip.aaclip)
		};
		[enc setVertexBytes:&pc length:sizeof(pc) atIndex:0];
		[enc setFragmentBytes:&pc length:sizeof(pc) atIndex:0];
		[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vertex.vCount];
	}

	void MetalCanvas::drawSDFImageMaskCmd(const VertexData &vertex, const PaintImage *paint, const Color4f &color,
			const Color4f &strokeColor, float stroke) {
		auto &shader = _render->_shaders.imageSdfMask;
		Qk_useTextureSlot0(paint, shader.fragment.image);
		Qk_useShader(enc, shader, vertex);
		MSLImageSdfMask::PcArgs pc{
			*((Vec4*)paint->coord.begin.val),
			color,
			strokeColor,
			stroke,
			_allScale,
			_zDepth,
			Qk_AACLIP(_state->clip.aaclip)
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
		Qk_useShader(shader, vertex);
		MSLColorRadial::PcArgs pc{
			*((Vec4*)paint->origin.val),
			color,
			count,
			_zDepth,
			Qk_AACLIP(_state->clip.aaclip) | (count == 2 ? (1u << 1): 0)
		};
		[enc setVertexBytes:&pc length:sizeof(pc) atIndex:0];
		[enc setFragmentBytes:&pc length:sizeof(pc) atIndex:0];
		[enc setFragmentBytes:paint->colors length:sizeof(Color4f) * count atIndex:shader.fragment.Colors];
		[enc setFragmentBytes:paint->positions length:sizeof(float) * count atIndex:shader.fragment.Positions];
		[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vertex.vCount];
	}

	void MetalCanvas::drawColorCmd(const VertexData &vertex, const Color4f &color) {
		Qk_useShader(_render->_shaders.color, vertex); // use shader and set vertex buffer for vertex data
		// set color and other args for shader push constants
		MSLColor::PcArgs pc{ color, _zDepth, Qk_AACLIP(_state->clip.aaclip) };
		// set vertex bytes
		[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
		// set fragment bytes
		[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
		// draw a full-screen triangle for clear
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
		auto enc = getEncoder();
		auto& sh = _render->_shaders.colorRrectBlur;
		setPipeline(enc, sh);
		int flags = Qk_AACLIP(_state->clip.aaclip);
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
				flags,
			};
			[enc setVertexBytes:v length:sizeof(v) atIndex:sh.bufferIndex];
			[enc setVertexBytes:&pc length:sizeof(pc) atIndex:0];
			[enc setFragmentBytes:&pc length:sizeof(pc) atIndex:0];
			[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
		}
	}

	void MetalCanvas::drawRegion(const Color4f &color, const Range &region, float depth) {
		auto &shader = _render->_shaders.color;
		auto enc = getEncoder();
		setPipeline(enc, shader);
		float x1 = region.begin.x(), y1 = region.begin.y();
		float x2 = region.end.x(), y2 = region.end.y();
		float vertex[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
		MSLColor::PcArgs pc{ color, depth, Qk_AACLIP(false) };
		[enc setVertexBytes:vertex length:sizeof(vertex) atIndex:shader.bufferIndex];
		[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
		[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
		[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
	}

	void MetalCanvas::clearRegion(const Range &region, float scale, float offsetY, float depth) {
		auto origin = region.begin * scale;
		auto end = region.end * scale;
		drawRegion({0,0,0,0}, {
			{origin.x(), origin.y() + offsetY},
			{end.x(),    end.y()    + offsetY}
		}, depth);
	}

	void MetalCanvas::blurFilterBeginCmd(Range bounds, float radius, float clearPad) {
		if (!_outTexA) {
			_outTexA = mtl_new_tex_renderbuffer(
				_device, _surfaceSize, mtl_pixel_format(_opts.colorType), 1, 1, 1);
		}
		if (!_outTexB) {
			_outTexB = mtl_new_tex_renderbuffer(
				_device, _surfaceSize, mtl_pixel_format(_opts.colorType), 1, 1, 1);
		}
		auto blend = _blendMode; // save current blend mode
		auto stencilTest = _enableStencilTest;
		if (blend > kSrcOver_BlendMode)
			_blendMode = kSrc_BlendMode; // set blend mode to src for blur filter
		_enableStencilTest = false; // disabling stencil test for blur filter
		_outColorTex = _outTexA; // output to texture A for blur filter then do post processing
		// begin a new pass for blur filter with texture A as render target
		beginPass(0, false);

		/*clear pixels within bounds
			bounds already includes the blur radius; keep an extra guard band for
			scaled/mipmapped blur samples near the edge of the temporary texture.
		|.|......|.|
		|.|.body.|.|
		|.|......|.|
		*/
		drawRegion({0,0,0,0}, {
			{bounds.begin.x() - clearPad, bounds.begin.y() - clearPad},
			{bounds.end.x() + clearPad, bounds.end.y() + clearPad},
		}, depth);
	
		if (stencilTest || blend != _blendMode) {
			endPass(); // end current pass if state will be changed
		}
		_blendMode = blend; // restore blend mode
		_enableStencilTest = stencilTest; // restore stencil test state
	}

	/**
		* Fast gaussian blur reference https://www.shadertoy.com/view/ctjcWR
		* Relevant information reference:
		*   https://www.shadertoy.com/view/WtKfD3
		*   https://blog.ivank.net/fastest-gaussian-blur.html
		*   https://elynxsdk.free.fr/ext-docs/Blur/Fast_box_blur.pdf
		*   https://www.peterkovesi.com/papers/FastGaussianSmoothing.pdf
		*/
	void MetalCanvas::blurFilterEndCmd(Range bounds, float radius, float clearPad, int sample, int imageLod) {
		float x1 = bounds.begin.x() - clearPad, y1 = bounds.begin.y() - clearPad;
		float x2 = bounds.end.x() + clearPad, y2 = bounds.end.y() + clearPad;
		float fullRadius = radius * _surfaceScale;
		Vec2 R = _surfaceSize; // viewport resolution of the vport_cp shader
		int oRw = R.x(), oRh = R.y();

		auto &cp = _render->_shaders.vportCp;
		auto blend = _blendMode; // save current blend mode
		auto stencilTest = _enableStencilTest;
		if (blend > kSrcOver_BlendMode)
			_blendMode = kSrc_BlendMode;
		_enableStencilTest = false;
		// get sampler state for paint image
		auto sampler = _render->get_sampler(PaintImage::kNearest_FilterMode,
			PaintImage::kLinearNearest_MipmapMode);

		int level = 0;
		while (level < imageLod && oRw && oRh) { // copy image, gen mipmap texture
			/* Copy more the x-axis regions, but ignore the y-axis blurred regions
			|.|......|.|
			|.|.body.|.|
			|.|......|.|
			*/
			float scale = float(oRw >> 1) / oRw; // ≈ 0.5 for each level
			x1 *= scale; y1 *= scale; x2 *= scale; y2 *= scale;
			oRw >>= 1; oRh >>= 1;
			float vertex[] = {
				x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0
			};
			MSLVportCp::PcArgs pc{
				Vec2(oRw, oRh), // iResolution
				Vec2(oRw, oRh), // oResolution
				{ 0, 0, 1, 1 }, // coord scale coefficient
				level++, // imageLod
				depth,
				0
			};
			beginPass(level, false); // begin new pass for next level
			auto enc = getEncoder();
			setPipeline(enc, cp);
			[enc setVertexBytes:vertex length:sizeof(vertex) atIndex:cp.bufferIndex];
			[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentTexture:_outTexA atIndex:cp.fragment.image];
			[enc setFragmentSamplerState:sampler atIndex:cp.fragment.sampler];
			[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
			depth += zDepthNextUnit;
		}

		_outColorTex = _outTexB; // output to texture B
		// begin new pass for blur filter with texture B as render target
		beginPass(level, false);

		// Choosing the right blur shader
		auto blur = &_render->_shaders.blur;
		/*switch (n) {
			case 3: blur += 1; break; // blur3
			case 7: blur += 2; break; // blur7
			case 13: blur += 3; break; // blur13
			case 19: blur += 4; break; // blur19
		}*/

		/* The x-axis regions
		|.|......|.|
		|.|.body.|.|
		|.|......|.|
		*/
		float vertex_x[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
		/* The y-axis regions
		|/|/|//////|/|/|
		|/|.|......|.|/|
		|/|.| body |.|/|
		|/|.|......|.|/|
		|/|/|//////|/|/|
		*/
		float y1_ = y1 - radius, y2_ = y2 + radius;
		float vertex_y[] = { x1,y1_,0, x2,y1_,0, x1,y2_,0, x2,y2_,0 };
		float oiScale = oRw / R.x(); // oResolution / iResolution
		float offsetY = (R.y() - oRh) / _surfaceScale;
		/* First clean the y-axis of buffer B
		|.|.|......|.|.|
		|.|.|......|.|.|
		|.|/|//////|/|.|
		|.|.|......|.|.|
		|.|.|......|.|.|
		*/
		//	oRw, oRh, R.x(), R.y(), offsetY, oiScale);
		//glClearBufferfv(GL_COLOR, 0, emptyColor);
		y1_-=radius; y2_+=radius;
		// clearRegion({{x1, y1_}, {x2, y1}}, oiScale, offsetY, depth); // clear top
		// clearRegion({{x1, y2}, {x2, y2_}}, oiScale, offsetY, depth); // clear bottom
		// clearRegion({{x1-3, y1_}, {x1, y2_}}, oiScale, offsetY, depth); // clear left
		// clearRegion({{x2, y1_}, {x2+3, y2_}}, oiScale, offsetY, depth); // clear right
		// Making blur of the x-axis direction
		blur->use(sizeof(float) * 12, vertex_x);
		glUniform1f(blur->pc_depth, depth);
		glUniform2f(blur->pc_iResolution, R.x(), R.y());
		glUniform2f(blur->pc_oResolution, oRw, oRh);
		glUniform1f(blur->pc_imageLod, imageLod);
		glUniform1f(blur->pc_detail, 1.0f/(sample-1));
		glUniform2f(blur->pc_size, fullRadius / R.x(), 0); // horizontal blur
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw blur

		// if (clipState) {
		// 	glEnable(GL_STENCIL_TEST); // recover clip state
		// }
		// _render->set_blend_mode(backMode); // restore blend mode

		// recover output target
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			recover ? recover->texture(0)->id(): _outTex, 0
		);

		glBindTexture(GL_TEXTURE_2D, _outTexB);

		// Making blur of the y-axis direction
		blur->use(sizeof(float) * 12, vertex_y);
		glUniform1f(blur->pc_depth, depth + zDepthNextUnit);
		glUniform2f(blur->pc_oResolution, R.x(), R.y());
		glUniform2f(blur->pc_size, 0, fullRadius / R.y()); // vertical blur
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw blur to main render buffer
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
			Qk_AACLIP(_state->clip.aaclip) | (triangles.isDarkColor ? (1u << 1): 0)
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

	void MetalCanvas::readImageCmd(const Rect &srcRect, ImageSource* src, ImageSource* dst) {
		auto w = dst->width(), h = dst->height();
		auto dstSize = Vec2(w, h);
		auto srcTex = src ? mtl_get_texture(src->texture(0)) : _outTex;
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
			auto &cp = _render->_shaders.vportCp;
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
			MSLVportCp::PcArgs pc{
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
		auto s = _canvas->_surfaceSize; // surface size
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
		_outColorTex = _state->output ? mtl_get_texture(_state->output->texture(0)) : _outTex;
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
