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
#import <Metal/Metal.h>

namespace qk {

	extern const float zDepthNextUnit = 1.0f / 5000000.0f;

	static void mat_to_float4x4(const Mat &mat, float out[16]) {
		out[0] = mat[0]; out[1] = mat[3]; out[2] = 0; out[3] = 0;
		out[4] = mat[1]; out[5] = mat[4]; out[6] = 0; out[7] = 0;
		out[8] = 0;      out[9] = 0;      out[10] = 1; out[11] = 0;
		out[12] = mat[2]; out[13] = mat[5]; out[14] = 0; out[15] = 1;
	}

	struct MatrixBlock {
		float value[16];
	};

	struct ColorArgs {
		float color[4];
		float depth;
		uint32_t flags;
	};

	struct ImageArgs {
		float texCoords[4];
		float color[4];
		float allScale;
		float depth;
		uint32_t flags;
	};

	struct ImageMaskArgs {
		float texCoords[4];
		float color[4];
		int32_t alphaIndex;
		float allScale;
		float depth;
		uint32_t flags;
	};

	struct SdfMaskArgs {
		float texCoords[4];
		float color[4];
		float strokeColor[4];
		float strokeWidth;
		float allScale;
		float depth;
		uint32_t flags;
	};

	static int alpha_index_for_type(ColorType type) {
		return type == kAlpha_8_ColorType ? 0:
			type == kLuminance_Alpha_88_ColorType ? 1: 3;
	}

	MetalCanvas::MetalCanvas(MetalRender *render, Render::Options opts)
		: _state(nullptr)
		, _render(render)
		, _cache(nullptr)
		, _opts(opts)
		, _rootMatrix()
		, _surfaceSize()
		, _size()
		, _scale(1)
		, _surfaceScale(1)
		, _colorTex(nil)
	{
		auto capacity = opts.maxCapacityForPathvCache ?
			opts.maxCapacityForPathvCache: 128000000;
		_cache = new PathvCache(Uint32::clamp(capacity, 1024000, 512000000), render, this);
		_stateStack.push({ .matrix = Mat() });
		_state = &_stateStack.back();
	}

	MetalCanvas::~MetalCanvas() {
		Releasep(_cache);
		_colorTex = nil;
	}

	int MetalCanvas::save() {
		_stateStack.push(*_state);
		_state = &_stateStack.back();
		return _stateStack.length();
	}

	void MetalCanvas::restore(uint32_t count) {
		if (!count || _stateStack.length() == 1) return;
		count = Uint32::min(count, _stateStack.length() - 1);
		_stateStack.pop(count);
		_state = &_stateStack.back();
	}

	int MetalCanvas::getSaveCount() const {
		return _stateStack.length() - 1;
	}

	const Mat& MetalCanvas::getMatrix() const {
		return _state->matrix;
	}

	void MetalCanvas::setMatrix(const Mat& mat) {
		_state->matrix = mat;
	}

	void MetalCanvas::setTranslate(Vec2 val) {
		_state->matrix.set_translate(val);
	}

	void MetalCanvas::translate(Vec2 val) {
		_state->matrix.translate(val);
	}

	void MetalCanvas::scale(Vec2 val) {
		_state->matrix.scale(val);
	}

	void MetalCanvas::rotate(float z) {
		_state->matrix.rotate(z);
	}

	void MetalCanvas::clipPath(const Path& path, ClipOp op, bool antiAlias) {
		(void)path; (void)op; (void)antiAlias;
	}

	void MetalCanvas::clipPathv(const Pathv& path, ClipOp op, bool antiAlias) {
		(void)path; (void)op; (void)antiAlias;
	}

	void MetalCanvas::clipRect(const Rect& rect, ClipOp op, bool antiAlias) {
		(void)rect; (void)op; (void)antiAlias;
	}

	void MetalCanvas::clearColor(const Color4f& color) {
		_ops.push({
			.type = kClear_Op,
			.matrix = _state->matrix,
			.color = color,
			.blendMode = kSrc_BlendMode,
			.fullClear = true,
		});
	}

	void MetalCanvas::drawColor(const Color4f& color, BlendMode mode) {
		if (mode == kSrc_BlendMode) {
			clearColor(color);
			return;
		}
		Rect rect{{0, 0}, _size};
		recordSolid(_cache->getRectPath(rect), color, mode);
	}

