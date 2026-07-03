/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#import "./mtl_canvas.h"
#include "src/render/render.h"
#import "./mtl_render.h"
#import "../source.h"
#import "../pixel.h"

// use pipeline and set vertex buffer for vertex data, if invalid vertex data, return and skip draw call
#define Qk_usePipeline(shader, ...) auto enc = usePipeline(shader,##__VA_ARGS__); if (!enc) return
// set texture for slot 0 and return encoder, if texture not ready, return nil and skip draw call
#define Qk_useTexture0(paint, dstSlot, ...) bool isYuv = false; \
	auto enc = useTexture0(paint, dstSlot, &isYuv); if (!enc) return __VA_ARGS__

namespace qk {
	MTLPixelFormat mtl_pixel_format(ColorType type);
	MTLTextureID mtl_new_texture(MTLDeviceID device, Vec2 size, MTLPixelFormat format, uint8_t flags);
	cTexStat* mtl_rebuild_texture(MTLDeviceID device, Vec2 size, ColorType type, cTexStat* texStat, TexStat &newStat, uint8_t flags);
	MTLTextureID mtl_get_texture(cTexStat *stat);
	MTLTextureID mtl_get_texture_from(const ImageSource* src, MTLTextureID _else = nil);
	void setTex_SourceImage(ImageSource* s, cPixelInfo &i, cTexStat *tex);
	void setvMatrixFromEnc(MTLEncoder enc, const Mat &mat);

	const MemBlockAllocator<MTLBufferID>::MemBlock& makeBuffer(MTL_CmdPack &cmd, const void *bytes, uint32_t length) {
		auto &block = cmd.buffer->alloc(length);
		Qk_ASSERT(block.end >= block.begin + length, "Not enough space in buffer block for CGAA data");
		memcpy((char*)block.val.contents + block.begin, bytes, length);
		return block;
	};

	// --------------------------------------------------------------------

	void MetalCanvas::setSurface(const Mat4& root, Vec2 surfaceSize, Vec2 scale) {
		// Convert Qk/GL surface clip convention to Metal:
		// - Y: Qk/GL path is bottom-left after projection, Metal framebuffer is top-left.
		// - Z: GL NDC depth is [-1, 1], Metal depth is [0, 1].
		_rootMatrix = root;
		_rootMatrix.scale_y(-1);
		_rootMatrix.translate_y(-surfaceSize.y() / scale.y());
		// translate and scale z to map depth from [-1, 1] to [0, 1]
		_rootMatrix.translate_z(0.5f);
		_rootMatrix.scale_z(0.5);
		GPUCanvas::setSurface(_rootMatrix, surfaceSize, scale);
	}

	void MetalCanvas::setSurfaceCmd(bool changeSize) {
		if (changeSize) {
			_outTex = mtl_new_texture(
				_device, _surfaceSize, mtl_pixel_format(_opts.colorType), 0);
		}
		_outColorTex = _outTex; // set to main texture by default

		endPass(); // end old pass if exist
		// start a new pass with new buffers
		auto pass = beginPass();
		pass.colorAttachments[0].loadAction = MTLLoadActionClear;
		pass.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 0); // clear to transparent

		// clear buffer allocators for new frame
		_cmdPack.buffer->clear();
		_cmdPackFront.buffer->clear();
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

