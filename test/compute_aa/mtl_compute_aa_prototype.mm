/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

// Prototype only. This file is not wired into the current Metal backend.

#import "./mtl_compute_aa_prototype.h"
#include "src/render/math.h"
#include "src/util/object.h"

#include <math.h>
#include <string.h>

namespace qk {

	static bool clip_boundary_line(Vec2 &p0, Vec2 &p1, Vec2 size) {
		float t0 = 0.0f, t1 = 1.0f;
		Vec2 d = p1 - p0;
		auto clip = [&](float p, float q) {
			if (p == 0.0f)
				return q >= 0.0f;
			float r = q / p;
			if (p < 0.0f) {
				if (r > t1) return false;
				t0 = std::max(t0, r);
			} else {
				if (r < t0) return false;
				t1 = std::min(t1, r);
			}
			return true;
		};
		if (!clip(-d.x(), p0.x()) || !clip(d.x(), size.x() - p0.x()) ||
			!clip(-d.y(), p0.y()) || !clip(d.y(), size.y() - p0.y()))
			return false;
		Vec2 start = p0;
		p0 = start + d * t0;
		p1 = start + d * t1;
		return true;
	}

	static void mark_boundary_tiles(ComputeAADrawData &out, const Array<Vec2> &lines,
		Array<uint8_t> &boundary)
	{
		boundary.reset(out.tileCountX * out.tileCountY);
		memset(boundary.val(), 0, boundary.length());
		Vec2 size = out.atlasSize;
		auto mark = [&](int tx, int ty) {
			if (tx >= 0 && tx < int(out.tileCountX) &&
				ty >= 0 && ty < int(out.tileCountY))
				boundary[ty * out.tileCountX + tx] = 1;
		};
		auto tile_x = [&](float x) {
			return std::min(int(x / kComputeAATileSize), int(out.tileCountX) - 1);
		};
		auto tile_y = [&](float y) {
			return std::min(int(y / kComputeAATileSize), int(out.tileCountY) - 1);
		};

		for (uint32_t i = 1; i < lines.length(); i += 2) {
			Vec2 p0 = lines[i-1] - out.bounds.begin;
			Vec2 p1 = lines[i] - out.bounds.begin;
			if (!clip_boundary_line(p0, p1, size))
				continue;

			float dx = p1.x() - p0.x();
			float dy = p1.y() - p0.y();
			if (dy == 0.0f) {
				int ty = tile_y(p0.y());
				int tx0 = tile_x(std::min(p0.x(), p1.x()));
				int tx1 = tile_x(std::max(p0.x(), p1.x()));
				bool onTileBoundary = fmodf(p0.y(), float(kComputeAATileSize)) == 0.0f;
				for (int tx = tx0; tx <= tx1; tx++) {
					mark(tx, ty);
					if (onTileBoundary)
						mark(tx, ty - 1);
				}
				continue;
			}
			if (dx == 0.0f) {
				int tx = tile_x(p0.x());
				int ty0 = tile_y(std::min(p0.y(), p1.y()));
				int ty1 = tile_y(std::max(p0.y(), p1.y()));
				bool onTileBoundary = fmodf(p0.x(), float(kComputeAATileSize)) == 0.0f;
				for (int ty = ty0; ty <= ty1; ty++) {
					mark(tx, ty);
					if (onTileBoundary)
						mark(tx - 1, ty);
				}
				continue;
			}

			int tx = tile_x(p0.x()), ty = tile_y(p0.y());
			int endTx = tile_x(p1.x()), endTy = tile_y(p1.y());
			int stepX = dx > 0.0f ? 1 : -1;
			int stepY = dy > 0.0f ? 1 : -1;
			float nextX = float((stepX > 0 ? tx + 1 : tx) * kComputeAATileSize);
			float nextY = float((stepY > 0 ? ty + 1 : ty) * kComputeAATileSize);
			float tMaxX = (nextX - p0.x()) / dx;
			float tMaxY = (nextY - p0.y()) / dy;
			float tDeltaX = float(kComputeAATileSize) / fabsf(dx);
			float tDeltaY = float(kComputeAATileSize) / fabsf(dy);
			mark(tx, ty);
			while (tx != endTx || ty != endTy) {
				if (tMaxX < tMaxY) {
					tx += stepX;
					tMaxX += tDeltaX;
				} else if (tMaxY < tMaxX) {
					ty += stepY;
					tMaxY += tDeltaY;
				} else {
					tx += stepX;
					ty += stepY;
					tMaxX += tDeltaX;
					tMaxY += tDeltaY;
				}
				mark(tx, ty);
			}
		}
	}

