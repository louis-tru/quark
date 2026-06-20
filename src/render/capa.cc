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

#include "./capa.h"
#include "./gpu_canvas.h"
#include "./path.h"

namespace qk {
	const int sampleGrid = kCAPASampleGrid;
	const int tileSampleCount = kCAPATileSize * sampleGrid;
	const float invHalfSampleGrid = 0.5f / float(sampleGrid);
	static_assert(
		(kCAPATileSize & (kCAPATileSize - 1)) == 0 &&
		(sampleGrid & (sampleGrid - 1)) == 0, "Compute AA tile/sample sizes must be powers of two"
	);
	// tileSampleShift 是 tile/sample 数量的二进制位数，用于整数转换代替除法。log2(16*4) = 6
	const int tileSampleShift = __builtin_ctz(tileSampleCount);

	template<bool NeedClip>
	static void append_edges(CAPADrawData &out, Range bounds, Array<Vec2> &lines) {
		auto begin = bounds.begin,
				end = bounds.end;
		for (uint32_t i = 1; i < lines.length(); i += 2) {
			auto p0 = lines[i-1], p1 = lines[i];
			// 需要裁剪边界外的线段，先进行一次粗略的 CPU 裁剪，减少后续 GPU 处理的边数,
			// 无法处理左边界为负的情况，因为x扫描线算法需要统计tile左边的backdrop值。
			if (NeedClip) {
				// 整条边都在bounds上方的无效区域，不生成任何数据
				if (p0.y() < begin.y() && p1.y() < begin.y())
					continue;
				// 整条边都在bounds下方的无效区域，不生成任何数据
				if (p0.y() > end.y() && p1.y() > end.y())
					continue;
				// 整条边都在bounds右侧的无效区域，不生成任何数据
				if (p0.x() > end.x() && p1.x() > end.x())
					continue;
			}
			p0 -= begin;
			p1 -= begin;
			// 屏幕坐标 y-down：向下为正 winding，向上为负 winding。
			// 水平边使用 winding=0，只参与 boundary tile 标记。
			int32_t winding = p1.y() > p0.y() ? 1 : p1.y() < p0.y() ? -1 : 0;
			float invDy = winding ? 1.0f / (p1.y() - p0.y()) : 0.0f;
			float dxdy = (p1.x() - p0.x()) * invDy;
			out.edges.push(CAPAEdge{
				.p0=p0, .p1=p1, .dxdy=dxdy, .winding=winding
			});
		}
	}

	static bool capa_inside(uint16_t fillRule, int winding) {
		switch(fillRule) {
			case kCAPANonZero_FillRule: return winding != 0;
			case kCAPAEvenOdd_FillRule: return (std::abs(winding) & 1) != 0;
			case kCAPAPositive_FillRule: return winding > 0;
			case kCAPANegative_FillRule: return winding < 0;
			default: return false;
		}
	}

	struct TileScratch {
		Array<CAPATileEdge> edges;
		int backdropBegin; // 当前tile的第0个y样本backdrop winding，来自tileX=-1的边界事件
		bool boundary;
	};
	// 标记为平凡类型避免TileScratchs一次初始全部tiles，因为大部分tiles都是空的，只有少部分boundary tiles需要初始化。
	template<> struct ObjectTraits<TileScratch>: ObjectTraitsBase<TileScratch> {
		static constexpr bool isOrdinary = true;
	};
	struct TileScratchs: Array<TileScratch> {
		TileScratchs(uint32_t count): Array<TileScratch>(count), boundaryCount(0) {
			// 强制初始化数据为0，确保boundary标记和backdropBegin初始为0，
			// 后续通过maekBoundary函数按需构造TileScratch。
			memset((void*)val(), 0, size());
		}
		~TileScratchs() {
			Qk_ASSERT(_ptr.allocator->isLinearAllocator(), "TileScratchs should use a linear allocator");
			// 由于目前的实现是线性分配器，直接丢弃整个数组即可，无需逐个调用析构函数。
			_ptr.extra = 0;
		}
		int boundaryCount; // 统计实际需要测试边的 tile 数量，用于后续分配连续数组
		void maekBoundary(uint32_t index) {
			auto &tile = operator[](index);
			if (!tile.boundary) {
				auto backdrop = tile.backdropBegin;
				new(&tile) TileScratch(); // 手动调用构造函数初始化 TileScratch
				tile.boundary = true;
				tile.backdropBegin = backdrop;
				boundaryCount++;
			}
		}
	};

