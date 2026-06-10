/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

// @private head
//
// Prototype only. This file is intentionally not included by the current
// renderer or build files. It documents the first Metal compute-AA data model:
//
//   CPU:
//     Path -> flattened line edges -> device/atlas coordinates
//          -> 16x16 local edge bins + per-tile-row backdrop spans
//   GPU:
//     one 16x16 threadgroup per tile
//          -> cooperatively resolve backdrop spans into threadgroup memory
//     one compute thread per output pixel
//          -> evaluate subpixel winding coverage
//          -> write one-channel coverage atlas
//   Graphics:
//     draw path bounds and sample coverage atlas, or run a second compute
//     prototype kernel to composite a solid color.

#ifndef __quark_test_mtl_compute_aa_prototype__
#define __quark_test_mtl_compute_aa_prototype__

#include "src/render/math.h"
#include "src/render/path.h"

#ifdef __OBJC__
#import <Metal/Metal.h>
#endif

namespace qk {

	static constexpr uint32_t kComputeAATileSize = 16;
	static constexpr uint32_t kComputeAASampleGrid = 4;

	enum ComputeAAFillRule: uint32_t {
		kComputeAANonZero_FillRule = 0,
		kComputeAAEvenOdd_FillRule = 1,
		kComputeAAPositive_FillRule = 2,
		kComputeAANegative_FillRule = 3,
	};

	struct alignas(16) ComputeAAEdge {
		Vec2 p0;          // atlas-space start point, y-down
		Vec2 p1;          // atlas-space end point, y-down
		float minY;       // cached y range for quick sample rejection
		float maxY;
		int32_t winding;  // +1 for downward edge, -1 for upward edge
		uint32_t _pad;
	};

	struct ComputeAATileEdge {
		uint32_t edgeIndex;
		// 当前 tile 内需要使用原始边做精确交点测试的 Y sample 范围。
		uint16_t sampleBegin; // local Y sample range [begin, end)
		uint16_t sampleEnd;
	};
	static_assert(sizeof(ComputeAATileEdge) == 8, "Metal tile-edge ABI mismatch");

	struct ComputeAABackdropEvent {
		// This is a compact span record, not an MTLEvent synchronization object.
		// 从 firstTileX 开始，当前 tile 行内的 sample 范围已经完全位于
		// 此边右侧，所有后续 tile 的 backdrop 都应加 winding。
		uint32_t firstTileX;
		uint16_t sampleBegin; // local Y sample range [begin, end)
		uint16_t sampleEnd;
		int32_t winding;
	};
	static_assert(sizeof(ComputeAABackdropEvent) == 12,
		"Metal backdrop-event ABI mismatch");

	struct ComputeAABackdropRow {
		// 当前 yTile 行在 ComputeAADrawData::backdropEvents 中的切片。
		uint32_t eventOffset;
		uint32_t eventCount;
	};
	static_assert(sizeof(ComputeAABackdropRow) == 8,
		"Metal backdrop-row ABI mismatch");

	struct alignas(16) ComputeAATile {
		uint32_t edgeOffset; // offset into ComputeAADrawData::tileEdges
		uint32_t edgeCount;
		uint32_t originX;    // atlas-space tile origin in pixels
		uint32_t originY;
	};
	static_assert(sizeof(ComputeAATile) == sizeof(uint32_t) * 4,
		"Metal tile ABI mismatch");

	struct alignas(16) ComputeAAParams {
		uint32_t width;
		uint32_t height;
		uint32_t tileCountX;
		uint32_t tileCountY;
		uint32_t fillRule;
		uint32_t sampleGrid;
		uint32_t outputOriginX;
		uint32_t outputOriginY;
		Vec4 color; // premultiplied when used by the composite kernel
	};

	struct ComputeAADrawData {
		Range bounds; // original path bounds before atlas origin subtraction
		Vec2 atlasOrigin;
		Vec2 atlasSize;
		uint32_t tileCountX = 0;
		uint32_t tileCountY = 0;
		Array<ComputeAAEdge> edges;
		Array<ComputeAATileEdge> tileEdges;
		Array<ComputeAATile> tiles;
		Array<ComputeAABackdropEvent> backdropEvents;
		Array<ComputeAABackdropRow> backdropRows;
	};

	class MetalComputeAAPrototype {
	public:
		// Builds CPU-side buffers for the compute prototype. The path may contain
		// curves; Path::getEdgeLines() flattens them first.
		static ComputeAADrawData buildDrawData(const Path &path,
			const Mat &viewMatrix,
			Vec2 atlasPadding = Vec2(1.0f),
			float flattenPrecision = 1.0f);

#ifdef __OBJC__
		// Encodes the prototype coverage pass. The caller owns pipeline creation
		// for qk_compute_aa_coverage in compute_aa_prototype.metal.
		static bool encodeCoverage(id<MTLDevice> device,
			id<MTLCommandBuffer> commandBuffer,
			id<MTLComputePipelineState> coveragePipeline,
			id<MTLTexture> coverageTexture,
			const ComputeAADrawData &drawData,
			ComputeAAFillRule fillRule = kComputeAANonZero_FillRule);

		// Optional second pass for inspecting the result without adding a render
		// pipeline. The caller owns qk_compute_aa_composite_solid.
		static bool encodeSolidComposite(id<MTLCommandBuffer> commandBuffer,
			id<MTLComputePipelineState> compositePipeline,
			id<MTLTexture> coverageTexture,
			id<MTLTexture> colorTexture,
			Vec4 premulColor,
			Vec2 outputOrigin,
			const ComputeAADrawData &drawData,
			ComputeAAFillRule fillRule = kComputeAANonZero_FillRule);
#endif
	};

} // namespace qk

#endif