	template<bool NeedClip>
	static void append_edges(ComputeAADrawData &out, Array<Vec2> &lines) {
		auto begin = out.bounds.begin,
				end = out.bounds.end;
		for (uint32_t i = 1; i < lines.length(); i += 2) {
			auto p0 = lines[i-1], p1 = lines[i];
			if (p0.y() == p1.y()) // x扫描线算法，水平边不计入边列表
				continue;
			// 需要裁剪边界外的线段，先进行一次粗略的 CPU 裁剪，减少后续 GPU 处理的边数,
			// 无法处理左边界为负的情况，因为x扫描线算法需要统计tile左边的backdrop值。
			if (NeedClip) {
				// 整条边都在bounds上方的无效区域，不生成任何数据
				if (p0.y() <= begin.y() && p1.y() <= begin.y())
					continue;
				// 整条边都在bounds下方的无效区域，不生成任何数据
				if (p0.y() >= end.y() && p1.y() >= end.y())
					continue;
				// 整条边都在bounds右侧的无效区域，不生成任何数据
				if (p0.x() >= end.x() && p1.x() >= end.x())
					continue;
			}
			p0 -= begin;
			p1 -= begin;
			// 屏幕坐标 y-down：向下为正 winding，向上为负 winding。
			int32_t winding = p1.y() > p0.y() ? 1 : -1;
			float dxdy = (p1.x() - p0.x()) / (p1.y() - p0.y());
			out.edges.push({p0,p1,dxdy,winding,{0,0}});
		}
	}

	struct TileScratch {
		Array<ComputeAATileEdge> edges;
	};
	struct BackdropRowScratch {
		Array<ComputeAABackdropEvent> events;
	};
	struct TileScratchs: Array<TileScratch> {
		~TileScratchs() {
			Qk_ASSERT(_ptr.allocator->isLinearAllocator(), "TileScratchs should use a linear allocator");
			// 由于目前的实现是线性分配器，直接丢弃整个数组即可，无需逐个调用析构函数。
			_ptr.extra = 0;
		}
	};
	struct BackdropRowScratchs: Array<BackdropRowScratch> {
		~BackdropRowScratchs() {
			// 由于目前的实现是线性分配器，直接丢弃整个数组即可，无需逐个调用析构函数。
			_ptr.extra = 0;
		}
	};

