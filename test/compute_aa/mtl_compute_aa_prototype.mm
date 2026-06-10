/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

// Prototype only. This file is not wired into the current Metal backend.

#import "./mtl_compute_aa_prototype.h"

#include <math.h>
#include <string.h>

namespace qk {

	static inline uint32_t ceil_to_u32(float v) {
		return uint32_t(ceilf(std::max(v, 0.0f)));
	}

	static bool append_edge(ComputeAADrawData &out, Vec2 p0, Vec2 p1) {
		if (p0.y() == p1.y()) // x轴扫描线算法，水平边不计入边列表
			return false;
		ComputeAAEdge edge;
		p0 -= out.atlasOrigin;
		p1 -= out.atlasOrigin;
		edge.p0 = p0;
		edge.p1 = p1;
		edge.minY = std::min(p0.y(), p1.y());
		edge.maxY = std::max(p0.y(), p1.y());
		edge.winding = p1.y() > p0.y() ? 1 : -1;
		edge._pad = 0;
		out.edges.push(edge);
		return true;
	}

	static void build_tile_edges(ComputeAADrawData &out) {
		// 临时数组按 tile / tile-row 分桶，最后再压平为连续 GPU buffer。
		// 这里的小 Array 分配目前仍是 CPU 构建阶段的主要性能问题之一。
		struct TileScratch {
			Array<ComputeAATileEdge> edges;
		};
		struct BackdropRowScratch {
			Array<ComputeAABackdropEvent> events;
		};
		Array<TileScratch> scratch;
		scratch.reset(out.tileCountX * out.tileCountY);
		Array<BackdropRowScratch> backdropScratch;
		backdropScratch.reset(out.tileCountY);
		out.tiles.reset(out.tileCountX * out.tileCountY);

		const int sampleGrid = kComputeAASampleGrid;
		const int tileSampleCount = kComputeAATileSize * sampleGrid;
		const float invTileSize = 1.0f / float(kComputeAATileSize);
		const int atlasSampleCount = out.tileCountY * tileSampleCount;
		static_assert((kComputeAATileSize & (kComputeAATileSize - 1)) == 0 &&
			(kComputeAASampleGrid & (kComputeAASampleGrid - 1)) == 0,
			"Compute AA tile/sample sizes must be powers of two");
		constexpr int tileSampleShift =
			__builtin_ctz(kComputeAATileSize * kComputeAASampleGrid);

		auto sample_grid_y = [&](float sampleY) {
			Qk_ASSERT(sampleY >= 0.0f, "sampleY should not be negative");
			// sampleY 已经是非负的 Y sample-grid 坐标。这里用整数转换快速近似
			// ceilf(sampleY - 0.5f)：0.499999f 的轻微负偏置让正好落在 sample
			// 中心边界上的点仍属于前一个半开区间，同时吸收边界附近很小的正向浮点误差。
			constexpr float sampleBoundaryEpsilon = 0.0001f; // 0.0000001f
			int y = int(sampleY + 0.5f - sampleBoundaryEpsilon);
			Qk_ASSERT(y <= atlasSampleCount, "sampleY should not exceed atlas sample count");
			return y;
		};

		auto add_sample_range = [&](uint32_t edgeIndex, int tx,
			int sampleBegin, int sampleEnd, int32_t winding)
		{
			Qk_ASSERT(sampleBegin != sampleEnd, "should not add empty sample range");
			if (sampleBegin > sampleEnd) {
				std::swap(sampleBegin, sampleEnd); // 确保 sampleBegin < sampleEnd
			}
			// [sampleBegin, sampleEnd) 是边在当前 X tile 步骤中新跨过的全局
			// Y sample 区间。它可能跨越多个 yTile，因此先按 yTile 拆分。
			int ty0 = sampleBegin >> tileSampleShift;
			int ty1 = (sampleEnd - 1) >> tileSampleShift;
			int firstTileX = tx + 1; // 这个 sample 区间从这个 tile 的下一列开始完全位于边的右侧
			Qk_ASSERT(firstTileX > 0, "firstTileX should be positive");

			for (int ty = ty0; ty <= ty1; ty++) {
				int tileSampleBegin = ty << tileSampleShift;
				int localBegin = I32::max(sampleBegin - tileSampleBegin, 0);
				int localEnd = I32::min(sampleEnd - tileSampleBegin, tileSampleCount);
				uint32_t tileRowOffset = ty * out.tileCountX;
				if (tx >= 0 && tx < out.tileCountX) {
					// 当前 tile 内，只有这个 local sample 区间仍需要 GPU
					// 使用原始边做精确 X 交点测试；tile 内其他 sample 不测试此边。
					ComputeAATileEdge tileEdge = {
						edgeIndex,
						uint16_t(localBegin),
						uint16_t(localEnd),
					};
					scratch[tileRowOffset + tx].edges.push(tileEdge);
				}
				if (firstTileX < out.tileCountX) {
					// 这个 sample 区间从下一列开始已经完全位于边的右侧。
					// CPU 只记录一个 span，不展开写入后面所有 tile。
					ComputeAABackdropEvent event = {
						uint32_t(firstTileX),
						uint16_t(localBegin),
						uint16_t(localEnd),
						winding,
					};
					backdropScratch[ty].events.push(event);
				}
			}
		};

		// 将 tile 当作粗像素沿 X 扫描。只有 Y sample 网格位置发生变化时，
		// 才为当前 tile 列添加边和为右侧 tile 添加 backdrop。
		for (uint32_t edgeIndex = 0; edgeIndex < out.edges.length(); edgeIndex++) {
			auto &edge = out.edges[edgeIndex];
			Vec2 left = edge.p0, right = edge.p1;
			if (left.x() > right.x()) {
				std::swap(left, right);
			}
			Qk_ASSERT(left.x() >= 0.0f, "edge should not have negative x coordinate");
			Qk_ASSERT(edge.minY >= 0.0f, "edge should not have negative y coordinate");

			float dx = right.x() - left.x();
			float lastSampleY = left.y() * sampleGrid;
			int lastSampleGridY = sample_grid_y(lastSampleY);

			if (dx == 0.0f) {
				// 竖边不需要沿 X 推进：边左侧 tile 做局部精确测试，
				// 边右侧 tile 从对应列开始继承 backdrop。
				int sampleGridY = sample_grid_y(right.y() * sampleGrid);
				if (lastSampleGridY != sampleGridY) {
					int tx = right.x() * invTileSize - 0.000001f; // 左闭右开
					add_sample_range(edgeIndex, tx, lastSampleGridY, sampleGridY, edge.winding);
				}
				continue;
			}

			// 向下取整，做为当前 tile 的 X 索引
			int tx = left.x() * invTileSize;
			// 右端点使用半开 X tile 范围：端点正好落在 tile 边界时，最后负责此边的仍是边界左侧 tile。
			int finalTx = right.x() * invTileSize - 0.000001f; // 左闭右开
			if (tx < finalTx) {
				float sampleSlope = (right.y() - left.y()) * sampleGrid / dx;
				float tileSampleYStep = sampleSlope * kComputeAATileSize;
				float firstTileRight = (tx + 1) * kComputeAATileSize;
				float nextSampleY = lastSampleY + (firstTileRight - left.x()) * sampleSlope;
				do {
					// 每次推进一个完整 X tile。若离散 Y sample 没有变化，
					// 说明当前列没有跨过任何扫描线，不生成任何 CPU/GPU 数据。
					int sampleGridY = sample_grid_y(nextSampleY);
					if (lastSampleGridY != sampleGridY) {
						add_sample_range(edgeIndex, tx, lastSampleGridY, sampleGridY, edge.winding);
						lastSampleGridY = sampleGridY;
					}
					// 推进 y sample grid 步
					nextSampleY += tileSampleYStep;
					tx++; // 推进一个 X tile
				} while (tx < finalTx);
			}

			// 最后一个 X tile 通常不足一个完整 tile 宽，直接使用右端点，
			// 避免增量浮点误差改变端点所属的半开 sample 区间。
			int sampleGridY = sample_grid_y(right.y() * sampleGrid);
			if (lastSampleGridY != sampleGridY) {
				add_sample_range(edgeIndex, tx, lastSampleGridY, sampleGridY, edge.winding);
			}
		}

		out.backdropRows.reset(out.tileCountY);
		// 临时分桶数据压平为连续数组，供 Metal buffer 直接上传。
		for (uint32_t ty = 0; ty < out.tileCountY; ty++) {
			uint32_t tileRowOffset = ty * out.tileCountX;
			for (uint32_t tx = 0; tx < out.tileCountX; tx++) {
				auto &tile = out.tiles[tileRowOffset + tx];
				auto &src = scratch[tileRowOffset + tx].edges;
				tile.edgeOffset = out.tileEdges.length();
				tile.edgeCount = src.length();
				tile.originX = tx * kComputeAATileSize;
				tile.originY = ty * kComputeAATileSize;
				out.tileEdges.write(src.val(), src.length());
			}
			auto &row = out.backdropRows[ty];
			auto &events = backdropScratch[ty].events;
			row.eventOffset = out.backdropEvents.length();
			row.eventCount = events.length();
			out.backdropEvents.write(events.val(), events.length());
		}
	}