	void MetalCanvas::recordSolid(const VertexData &vertex, const Color4f &color, BlendMode mode) {
		if (!vertex.vCount) return;
		_ops.push({
			.type = kSolid_Op,
			.matrix = _state->matrix,
			.vertex = vertex,
			.color = color,
			.blendMode = mode,
			.strokeWidth = 0,
			.alphaIndex = 3,
			.fullClear = false,
		});
	}

	void MetalCanvas::recordImage(OpType type, const VertexData &vertex, const PaintImage *image,
		const Color4f &color, BlendMode mode, int alphaIndex,
		const Color4f &strokeColor, float strokeWidth)
	{
		if (!vertex.vCount || !image) return;
		DrawOp op{
			.type = type,
			.matrix = _state->matrix,
			.vertex = vertex,
			.color = color,
			.strokeColor = strokeColor,
			.image = *image,
			.blendMode = mode,
			.strokeWidth = strokeWidth,
			.alphaIndex = alphaIndex,
			.fullClear = false,
		};
		if (image->_isCanvas) {
			auto canvas = static_cast<MetalCanvas*>(image->canvas);
			if (canvas && canvas != this && canvas->isGpu()) {
				canvas->flushBuffer();
			}
		} else if (image->image) {
			op.imageSource = image->image;
		}
		_ops.push(std::move(op));
	}

	void MetalCanvas::drawPaintPathv(const Pathv &path, const Paint &paint) {
		if (paint.filter || paint.mask || paint.fill.gradient || paint.stroke.gradient) {
			Qk_Warn("MetalCanvas: gradient/filter/mask paint is not implemented yet");
		}
		if (paint.style == Paint::kFill_Style || paint.style == Paint::kStrokeAndFill_Style) {
			if (paint.fill.image) {
				recordImage(kImage_Op, path, paint.fill.image, paint.fill.color, paint.blendMode);
			} else {
				recordSolid(path, paint.fill.color, paint.blendMode);
			}
		}
		if (paint.style == Paint::kStroke_Style || paint.style == Paint::kStrokeAndFill_Style) {
			auto stroke = _cache->getStrokePath(path.path, paint.strokeWidth, paint.cap, paint.join, 0);
			auto &vertex = _cache->getPathTriangles(stroke);
			if (paint.stroke.image) {
				recordImage(kImage_Op, vertex, paint.stroke.image, paint.stroke.color, paint.blendMode);
			} else {
				recordSolid(vertex, paint.stroke.color, paint.blendMode);
			}
		}
	}

	void MetalCanvas::drawPath(const Path& path, const Paint& paint) {
		auto &p = _cache->getNormalizedPath(path);
		Pathv pv;
		pv.path = p;
		static_cast<VertexData&>(pv) = _cache->getPathTriangles(p);
		drawPaintPathv(pv, paint);
	}

	void MetalCanvas::drawPathv(const Pathv& path, const Paint& paint) {
		drawPaintPathv(path, paint);
	}

	void MetalCanvas::drawPathvColor(const Pathv &path, const Color4f &color, BlendMode mode, bool antiAlias) {
		(void)antiAlias;
		recordSolid(path, color, mode);
	}

	void MetalCanvas::drawPathvColors(const Pathv* path[], int count, const Color4f &color, BlendMode mode, bool antiAlias) {
		(void)antiAlias;
		for (int i = 0; i < count; i++) {
			if (path[i]) recordSolid(*path[i], color, mode);
		}
	}

	void MetalCanvas::drawRRectBlurColor(const Rect& rect, const float radius[4], float blur, const Color4f &color, BlendMode mode) {
		(void)blur;
		Path::BorderRadius r{{radius[0], radius[0]}, {radius[1], radius[1]}, {radius[2], radius[2]}, {radius[3], radius[3]}};
		recordSolid(_cache->getRRectPath(rect, r), color, mode);
	}

	void MetalCanvas::drawRect(const Rect& rect, const Paint& paint) {
		drawPaintPathv(_cache->getRectPath(rect), paint);
	}

	void MetalCanvas::drawRRect(const Rect& rect, const Path::BorderRadius &radius, const Paint& paint) {
		drawPaintPathv(_cache->getRRectPath(rect, radius), paint);
	}

	float MetalCanvas::drawGlyphs(const FontGlyphs &glyphs, Vec2 origin, const Array<Vec2> *offset, const Paint &paint) {
		Array<Vec2> offset2, *offsetP = nullptr;
		if (offset) {
			offset2 = *offset;
			offsetP = &offset2;
			for (auto &o: offset2) o *= _surfaceScale;
		}
		auto isSDF = paint.style != Paint::kFill_Style;
		auto tf = glyphs.typeface();
		auto img = isSDF ?
			tf->getSDFImage(glyphs.glyphs(), glyphs.fontSize() * _surfaceScale, offsetP, false, _render):
			tf->getImage(glyphs.glyphs(), glyphs.fontSize() * _surfaceScale, offsetP, _render);
		auto scale = drawTextImage(img, _surfaceScale, origin, paint);
		return scale * img.width;
	}