	void MetalCanvas::drawClipCmd(const VertexData &vertex, GC_State::Clip *last, GC_State::Clip *clip, ClipOp rawOp) {
		auto begin = clip->range.begin,
				 end = clip->range.end, size = end - begin;
		auto blend = _blendMode; // save current blend mode
		auto colorTex = _outColorTex; // save current color texture
		// switch blend mode to src
		_blendMode = kSrc_BlendMode;
		// output to clip mask texture
		_outColorTex = mtl_get_texture_from(*clip->mask);

		endPass(); // end current pass

		auto drawClipMask = [&](bool black, bool clip) {
			auto scale = Vec2(1) / _surfaceScale;
			Vec4 surface = {-begin.x(), -begin.y(), scale.x(), scale.y()};
			// Difference clip cannot directly render solid black with AA,
			// otherwise edge blending becomes incorrect.
			// Instead, invert the AA coverage curve.
			// This produces a smooth subtractive mask edge.
			int flags = black ? Qk_FLAG_AASIDE_Inverted : 0; // Qk_FLAG_AASIDE_Inverted
			flags |= Qk_CLIP(clip); // set clip flag if have clip
			drawColor(vertex, Color4f(1,1,1,1), surface, flags);
		};
		if (rawOp == Canvas::kIntersect_ClipOp || !last) {
			// clear clipTex with black color
			clearColor({0,0,0,0}, nullptr);
			// draw clip shape to clipTex with white color
			drawClipMask(false, last);
		} else { // if (rawOp == Canvas::kDifference_ClipOp)
			beginPass(0, false); // begin a new pass with don't load color
			// copy last clip color to clipTex as the clear color
			copyImage(last->mask.get(), begin - last->range.begin, {0,size}, size);
			// draw clip shape to clipTex with white color if last op equal difference,
			// or black color if last op equal intersect
			auto black = last->op == Canvas::kIntersect_ClipOp;
			// draw clip shape to clipTex with color
			drawClipMask(black, true);
		}
		endPass();
		// restore framebuffer and blend mode
		_blendMode = blend;
		_outColorTex = colorTex;
		restoreClipCmd(clip); // set clip data
	}

	void MetalCanvas::restoreClipCmd(GC_State::Clip* clip) {
		if (_cmdPack.enc == nil)
			return;
		if (clip) {
			MSLColor::ClipStatBlock clipStat = { *((Vec4*)clip->range.begin.val), clip->op };
			[_cmdPack.enc setFragmentBytes:&clipStat length:sizeof(clipStat) atIndex:3];
			[_cmdPack.enc setFragmentTexture:mtl_get_texture_from(*clip->mask) atIndex:0];
			[_cmdPack.enc setFragmentSamplerState:_mtlrender->_nearestSampler atIndex:0];
		} else {
			[_cmdPack.enc setFragmentTexture:nil atIndex:0];
		}
	}