	ComputeAADrawData MetalComputeAAPrototype::buildDrawData(const Path &path,
		const Mat &viewMatrix,
		Vec2 atlasPadding,
		float flattenPrecision)
	{
		ComputeAADrawData out;
		Path transformPath(path); // copy
		transformPath.transform(viewMatrix);
		auto lines = transformPath.getEdgeLines(flattenPrecision);
		if (!lines.length())
			return out;

		out.bounds = transformPath.getBounds();
		out.atlasOrigin = (out.bounds.begin-atlasPadding).floor();
		Vec2 atlasEnd = (out.bounds.end+atlasPadding).ceil();
		out.atlasSize = atlasEnd - out.atlasOrigin;
		out.tileCountX = (ceil_to_u32(out.atlasSize.x()) + kComputeAATileSize - 1) / kComputeAATileSize;
		out.tileCountY = (ceil_to_u32(out.atlasSize.y()) + kComputeAATileSize - 1) / kComputeAATileSize;

		for (uint32_t i = 1; i < lines.length(); i += 2) {
			append_edge(out, lines[i-1], lines[i]);
		}

		build_tile_edges(out);
		return out;
	}

	bool MetalComputeAAPrototype::encodeCoverage(id<MTLDevice> device,
		id<MTLCommandBuffer> commandBuffer,
		id<MTLComputePipelineState> coveragePipeline,
		id<MTLTexture> coverageTexture,
		const ComputeAADrawData &drawData,
		ComputeAAFillRule fillRule)
	{
		if (!device || !commandBuffer || !coveragePipeline || !coverageTexture ||
			!drawData.edges.length() || !drawData.tiles.length())
			return false;

		auto edgeBytes = drawData.edges.length() * sizeof(ComputeAAEdge);
		auto indexBytes = drawData.tileEdges.length() * sizeof(ComputeAATileEdge);
		auto tileBytes = drawData.tiles.length() * sizeof(ComputeAATile);
		auto backdropEventBytes =
			drawData.backdropEvents.length() * sizeof(ComputeAABackdropEvent);
		auto backdropRowBytes =
			drawData.backdropRows.length() * sizeof(ComputeAABackdropRow);
		id<MTLBuffer> edges = [device newBufferWithBytes:drawData.edges.val()
				length:edgeBytes options:MTLResourceStorageModeShared];
		id<MTLBuffer> indices = indexBytes ?
			[device newBufferWithBytes:drawData.tileEdges.val()
				length:indexBytes options:MTLResourceStorageModeShared]:
			[device newBufferWithLength:sizeof(ComputeAATileEdge)
				options:MTLResourceStorageModeShared];
		id<MTLBuffer> tiles = [device newBufferWithBytes:drawData.tiles.val()
				length:tileBytes options:MTLResourceStorageModeShared];
		id<MTLBuffer> backdropEvents = backdropEventBytes ?
			[device newBufferWithBytes:drawData.backdropEvents.val()
				length:backdropEventBytes options:MTLResourceStorageModeShared]:
			[device newBufferWithLength:sizeof(ComputeAABackdropEvent)
				options:MTLResourceStorageModeShared];
		id<MTLBuffer> backdropRows = [device newBufferWithBytes:drawData.backdropRows.val()
				length:backdropRowBytes options:MTLResourceStorageModeShared];

		ComputeAAParams params = {};
		params.width = uint32_t(drawData.atlasSize.x());
		params.height = uint32_t(drawData.atlasSize.y());
		params.tileCountX = drawData.tileCountX;
		params.tileCountY = drawData.tileCountY;
		params.fillRule = fillRule;
		params.sampleGrid = kComputeAASampleGrid;

		auto enc = [commandBuffer computeCommandEncoder];
		[enc setComputePipelineState:coveragePipeline];
		[enc setBytes:&params length:sizeof(params) atIndex:0];
		[enc setBuffer:edges offset:0 atIndex:1];
		[enc setBuffer:indices offset:0 atIndex:2];
		[enc setBuffer:tiles offset:0 atIndex:3];
		[enc setBuffer:backdropEvents offset:0 atIndex:4];
		[enc setBuffer:backdropRows offset:0 atIndex:5];
		[enc setTexture:coverageTexture atIndex:0];

		MTLSize tg = MTLSizeMake(kComputeAATileSize, kComputeAATileSize, 1);
		MTLSize groups = MTLSizeMake(params.tileCountX, params.tileCountY, 1);
		[enc dispatchThreadgroups:groups threadsPerThreadgroup:tg];
		[enc endEncoding];
		return true;
	}

