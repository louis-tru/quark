/*
 * Compute AA 实验着色器。
 *
 * 当前文件没有接入正式的 shader 生成系统，故意直接使用 MSL 编写，方便独立
 * 阅读和测试算法。
 *
 * CPU 阶段：
 *   - 将路径扁平化为有方向的线段，并转换到 Y 轴向下的 atlas 坐标；
 *   - 将 tile 分成没有边界经过的 uniform tile 与需要精确采样的 boundary tile；
 *   - 为每个 boundary tile 保存局部边列表；
 *   - 使用 backdrop event 描述 tile 左侧已经完整穿过的边及其 winding。
 *
 * GPU 阶段：
 *   - Uniform pass 为 uniform tile 直接写入全 0 或全 1 coverage；
 *   - Boundary pass 为每个 16x16 tile 启动一个 threadgroup；
 *   - 每个线程负责一条 Y sample 行，生成该行的 64-bit inside mask；
 *   - threadgroup 合并 X/Y sample，输出每个像素的 coverage；
 *   - Composite pass 将 R8 coverage atlas 与颜色相乘后绘制到目标纹理。
 */

#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

// 每个 tile 包含 16x16 个像素，每个像素使用 4x4 个 coverage sample。
// 因此一个 tile 的每条 sample 行包含 16 * 4 = 64 个 X sample，刚好可用
// 一个 ulong 表示完整的 inside mask。
constant uint QK_COMPUTE_AA_TILE_SIZE = 16;
constant uint QK_COMPUTE_AA_SAMPLE_GRID = 4;

// 支持的路径填充规则，数值必须与 CPU 侧 ComputeAAFillRule 保持一致。
constant uint QK_COMPUTE_AA_NON_ZERO = 0;
constant uint QK_COMPUTE_AA_EVEN_ODD = 1;
constant uint QK_COMPUTE_AA_POSITIVE = 2;
constant uint QK_COMPUTE_AA_NEGATIVE = 3;

// CPU 扁平化后生成的有方向线段。p0/p1 位于 atlas 坐标系，Y 轴向下。
struct ComputeAAEdge {
	float2 p0;
	float2 p1;
	float dxdy; // 预计算 dx/dy，用于快速求水平 sample 线的 X 交点
	int winding; // 向下边为 +1，向上边为 -1
	uint2 _pad;
};

// Boundary tile 对全局边数组的一条引用，并限制该边实际影响的局部 Y sample 范围。
struct ComputeAATileEdge {
	uint edgeIndex;
	ushort sampleBegin; // 局部 Y sample 半开范围 [sampleBegin, sampleEnd)
	ushort sampleEnd;
};

// 一条边完全越过某个 X tile 后，从 firstTileX 开始成为后续 tile 的 backdrop。
struct ComputeAABackdropEvent {
	uint firstTileX;
	ushort sampleBegin; // 该事件影响的局部 Y sample 半开范围
	ushort sampleEnd;
	int winding;
};

// 每个 Y tile 行在 backdropEvents 数组中的连续切片。
struct ComputeAABackdropRow {
	uint eventOffset;
	uint eventCount;
};

// 只保存需要运行精确 GRID coverage 的 boundary tile。
struct ComputeAATile {
	uint originX; // atlas 像素坐标中的 tile 左上角
	uint originY;
	uint edgeOffset; // 当前 tile 在 tileEdges 数组中的连续切片
	uint edgeCount;
};

// 没有路径边界经过的 tile，整个 tile 只需根据 winding 写全 0 或全 1。
struct ComputeAAUniformTile {
	uint originX;
	uint originY;
	int winding;
	uint _pad;
};

// Coverage 与 Composite 共用的参数，布局必须与 CPU 侧完全一致。
struct ComputeAAParams {
	uint width; // coverage atlas 的有效像素尺寸
	uint height;
	uint tileCountX;
	uint tileCountY;
	uint fillRule;
	uint sampleGrid;
	uint outputOriginX; // Composite 在最终目标纹理中的输出位置
	uint outputOriginY;
	uint outputWidth;
	uint outputHeight;
	uint2 _pad;
	float4 color; // Composite 使用的预乘颜色
};