	void MetalCanvas::drawTextBlob(TextBlob *blob, Vec2 origin, float fontSize, const Paint &paint) {
		if (!blob) return;
		auto fixedFSize = fontSize * _surfaceScale;
		auto scale = fixedFSize / fontSize;
		auto isSDF = paint.style != Paint::kFill_Style;

		if (blob->img.fontSize != fixedFSize || !blob->img.image ||
			(isSDF ? blob->img.image->type() != kSDF_Unsigned_F32_ColorType: false)
		) {
			Array<Vec2> offset;
			if (blob->offset.length() >= blob->glyphs.length()) {
				offset = blob->offset;
				for (auto &o: offset) o *= scale;
			}
			blob->img = isSDF ?
				blob->typeface->getSDFImage(blob->glyphs, fixedFSize, &offset, false, _render):
				blob->typeface->getImage(blob->glyphs, fixedFSize, &offset, _render);
		}
		auto img = blob->img.image.get();
		if (img && img->width() && img->height()) {
			drawTextImage(blob->img, scale, origin, paint);
		}
	}

	void MetalCanvas::drawTriangles(const Triangles& triangles, const Paint &paint) {
		if (!triangles.verts || !triangles.indices || !triangles.indexCount) return;
		Array<Vec3> verts;
		for (uint32_t i = 0; i < triangles.indexCount; i++) {
			auto &v = triangles.verts[triangles.indices[i]];
			verts.push(Vec3(v.vertices.x(), v.vertices.y(), v.vertices.z()));
		}
		VertexData vertex{nullptr, triangles.indexCount, std::move(verts)};
		recordSolid(vertex, paint.fill.color, paint.blendMode);
	}

	float MetalCanvas::drawTextImage(Typeface::TextImage &img, float scale, Vec2 origin, const Paint &paint) {
		auto pix = img.image->pixel(0);
		if (!pix) return 0;
		auto scale_1 = 1.0f / scale;
		PaintImage p;
		Vec2 dst_start(origin.x() - img.left * scale_1, origin.y() - img.top * scale_1);
		Vec2 dst_size(pix->width() * scale_1, pix->height() * scale_1);
		Rect rect{dst_start, dst_size};

		p.setImage(*img.image, rect);
		p.mipmapMode = PaintImage::kLinear_MipmapMode;
		p.filterMode = PaintImage::kLinear_FilterMode;

		Vec2 top_right(dst_start.x() + dst_size.x(), dst_start.y());
		Vec2 left_bottom(dst_start.x(), dst_start.y() + dst_size.y());
		Vec2 right_bottom(dst_start + dst_size);
		VertexData vertex{nullptr, 6, {
			dst_start,
			top_right, left_bottom,
			top_right,
			right_bottom, left_bottom,
		}};

		if (img.image->type() == kSDF_Unsigned_F32_ColorType) {
			auto strokeWidth = paint.style == Paint::kFill_Style ? 0.0f: paint.strokeWidth;
			recordImage(kSdfMask_Op, vertex, &p, paint.fill.color, paint.blendMode,
				0, paint.stroke.color, strokeWidth * scale);
		} else {
			recordImage(kImageMask_Op, vertex, &p, paint.fill.color, paint.blendMode,
				alpha_index_for_type(img.image->type()));
		}
		return scale_1;
	}

	bool MetalCanvas::readPixels(uint32_t srcX, uint32_t srcY, Pixel* dst) {
		if (!_colorTex || !dst) return false;
		flushBuffer();
		auto sync = [_render->_commandQueue commandBuffer];
		[sync commit];
		[sync waitUntilCompleted];
		auto bpp = Pixel::bytes_per_pixel(dst->type());
		auto rowBytes = (NSUInteger)dst->width() * bpp;
		auto region = MTLRegionMake2D(srcX, srcY, dst->width(), dst->height());
		[_colorTex getBytes:dst->val() bytesPerRow:rowBytes fromRegion:region mipmapLevel:0];
		return true;
	}

