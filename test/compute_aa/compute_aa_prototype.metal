/*
 * Prototype only. This Metal shader is not wired into the current generated
 * shader system. It is intentionally written as plain MSL so the compute-AA
 * algorithm can be read without the GLSL-to-MSL generator in the way.
 *
 * Coverage pass:
 *   - one thread maps to one atlas pixel;
 *   - each tile stores winding already accumulated to its left boundary;
 *   - the tile edge list contains only edges overlapping the tile itself;
 *   - the kernel evaluates a fixed sample grid and writes one coverage.
 *
 * The CPU side must build edges in atlas coordinates with y pointing down.
 */

#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

constant uint QK_COMPUTE_AA_TILE_SIZE = 16;
constant uint QK_COMPUTE_AA_SAMPLE_GRID = 4;
constant uint QK_COMPUTE_AA_NON_ZERO = 0;
constant uint QK_COMPUTE_AA_EVEN_ODD = 1;
constant uint QK_COMPUTE_AA_POSITIVE = 2;
constant uint QK_COMPUTE_AA_NEGATIVE = 3;

struct ComputeAAEdge {
	float2 p0;
	float2 p1;
	float dxdy;
	int winding;
	uint2 _pad;
};

struct ComputeAATileEdge {
	uint edgeIndex;
	ushort sampleBegin;
	ushort sampleEnd;
};

struct ComputeAABackdropEvent {
	uint firstTileX;
	ushort sampleBegin;
	ushort sampleEnd;
	int winding;
};

struct ComputeAABackdropRow {
	uint eventOffset;
	uint eventCount;
};

struct ComputeAATile {
	uint originX;
	uint originY;
	uint edgeOffset;
	uint edgeCount;
};

struct ComputeAAUniformTile {
	uint originX;
	uint originY;
	int winding;
	uint _pad;
};

struct ComputeAAParams {
	uint width;
	uint height;
	uint tileCountX;
	uint tileCountY;
	uint fillRule;
	uint sampleGrid;
	uint outputOriginX;
	uint outputOriginY;
	uint outputWidth;
	uint outputHeight;
	uint2 _pad;
	float4 color;
};