// 根据填充规则判断当前 winding 是否位于路径内部。
static inline bool aa_inside(int winding, uint fillRule) {
	if (fillRule == QK_COMPUTE_AA_EVEN_ODD) {
		return (abs(winding) & 1) != 0;
	}
	if (fillRule == QK_COMPUTE_AA_POSITIVE) {
		return winding > 0;
	}
	if (fillRule == QK_COMPUTE_AA_NEGATIVE) {
		return winding < 0;
	}
	return winding != 0;
}

// 求水平 sample 线 sampleY 与边的 X 交点。
static inline float aa_edge_cross_x(float sampleY, const device ComputeAAEdge &edge) {
	return fma(sampleY - edge.p0.y, edge.dxdy, edge.p0.x);
}

// 生成半开区间 [begin, end) 对应的 64-bit mask。
// end 可以等于 64，此时通过 ~0ul 避免执行未定义的 1ul << 64。
static inline ulong aa_sample_range_mask(uint begin, uint end) {
	ulong beforeBegin = begin ? ((1ul << begin) - 1ul) : 0ul;
	ulong throughEnd = end < 64 ? ((1ul << end) - 1ul) : ~0ul;
	return throughEnd & ~beforeBegin;
}

// Uniform tile 不需要执行边交点测试，一个 threadgroup 直接写满一个 tile。
kernel void qk_compute_aa_uniform_tiles(
	constant ComputeAAParams &params [[buffer(0)]],
	const device ComputeAAUniformTile *uniformTiles [[buffer(1)]],
	texture2d<float, access::write> coverageTex [[texture(0)]],
	uint tileIndex [[threadgroup_position_in_grid]],
	uint threadIndex [[thread_index_in_threadgroup]])
{
	const device ComputeAAUniformTile &tile = uniformTiles[tileIndex];
	float coverage = aa_inside(tile.winding, params.fillRule) ? 1.0 : 0.0;

	// 当前 threadgroup 有 16 * 4 = 64 个线程，共同写入 16 * 16 = 256
	// 个像素，因此每个线程通常写 4 个像素。
	constexpr uint pixelCount = QK_COMPUTE_AA_TILE_SIZE * QK_COMPUTE_AA_TILE_SIZE;
	constexpr uint threadCount = QK_COMPUTE_AA_TILE_SIZE * QK_COMPUTE_AA_SAMPLE_GRID;
	for (uint pixelIndex = threadIndex; pixelIndex < pixelCount; pixelIndex += threadCount) {
		uint2 pixel = uint2(
			tile.originX + (pixelIndex & (QK_COMPUTE_AA_TILE_SIZE - 1)),
			tile.originY + (pixelIndex >> 4)
		);
		if (pixel.x < params.width && pixel.y < params.height) {
			coverageTex.write(float4(coverage, 0.0, 0.0, 1.0), pixel);
		}
	}
}

