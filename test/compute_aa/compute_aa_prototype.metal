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
	float minY;
	float maxY;
	int winding;
	uint _pad;
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
	uint edgeOffset;
	uint edgeCount;
	uint originX;
	uint originY;
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
	float4 color;
};

kernel void qk_compute_aa_clear(
	constant ComputeAAParams &params [[buffer(0)]],
	texture2d<float, access::write> colorTex [[texture(0)]],
	uint2 gid [[thread_position_in_grid]])
{
	if (gid.x >= colorTex.get_width() || gid.y >= colorTex.get_height()) {
		return;
	}
	colorTex.write(params.color, gid);
}

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

static inline int qk_aa_add_edge_crossing(float2 sample,
	const device ComputeAAEdge &edge)
{
	if (sample.y < edge.minY || sample.y >= edge.maxY) {
		return 0;
	}
	float dy = edge.p1.y - edge.p0.y;
	if (dy == 0.0) {
		return 0;
	}
	float t = (sample.y - edge.p0.y) / dy;
	float crossX = edge.p0.x + (edge.p1.x - edge.p0.x) * t;
	return crossX <= sample.x ? edge.winding : 0;
}

kernel void qk_compute_aa_coverage(
	constant ComputeAAParams &params [[buffer(0)]],
	const device ComputeAAEdge *edges [[buffer(1)]],
	const device ComputeAATileEdge *tileEdges [[buffer(2)]],
	const device ComputeAATile *tiles [[buffer(3)]],
	const device ComputeAABackdropEvent *backdropEvents [[buffer(4)]],
	const device ComputeAABackdropRow *backdropRows [[buffer(5)]],
	texture2d<float, access::write> coverageTex [[texture(0)]],
	uint2 gid [[thread_position_in_grid]],
	uint2 tileId [[threadgroup_position_in_grid]],
	uint threadIndex [[thread_index_in_threadgroup]])
{
	// 一个 threadgroup 对应一个 16x16 tile。前 64 个线程各自负责一个
	// local Y sample，把当前 tile 左边界处的 winding 解析到共享内存。
	threadgroup int backdrop[QK_COMPUTE_AA_TILE_SIZE * QK_COMPUTE_AA_SAMPLE_GRID];
	uint backdropSampleCount = QK_COMPUTE_AA_TILE_SIZE * QK_COMPUTE_AA_SAMPLE_GRID;
	if (threadIndex < backdropSampleCount && tileId.y < params.tileCountY) {
		const device ComputeAABackdropRow &row = backdropRows[tileId.y];
		int winding = 0;
		for (uint i = 0; i < row.eventCount; i++) {
			const device ComputeAABackdropEvent &event =
				backdropEvents[row.eventOffset + i];
			if (event.firstTileX <= tileId.x &&
				threadIndex >= event.sampleBegin &&
				threadIndex < event.sampleEnd)
			{
				winding += event.winding;
			}
		}
		backdrop[threadIndex] = winding;
	}
	// 所有像素线程都必须等 backdrop 初始化完成后才能读取；越界线程也必须
	// 先到达 barrier，因此边缘 tile 的越界 return 放在 barrier 之后。
	threadgroup_barrier(mem_flags::mem_threadgroup);

	if (gid.x >= params.width || gid.y >= params.height ||
		tileId.x >= params.tileCountX || tileId.y >= params.tileCountY)
		return;

	const device ComputeAATile &tile = tiles[tileId.y * params.tileCountX + tileId.x];
	uint grid = min(params.sampleGrid, QK_COMPUTE_AA_SAMPLE_GRID);
	float invGrid = 1.0 / float(grid);
	float covered = 0.0;

	for (uint sy = 0; sy < grid; sy++) {
		for (uint sx = 0; sx < grid; sx++) {
			float2 sample = float2(gid) + (float2(sx, sy) + 0.5) * invGrid;
			uint localY = gid.y - tile.originY;
			int winding = backdrop[localY * QK_COMPUTE_AA_SAMPLE_GRID + sy];

			for (uint i = 0; i < tile.edgeCount; i++) {
				const device ComputeAATileEdge &tileEdge = tileEdges[tile.edgeOffset + i];
				uint localSample = localY * params.sampleGrid + sy;
				// 同一条原始边可以被多个 X tile 引用，但每个引用只作用于
				// 该 X tile 推进步骤中新跨过的 Y sample，避免与 backdrop 重复。
				if (localSample >= tileEdge.sampleBegin &&
					localSample < tileEdge.sampleEnd)
				{
					winding += qk_aa_add_edge_crossing(sample, edges[tileEdge.edgeIndex]);
				}
			}

			covered += qk_aa_inside(winding, params.fillRule) ? 1.0 : 0.0;
		}
	}

	float coverage = covered * invGrid * invGrid;
	coverageTex.write(float4(coverage, 0.0, 0.0, 1.0), gid);
}

kernel void qk_compute_aa_composite_solid(
	constant ComputeAAParams &params [[buffer(0)]],
	texture2d<float, access::read> coverageTex [[texture(0)]],
	texture2d<float, access::read_write> colorTex [[texture(1)]],
	uint2 gid [[thread_position_in_grid]])
{
	if (gid.x >= params.width || gid.y >= params.height) {
		return;
	}

	float coverage = coverageTex.read(gid).r;
	float4 src = params.color * coverage;
	uint2 dstCoord = gid + uint2(params.outputOriginX, params.outputOriginY);
	if (dstCoord.x >= colorTex.get_width() || dstCoord.y >= colorTex.get_height()) {
		return;
	}
	float4 dst = colorTex.read(dstCoord);
	float4 outColor = src + dst * (1.0 - src.a);
	colorTex.write(outColor, dstCoord);
}