	void MetalCanvas::copyImage(ImageSource *src, Vec2 srcOffset, Range dst, Vec2 resolution) {
		float x1 = dst.begin.x(), y1 = dst.begin.y();
		float x2 = dst.end.x(), y2 = dst.end.y();
		float vertex[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0, };
		auto &cp = _shaders.cp;
		auto scale = resolution / src->size();
		auto offset = (srcOffset - dst.begin) / src->size();
		auto coord = Vec4(offset.x(), offset.y(), scale.x(), scale.y());
		auto enc = usePipeline(cp);
		MSLCp::PcArgs pc{ resolution, resolution, coord, 0, 0 };
		[enc setVertexBytes:vertex length:sizeof(vertex) atIndex:cp.bufferIndex];
		[enc setVertexBytes:&pc length:sizeof(pc) atIndex:0];
		[enc setFragmentBytes:&pc length:sizeof(pc) atIndex:0];
		[enc setFragmentTexture:mtl_get_texture_from(src) atIndex:cp.fragment.image];
		[enc setFragmentSamplerState:_mtlrender->_nearestSampler atIndex:cp.fragment.image];
		[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
	}

	void MetalCanvas::drawColor(const VertexData &vertex, const Color4f &color, Vec4 offset, uint32_t flags) {
		Qk_usePipeline(_shaders.color, vertex); // use shader and set vertex buffer for vertex data
		// set color and other args for shader push constants
		MSLColor::PcArgs pc{ 0,0, color, offset, flags };
		[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
		[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
		[enc setVertexBuffer:_mtlrender->_emptyBuffer offset:0 atIndex:_shaders.color.vertex.paths];
		[enc setVertexBuffer:_mtlrender->_emptyBuffer offset:0 atIndex:_shaders.color.vertex.tiles];
		// draw a full-screen triangle for clear
		[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vertex.vCount];
	}

	void MetalCanvas::clearColor(const Color4f &color, const Range *surfaceRange) {
		if (!surfaceRange) {
			// clear color by load action if no encoder and clear full surface,
			// which is more efficient than drawing a rect
			auto pass = beginPass();
			pass.colorAttachments[0].loadAction = MTLLoadActionClear;
			pass.colorAttachments[0].clearColor = MTLClearColorMake(color.r(), color.g(), color.b(), color.a());
		} else {
			// clear color by drawing a rect
			Range fullScreen{{0,0}, _surfaceSize};
			surfaceRange = surfaceRange ? surfaceRange : &fullScreen;
			auto begin = surfaceRange->begin.floor();
			auto end = surfaceRange->end.ceil();
			float x1 = begin.x(), y1 = begin.y(), x2 = end.x(), y2 = end.y();
			float vertex[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
			auto &clear = _shaders.clear;
			auto enc = usePipeline(clear); // use pipeline state for clear shader
			// set color and other args for shader push constants
			MSLClear::PcArgs pc{ color, 0 };
			[enc setVertexBytes:vertex length:sizeof(vertex) atIndex:clear.bufferIndex];
			[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
			[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
		}
	}

	void MetalCanvas::drawColorCmd(const VertexData &vertex, const Color4f &color) {
		drawColor(vertex, premul_alpha(color), Vec4(0), _flags);
	}

	void MetalCanvas::makeCGAAAtlasCmd(cCGAADrawData &data) {
		auto tileCount = data.tiles.length();
		if (!tileCount)
			return;
		endPass(); // end current pass

		Qk_ASSERT(data.atlas, "CGAA atlas texture is null");

		auto edges = makeBuffer(_cmdPack, data.edges.val(), data.edges.size());
		auto tileEdges = makeBuffer(_cmdPack, data.tileEdges.val(), data.tileEdges.size());
		auto tiles = makeBuffer(_cmdPack, data.tiles.val(), data.tiles.size());

		auto &shader = _shaders.cgaa;
		MSLCgaa::PcArgs pc{
			.atlasTileCountX=uint32_t(data.atlas->width() >> kCGAATileSizeShift),
			.atlasTileCountY=uint32_t(data.atlas->height() >> kCGAATileSizeShift),
			.flags=_flags,
		};
		auto enc = [_cmdPack.current computeCommandEncoder];
		enc.label = @"CGAA atlas Coverage";
		[enc setComputePipelineState:shader.getComputePipeline()];
		[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
		[enc setBuffer:edges.val offset:edges.begin atIndex:shader.compute.edges];
		[enc setBuffer:tileEdges.val offset:tileEdges.begin atIndex:shader.compute.tileEdges];
		[enc setBuffer:tiles.val offset:tiles.begin atIndex:shader.compute.tiles];
		[enc setTexture:mtl_get_texture_from(data.atlas.get()) atIndex:shader.compute.atlasTex];

		[enc dispatchThreadgroups:MTLSizeMake(tileCount, 1, 1)
			threadsPerThreadgroup:MTLSizeMake(kCGAATileSize * kCGAASampleGrid, 1, 1)];
		[enc endEncoding];
		_cmdPack.recorded = true;
	}

	void MetalCanvas::drawCGAAColorCmd(cCGAADrawData &data) {
		auto tileCount = data.compositeTiles.length();
		if (!tileCount)
			return;
		Qk_ASSERT(data.atlas, "CGAA atlas texture is null");

		auto &shader = _shaders.color;
		auto enc = usePipeline(shader);
		enc.label = @"CGAA Color";

		[enc setFragmentTexture:mtl_get_texture_from(data.atlas.get()) atIndex:shader.fragment.atlasTex];
		[enc setFragmentSamplerState:_mtlrender->_nearestSampler atIndex:shader.fragment.atlasTex];

		auto paths = makeBuffer(_cmdPack, data.paths.val(), data.paths.size());
		auto tiles = makeBuffer(_cmdPack, data.compositeTiles.val(), data.compositeTiles.size());
		MSLColor::PcArgs pc{
			.cgaaAtlasTiles=int32_t(data.atlas->width() >> kCGAATileSizeShift),
			.flags=_flags | Qk_FLAG_CGAA,
		};
		[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
		[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
		[enc setVertexBuffer:_mtlrender->_emptyBuffer offset:0 atIndex:shader.bufferIndex];
		[enc setVertexBuffer:paths.val offset:paths.begin atIndex:shader.vertex.paths];
		[enc setVertexBuffer:tiles.val offset:tiles.begin atIndex:shader.vertex.tiles];
		[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip
						vertexStart:0
						vertexCount:4
					instanceCount:tileCount];
	}

	const MemBlockAllocator<MTLBufferID>::MemBlock&
	MetalCanvas::buildGradientBuffer(const PaintGradient *paint, const Color4f &color) {
		int count = Qk_Min(64, paint->count);
		Array<Color4f> colors(count);
		for (int i = 0; i < count; i++) {
			colors[i] = premul_alpha(paint->colors[i]);
		}
		// align color and position data to 16 bytes for std140 packing rules
		auto colorSize = alignUp(sizeof(Color4f) * count, 16); // align to 16 bytes
		auto pointSize = alignUp(sizeof(float) * count, 16); // align to 16 bytes

		// allocate a buffer for gradient colors and positions, and copy data to the buffer
		auto &block = _cmdPack.buffer->alloc(
			colorSize + pointSize, sizeof(MSLColorGradient::Colors) + sizeof(MSLColorGradient::Positions)
		);
		// copy colors and positions to the buffer, colors first then positions, and set them to fragment shader
		auto buff = (char*)block.val.contents + block.begin;
		memcpy(reinterpret_cast<Color4f*>(buff), colors.val(), colorSize);
		memcpy(reinterpret_cast<float*>((uint8_t*)buff + colorSize), paint->positions, pointSize);
		return block;
	}

	void MetalCanvas::drawCGAAGradientCmd(cCGAADrawData &data, const PaintGradient *paint, const Color4f &color) {
		auto tileCount = data.compositeTiles.length();
		if (!tileCount)
			return;
		Qk_ASSERT(data.atlas, "CGAA atlas texture is null");

		int count = Qk_Min(64, paint->count);
		auto &shader = _shaders.colorGradient;
		auto enc = usePipeline(shader);
		enc.label = @"CGAA Gradient";

		[enc setFragmentTexture:mtl_get_texture_from(data.atlas.get()) atIndex:shader.fragment.atlasTex];
		[enc setFragmentSamplerState:_mtlrender->_nearestSampler atIndex:shader.fragment.atlasTex];

		auto paths = makeBuffer(_cmdPack, data.paths.val(), data.paths.size());
		auto tiles = makeBuffer(_cmdPack, data.compositeTiles.val(), data.compositeTiles.size());
		MSLColorGradient::PcArgs pc{
			.cgaaAtlasTiles=int32_t(data.atlas->width() >> kCGAATileSizeShift),
			.range=*((Vec4*)paint->origin.val),
			.color=premul_alpha(color),
			.count=count,
			.flags = _flags | Qk_FLAG_CGAA |
				(count == 2 ? Qk_FLAG_GRADIENT_COUNT2: 0) |
				(paint->type == PaintGradient::kRadial_Type ? Qk_FLAG_RADIAL_GRADIENT: 0),
		};
		auto colorSize = alignUp(sizeof(Color4f) * count, 16);
		auto &block = buildGradientBuffer(paint, color);

		[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
		[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
		[enc setVertexBuffer:_mtlrender->_emptyBuffer offset:0 atIndex:shader.bufferIndex];
		[enc setVertexBuffer:paths.val offset:paths.begin atIndex:shader.vertex.paths];
		[enc setVertexBuffer:tiles.val offset:tiles.begin atIndex:shader.vertex.tiles];
		[enc setFragmentBuffer:block.val offset:block.begin atIndex:shader.fragment.colors];
		[enc setFragmentBuffer:block.val offset:block.begin + colorSize atIndex:shader.fragment.positions];
		[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip
						vertexStart:0
						vertexCount:4
					instanceCount:tileCount];
	}

	void MetalCanvas::drawCGAAImageCmd(cCGAADrawData &data, const GC_ImageDrawInfo &info) {
		auto tileCount = data.compositeTiles.length();
		if (!tileCount)
			return;
		Qk_ASSERT(data.atlas, "CGAA atlas texture is null");

		auto &shader = _shaders.image;
		Qk_useTexture0(info.paint, shader.fragment.image);
		setPipeline(enc, shader);
		enc.label = @"CGAA Image";

		[enc setFragmentTexture:mtl_get_texture_from(data.atlas.get()) atIndex:shader.fragment.atlasTex];
		[enc setFragmentSamplerState:_mtlrender->_nearestSampler atIndex:shader.fragment.atlasTex];

		auto paths = makeBuffer(_cmdPack, data.paths.val(), data.paths.size());
		auto tiles = makeBuffer(_cmdPack, data.compositeTiles.val(), data.compositeTiles.size());
		auto type = info.paint->_isCanvas ? kRGBA_8888_ColorType: info.paint->image->type();
		MSLImage::PcArgs pc{
			.cgaaAtlasTiles=int32_t(data.atlas->width() >> kCGAATileSizeShift),
			.texCoords=*((Vec4*)info.paint->coord.begin.val),
			.color=premul_alpha(info.color),
			.strokeColor=premul_alpha(info.stroke <= 0 ? info.color: info.strokeColor),
			.strokeWidth=info.stroke,
			.alphaIndex=info.kind == kMask_DrawKind ?
				(type == kAlpha_8_ColorType ? 0 : type == kLuminance_Alpha_88_ColorType ? 1 : 3): 0,
			.flags = _flags | Qk_FLAG_CGAA |
				(info.kind == kMask_DrawKind ? Qk_FLAG_IMAGE_MASK: 0) |
				(info.kind == kSDFMask_DrawKind ? Qk_FLAG_IMAGE_SDF_MASK: 0),
		};
		[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
		[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
		[enc setVertexBuffer:_mtlrender->_emptyBuffer offset:0 atIndex:shader.bufferIndex];
		[enc setVertexBuffer:paths.val offset:paths.begin atIndex:shader.vertex.paths];
		[enc setVertexBuffer:tiles.val offset:tiles.begin atIndex:shader.vertex.tiles];
		[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip
						vertexStart:0
						vertexCount:4
					instanceCount:tileCount];
	}

	void MetalCanvas::clearColorCmd(const Color4f &color, GC_ClearFlags flags) {
		endPass(); // end current pass if exist
		auto pass = beginPass();
		pass.colorAttachments[0].loadAction = MTLLoadActionClear;
		pass.colorAttachments[0].clearColor = MTLClearColorMake(color.r(), color.g(), color.b(), color.a());
	}

	MTLEncoder MetalCanvas::useTexture0(const PaintImage *paint, int dstSlot, bool* isYuv) {
		if (paint->_isCanvas) { // flush canvas to current canvas
			auto srcC = static_cast<MetalCanvas*>(paint->canvas);
			if (srcC == this || !srcC->isGpu())
				return nil; // if the source canvas is the same as current canvas or not gpu, skip
			if (srcC->render() != _mtlrender)
				return nil; // if the source canvas is not from the same render, skip
			if (srcC->_outTex == nil)
				return nil; // if the source canvas has no output texture, skip
			flushSubcanvasCmd(srcC); // flush subcanvas to current canvas
			auto enc = getEncoder();
			set_texture_param(enc, srcC->_outTex, dstSlot, paint);
			return enc;
		} else {
			auto enc = getEncoder();
			if (kYUV420P_Y_8_ColorType == paint->image->type()) { // yuv420p or yuv420sp
				if (isYuv) *isYuv = true;
			}
			if (use_texture(enc, paint->image, 0, dstSlot, paint))
				return enc;
			return nil;
		}
	}

	void MetalCanvas::drawImageCmd(const VertexData &vertex, const GC_ImageDrawInfo &info) {
		auto &shader = _shaders.image;
		// set texture for slot 0 and return encoder, if texture not ready, return nil and skip draw call
		Qk_useTexture0(info.paint, shader.fragment.image); // slot 0 default match dst slot to 1
		if (info.kind == kImage_DrawKind && isYuv) { // yuv420p or yuv420sp
			auto &yuv = _shaders.imageYuv;
			enc = usePipeline(yuv, vertex, enc);
			if (!enc) return;
			auto src = info.paint->image;
			Qk_ASSERT_EQ(true, use_texture(enc, src, 0, yuv.fragment.image, info.paint)); // y
			Qk_ASSERT_EQ(true, use_texture(enc, src, 1, yuv.fragment.image_uv, info.paint)); // u or uv
			int format = 0; // default to yuv420sp
			if (src->pixel(1)->type() == kYUV420P_U_8_ColorType) {
				Qk_ASSERT_EQ(true, use_texture(enc, src, 2, yuv.fragment.image_v, info.paint)); // v
				format = 1; // yuv420p
			}
			MSLImageYuv::PcArgs pc{
				.texCoords=*((Vec4*)info.paint->coord.begin.val),
				.color=premul_alpha(info.color),
				.format=format,
				.flags=_flags
			};
			[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
		} else {
			enc = usePipeline(shader, vertex, enc);
			if (!enc) return;
			auto type = info.paint->_isCanvas ? kRGBA_8888_ColorType: info.paint->image->type();
			// set atlas texture for fragment shader,
			// because resources cannot be empty, although not used in non-cgaa draw,
			// so set a texture for non-cgaa draw to avoid error.
			// Qk_ASSERT_EQ(true, use_texture(enc, src, 0, shader.fragment.atlasTex, info.paint));
			// set color and other args for shader push constants
			MSLImage::PcArgs pc{
				.texCoords=*((Vec4*)info.paint->coord.begin.val),
				.color=premul_alpha(info.color),
				.strokeColor=premul_alpha(info.stroke <= 0 ? info.color: info.strokeColor),
				.strokeWidth=info.stroke,
				.alphaIndex=info.kind == kMask_DrawKind ?
					(type == kAlpha_8_ColorType ? 0 : type == kLuminance_Alpha_88_ColorType ? 1 : 3): 0,
				.flags = _flags |
					(info.kind == kMask_DrawKind ? Qk_FLAG_IMAGE_MASK: 0) |
					(info.kind == kSDFMask_DrawKind ? Qk_FLAG_IMAGE_SDF_MASK: 0),
			};
			[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setVertexBuffer:_mtlrender->_emptyBuffer offset:0 atIndex:shader.vertex.paths];
			[enc setVertexBuffer:_mtlrender->_emptyBuffer offset:0 atIndex:shader.vertex.tiles];
			[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
		}
		[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vertex.vCount];
	}

	void MetalCanvas::drawGradientCmd(const VertexData &vertex, const PaintGradient *paint, const Color4f &color) {
		int count = Qk_Min(64, paint->count);
		auto &shader = _shaders.colorGradient;
		Qk_usePipeline(shader, vertex);
		MSLColorGradient::PcArgs pc{
			{0},
			0,
			.range=*((Vec4*)paint->origin.val),
			.color=premul_alpha(color),
			.count=count,
			.flags=_flags |
				(count == 2 ? Qk_FLAG_GRADIENT_COUNT2: 0) |
				(paint->type == PaintGradient::kRadial_Type ? Qk_FLAG_RADIAL_GRADIENT: 0)
		};
		auto colorSize = alignUp(sizeof(Color4f) * count, 16);
		auto &block = buildGradientBuffer(paint, color);
		[enc setVertexBytes:&pc length:sizeof(pc) atIndex:0];
		[enc setFragmentBytes:&pc length:sizeof(pc) atIndex:0];
		[enc setVertexBuffer:_mtlrender->_emptyBuffer offset:0 atIndex:_shaders.color.vertex.paths];
		[enc setVertexBuffer:_mtlrender->_emptyBuffer offset:0 atIndex:_shaders.color.vertex.tiles];
		[enc setFragmentBuffer:block.val offset:block.begin atIndex:shader.fragment.colors];
		[enc setFragmentBuffer:block.val offset:block.begin + colorSize atIndex:shader.fragment.positions];
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
		auto& sh = _shaders.colorRrectBlur;
		auto enc = usePipeline(sh);
		float s_inv = 1.0f / blur;

		for (int i = 0; i < 4; i++) {
			auto horn = horns[i];
			float v[] = { c[0],c[1],horn[0],c[1],horn[0],horn[1],c[0],horn[1] };
			float r0 = F32::min(Vec2(radius[i], s1).length(), rmax);
			float r1 = F32::min(Vec2(radius[i], s2).length(), rmax);
			float n = 2.0 * r1 / r0;
			MSLColorRrectBlur::PcArgs pc{
				horn,
				color,
				{ Vec3(r1, n, 1.0 / n), 0 },
				min_edge,
				s_inv,
				_flags,
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
		auto &shader = _shaders.triangles;
		Qk_useTexture0(paint, shader.fragment.image);
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
			_flags | (triangles.isDarkColor ? Qk_FLAGS_DARK_COLOR : 0)
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
		_rootMatrix = blurRootMat;
		_outColorTex = outTexA; // output to texture A for blur filter then do post processing
		clearColor({0,0,0,0}, nullptr);
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
		if (_cmdPack.enc == nil)
			return; // if no drawing command recorded for blur filter, skip post processing
		auto texA = mtl_get_texture_from(tmpA);
		auto texB = mtl_get_texture_from(tmpB);
		Qk_ASSERT(texA && texB, "blurFilterEndCmd temp texture is null");
		auto offset = bounds.begin.max(0);
		auto begin = bounds.begin, end = bounds.end;
		float x1 = begin.x() - offset.x(), y1 = begin.y() - offset.y(),
					x2 = end.x() - offset.x(), y2 = end.y() - offset.y();
		float radius2 = radius * _surfaceScaleAverage; // radius in pixel unit
		Vec2 iR = tmpA->size(); // input resolution
		int oRw = iR.x(), oRh = iR.y();

		auto blend = _blendMode; // save current blend mode
		_blendMode = kSrc_BlendMode;
		_rootMatrix = recoverRootMat; // recover root matrix
		// get sampler state for paint image
		auto sampler = _mtlrender->_linearSampler;
		auto &cp = _shaders.cp;
		// Choosing the right blur shader
		auto blur = &_shaders.blur;

		if (imageLod) {
			if (oRw >> imageLod == 0 || oRh >> imageLod == 0) {
				_outColorTex = mtl_get_texture_from(*_state->output, _outTex);
				_blendMode = blend;
				return endPass(); // end pass
			}
			// |r|r|rrrrrr|r|r|
			// |r|r|rrrrrr|r|r|
			// |r|r| body |r|r|
			// |r|r|rrrrrr|r|r|
			// |r|r|rrrrrr|r|r|
			int level = 0;
			float vertex[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
			do { // Copy the image to smaller texture for next level
				oRw >>= 1; oRh >>= 1;
				MSLCp::PcArgs pc{ iR, Vec2(oRw, oRh), { 0, 0, 1, 1 }, float(level++), 0};
				beginPass(level, false); // begin new pass for next level
				auto enc = usePipeline(cp);
				[enc setVertexBytes:vertex length:sizeof(vertex) atIndex:cp.bufferIndex];
				[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
				[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
				[enc setFragmentTexture:texA atIndex:cp.fragment.image];
				[enc setFragmentSamplerState:sampler atIndex:cp.fragment.image];
				[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
			} while (level < imageLod);
		}
		{
			// The blur regions for x-axis
			// |r|rrrrrr|r|
			// |r|rrrrrr|r|
			// |r| body |r|
			// |r|rrrrrr|r|
			// |r|rrrrrr|r|
			_outColorTex = texB; // output to texture B
			beginPass(imageLod, false); // begin new pass for blur x-axis
			// Making blur of the x-axis direction
			float vertex[] = { x1+radius,y1,0, x2-radius,y1,0, x1+radius,y2,0, x2-radius,y2,0 };
			MSLBlur::PcArgs pc{ iR, Vec2(oRw, oRh), Vec2(radius2 / iR.x(), 0), {0,0},
				1.0f/(sample-1), float(imageLod), 0 };
			auto enc = usePipeline(*blur);
			[enc setVertexBytes:vertex length:sizeof(vertex) atIndex:blur->bufferIndex];
			[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentTexture:texA atIndex:blur->fragment.image];
			[enc setFragmentSamplerState:sampler atIndex:blur->fragment.image];
			// draw blur to texture B
			[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
		}
		{
			// The blur regions for y-axis
			// |r|rrrrrr|r|
			// |r| body |r|
			// |r|rrrrrr|r|
			float padding = radius + clearPad;
			x1 = begin.x() + padding, y1 = begin.y() + padding;
			x2 = end.x() - padding, y2 = end.y() - padding;
			float vertex[] = { x1,y1,0, x2,y1,0, x1,y2,0, x2,y2,0 };
			auto uv_offset = -offset * _surfaceScale / iR;
			_outColorTex = mtl_get_texture_from(*_state->output, _outTex);
			_blendMode = blend;
			beginPass(); // begin new pass for main render target
			MSLBlur::PcArgs pc = { iR, iR, Vec2(0, radius2 / iR.y()), uv_offset,
				1.0f/(sample-1), float(imageLod), 0 };
			auto enc = usePipeline(*blur);
			[enc setVertexBytes:vertex length:sizeof(vertex) atIndex:blur->bufferIndex];
			[enc setVertexBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentBytes:&pc length: sizeof(pc) atIndex:0];
			[enc setFragmentTexture:texB atIndex:blur->fragment.image];
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
		uint8_t flags = dst->mipmap() ? kMipmap_TextureFlags: 0;
		auto texStat = mtl_rebuild_texture(_device, dstSize, dst->type(), dst->texture(0), storeStat, flags);
		if (!texStat)
			return;
		auto tex = mtl_get_texture(texStat);
		endPass(); // end current pass

		id<MTLBlitCommandEncoder> blitEnc = nil;
		if (_blendMode == kSrc_BlendMode &&
				srcRect.size == dstSize &&
				srcTex.pixelFormat == tex.pixelFormat
		) {
			blitEnc = [_cmdPack.current blitCommandEncoder];
			[blitEnc copyFromTexture:srcTex
									sourceSlice:0
									sourceLevel:0
								sourceOrigin:MTLOriginMake(srcRect.begin.x(), srcRect.begin.y(), 0)
									sourceSize:MTLSizeMake(w, h, 1)
										toTexture:tex
						destinationSlice:0
						destinationLevel:0
						destinationOrigin:MTLOriginMake(0, 0, 0)];
			_cmdPack.recorded = true; // mark cmd pack as recorded after encoding commands
		} else {
			auto sampler = srcRect.size == dstSize ? _mtlrender->_nearestSampler : _mtlrender->_linearSampler;
			auto colorTex = _outColorTex;
			_outColorTex = tex;
			// load raw color if need to blend with existing color
			beginPass(0, _blendMode > kSrc_BlendMode ? true: false);
			auto &cp = _shaders.cp;
			auto enc = usePipeline(cp);
			float x2 = _size[0], y2 = _size[1]; // canvas size
			float vertex[] = { 0,0,0, x2,0,0, 0,y2,0, x2,y2,0 };
			auto begin = srcRect.begin / _surfaceSize;
			auto scale = srcRect.size / _surfaceSize;
			auto coord = Vec4(begin.x(), begin.y(), scale.x(), scale.y());
			MSLCp::PcArgs pc{ _surfaceSize, dstSize, coord, 0, 0 };
			[enc setVertexBytes:vertex length:sizeof(vertex) atIndex:cp.bufferIndex];
			[enc setVertexBytes:&pc length:sizeof(pc) atIndex:0];
			[enc setFragmentBytes:&pc length:sizeof(pc) atIndex:0];
			[enc setFragmentTexture:srcTex atIndex:cp.fragment.image];
			[enc setFragmentSamplerState:sampler atIndex:cp.fragment.image];
			[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
			_outColorTex = colorTex; // restore output color texture
			endPass();
		}

		if (dst->mipmap()) {
			if (!blitEnc)
				blitEnc = [_cmdPack.current blitCommandEncoder];
			[blitEnc generateMipmapsForTexture:tex];
		}
		if (blitEnc) {
			[blitEnc endEncoding];
		}

		setTex_SourceImage(dst, dst->info(), texStat);
	}

	void MetalCanvas::outputImageBeginCmd(ImageSource* dst) {
		endPass(); // end pass, change outTex for next pass
		auto s = _surfaceSize; // surface size
		TexStat storeStat;
		uint8_t flags = dst->mipmap() ? kMipmap_TextureFlags: 0;
		auto tex = mtl_rebuild_texture(_device, s, _opts.colorType, dst->texture(0), storeStat, flags);
		if (!tex) {
			// texture rebuild failed after current pass was ended.
			// upper layer currently does not track this failure state,
			// so subsequent rendering commands may fail due to missing
			// output render target texture.
			return;
		}
		_outColorTex = mtl_get_texture(tex); // set new output color texture for next pass
		setTex_SourceImage(dst, {int(s[0]),int(s[1]),_opts.colorType,dst->info().alphaType()}, tex);
	}

	void MetalCanvas::outputImageEndCmd(ImageSource* exit) {
		endPass(); // end current pass, change outTex back to canvas's own texture for next pass
		// restore output color texture for next pass
		_outColorTex = mtl_get_texture_from(*_state->output, _outTex);
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
