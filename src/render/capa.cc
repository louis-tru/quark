/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./gpu_canvas.h"
#include "./capa.h"

namespace qk {
	constexpr float kCAPACoverageBudgetMultiplier = 1.2f;
	constexpr uint32_t kCAPAMaxBoundaryTileCapacity = 1u << 16;
	constexpr uint32_t kCAPACoveragePageBytes = sizeof(MSLCapaBackdrop::CAPABoundaryTile);

	IVec2 capa_floor_tile_origin(Vec2 origin) {
		Qk_ASSERT(origin.x() >= 0.0f && origin.y() >= 0.0f, "capa_floor_tile_origin: origin must be non-negative");
		return IVec2(int(floorf(origin.x())) >> kCAPATileSizeShift, int(floorf(origin.y())) >> kCAPATileSizeShift);
	}

	IVec2 capa_ceil_tile_end(Vec2 end) {
		Qk_ASSERT(end.x() >= 0.0f && end.y() >= 0.0f, "capa_ceil_tile_end: end must be non-negative");
		return IVec2(
			(int(ceilf(end.x())) + kCAPATileSize - 1) >> kCAPATileSizeShift,
			(int(ceilf(end.y())) + kCAPATileSize - 1) >> kCAPATileSizeShift
		);
	}

	uint32_t capa_maxShortEdgeCount(float totalEdgeLen, uint32_t edgeCount) {
		constexpr float kInvCAPAShortEdgeLength = 1.0f / kCAPAShortEdgeLength;
		return uint32_t(ceilf(totalEdgeLen * kInvCAPAShortEdgeLength)) + edgeCount;
	}

	uint32_t capa_maxBoundaryTileCount(float totalEdgeLen, uint32_t edgeCount) {
		constexpr float kInvCAPATileSize = 1.0f / float(kCAPATileSize);
		// Edge length divided by tile size overestimates boundary coverage pages
		// without exploding to conservative bounds area for large filled shapes.
		return uint32_t(ceilf(totalEdgeLen * kInvCAPATileSize)) + edgeCount;
		// return uint32_t(ceilf(totalEdgeLen * kInvCAPATileSize * kCAPACoverageBudgetMultiplier));
	}

	Range capa_bounds_transform(const Mat& mat, const Range &bounds) {
		Vec2 points[4] = {
			bounds.begin, Vec2(bounds.end.x(), bounds.begin.y()),
			bounds.end, Vec2(bounds.begin.x(), bounds.end.y()),
		};
		mat.mul_vec2_batch(points, 4);
		Vec2 begin = points[0], end = points[0];
		for (int i = 1; i < 4; i++) {
			float x = points[i].x();
			float y = points[i].y();
			if (x < begin[0]) {
				begin[0] = x;
			} else if (x > end[0]) {
				end[0] = x;
			}
			if (y < begin[1]) {
				begin[1] = y;
			} else if (y > end[1]) {
				end[1] = y;
			}
		}
		return {begin, end};
	}

	float capa_path_scale(const Mat& mat) {
		float a = mat[0];
		float b = mat[1];
		float c = mat[3];
		float d = mat[4];
		float s1 = a*a + b*b + c*c + d*d; // ||M||F²
		// Frobenius Norm, ||M||F = sqrt(a² + b² + c² + d²)
		// return sqrtf(s1);
		float det = a*d - b*c; // det(M) = ad - bc
		float disc = sqrtf(std::max(0.0f, s1*s1 - 4.0f * det*det));
		return sqrtf(0.5f * (s1 + disc)); // 最大奇异值 σmax <= ||M||F
	}

	CAPABuilder::CAPABuilder(GPUCanvas *owner): _owner(owner) {
		reset();
	}

	int CAPABuilder::findImageTexture(const PaintImage *paint) const {
		for (uint32_t i = 0; i < _data.imageSources.length(); i++)
			if (_data.imageSources[i].get() == paint->image)
				return i;
		return -1;
	}

	int CAPABuilder::findImageSampler(const PaintImage *paint) const {
		for (uint32_t i = 0; i < _data.imageSamplers.length(); i++)
			if (_data.imageSamplers[i].bitfields == paint->bitfields)
				return i;
		return -1;
	}

	bool CAPABuilder::canAddImageTexture(const PaintImage *paint) const {
		if (_data.imageSources.length() == kCAPAMaxImageCount)
			if (findImageTexture(paint) == -1)
				return false;
		if (_data.imageSamplers.length() == kCAPAMaxImageCount)
			if (findImageSampler(paint) == -1)
				return false;
		return true;
	}

	uint32_t CAPABuilder::addImageTexture(const PaintImage *paint) {
		int index = findImageTexture(paint);
		if (index >= 0)
			return index;
		index = _data.imageSources.length();
		_data.imageSources.push(paint->image);
		return index;
	}