	bool CAPABuilder::buildTileEdges(Range &bounds, int edgeIndex, int edgeEnd, int tileCountX, int tileCountY) {
		// 临时数组按 tile / tile-row 分桶，最后再压平为连续 GPU buffer。
		Allocator::pushAllocator(&_alloc2); // 临时数组使用另一个线性分配器。
		TileScratchs scratch(tileCountX * tileCountY);
		const int totalSampleCountY = tileCountY * tileSampleCount;

		auto sample_grid_y = [](float sampleY) {
			// 直接使用 ceilf 来计算 sample 网格索引，确保 sampleY 落在 [n+0.5, n+1.5) 区间时得到正确的 n+1 索引。
			int y = ceilf(sampleY - 0.5f);
			return y;
		};

		auto add_sample_range = [&](uint32_t edgeIndex, int tx, int sampleBegin, int sampleEnd, int winding)
		{
			Qk_ASSERT(tx < tileCountX, "tile x index out of bounds");
			if (sampleBegin > sampleEnd) {
				std::swap(sampleBegin, sampleEnd); // 确保 sampleBegin < sampleEnd
			}
			if (sampleEnd <= 0) {
				return; // 整个 sample 区间在 tile 上方，无需处理
			}
			if (sampleBegin >= totalSampleCountY) {
				return; // 整个 sample 区间在 tile 下方，无需处理
			}
			sampleBegin = std::max(sampleBegin, 0); // 限制在 tile 内的 sample 网格范围
			sampleEnd = std::min(sampleEnd, totalSampleCountY); // 限制在 tile 内的 sample 网格范围
			// [sampleBegin, sampleEnd) 是边在当前 X tile 步骤中新跨过的全局
			// Y sample 区间。它可能跨越多个 yTile，因此先按 yTile 拆分。
			int tileY0 = sampleBegin >> tileSampleShift; // sampleBegin / tileSampleCount
			int tileRowOffset = tileY0 * tileCountX; // 当前 tile 行的起始偏移
			if (sampleBegin == sampleEnd) {
				if ((sampleBegin & (tileSampleCount - 1)) == 0)
					return; // sample 正好位于上下 tile 边界，不属于任一 tile 内部
				if (tx >= 0)
					scratch.maekBoundary(tileRowOffset + tx); // 这个 tile 内有一个 sample 位于边界上。
				return;
			}
			int tileY1 = (sampleEnd - 1) >> tileSampleShift;
			int tileX = std::max(0, tx); // 安全索引不能小于0，负值表示边界外的 tile，仅用于 backdrop 计算
			int nextTileX = std::max(0, tx + 1); // 这个 sample 区间从这个 tile 的下一列开始完全位于边的右侧

			for (int ty = tileY0; ty <= tileY1; ty++, tileRowOffset += tileCountX) {
				int tileSampleBegin = ty << tileSampleShift; // 当前 tile 的 Y sample 网格起点
				int localBegin = std::max(sampleBegin - tileSampleBegin, 0); // 限制在 tile 内的局部 sample 区间
				int localEnd = std::min(sampleEnd - tileSampleBegin, tileSampleCount);
				scratch.maekBoundary(tileRowOffset + tileX); // 标记为 boundary tile
				// 当前 tile 内，只有这个 local sample 区间仍需要 GPU
				// 使用原始边做精确 X 交点测试；tile 内其他 sample 不测试此边。
				scratch[tileRowOffset + tileX].edges.push(CAPATileEdge{
					.edgeIndex=edgeIndex,
					.tileX=int16_t(tx), // 小于0表示边界不需要做精确测试，仅用于 backdrop 计算
					.sampleBegin=uint16_t(localBegin),
					.sampleEnd=uint16_t(localEnd),
					.winding=int16_t(winding),
				});
				if (nextTileX < tileCountX && localBegin == 0) {
					// 这个 tile 内的第一个 sample 位于边界上，属于右侧 tile 的 backdrop 部分。
					scratch[tileRowOffset + nextTileX].backdropBegin += winding;
				}
			}
		};

		auto add_sample_range_final = [&](uint32_t edgeIndex, int tx,
			int sampleBegin, float rightY, int winding)
		{
			if (tx < tileCountX) {
				int sampleEnd = sample_grid_y(rightY * sampleGrid);
				add_sample_range(edgeIndex, tx, sampleBegin, sampleEnd, winding);
			}
		};

		// 将 tile 当作粗像素沿 X 扫描。只有 Y sample 网格位置发生变化时，
		// 才为当前 tile 列添加边和为右侧 tile 添加 backdrop。
		for (int edgeIdx = edgeIndex; edgeIdx < edgeEnd; edgeIdx++) {
			auto &edge = _data.edges[edgeIdx];
			Vec2 left = edge.p0, right = edge.p1;
			if (left.x() > right.x()) {
				std::swap(left, right);
			}

			float dx = right.x() - left.x();
			float lastSampleY = left.y() * sampleGrid;
			int lastSampleGridY = sample_grid_y(lastSampleY);
			// 向下取整，做为当前 tile 的 X 索引
			int tx = floorf(left.x() * kInvCAPATileSize);
			if (dx < invHalfSampleGrid) { // 竖边或近竖边，x跨度不足0.5个sample。
				// 竖边或近竖边不需要沿 X 推进：边所在的左侧 tile 做局部精确测试，
				// 边右侧 tile 从对应列开始继承 backdrop。
				add_sample_range_final(edgeIdx, tx, lastSampleGridY, right.y(), edge.winding);
				continue;
			}

			// 右端点使用半开 X tile 范围：端点正好落在 tile 边界时，最后负责此边的仍是边界左侧 tile。
			int finalTx = I32::min(ceilf(right.x() * kInvCAPATileSize) - 1, tileCountX); // 左闭右开
			if (tx < finalTx) {
				float sampleSlope = (right.y() - left.y()) * sampleGrid / dx;
				float tileSampleYStep = sampleSlope * kCAPATileSize;
				float firstTileRight = (tx + 1) * kCAPATileSize;
				float nextSampleY = lastSampleY + (firstTileRight - left.x()) * sampleSlope;
				do {
					// 每次推进一个完整 X tile。若离散 Y sample 没有变化，
					// 说明当前列没有跨过任何扫描线，不生成任何 CPU/GPU 数据。
					int sampleGridY = sample_grid_y(nextSampleY);
					add_sample_range(edgeIdx, tx, lastSampleGridY, sampleGridY, edge.winding);
					lastSampleGridY = sampleGridY;
					// 推进 y sample grid 步
					nextSampleY += tileSampleYStep;
					tx++; // 推进一个 X tile
				} while (tx < finalTx);
			}
			// 最后一个 X tile 通常不足一个完整 tile 宽，直接使用右端点，
			// 避免增量浮点误差改变端点所属的半开 sample 区间。
			add_sample_range_final(edgeIdx, tx, lastSampleGridY, right.y(), edge.winding);
		}

		Allocator::popAllocator();
		// 重置分配器并不会释放内存，Scratch的内存还是有效的，但不能再调用scratch改变内容长度。
		_alloc2.reset();

		// 单个 路径 tile 数量超过最大限制，无法使用 16-bit 索引的 GPU 数据结构，
		// 最大不能超过UINT16_MAX-1个tile，留一个数给纯色tile做为标记。
		if (scratch.boundaryCount >= UINT16_MAX) {
			_data.edges.pop(edgeEnd - edgeIndex); // 回退未提交的边界数据
			return false;
		}
		// 临时分桶数据压平为连续数组，供 Metal buffer 直接上传。
		uint32_t atlasTileIndex = _data.tiles.length();
		uint32_t atlasTileCount = atlasTileIndex + scratch.boundaryCount;
		if ( atlasTileCount >= UINT16_MAX) {
			Array<CAPAEdge> edges;
			edges.write(_data.edges.val() + edgeIndex, edgeEnd - edgeIndex);
			commit(); // 提交并清空当前 CAPA 数据到 GPU
			Qk_ASSERT(_data.edges.isNull(), "edges should have been moved to GPU buffers");
			_data.edges.concat(edges); // copy back remaining edges for next build
			atlasTileIndex = 0;
			atlasTileCount = scratch.boundaryCount;
		}
		// 提前分配足够的 tiles 空间，后续直接写入。
		_data.tiles.reset(atlasTileCount);

		uint32_t tileOffset = _data.compositeTiles.length();
		uint32_t tileRowOffset = 0;
		uint16_t pathIndex = uint16_t(_data.paths.length());

		for (uint16_t ty = 0, originY = 0; ty < tileCountY; ty++) {
			int32_t winding = 0;
			uint16_t prevTileX = UINT16_MAX; // 上一个有边界事件的 tileX 索引
			for (uint16_t tx = 0, originX = 0; tx < tileCountX; tx++) {
				uint32_t tileIndex = tileRowOffset + tx;
				auto &tile = scratch[tileIndex];
				winding += tile.backdropBegin; // 加上 tileX=-1 的边界事件贡献的 backdrop winding
				if (tile.boundary) {
					_data.tiles[atlasTileIndex] = (CAPATile{
						.tileX=tx,
						.tileY=ty,
						.fillRule=uint16_t(fillRule),
						.prevTileX=prevTileX,
						.edgeOffset=_data.tileEdges.length(),
						.edgeCount=tile.edges.length()
					});
					_data.compositeTiles.push(CAPACompositeTile{
						.originX=originX,
						.originY=originY,
						.spanX=1,
						.spanY=1,
						.atlasTileIndex=uint16_t(atlasTileIndex),
						.pathIndex=pathIndex
					});
					_data.tileEdges.write(tile.edges.val(), tile.edges.length());
					prevTileX = atlasTileIndex;
					atlasTileIndex++;
				}
				// Direct-target production drawing does not need empty outside tiles.
				else if (winding && capa_inside(fillRule, winding)) {
					Qk_ASSERT(!tile.edges.length(), "uniform tile should not contain coverage edges");
					Qk_ASSERT(_data.compositeTiles.length(), "there should be at least one composite tile before any outside tile");
					auto &back = _data.compositeTiles.back();
					if (back.atlasTileIndex == UINT16_MAX && back.originY == originY) {
						// 同一行的连续纯色 tile 可以合并为一个 composite tile，减少 GPU 顶点数量，y方向暂时不合并。
						back.spanX++;
					} else {
					_data.compositeTiles.push(CAPACompositeTile{
						.originX=originX,
						.originY=originY,
						.spanX=1,
						.spanY=1,
						.atlasTileIndex=UINT16_MAX, // 使用特殊 atlasTileIndex 表示纯色 tile
						.pathIndex=pathIndex
					});
					}
				}
				originX += kCAPATileSize;
			}
			tileRowOffset += tileCountX;
			originY += kCAPATileSize;
		}

		// 当前路径的边界数据已经全部生成，记录路径信息供后续绘制使用。
		_data.paths.push(CAPAPath{
			.originX=uint32_t(bounds.begin.x()),
			.originY=uint32_t(bounds.begin.y()),
			.fillRule=uint32_t(fillRule),
			.flags=0,
			.color=color,
			.tileOffset=tileOffset,
			.tileCount=_data.compositeTiles.length() - tileOffset,
		});

		return true;
	}

