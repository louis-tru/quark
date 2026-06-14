/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

// @private head

#ifndef __quark_render_cgaa__
#define __quark_render_cgaa__

#include "./path.h"

namespace qk {
	
	static constexpr uint32_t kCGAATileSize = 16;
	static constexpr uint32_t kCGAASampleGrid = 4;
	static_assert(kCGAATileSize * kCGAASampleGrid <= 64,
		"Compute AA inside mask only supports up to 64 X samples per tile");

	enum CGAAFillRule: uint32_t {
		kCGAANonZero_FillRule = 0,
		kCGAAEvenOdd_FillRule = 1,
		kCGAAPositive_FillRule = 2,
		kCGAANegative_FillRule = 3,
	};

	struct alignas(16) CGAAEdge {
		Vec2 p0;          // atlas-space start point, y-down
		Vec2 p1;          // atlas-space end point, y-down
		float dxdy;       // cached dx/dy for GPU X intersection
		int32_t winding;  // +1 for downward edge, -1 for upward edge
		uint32_t _pad[2];
	};
	static_assert(sizeof(CGAAEdge) == sizeof(float) * 8,
		"Metal edge ABI mismatch");

	struct CGAATileEdge {
		uint32_t edgeIndex;
		// 当前 tile 内需要使用原始边做精确交点测试的 Y sample 范围。
		uint16_t sampleBegin; // local Y sample range [begin, end)
		uint16_t sampleEnd;
	};
	static_assert(sizeof(CGAATileEdge) == 8, "Metal tile-edge ABI mismatch");

	struct CGAABackdropEvent {
		// This is a compact span record, not an MTLEvent synchronization object.
		// 从 firstTileX 开始，当前 tile 行内的 sample 范围已经完全位于
		// 此边右侧，所有后续 tile 的 backdrop 都应加 winding。
		uint32_t firstTileX;
		uint16_t sampleBegin; // local Y sample range [begin, end)
		uint16_t sampleEnd;
		int32_t winding;
	};
	static_assert(sizeof(CGAABackdropEvent) == 12,
		"Metal backdrop-event ABI mismatch");

	struct CGAABackdropRow {
		// 当前 yTile 行在 CGAADrawData::backdropEvents 中的切片。
		uint32_t eventOffset;
		uint32_t eventCount;
	};
	static_assert(sizeof(CGAABackdropRow) == 8,
		"Metal backdrop-row ABI mismatch");

	struct alignas(16) CGAATile {
		uint32_t originX;    // atlas-space tile origin in pixels
		uint32_t originY;
		uint32_t edgeOffset; // offset into CGAADrawData::tileEdges
		uint32_t edgeCount;
	};
	static_assert(sizeof(CGAATile) == sizeof(uint32_t) * 4,
		"Metal tile ABI mismatch");

	struct alignas(16) CGAAUniformTile {
		uint32_t originX;
		uint32_t originY;
		int32_t winding;
		uint32_t _pad;
	};
	static_assert(sizeof(CGAAUniformTile) == sizeof(uint32_t) * 4,
		"Metal uniform-tile ABI mismatch");

	struct alignas(16) CGAAParams {
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

	struct CGAADrawData {
		Range bounds; // original path bounds before atlas origin subtraction
		Vec2 atlasOrigin;
		Vec2 atlasSize;
		uint32_t tileCountX = 0;
		uint32_t tileCountY = 0;
		Array<CGAAEdge> edges;
		Array<CGAATileEdge> tileEdges;
		Array<CGAATile> boundaryTiles; // compact; no entries for uniform tiles
		Array<CGAAUniformTile> uniformTiles;
		Array<CGAABackdropEvent> backdropEvents;
		Array<CGAABackdropRow> backdropRows;
	};

	// 配置以使用普通的平凡类型，这在容器中可以优化为不调用构造函数和析构函数。
	template<> struct IsOrdinaryType<CGAAEdge> { static constexpr bool value = true; };
	template<> struct IsOrdinaryType<CGAATileEdge> { static constexpr bool value = true; };
	template<> struct IsOrdinaryType<CGAATile> { static constexpr bool value = true; };
	template<> struct IsOrdinaryType<CGAAUniformTile> { static constexpr bool value = true; };
	template<> struct IsOrdinaryType<CGAABackdropEvent> { static constexpr bool value = true; };
	template<> struct IsOrdinaryType<CGAABackdropRow> { static constexpr bool value = true; };
	// 配置最小容量以避频繁扩容，分配器默认最小容量为 1。
	template<> struct AllocatorConfig<CGAAEdge> { static constexpr uint32_t kMinCapacity = 4; };
	template<> struct AllocatorConfig<CGAATileEdge> { static constexpr uint32_t kMinCapacity = 4; };
	template<> struct AllocatorConfig<CGAABackdropEvent> { static constexpr uint32_t kMinCapacity = 4; };

		// Builds CPU-side buffers for the compute prototype. The path may contain
	// curves; Path::getEdgeLines() flattens them first.
	CGAADrawData buildCGAADrawData(const Path &path,
		Range *clip = nullptr, const Mat *mat = nullptr, float flattenPrecision = 1.0f);
}
#endif