	uint32_t CAPABuilder::addImageSampler(const PaintImage *paint) {
		int index = findImageSampler(paint);
		if (index >= 0)
			return index;
		index = _data.imageSamplers.length();
		_data.imageSamplers.push(*paint);
		return index;
	}

	void CAPABuilder::steupPaint(CAPAPath &path, CAPAPaint* paint, const Mat& mat) {
		path.flags = path.color.a() >= 1.0f ? kCAPA_FLAG_PAINT_OPAQUE : 0u;
		if (!paint)
			return;
		if (paint->type == kCAPA_PAINT_IMAGE) {
			auto &info = *paint->image;
			auto image = info.paint->image;
			auto type = info.paint->_isCanvas ? kRGBA_8888_ColorType: image->type();
			auto size = info.paint->_isCanvas ? info.paint->canvas->surfaceSize(): image->size();
			if (info.paint->_isCanvas || image->info().alphaType() != kOpaque_AlphaType) {
				path.flags &= ~kCAPA_FLAG_PAINT_OPAQUE;
			}
			path.paintType = kCAPA_PAINT_IMAGE;
			path.paintIndex = _data.imagePaints.length();
			_data.imagePaints.push(CAPAImagePaint{
				.coord = Vec4(info.paint->coord.begin, info.paint->coord.end),
				.strokeColor = info.stroke <= 0 ? path.color: premul_alpha(info.strokeColor),
				.size = size,
				.textureIndex = addImageTexture(info.paint),
				.samplerIndex = addImageSampler(info.paint),
				.stroke = info.stroke,
				.lod = 0.0,
				.alphaIndex = info.kind == kMask_DrawKind ?
					(type == kAlpha_8_ColorType ? 0 : type == kLuminance_Alpha_88_ColorType ? 1 : 3): 0,
				.kind = info.kind,
			});
			if (info.paint->mipmapMode == PaintImage::kNone_MipmapMode) {
				path.flags |= kCAPA_FLAG_NONE_MIPMAP_MODE;
			}
		} else if (paint->type == kCAPA_PAINT_GRADIENT) {
			if (path.flags & kCAPA_FLAG_PAINT_OPAQUE) {
				for (int i = 0; i < paint->gradient->count; i++) {
					if (paint->gradient->colors[i].a() < 1.0f) {
						path.flags &= ~kCAPA_FLAG_PAINT_OPAQUE;
						break;
					}
				}
			}
			path.paintType = kCAPA_PAINT_GRADIENT;
			path.paintIndex = _data.gradientPaints.length();
			_data.gradientPaints.push(CAPAGradientPaint{
				.origin = paint->gradient->origin,
				.endOrRadius = paint->gradient->endOrRadius,
				.type = paint->gradient->type,
				.count = paint->gradient->count,
				.colors = _data.colors.length(),
				.positions = _data.positions.length(),
			});
			for (uint32_t i = 0; i < paint->gradient->count; i++)
				_data.colors.push(premul_alpha(paint->gradient->colors[i]).mul(path.color));
			_data.positions.write(paint->gradient->positions, paint->gradient->count);
		}
	}