	bool CAPABuilder::build(const Path &path, Range *clip, const Mat *mat, float precision) {
		AllocatorScope scope(&_alloc);
		auto lines = path.getEdgeLines(precision, mat);
		if (!lines.length())
			return false;
		auto bounds = Path::getBoundsFromPoints(lines.val(), lines.length()).expandToInteger();
		// clip 是可选的，允许调用方限制 atlas 的有效区域，超出部分会被裁剪掉并且不参与后续 GPU 处理。
		Qk_ASSERT(!clip || clip->begin.max(0) == clip->begin, "clip should have non-negative origin");
		Qk_ASSERT(!clip || clip->expandToInteger() == *clip, "clip should have integer bounds");
		auto clipBounds = bounds.clip(clip ? *clip : Range{0, bounds.end});
		auto size = clipBounds.size();
		if (size.x() <= 0 || size.y() <= 0)
			return false;

		auto edgeIndex = _data.edges.length();
		if (bounds.begin.y() < clipBounds.begin.y() || bounds.end.y() > clipBounds.end.y()
			|| bounds.end.x() > clipBounds.end.x()
		) {
			append_edges<true>(_data, clipBounds, lines); // 线段可能在边界外，需要裁剪
		} else { // 所有线段都在边界内，无需裁剪，直接添加
			append_edges<false>(_data, clipBounds, lines);
		}
		if (edgeIndex == _data.edges.length())
			return false; // 没有有效边，直接返回

		int tileCountX = ceilf(size.x() * kInvCAPATileSize);
		int tileCountY = ceilf(size.y() * kInvCAPATileSize);

		return buildTileEdges(clipBounds, edgeIndex, _data.edges.length(), tileCountX, tileCountY);
	}