	static void build_tile_edges(ComputeAADrawData &out, const Array<uint8_t> &boundary) {
		// 临时数组按 tile / tile-row 分桶，最后再压平为连续 GPU buffer。
		// 这里的小 Array 分配必需使用线性分配器，后续直接丢弃整个数组，无需逐个调用析构函数。
		TileScratchs scratch;
		scratch.reset(out.tileCountX * out.tileCountY);
		BackdropRowScratchs backdropScratch;
		backdropScratch.reset(out.tileCountY);
		out.tiles.reset(out.tileCountX * out.tileCountY);

		const int sampleGrid = kComputeAASampleGrid;
		const int tileSampleCount = kComputeAATileSize * sampleGrid;
		const float invTileSize = 1.0f / float(kComputeAATileSize);
		static_assert(
			(kComputeAATileSize & (kComputeAATileSize - 1)) == 0 &&
			(kComputeAASampleGrid & (kComputeAASampleGrid - 1)) == 0, "Compute AA tile/sample sizes must be powers of two"
		);
		// tileSampleShift 是 tile/sample 数量的二进制位数，用于整数转换代替除法。log2(16*4) = 6
		const int tileSampleShift = __builtin_ctz(tileSampleCount);

		auto sample_grid_y = [](float sampleY) {
			// 直接使用 ceilf 来计算 sample 网格索引，确保 sampleY 落在 [n+0.5, n+1.5) 区间时得到正确的 n+1 索引。
			int y = ceilf(sampleY - 0.5f);
			return y;
		};

		auto add_sample_range = [&](uint32_t edgeIndex, int tx,
			int sampleBegin, int sampleEnd, int32_t winding)
		{
			Qk_ASSERT(tx < out.tileCountX, "tile x index out of bounds");
			Qk_ASSERT(sampleBegin != sampleEnd, "should not add empty sample range");
			if (sampleBegin > sampleEnd) {
				std::swap(sampleBegin, sampleEnd); // 确保 sampleBegin < sampleEnd
			}
			// [sampleBegin, sampleEnd) 是边在当前 X tile 步骤中新跨过的全局
			// Y sample 区间。它可能跨越多个 yTile，因此先按 yTile 拆分。
			int tileY0 = sampleBegin >> tileSampleShift; // sampleBegin / tileSampleCount
			int tileY1 = (sampleEnd - 1) >> tileSampleShift;
			int firstTileX = std::max(0, tx + 1); // 这个 sample 区间从这个 tile 的下一列开始完全位于边的右侧
			int tileRowOffset = tileY0 * out.tileCountX; // 当前 tile 行的起始偏移

			for (int ty = tileY0; ty <= tileY1; ty++, tileRowOffset += out.tileCountX) {
				int tileSampleBegin = ty << tileSampleShift; // 当前 tile 的 Y sample 网格起点
				int localBegin = std::max(sampleBegin - tileSampleBegin, 0); // 限制在 tile 内的局部 sample 区间
				int localEnd = std::min(sampleEnd - tileSampleBegin, tileSampleCount);
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

		auto add_sample_range_final = [&](uint32_t edgeIndex, int tx,
			int sampleBegin, float rightY, int32_t winding)
		{
			if (tx < out.tileCountX) {
				int sampleEnd = sample_grid_y(rightY * sampleGrid);
				if (sampleBegin != sampleEnd) {
					add_sample_range(edgeIndex, tx, sampleBegin, sampleEnd, winding);
				}
			}
		};

		int edgeIndex = 0;
		// 将 tile 当作粗像素沿 X 扫描。只有 Y sample 网格位置发生变化时，
		// 才为当前 tile 列添加边和为右侧 tile 添加 backdrop。
		for (auto &edge : out.edges) {
			Vec2 left = edge.p0, right = edge.p1;
			if (left.x() > right.x()) {
				std::swap(left, right);
			}

			float dx = right.x() - left.x();
			float lastSampleY = left.y() * sampleGrid;
			int lastSampleGridY = sample_grid_y(lastSampleY);

			if (dx == 0.0f) {
				// 竖边不需要沿 X 推进：边左侧 tile 做局部精确测试，
				// 边右侧 tile 从对应列开始继承 backdrop。
				int tx = right.x() * invTileSize - 0.000001f; // 左闭右开
				add_sample_range_final(edgeIndex++, tx, lastSampleGridY, right.y(), edge.winding);
				continue;
			}

			// 向下取整，做为当前 tile 的 X 索引
			int tx = left.x() * invTileSize;
			// 右端点使用半开 X tile 范围：端点正好落在 tile 边界时，最后负责此边的仍是边界左侧 tile。
			int finalTx = I32::min(right.x() * invTileSize - 0.000001f, out.tileCountX); // 左闭右开
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
			add_sample_range_final(edgeIndex, tx, lastSampleGridY, right.y(), edge.winding);
			edgeIndex++;
		}

		out.backdropRows.reset(out.tileCountY);
		out.boundaryTileIndices.clear();
		out.uniformTiles.clear();
		// 临时分桶数据压平为连续数组，供 Metal buffer 直接上传。
		uint32_t tileRowOffset = 0;
		for (uint32_t ty = 0, originY = 0; ty < out.tileCountY; ty++) {
			Array<int32_t> tileWinding;
			tileWinding.reset(out.tileCountX + 1);
			memset(tileWinding.val(), 0, tileWinding.length() * sizeof(int32_t));
			const int representativeSample = tileSampleCount >> 1;
			for (auto &event : backdropScratch[ty].events) {
				if (representativeSample >= event.sampleBegin &&
					representativeSample < event.sampleEnd)
					tileWinding[event.firstTileX] += event.winding;
			}
			int32_t winding = 0;
			for (uint32_t tx = 0, originX = 0; tx < out.tileCountX; tx++) {
				uint32_t tileIndex = tileRowOffset + tx;
				auto &tile = out.tiles[tileIndex];
				auto &src = scratch[tileRowOffset + tx].edges;
				out.tiles[tileRowOffset + tx] = {
					.edgeOffset=out.tileEdges.length(),
					.edgeCount=src.length(),
					.originX=originX,
					.originY=originY
				};
				out.tileEdges.write(src.val(), src.length());
				winding += tileWinding[tx];
				if (boundary[tileIndex]) {
					out.boundaryTileIndices.push(tileIndex);
				} else {
					Qk_ASSERT(!src.length(), "uniform tile should not contain coverage edges");
					out.uniformTiles.push({originX, originY, winding, 0});
				}
				originX += kComputeAATileSize;
			}
			auto &events = backdropScratch[ty].events;
			out.backdropRows[ty] = {
				.eventOffset=out.backdropEvents.length(),
				.eventCount=events.length()
			};
			out.backdropEvents.write(events.val(), events.length());
			tileRowOffset += out.tileCountX;
			originY += kComputeAATileSize;
		}
	}

	ComputeAADrawData MetalComputeAAPrototype::buildDrawData(const Path &path,
		const Mat &viewMatrix,
		float flattenPrecision)
	{
		ComputeAADrawData out;
		Path transformPath(path); // copy
		transformPath.transform(viewMatrix);
		auto lines = transformPath.getEdgeLines(flattenPrecision);
		if (!lines.length())
			return out;

		// 使用实际参与 coverage 的扁平化边计算边界。Path::getBounds()
		// 会把 Bézier 控制点也计入范围，可能产生固定的大块空白 atlas。
		auto bounds = Path::getBoundsFromPoints(lines.val(), lines.length()).expandToInteger();
		// 目前先限制为非负坐标，如果有裁剪参数使用参数进行更灵活的边界控制。
		out.bounds = bounds.clip({0, bounds.end});
		out.atlasOrigin = Vec2(); // 目前直接在原点处生成 atlas，后续可根据实际边界进行更紧凑的布局
		out.atlasSize = out.bounds.size(); // 目前直接使用 bounds 大小作为 atlas 大小
		out.tileCountX = ceilf(out.atlasSize.x()/kComputeAATileSize);
		out.tileCountY = ceilf(out.atlasSize.y()/kComputeAATileSize);
		Array<uint8_t> boundary;
		mark_boundary_tiles(out, lines, boundary);

		if (bounds.begin.y() < out.bounds.begin.y() || bounds.end.y() > out.bounds.end.y()
			|| bounds.end.x() > out.bounds.end.x()
		) {
			append_edges<true>(out, lines); // 线段可能在边界外，需要裁剪
		} else { // 所有线段都在边界内，无需裁剪，直接添加
			append_edges<false>(out, lines);
		}
		build_tile_edges(out, boundary);
		return out;
	}

	bool MetalComputeAAPrototype::encodeCoverage(id<MTLDevice> device,
		id<MTLCommandBuffer> commandBuffer,
		id<MTLComputePipelineState> uniformPipeline,
		id<MTLComputePipelineState> coveragePipeline,
		id<MTLTexture> coverageTexture,
		const ComputeAADrawData &drawData,
		ComputeAAFillRule fillRule)
	{
		if (!device || !commandBuffer || !uniformPipeline || !coveragePipeline || !coverageTexture ||
			!drawData.edges.length() || !drawData.tiles.length())
			return false;

		auto edgeBytes = drawData.edges.length() * sizeof(ComputeAAEdge);
		auto indexBytes = drawData.tileEdges.length() * sizeof(ComputeAATileEdge);
		auto tileBytes = drawData.tiles.length() * sizeof(ComputeAATile);
		auto backdropEventBytes =
			drawData.backdropEvents.length() * sizeof(ComputeAABackdropEvent);
		auto backdropRowBytes =
			drawData.backdropRows.length() * sizeof(ComputeAABackdropRow);
		auto boundaryTileIndexBytes =
			drawData.boundaryTileIndices.length() * sizeof(uint32_t);
		auto uniformTileBytes =
			drawData.uniformTiles.length() * sizeof(ComputeAAUniformTile);
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
		id<MTLBuffer> boundaryTileIndices = boundaryTileIndexBytes ?
			[device newBufferWithBytes:drawData.boundaryTileIndices.val()
				length:boundaryTileIndexBytes options:MTLResourceStorageModeShared]:
			[device newBufferWithLength:sizeof(uint32_t)
				options:MTLResourceStorageModeShared];
		id<MTLBuffer> uniformTiles = uniformTileBytes ?
			[device newBufferWithBytes:drawData.uniformTiles.val()
				length:uniformTileBytes options:MTLResourceStorageModeShared]:
			[device newBufferWithLength:sizeof(ComputeAAUniformTile)
				options:MTLResourceStorageModeShared];

		ComputeAAParams params = {};
		params.width = uint32_t(drawData.atlasSize.x());
		params.height = uint32_t(drawData.atlasSize.y());
		params.tileCountX = drawData.tileCountX;
		params.tileCountY = drawData.tileCountY;
		params.fillRule = fillRule;
		params.sampleGrid = kComputeAASampleGrid;

		MTLSize tg = MTLSizeMake(kComputeAATileSize * kComputeAASampleGrid, 1, 1);
		if (drawData.uniformTiles.length()) {
			auto enc = [commandBuffer computeCommandEncoder];
			enc.label = @"Compute AA Uniform Tiles";
			[enc setComputePipelineState:uniformPipeline];
			[enc setBytes:&params length:sizeof(params) atIndex:0];
			[enc setBuffer:uniformTiles offset:0 atIndex:1];
			[enc setTexture:coverageTexture atIndex:0];
			[enc dispatchThreadgroups:MTLSizeMake(drawData.uniformTiles.length(), 1, 1)
				threadsPerThreadgroup:tg];
			[enc endEncoding];
		}
		if (drawData.boundaryTileIndices.length()) {
			auto enc = [commandBuffer computeCommandEncoder];
			enc.label = @"Compute AA Boundary Coverage";
			[enc setComputePipelineState:coveragePipeline];
			[enc setBytes:&params length:sizeof(params) atIndex:0];
			[enc setBuffer:edges offset:0 atIndex:1];
			[enc setBuffer:indices offset:0 atIndex:2];
			[enc setBuffer:tiles offset:0 atIndex:3];
			[enc setBuffer:backdropEvents offset:0 atIndex:4];
			[enc setBuffer:backdropRows offset:0 atIndex:5];
			[enc setBuffer:boundaryTileIndices offset:0 atIndex:6];
			[enc setTexture:coverageTexture atIndex:0];
			[enc dispatchThreadgroups:MTLSizeMake(drawData.boundaryTileIndices.length(), 1, 1)
				threadsPerThreadgroup:tg];
			[enc endEncoding];
		}
		return true;
	}

	bool MetalComputeAAPrototype::encodeSolidComposite(id<MTLCommandBuffer> commandBuffer,
		id<MTLRenderPipelineState> compositePipeline,
		id<MTLTexture> coverageTexture,
		id<MTLTexture> colorTexture,
		Vec4 clearColor,
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
		params.outputWidth = uint32_t(colorTexture.width);
		params.outputHeight = uint32_t(colorTexture.height);
		params.color = premulColor;

		MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
		pass.colorAttachments[0].texture = colorTexture;
		pass.colorAttachments[0].loadAction = MTLLoadActionClear;
		pass.colorAttachments[0].storeAction = MTLStoreActionStore;
		pass.colorAttachments[0].clearColor = MTLClearColorMake(
			clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
		auto enc = [commandBuffer renderCommandEncoderWithDescriptor:pass];
		enc.label = @"Compute AA Composite";
		[enc setRenderPipelineState:compositePipeline];
		[enc setVertexBytes:&params length:sizeof(params) atIndex:0];
		[enc setFragmentBytes:&params length:sizeof(params) atIndex:0];
		[enc setFragmentTexture:coverageTexture atIndex:0];
		[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
		[enc endEncoding];
		return true;
	}

} // namespace qk