	Sp<ImageSource> MetalCanvas::readImage(const Rect &src, Vec2 dest, ColorType type, BlendMode mode, bool isMipmap) {
		(void)src; (void)dest; (void)type; (void)mode; (void)isMipmap;
		Qk_Warn("MetalCanvas: readImage is not implemented yet");
		return nullptr;
	}

	Sp<ImageSource> MetalCanvas::outputImage(ImageSource* dest, bool isMipmap) {
		(void)dest; (void)isMipmap;
		Qk_Warn("MetalCanvas: outputImage is not implemented yet");
		return nullptr;
	}

	void MetalCanvas::swapBuffer() {
		_mutex.mutex.lock();
		_ops.swap(_frontOps);
		_ops.clear();
		_mutex.mutex.unlock();
	}

	void MetalCanvas::ensureTarget() {
		if (_colorTex || _surfaceSize.is_zero_axis()) return;
		auto desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:_render->offscreenPixelFormat()
		                                                               width:(NSUInteger)_surfaceSize.x()
		                                                              height:(NSUInteger)_surfaceSize.y()
		                                                           mipmapped:NO];
		desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
		desc.storageMode = MTLStorageModeShared;
		_colorTex = [_render->mtlDevice() newTextureWithDescriptor:desc];
	}

	void MetalCanvas::flushBuffer() {
		_mutex.mutex.lock();
		ensureTarget();
		if (!_colorTex || !_frontOps.length()) {
			_mutex.mutex.unlock();
			return;
		}

		auto cb = [_render->_commandQueue commandBuffer];
		id<MTLRenderCommandEncoder> enc = nil;
		bool passStarted = false;

		auto beginPass = [&](bool clear, const Color4f &color) {
			if (enc) {
				[enc endEncoding];
				enc = nil;
			}
			auto pass = [MTLRenderPassDescriptor new];
			pass.colorAttachments[0].texture = _colorTex;
			pass.colorAttachments[0].loadAction = clear || !passStarted ? MTLLoadActionClear: MTLLoadActionLoad;
			pass.colorAttachments[0].storeAction = MTLStoreActionStore;
			pass.colorAttachments[0].clearColor = MTLClearColorMake(color.r(), color.g(), color.b(), color.a());
			enc = [cb renderCommandEncoderWithDescriptor:pass];
			passStarted = true;
		};

		beginPass(false, Color4f(0, 0, 0, 0));
		for (auto &op: _frontOps) {
			if (op.type == kClear_Op && op.fullClear) {
				beginPass(true, op.color);
				continue;
			}
			if (!op.vertex.vCount) continue;

			if (op.type == kImage_Op || op.type == kImageMask_Op || op.type == kSdfMask_Op) {
				id<MTLTexture> tex = nil;
				if (op.image._isCanvas) {
					auto src = static_cast<MetalCanvas*>(op.image.canvas);
					if (src && src != this && src->isGpu()) {
						src->flushBuffer();
						tex = src->colorTexture();
					}
				} else if (op.imageSource) {
					if (!op.imageSource->texture(0)) {
						op.imageSource->markAsTexture(_render);
					}
					tex = _render->getTexture(op.imageSource->texture(0));
				}
				if (!tex) continue;

				id<MTLRenderPipelineState> pso = nil;
				if (op.type == kImage_Op) {
					pso = _render->imagePipeline(_render->offscreenPixelFormat(), op.blendMode);
				} else if (op.type == kImageMask_Op) {
					pso = _render->imageMaskPipeline(_render->offscreenPixelFormat(), op.blendMode);
				} else {
					pso = _render->sdfMaskPipeline(_render->offscreenPixelFormat(), op.blendMode);
				}
				[enc setRenderPipelineState:pso];

				MatrixBlock root, view;
				memcpy(root.value, _rootMatrix.val, sizeof(root.value));
				mat_to_float4x4(op.matrix, view.value);
				[enc setVertexBytes:&view length:sizeof(view) atIndex:0];
				[enc setVertexBytes:&root length:sizeof(root) atIndex:2];

				if (op.type == kImage_Op) {
					ImageArgs args;
					memcpy(args.texCoords, op.image.coord.begin.val, sizeof(args.texCoords));
					args.color[0] = op.color.r();
					args.color[1] = op.color.g();
					args.color[2] = op.color.b();
					args.color[3] = op.color.a();
					args.allScale = _surfaceScale;
					args.depth = 0;
					args.flags = 0;
					[enc setVertexBytes:&args length:sizeof(args) atIndex:1];
					[enc setFragmentBytes:&args length:sizeof(args) atIndex:0];
				} else if (op.type == kImageMask_Op) {
					ImageMaskArgs args;
					memcpy(args.texCoords, op.image.coord.begin.val, sizeof(args.texCoords));
					args.color[0] = op.color.r();
					args.color[1] = op.color.g();
					args.color[2] = op.color.b();
					args.color[3] = op.color.a();
					args.alphaIndex = op.alphaIndex;
					args.allScale = _surfaceScale;
					args.depth = 0;
					args.flags = 0;
					[enc setVertexBytes:&args length:sizeof(args) atIndex:1];
					[enc setFragmentBytes:&args length:sizeof(args) atIndex:0];
				} else {
					SdfMaskArgs args;
					memcpy(args.texCoords, op.image.coord.begin.val, sizeof(args.texCoords));
					args.color[0] = op.color.r();
					args.color[1] = op.color.g();
					args.color[2] = op.color.b();
					args.color[3] = op.color.a();
					args.strokeColor[0] = op.strokeColor.r();
					args.strokeColor[1] = op.strokeColor.g();
					args.strokeColor[2] = op.strokeColor.b();
					args.strokeColor[3] = op.strokeColor.a();
					args.strokeWidth = op.strokeWidth;
					args.allScale = _surfaceScale;
					args.depth = 0;
					args.flags = 0;
					[enc setVertexBytes:&args length:sizeof(args) atIndex:1];
					[enc setFragmentBytes:&args length:sizeof(args) atIndex:0];
				}
				[enc setFragmentTexture:tex atIndex:0];
				[enc setFragmentSamplerState:_render->mtlSampler() atIndex:0];

				if (RenderBackend::setVertexData(op.vertex.id)) {
					auto buf = _render->getVertexBuffer(const_cast<VertexData::ID*>(op.vertex.id));
					[enc setVertexBuffer:buf offset:0 atIndex:0];
				} else if (op.vertex.vertex.val()) {
					[enc setVertexBytes:op.vertex.vertex.val() length:op.vertex.vertex.size() atIndex:0];
				} else {
					continue;
				}
				[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:op.vertex.vCount];
				continue;
			}

			if (op.type != kSolid_Op) continue;

			auto pso = _render->solidPipeline(_render->offscreenPixelFormat(), op.blendMode);
			[enc setRenderPipelineState:pso];

			MatrixBlock root, view;
			ColorArgs args;
			memcpy(root.value, _rootMatrix.val, sizeof(root.value));
			mat_to_float4x4(op.matrix, view.value);
			args.color[0] = op.color.r();
			args.color[1] = op.color.g();
			args.color[2] = op.color.b();
			args.color[3] = op.color.a();
			args.depth = 0;
			args.flags = 0;
			[enc setVertexBytes:&root length:sizeof(root) atIndex:0];
			[enc setVertexBytes:&view length:sizeof(view) atIndex:1];
			[enc setVertexBytes:&args length:sizeof(args) atIndex:2];
			[enc setFragmentBytes:&args length:sizeof(args) atIndex:0];

			if (RenderBackend::setVertexData(op.vertex.id)) {
				auto buf = _render->getVertexBuffer(const_cast<VertexData::ID*>(op.vertex.id));
				[enc setVertexBuffer:buf offset:0 atIndex:0];
			} else if (op.vertex.vertex.val()) {
				[enc setVertexBytes:op.vertex.vertex.val() length:op.vertex.vertex.size() atIndex:0];
			} else {
				continue;
			}
			[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:op.vertex.vCount];
		}

		if (enc) [enc endEncoding];
		[cb commit];
		_frontOps.clear();
		_mutex.mutex.unlock();
	}

	PathvCache* MetalCanvas::getPathvCache() {
		return _cache;
	}

	void MetalCanvas::setSurface(const Mat4& root, Vec2 surfaceSize, Vec2 scale) {
		auto chSize = surfaceSize != _surfaceSize;
		_surfaceSize = surfaceSize;
		_size = scale.is_zero_axis() ? surfaceSize: surfaceSize / scale;
		_scale = scale;
		_surfaceScale = (scale.x() + scale.y()) * 0.5f;
		_rootMatrix = root.transpose();
		if (chSize) _colorTex = nil;
	}

	Vec2 MetalCanvas::size() {
		return _size;
	}

	bool MetalCanvas::isGpu() {
		return true;
	}

	void MetalCanvas::lock() {
		_mutex.mutex.lock();
	}

	void MetalCanvas::unlock() {
		_mutex.mutex.unlock();
	}

} // namespace qk