	bool CAPABuilder::build(const Path &path) {
		Mat mat(_owner->_state->matrix);
		Vec2 surfaceScale = _owner->_surfaceScale;
		// 这里等价于Mat({0}, surfaceScale, 0, {0}) * _state->matrix)，
		// 但避免了完整矩阵乘法的开销。
		mat[0] *= surfaceScale.x();
		mat[1] *= surfaceScale.x();
		mat[2] *= surfaceScale.x();
		mat[3] *= surfaceScale.y();
		mat[4] *= surfaceScale.y();
		mat[5] *= surfaceScale.y();
		Range clip{{0,0}, _owner->_surfaceSize};
		if (_owner->_clipState) {
			clip = _owner->_clipState->range;
		}
		return build(path, &clip, &mat, _owner->_allScaleAverage * 0.5f);
	}

	cCAPADrawData& CAPABuilder::endBuild() {
		if (_data.tiles.length() == 0)
			return _data;
		float atlasSize = ceil(sqrtf(_data.tiles.length())) * kCAPATileSize;
		auto isSelfAlloc = &_alloc == Allocator::current();
		if (isSelfAlloc)
			// 这里必需弹出当前分配器，因为后续创建纹理不能使用这个线性分配器，因为这里的内存可能随时释放。
			Allocator::popAllocator();
		_data.atlas = _owner->getTextureFromPool(atlasSize, kLuminance_8_ColorType, 4096, kComputeWrite_TextureFlags);
		if (isSelfAlloc)
			// 重新压入当前分配器，确保后续的 CAPA 数据仍然使用这个线性分配器。
			Allocator::pushAllocator(&_alloc);
		return _data;
	}

	void CAPABuilder::commit() {
		if (_data.tiles.length() == 0)
			return;
		endBuild();
		_owner->makeCAPAAtlasCmd(_data); // 生成 atlas 纹理
		// 默认都提交到纯色绘制，图像绘制应该在build后立即生成atlas，只有纯色可以使用批处理。
		_owner->drawCAPAColorCmd(_data); // 绘制 CAPA 路径，使用CAPA纯色着色器
		reset();
	}

	CAPABuilder::CAPABuilder(GPUCanvas *owner): _owner(owner), _blendMode(owner->_blendMode) {
		reset();
	}

	void CAPABuilder::reset(bool clear) {
		if (&_alloc == Allocator::current()) {
			_data = {};
		} else {
			// 确保_data使用_alloc分配器。
			AllocatorScope scope(&_alloc);
			_data = {};
		}
		if (clear) {
			_alloc.clear();
			_alloc2.clear();
		} else {
			_alloc.reset(); // delete all data
		}
	}

	void CAPABuilder::setBlendMode(BlendMode mode) {
		if (_blendMode != mode) {
			commit(); // commit current batch before changing blend mode
			_blendMode = mode;
		}
	}
}
