/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./capa.h"
#include "./gpu_canvas.h"

namespace qk {
	constexpr float kCAPACoverageBudgetMultiplier = 1.2f;
	constexpr uint32_t kCAPAMaxBoundaryTileCapacity = 1u << 20;
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

	float capa_path_scale(const CAPAPath& path) {
		float a = path.matrixX.x();
		float b = path.matrixX.y();
		float c = path.matrixY.x();
		float d = path.matrixY.y();
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

	bool CAPABuilder::build(const Path &rawPath, const Color4f &color) {
		auto &info = _owner->_cache->getEdgeInfo(rawPath, _owner->_allScaleAverage * 0.5f);
		if (info.edges.length() < 2)
			return false;

		auto &matrix = _owner->_state->matrix;
		Vec2 surfaceScale = _owner->_surfaceScale;
		auto pathIndex = _data.paths.length();
		auto edgeOffset = _data.edges.length();
		Mat mat(
			matrix[0] * surfaceScale.x(),
			matrix[1] * surfaceScale.x(), matrix[2] * surfaceScale.x(),
			matrix[3] * surfaceScale.y(),
			matrix[4] * surfaceScale.y(), matrix[5] * surfaceScale.y()
		);
		Range clip{{0,0}, _owner->_surfaceSize};
		if (_owner->_clipState) {
			clip = _owner->_clipState->range;
		}
		// CAPAPath starts with CPU metadata plus path-space edge offsets. The
		// prepare/prepare_tiles passes fill surface bounds and tile ranges.
		CAPAPath path {
			.matrixX = Vec4(mat[0], mat[1], mat[2]),
			.matrixY = Vec4(mat[3], mat[4], mat[5]),
			.clip = Vec4(clip.begin, clip.end),
			.bounds = IVec4(0x7fffffff, 0x7fffffff, -0x7fffffff, -0x7fffffff),
			.color = color,
			.edgeOffset = edgeOffset,
			.edgeCount = info.edges.length() >> 1,
			.blendMode = uint32_t(_owner->_blendMode),
			.fillRule = uint32_t(fillRule),
			.tileOffset = 0,
			.tileRect = IVec4(0, 0, 0, 0),
			.tileEnd = IVec2(0, 0),
		};
		// Budget space for staging small tiles, short-edge nodes, row records,
		// boundary tiles, and final z-linear coverage pages.
		auto edgeCount = edgeOffset + path.edgeCount;
		auto totalEdgeLen = _totalEdgeLength + info.totalEdgeLength * capa_path_scale(path);
		auto maxShortEdgeCount = capa_maxShortEdgeCount(totalEdgeLen, edgeCount);
		auto maxBoundaryTileCount = capa_maxBoundaryTileCount(totalEdgeLen, edgeCount);

		if (maxBoundaryTileCount > kCAPAMaxBoundaryTileCapacity) {
			// TODO: cancel CAPA draw if boundary tile count exceeds max capacity
		}
		_data.paths.push(path);
		_data.edges.reset(edgeCount);
		_totalEdgeLength = totalEdgeLen;
		auto bounds = capa_bounds_transform(mat, info.bounds)
			.expandToInteger()
			.clip(clip);
		IRange tileBounds = IRange{
			capa_floor_tile_origin(bounds.begin), capa_ceil_tile_end(bounds.end)
		};
		auto tileSpan = tileBounds.size();
		auto &budget = _data.budget;
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

	void CAPABuilder::commit() {
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