	bool MetalComputeAAPrototype::encodeSolidComposite(id<MTLCommandBuffer> commandBuffer,
		id<MTLComputePipelineState> compositePipeline,
		id<MTLTexture> coverageTexture,
		id<MTLTexture> colorTexture,
		Vec4 premulColor,
		Vec2 outputOrigin,
		const ComputeAADrawData &drawData,
		ComputeAAFillRule fillRule)
	{
		if (!commandBuffer || !compositePipeline || !coverageTexture || !colorTexture)
			return false;

		ComputeAAParams params = {};
		params.width = uint32_t(drawData.atlasSize.x());
		params.height = uint32_t(drawData.atlasSize.y());
		params.tileCountX = drawData.tileCountX;
		params.tileCountY = drawData.tileCountY;
		params.fillRule = fillRule;
		params.sampleGrid = kComputeAASampleGrid;
		params.outputOriginX = uint32_t(outputOrigin.x());
		params.outputOriginY = uint32_t(outputOrigin.y());
		params.color = premulColor;

		auto enc = [commandBuffer computeCommandEncoder];
		[enc setComputePipelineState:compositePipeline];
		[enc setBytes:&params length:sizeof(params) atIndex:0];
		[enc setTexture:coverageTexture atIndex:0];
		[enc setTexture:colorTexture atIndex:1];
		MTLSize tg = MTLSizeMake(kComputeAATileSize, kComputeAATileSize, 1);
		MTLSize grid = MTLSizeMake(params.width, params.height, 1);
		[enc dispatchThreads:grid threadsPerThreadgroup:tg];
		[enc endEncoding];
		return true;
	}

} // namespace qk
