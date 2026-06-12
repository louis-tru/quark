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
//     one tileSampleCount-thread threadgroup per 16x16 tile
//          -> one thread per local Y sample row builds a 64-bit inside mask
//          -> the same threads cooperatively merge and write all tile pixels
//          -> write one-channel coverage atlas
//   Graphics:
//     clear and composite the coverage atlas through one render pass.

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
	static_assert(kComputeAATileSize * kComputeAASampleGrid <= 64,
		"Compute AA inside mask only supports up to 64 X samples per tile");

	struct AllocatorA: LinearAllocator {
	};

	enum ComputeAAFillRule: uint32_t {
		kComputeAANonZero_FillRule = 0,
		kComputeAAEvenOdd_FillRule = 1,
		kComputeAAPositive_FillRule = 2,
		kComputeAANegative_FillRule = 3,
	};

	struct alignas(16) ComputeAAEdge {
		Vec2 p0;          // atlas-space start point, y-down
		Vec2 p1;          // atlas-space end point, y-down
		float dxdy;       // cached dx/dy for GPU X intersection
		int32_t winding;  // +1 for downward edge, -1 for upward edge
		uint32_t _pad[2];
	};
	static_assert(sizeof(ComputeAAEdge) == sizeof(float) * 8,
		"Metal edge ABI mismatch");

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

	struct alignas(16) ComputeAAUniformTile {
		uint32_t originX;
		uint32_t originY;
		int32_t winding;
		uint32_t _pad;
	};
	static_assert(sizeof(ComputeAAUniformTile) == sizeof(uint32_t) * 4,
		"Metal uniform-tile ABI mismatch");

	struct alignas(16) ComputeAAParams {
		uint32_t width;
		uint32_t height;
		uint32_t tileCountX;
		uint32_t tileCountY;
		uint32_t fillRule;
		uint32_t sampleGrid;
		uint32_t outputOriginX;
		uint32_t outputOriginY;
		uint32_t outputWidth;
		uint32_t outputHeight;
		uint32_t _pad[2];
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
		Array<uint32_t> boundaryTileIndices;
		Array<ComputeAAUniformTile> uniformTiles;
		Array<ComputeAABackdropEvent> backdropEvents;
		Array<ComputeAABackdropRow> backdropRows;
	};

	// 配置以使用普通的平凡类型，这在容器中可以优化为不调用构造函数和析构函数。
	template<> struct IsOrdinaryType<ComputeAAEdge> { static constexpr bool value = true; };
	template<> struct IsOrdinaryType<ComputeAATileEdge> { static constexpr bool value = true; };
	template<> struct IsOrdinaryType<ComputeAATile> { static constexpr bool value = true; };
	template<> struct IsOrdinaryType<ComputeAAUniformTile> { static constexpr bool value = true; };
	template<> struct IsOrdinaryType<ComputeAABackdropEvent> { static constexpr bool value = true; };
	template<> struct IsOrdinaryType<ComputeAABackdropRow> { static constexpr bool value = true; };
	// 配置最小容量以避频繁扩容，分配器默认最小容量为 1。
	template<> struct AllocatorConfig<ComputeAAEdge> { static constexpr uint32_t kMinCapacity = 4; };
	template<> struct AllocatorConfig<ComputeAATileEdge> { static constexpr uint32_t kMinCapacity = 4; };
	template<> struct AllocatorConfig<ComputeAABackdropEvent> { static constexpr uint32_t kMinCapacity = 4; };

	class MetalComputeAAPrototype {
	public:
		// Builds CPU-side buffers for the compute prototype. The path may contain
		// curves; Path::getEdgeLines() flattens them first.
		static ComputeAADrawData buildDrawData(const Path &path,
			const Mat &viewMatrix,
			float flattenPrecision = 1.0f);

#ifdef __OBJC__
		// Encodes the prototype coverage pass. The caller owns pipeline creation
		// for qk_compute_aa_coverage in compute_aa_prototype.metal.
		static bool encodeCoverage(id<MTLDevice> device,
			id<MTLCommandBuffer> commandBuffer,
			id<MTLComputePipelineState> uniformPipeline,
			id<MTLComputePipelineState> coveragePipeline,
			id<MTLTexture> coverageTexture,
			const ComputeAADrawData &drawData,
			ComputeAAFillRule fillRule = kComputeAANonZero_FillRule);

		// Draws the coverage atlas through a premultiplied-alpha render pipeline.
		static bool encodeSolidComposite(id<MTLCommandBuffer> commandBuffer,
			id<MTLRenderPipelineState> compositePipeline,
			id<MTLTexture> coverageTexture,
			id<MTLTexture> colorTexture,
			Vec4 clearColor,
			Vec4 premulColor,
			Vec2 outputOrigin,
			const ComputeAADrawData &drawData,
			ComputeAAFillRule fillRule = kComputeAANonZero_FillRule);
#endif
	};

} // namespace qk

#endif