static inline bool qk_aa_inside(int winding, uint fillRule) {
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

static inline float qk_aa_edge_cross_x(float sampleY, const device ComputeAAEdge &edge) {
	return fma(sampleY - edge.p0.y, edge.dxdy, edge.p0.x);
}

kernel void qk_compute_aa_uniform_tiles(
	constant ComputeAAParams &params [[buffer(0)]],
	const device ComputeAAUniformTile *uniformTiles [[buffer(1)]],
	texture2d<float, access::write> coverageTex [[texture(0)]],
	uint tileIndex [[threadgroup_position_in_grid]],
	uint threadIndex [[thread_index_in_threadgroup]])
{
	const device ComputeAAUniformTile &tile = uniformTiles[tileIndex];
	float coverage = qk_aa_inside(tile.winding, params.fillRule) ? 1.0 : 0.0;
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
	// sample 行。每条边在该 Y sample 上只计算一次交点，然后通过 X
	// sample delta 前缀和生成这一整行的 inside mask。
	// windingDelta 由当前线程独占，用于隔离共享 delta 的成本；insideMask
	// 仍由 threadgroup 共享，以便第二阶段合并相邻 Y sample 行。
	constexpr uint sampleCount = QK_COMPUTE_AA_TILE_SIZE * QK_COMPUTE_AA_SAMPLE_GRID;
	short windingDelta[sampleCount];
	threadgroup ulong insideMask[
		QK_COMPUTE_AA_TILE_SIZE * QK_COMPUTE_AA_SAMPLE_GRID
	];

	const device ComputeAATile &tile = boundaryTiles[boundaryTileIndex];
	uint tileX = tile.originX >> 4;
	uint tileY = tile.originY >> 4;
	constexpr float invGrid = 1.0 / float(QK_COMPUTE_AA_SAMPLE_GRID);
	float sampleY = float(tile.originY) + (float(threadIndex) + 0.5) * invGrid;

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

	for (uint sampleX = 0; sampleX < sampleCount; sampleX++) {
		windingDelta[sampleX] = 0;
	}
	for (uint i = 0; i < tile.edgeCount; i++) {
		const device ComputeAATileEdge &tileEdge = tileEdges[tile.edgeOffset + i];
		if (threadIndex >= tileEdge.sampleBegin && threadIndex < tileEdge.sampleEnd) {
			const device ComputeAAEdge &edge = edges[tileEdge.edgeIndex];
			float crossX = qk_aa_edge_cross_x(sampleY, edge);
			int firstSampleX = int(ceil(
				(crossX - float(tile.originX)) * float(QK_COMPUTE_AA_SAMPLE_GRID) - 0.5
			));
			firstSampleX = clamp(firstSampleX, 0, int(sampleCount));
			if (firstSampleX < int(sampleCount)) {
				windingDelta[firstSampleX] += short(edge.winding);
			}
		}
	}

	ulong mask = 0;
	for (uint sampleX = 0; sampleX < sampleCount; sampleX++) {
		winding += int(windingDelta[sampleX]);
		if (qk_aa_inside(winding, params.fillRule)) {
			mask |= 1ul << sampleX;
		}
	}
	insideMask[threadIndex] = mask;

	// 第二阶段由同一批线程分担 tile 内全部像素。映射只依赖 sampleCount，
	// 因此 GRID=1/2/4 都不需要固定的线程数量或像素行分组。
	threadgroup_barrier(mem_flags::mem_threadgroup);

	constexpr uint pixelCount = QK_COMPUTE_AA_TILE_SIZE * QK_COMPUTE_AA_TILE_SIZE;
	constexpr ulong sampleMask = (1ul << QK_COMPUTE_AA_SAMPLE_GRID) - 1ul;
	constexpr float invSampleCount =
		1.0 / float(QK_COMPUTE_AA_SAMPLE_GRID * QK_COMPUTE_AA_SAMPLE_GRID);
	for (uint pixelIndex = threadIndex; pixelIndex < pixelCount; pixelIndex += sampleCount) {
		uint pixelX = pixelIndex & (QK_COMPUTE_AA_TILE_SIZE - 1);
		uint pixelY = pixelIndex >> 4;
		uint firstSampleX = pixelX * QK_COMPUTE_AA_SAMPLE_GRID;
		ulong sampleBits = sampleMask << firstSampleX;
		uint covered = 0;
		uint firstSampleY = pixelY * QK_COMPUTE_AA_SAMPLE_GRID;
		for (uint sy = 0; sy < QK_COMPUTE_AA_SAMPLE_GRID; sy++) {
			covered += popcount(insideMask[firstSampleY + sy] & sampleBits);
		}
		uint2 pixel = uint2(tile.originX + pixelX, tile.originY + pixelY);
		if (pixel.x < params.width && pixel.y < params.height) {
			float coverage = float(covered) * invSampleCount;
			coverageTex.write(float4(coverage, 0.0, 0.0, 1.0), pixel);
		}
	}
}

struct CompositeVertexOut {
	float4 position [[position]];
	float2 atlasCoord;
};

vertex CompositeVertexOut qk_compute_aa_composite_vertex(
	constant ComputeAAParams &params [[buffer(0)]],
	uint vertexId [[vertex_id]])
{
	const float2 corners[4] = {
		float2(0.0, 0.0), float2(1.0, 0.0),
		float2(0.0, 1.0), float2(1.0, 1.0),
	};
	float2 corner = corners[vertexId];
	float2 pixel = float2(params.outputOriginX, params.outputOriginY) +
		corner * float2(params.width, params.height);
	float2 viewport = float2(params.outputWidth, params.outputHeight);

	CompositeVertexOut out;
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
	uint2 coord = min(uint2(in.atlasCoord), uint2(params.width - 1, params.height - 1));
	return params.color * coverageTex.read(coord).r;
}
