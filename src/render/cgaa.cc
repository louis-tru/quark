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

#include "./cgaa.h"
#include "src/util/macros.h"

namespace qk {

	template<bool NeedClip>
	static void append_edges(CGAADrawData &out, Array<Vec2> &lines) {
		auto begin = out.bounds.begin,
				end = out.bounds.end;
		for (uint32_t i = 1; i < lines.length(); i += 2) {
			auto p0 = lines[i-1], p1 = lines[i];
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
			// 水平边使用 winding=0，只参与 boundary tile 标记。
			int32_t winding = p1.y() > p0.y() ? 1 : p1.y() < p0.y() ? -1 : 0;
			float invDy = winding ? 1.0f / (p1.y() - p0.y()) : 0.0f;
			float dxdy = (p1.x() - p0.x()) * invDy;
			out.edges.push({p0,p1,dxdy,winding,{0,0}});
		}
	}

	struct TileScratch {
		Array<CGAATileEdge> edges;
		bool boundary = false;
	};
	struct BackdropRowScratch {
		Array<CGAABackdropEvent> events;
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

	static void build_tile_edges(CGAADrawData &out) {
		// 临时数组按 tile / tile-row 分桶，最后再压平为连续 GPU buffer。
		// 这里的小 Array 分配必需使用线性分配器，后续直接丢弃整个数组，无需逐个调用析构函数。
		TileScratchs scratch;
		scratch.reset(out.tileCountX * out.tileCountY);
		BackdropRowScratchs backdropScratch;
		backdropScratch.reset(out.tileCountY);

		const int sampleGrid = kCGAASampleGrid;
		const int tileSampleCount = kCGAATileSize * sampleGrid;
		const int totalSampleCountY = out.tileCountY * tileSampleCount;
		const float invTileSize = 1.0f / float(kCGAATileSize);
		static_assert(
			(kCGAATileSize & (kCGAATileSize - 1)) == 0 &&
			(kCGAASampleGrid & (kCGAASampleGrid - 1)) == 0, "Compute AA tile/sample sizes must be powers of two"
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
			if (sampleBegin > sampleEnd) {
				std::swap(sampleBegin, sampleEnd); // 确保 sampleBegin < sampleEnd
			}
			if (sampleEnd <= 0) {
				return; // 整个 sample 区间在 tile 上方，无需处理
			}
			if (sampleBegin >= totalSampleCountY) {
				return; // 整个 sample 区间在 tile 下方，无需处理
			}
			// [sampleBegin, sampleEnd) 是边在当前 X tile 步骤中新跨过的全局
			// Y sample 区间。它可能跨越多个 yTile，因此先按 yTile 拆分。
			int tileY0 = sampleBegin >> tileSampleShift; // sampleBegin / tileSampleCount
			int tileRowOffset = tileY0 * out.tileCountX; // 当前 tile 行的起始偏移
			if (sampleBegin == sampleEnd) {
				if (tx >= 0)
					scratch[tileRowOffset + tx].boundary = true; // 这个 tile 内有一个 sample 位于边界上。
				return;
			}
			int tileY1 = (sampleEnd - 1) >> tileSampleShift;
			int firstTileX = std::max(0, tx + 1); // 这个 sample 区间从这个 tile 的下一列开始完全位于边的右侧

			for (int ty = tileY0; ty <= tileY1; ty++, tileRowOffset += out.tileCountX) {
				int tileSampleBegin = ty << tileSampleShift; // 当前 tile 的 Y sample 网格起点
				int localBegin = std::max(sampleBegin - tileSampleBegin, 0); // 限制在 tile 内的局部 sample 区间
				int localEnd = std::min(sampleEnd - tileSampleBegin, tileSampleCount);
				if (tx >= 0) {
					// 当前 tile 内，只有这个 local sample 区间仍需要 GPU
					// 使用原始边做精确 X 交点测试；tile 内其他 sample 不测试此边。
					CGAATileEdge tileEdge = {
						edgeIndex,
						uint16_t(localBegin),
						uint16_t(localEnd),
					};
					scratch[tileRowOffset + tx].edges.push(tileEdge);
					scratch[tileRowOffset + tx].boundary = true; // 标记为 boundary tile，GPU 需要测试边界交点
				}
				if (firstTileX < out.tileCountX) {
					// 这个 sample 区间从下一列开始已经完全位于边的右侧。
					// CPU 只记录一个 span，不展开写入后面所有 tile。
					CGAABackdropEvent event = {
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
				add_sample_range(edgeIndex, tx, sampleBegin, sampleEnd, winding);
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
				float tileSampleYStep = sampleSlope * kCGAATileSize;
				float firstTileRight = (tx + 1) * kCGAATileSize;
				float nextSampleY = lastSampleY + (firstTileRight - left.x()) * sampleSlope;
				do {
					// 每次推进一个完整 X tile。若离散 Y sample 没有变化，
					// 说明当前列没有跨过任何扫描线，不生成任何 CPU/GPU 数据。
					int sampleGridY = sample_grid_y(nextSampleY);
					add_sample_range(edgeIndex, tx, lastSampleGridY, sampleGridY, edge.winding);
					lastSampleGridY = sampleGridY;
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
		out.boundaryTiles.clear();
		out.uniformTiles.clear();
		// 临时分桶数据压平为连续数组，供 Metal buffer 直接上传。
		uint32_t tileRowOffset = 0;
		for (uint32_t ty = 0, originY = 0; ty < out.tileCountY; ty++) {
			Array<int32_t> tileWinding(out.tileCountX);
			memset(tileWinding.val(), 0, tileWinding.size());
			for (auto &event : backdropScratch[ty].events) {
				// Uniform tile 内所有 Y sample 的 winding 相同，只需统计第一个。
				if (event.sampleBegin == 0)
					tileWinding[event.firstTileX] += event.winding;
			}
			int32_t winding = 0;
			for (uint32_t tx = 0, originX = 0; tx < out.tileCountX; tx++) {
				uint32_t tileIndex = tileRowOffset + tx;
				auto &edges = scratch[tileIndex].edges;
				winding += tileWinding[tx];
				if (scratch[tileIndex].boundary) {
					out.boundaryTiles.push({
						.originX=originX,
						.originY=originY,
						.edgeOffset=out.tileEdges.length(),
						.edgeCount=edges.length()
					});
					out.tileEdges.write(edges.val(), edges.length());
				} else {
					Qk_ASSERT(!edges.length(), "uniform tile should not contain coverage edges");
					out.uniformTiles.push({originX, originY, winding, 0});
				}
				originX += kCGAATileSize;
			}
			auto &events = backdropScratch[ty].events;
			out.backdropRows[ty] = {
				.eventOffset=out.backdropEvents.length(),
				.eventCount=events.length()
			};
			out.backdropEvents.write(events.val(), events.length());
			tileRowOffset += out.tileCountX;
			originY += kCGAATileSize;
		}
	}

	CGAADrawData buildCGAADrawData(const Path &path, Range *clip, const Mat *mat, float precision)
	{
		CGAADrawData out;
		auto lines = path.getEdgeLines(precision, mat);
		if (!lines.length())
			Qk_ReturnLocal(out);
		auto bounds = Path::getBoundsFromPoints(lines.val(), lines.length()).expandToInteger();
		// clip 是可选的，允许调用方限制 atlas 的有效区域，超出部分会被裁剪掉并且不参与后续 GPU 处理。
		Qk_ASSERT(!clip && clip->begin.max(0) == clip->begin, "clip should have non-negative origin");
		Qk_ASSERT(!clip && clip->expandToInteger() == *clip, "clip should have integer bounds");
		out.bounds = bounds.clip(clip ? *clip : Range{0, bounds.end});
		out.atlasOrigin = out.bounds.begin; // 使用 bounds 的左上角作为 atlas 原点,因为目前直接绘制到屏幕上。
		out.atlasSize = out.bounds.end; // 使用 bounds 的右下角作为 atlas 尺寸，这里主要是限制tile向右下扩展时多出的边界空间。
		constexpr float invTileSize = 1.0f / float(kCGAATileSize);
		auto size = out.bounds.size();
		out.tileCountX = ceilf(size.x() * invTileSize);
		out.tileCountY = ceilf(size.y() * invTileSize);

		if (bounds.begin.y() < out.bounds.begin.y() || bounds.end.y() > out.bounds.end.y()
			|| bounds.end.x() > out.bounds.end.x()
		) {
			append_edges<true>(out, lines); // 线段可能在边界外，需要裁剪
		} else { // 所有线段都在边界内，无需裁剪，直接添加
			append_edges<false>(out, lines);
		}
		build_tile_edges(out);
		Qk_ReturnLocal(out);
	}
}