	bool CAPABuilder::build(const Path &rawPath, const Color4f& color, CAPAPaint* paint) {
		auto &info = _owner->_cache->getEdgeInfo(rawPath, _owner->_allScaleAverage * 0.5f);
		if (info.edges.length() < 4)
			return true; // Skip empty or degenerate paths

		auto &budget = _data.budget;
		auto &matrix = _owner->_state->matrix;
		Vec2 surfaceScale = _owner->_surfaceScale;
		auto edgeOffset = _data.edges.length();
		Vec2 surfaceOffset(_data.surfaceOffset.x(), _data.surfaceOffset.y());
		Mat mat(
			matrix[0] * surfaceScale.x(),
			matrix[1] * surfaceScale.x(),
			matrix[2] * surfaceScale.x() + surfaceOffset.x(),
			matrix[3] * surfaceScale.y(),
			matrix[4] * surfaceScale.y(),
			matrix[5] * surfaceScale.y() + surfaceOffset.y()
		);
		// Budget space for staging small tiles, short-edge nodes, row records,
		// boundary tiles, and final z-linear coverage pages.
		auto edgeCount = edgeOffset + (info.edges.length() >> 1);
		auto totalEdgeLen = _totalEdgeLength + info.totalEdgeLength * capa_path_scale(mat);
		auto maxShortEdgeCount = capa_maxShortEdgeCount(totalEdgeLen, edgeCount);
		auto maxBoundaryTileCount = capa_maxBoundaryTileCount(totalEdgeLen, edgeCount);

		if (maxBoundaryTileCount > kCAPAMaxBoundaryTileCapacity) {
			if (budget.maxBoundaryTileCount == 0)
				return false; // Path is too large to fit in a single CAPA draw call
			flush(); // Flush current draw data to free up boundary tile capacity
			return build(rawPath, color, paint); // Retry after flush
		}

		Range clip{{0,0}, _owner->_state->output ?
			_owner->_state->output->size() : _owner->_surfaceSize};
		if (_owner->_clipState) {
			clip = clip.clip(_owner->_clipState->range.offset(surfaceOffset));
		}
		auto bounds = capa_bounds_transform(mat, info.bounds)
			.expandToInteger()
			.clip(clip);
		if (bounds.isEmpty())
			return true; // Skip empty or degenerate paths
		// CAPAPath starts with CPU metadata plus path-space edge offsets. The
		// prepare/prepare_tiles passes fill surface bounds and tile ranges.
		CAPAPath path {
			.matrixX = Vec4(mat[0], mat[1], mat[2]),
			.matrixY = Vec4(mat[3], mat[4], mat[5]),
			.clip = Vec4(clip.begin, clip.end),
			.bounds = IVec4(0x7fffffff, 0x7fffffff, -0x7fffffff, -0x7fffffff),
			.color = premul_alpha(color), // premultiplied color for solid fill
			.edgeOffset = edgeOffset,
			.edgeCount = info.edges.length() >> 1,
			.blendMode = uint32_t(_owner->_blendMode),
			.fillRule = uint32_t(fillRule),
			.tileOffset = 0,
			.tileRect = IVec4(0, 0, 0, 0),
			.tileEnd = IVec2(0, 0),
			.paintType = kCAPA_PAINT_SOLID,
		};
		steupPaint(path, paint, mat);

		auto pathIndex = _data.paths.length();
		_data.paths.push(std::move(path));
		_data.edges.reset(edgeCount);
		_totalEdgeLength = totalEdgeLen;
		IRange tileBounds = IRange{
			capa_floor_tile_origin(bounds.begin), capa_ceil_tile_end(bounds.end)
		};
		auto tileSpan = tileBounds.size();
		budget.globalBounds = budget.globalBounds.join(bounds);
		budget.maxPathTileCount += tileSpan.x() * tileSpan.y();
		budget.maxShortEdgeCount = maxShortEdgeCount;
		budget.maxBoundaryTileCount = maxBoundaryTileCount;
		budget.maxPathTileRowCount += tileSpan.y();

		for (uint32_t i = 1; i < info.edges.length(); i += 2) {
			_data.edges[edgeOffset++] = CAPAEdge{
				.p0 = info.edges[i - 1],
				.p1 = info.edges[i],
				.pathIndex = pathIndex,
			};
		}
		return true;
	}

	bool CAPABuilder::buildGradient(const Path &path, const PaintGradient *gradient, const Color4f &color) {
		CAPAPaint paint{.gradient = gradient, .type = kCAPA_PAINT_GRADIENT};
		return build(path, color, &paint);
	}

	bool CAPABuilder::buildImage(const Path &path, const GC_ImageDrawInfo &info) {
		auto paint = info.paint;
		if (!paint->image)
			return true; // Skip empty images
		if (paint->coord.end.x() <= 0.0f || paint->coord.end.y() <= 0.0f)
			return true; // Skip images with zero size
		if (paint->_isCanvas) { // flush canvas to current canvas
			auto sub = static_cast<GPUCanvas*>(paint->canvas);
			if (sub == _owner || !sub->isGpu())
				return true; // skip if the source canvas is the same as the owner or not a GPU canvas
			if (sub->render() != _owner->render())
				return true; // skip if the source canvas is not the same render
			auto size = sub->surfaceSize();
			if (size.x() <= 0 || size.y() <= 0)
				return true; // skip if the source canvas has zero size
			_owner->flushSubcanvasCmd(sub);
		} else {
			auto src = paint->image;
			if (src->count() == 0)
				return true; // skip empty image source
			if (kYUV420P_Y_8_ColorType == src->type())
				return false; // not support YUV420 image source for CAPA render
			if (src->width() <= 0 || src->height() <= 0)
				return true; // Skip images with zero size
			// mark image source as texture for this render
			src->markAsTexture();
			if (!src->texture(0)->ptr())
				return true; // skip if texture is not ready
		}
		if (!canAddImageTexture(paint))
			flush();
		CAPAPaint capaPaint{.image = &info, .type = kCAPA_PAINT_IMAGE};
		return build(path, info.color, &capaPaint);
	}

	void CAPABuilder::flush() {
		if (!_data.edges.length())
			return;
		auto &budget = _data.budget;
		budget.globalTileBounds = IRange{
			capa_floor_tile_origin(budget.globalBounds.begin),
			capa_ceil_tile_end(budget.globalBounds.end),
		};
		auto tileSpan = budget.globalTileBounds.size();
		budget.globalTileCount = tileSpan.x() * tileSpan.y();
		_owner->drawCAPACmd(_data);
		reset();
	}

	void CAPABuilder::reset(bool clear) {
		AllocatorScope scope(&_alloc);
		_totalEdgeLength = 0;
		_data = {};
		if (clear)
			_alloc.clear();
		else
			_alloc.reset();
	}
}