// Boundary tile coverage：计算边交点、生成 sample inside mask，再合并为像素 coverage。
kernel void qk_compute_aa_coverage(
	constant ComputeAAParams &params [[buffer(0)]],
	const device ComputeAAEdge *edges [[buffer(1)]],
	const device ComputeAATileEdge *tileEdges [[buffer(2)]],
	const device ComputeAATile *boundaryTiles [[buffer(3)]],
	const device ComputeAABackdropEvent *backdropEvents [[buffer(4)]],
	const device ComputeAABackdropRow *backdropRows [[buffer(5)]],
	texture2d<float, access::write> coverageTex [[texture(0)]],
	uint boundaryTileIndex [[threadgroup_position_in_grid]],
	uint threadIndex [[thread_index_in_threadgroup]])
{
	// 一个 threadgroup 使用 sampleCount 个线程，每个线程负责一条 local Y
	// sample 行。每条边在该 Y sample 上只计算一次交点，并将 winding
	// 直接累加到离散 X sample 对应的私有 bucket。crossingMask 记录非空
	// bucket，随后通过最低置位索引按 X 顺序生成相邻交点区间。
	constexpr uint sampleCount = QK_COMPUTE_AA_TILE_SIZE * QK_COMPUTE_AA_SAMPLE_GRID;

	const device ComputeAATile &tile = boundaryTiles[boundaryTileIndex];
	// Tile size 固定为 16，因此右移 4 位即可得到 tile 坐标。
	uint tileX = tile.originX >> 4;
	uint tileY = tile.originY >> 4;
	constexpr float invGrid = 1.0 / float(QK_COMPUTE_AA_SAMPLE_GRID);
	// threadIndex 对应局部 Y sample 索引；sample 位于每个子采样格的中心。
	// y为当前线程全局x扫描线位置，例如 threadIndex=0 的 y 位于 tile.originY + 0.5 * invGrid。
	// 这个y与当前tile里所有边做真实交点计算得到的交点x位置后并追加该位置winding。
	float y = float(tile.originY) + (float(threadIndex) + 0.5) * invGrid;

	// 先通过 backdrop event 求出当前 sample 行进入 tile 左边界时的初始 winding。
	const device ComputeAABackdropRow &row = backdropRows[tileY];
	int winding = 0;
	for (uint i = 0; i < row.eventCount; i++) {
		const device ComputeAABackdropEvent &event = backdropEvents[row.eventOffset + i];
		if (event.firstTileX <= tileX &&
			threadIndex >= event.sampleBegin &&
			threadIndex < event.sampleEnd)
		{
			winding += event.winding;
		}
	}

	// crossingDelta[x] 保存离散 X 位置上的累计 winding 变化。
	short crossingDelta[sampleCount];
	// crossingMask 的第 x 位表示 crossingDelta[x] 已初始化且存在交点。
	// 首次写 bucket 时直接赋值，因此无需预先清零完整 crossingDelta 数组。
	ulong crossingMask = 0;
	for (uint i = 0; i < tile.edgeCount; i++) {
		const device ComputeAATileEdge &tileEdge = tileEdges[tile.edgeOffset + i];
		if (threadIndex >= tileEdge.sampleBegin && threadIndex < tileEdge.sampleEnd) {
			const device ComputeAAEdge &edge = edges[tileEdge.edgeIndex];
			float localCrossX = aa_edge_cross_x(y, edge) - float(tile.originX);
			// 求第一个位于交点右侧的 X sample 索引。减 0.5 是因为 sample
			// 位于子采样格中心，ceil 将连续交点映射到离散 sample 边界。
			int sampleGridX = int(ceil(localCrossX * float(QK_COMPUTE_AA_SAMPLE_GRID) - 0.5));
			sampleGridX = clamp(sampleGridX, 0, int(sampleCount));
			// sampleGridX == 64 表示 winding 只会在当前 tile 最后一个
			// sample 之后改变，不影响当前 tile，由后续 tile 的 backdrop 接管。
			if (sampleGridX < int(sampleCount)) {
				ulong crossingBit = 1ul << uint(sampleGridX);
				// 多条边量化到同一 X sample 时，直接累计其 winding delta。
				if (crossingMask & crossingBit) {
					crossingDelta[sampleGridX] += short(edge.winding);
				} else {
					crossingDelta[sampleGridX] = short(edge.winding);
					crossingMask |= crossingBit;
				}
			}
		}
	}

	// crossingMask 的最低置位始终对应最左侧未处理交点，因此无需排序。
	// 每次根据交点前的 winding 填充 [intervalBegin, crossingX)，再应用
	// crossingDelta 进入下一个区间。
	ulong mask = 0;
	uint intervalBegin = 0;
	while (crossingMask) {
		uint crossingX = ctz(crossingMask);
		if (aa_inside(winding, params.fillRule)) {
			mask |= aa_sample_range_mask(intervalBegin, crossingX);
		}
		winding += int(crossingDelta[crossingX]);
		intervalBegin = crossingX;
		// 清除最低置位，继续处理下一个非空 X bucket。
		crossingMask &= crossingMask - 1ul;
	}
	// 最后一个交点之后若仍处于内部，填充直到当前 sample 行的结尾 64。
	if (aa_inside(winding, params.fillRule)) {
		mask |= aa_sample_range_mask(intervalBegin, sampleCount);
	}
	threadgroup ulong insideMask[
		QK_COMPUTE_AA_TILE_SIZE * QK_COMPUTE_AA_SAMPLE_GRID
	];
	insideMask[threadIndex] = mask;

	// 第二阶段由同一批线程分担 tile 内全部像素。映射只依赖 sampleCount，
	// 因此 GRID=1/2/4 都不需要固定的线程数量或像素行分组。
	threadgroup_barrier(mem_flags::mem_threadgroup);

	constexpr uint pixelCount = QK_COMPUTE_AA_TILE_SIZE * QK_COMPUTE_AA_TILE_SIZE;
	constexpr ulong sampleMask = (1ul << QK_COMPUTE_AA_SAMPLE_GRID) - 1ul;
	constexpr float invSampleCount =
		1.0 / float(QK_COMPUTE_AA_SAMPLE_GRID * QK_COMPUTE_AA_SAMPLE_GRID);
	for (uint pixelIndex = threadIndex; pixelIndex < pixelCount; pixelIndex += sampleCount) {
		// 一个像素对应 GRID 个连续 X sample 与 GRID 条连续 Y sample 行。
		uint pixelX = pixelIndex & (QK_COMPUTE_AA_TILE_SIZE - 1);
		uint pixelY = pixelIndex >> 4;
		uint firstSampleX = pixelX * QK_COMPUTE_AA_SAMPLE_GRID;
		ulong sampleBits = sampleMask << firstSampleX;
		uint covered = 0;
		uint firstSampleY = pixelY * QK_COMPUTE_AA_SAMPLE_GRID;
		for (uint sy = 0; sy < QK_COMPUTE_AA_SAMPLE_GRID; sy++) {
			// popcount 统计当前像素在这一条 Y sample 行中被覆盖的 X sample 数。
			covered += popcount(insideMask[firstSampleY + sy] & sampleBits);
		}
		uint2 pixel = uint2(tile.originX + pixelX, tile.originY + pixelY);
		if (pixel.x < params.width && pixel.y < params.height) {
			float coverage = float(covered) * invSampleCount;
			coverageTex.write(float4(coverage, 0.0, 0.0, 1.0), pixel);
		}
	}
}

// Composite pass 的顶点输出：绘制一个覆盖整个 coverage atlas 的矩形。
struct CompositeVertexOut {
	float4 position [[position]];
	float2 atlasCoord;
};

vertex CompositeVertexOut qk_compute_aa_composite_vertex(
	constant ComputeAAParams &params [[buffer(0)]],
	uint vertexId [[vertex_id]])
{
	// 使用 triangle strip 的四个角生成矩形，不需要顶点缓冲。
	const float2 corners[4] = {
		float2(0.0, 0.0), float2(1.0, 0.0),
		float2(0.0, 1.0), float2(1.0, 1.0),
	};
	float2 corner = corners[vertexId];
	float2 pixel = float2(params.outputOriginX, params.outputOriginY) +
		corner * float2(params.width, params.height);
	float2 viewport = float2(params.outputWidth, params.outputHeight);

	CompositeVertexOut out;
	// 将目标纹理中的像素坐标转换为 Metal clip-space 坐标，并翻转 Y。
	out.position = float4(
		pixel.x * 2.0 / viewport.x - 1.0,
		1.0 - pixel.y * 2.0 / viewport.y,
		0.0, 1.0
	);
	out.atlasCoord = corner * float2(params.width, params.height);
	return out;
}

fragment float4 qk_compute_aa_composite_fragment(
	CompositeVertexOut in [[stage_in]],
	constant ComputeAAParams &params [[buffer(0)]],
	texture2d<float, access::read> coverageTex [[texture(0)]])
{
	// atlasCoord 位于矩形右下边界时可能等于 width/height，读取前钳制到
	// 最后一个有效 texel。coverage 只使用 R 通道。
	uint2 coord = min(uint2(in.atlasCoord), uint2(params.width - 1, params.height - 1));
	return params.color * coverageTex.read(coord).r;
}
