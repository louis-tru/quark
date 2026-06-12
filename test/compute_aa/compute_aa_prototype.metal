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

kernel void qk_compute_aa_coverage(
	constant ComputeAAParams &params [[buffer(0)]],
	const device ComputeAAEdge *edges [[buffer(1)]],
	const device ComputeAATileEdge *tileEdges [[buffer(2)]],
	const device ComputeAATile *tiles [[buffer(3)]],
	const device int *backdrops [[buffer(4)]],
	texture2d<float, access::write> coverageTex [[texture(0)]],
	uint2 tileGroupId [[threadgroup_position_in_grid]],
	uint threadIndex [[thread_index_in_threadgroup]])
{
	constexpr uint sampleCount = QK_COMPUTE_AA_TILE_SIZE * QK_COMPUTE_AA_SAMPLE_GRID;
	uint tileInGroup = threadIndex >> 4;
	uint pixelY = threadIndex & (QK_COMPUTE_AA_TILE_SIZE - 1);
	uint tileX = tileGroupId.x * 2 + tileInGroup;
	if (tileX >= params.tileCountX) {
		return;
	}
	uint tileIndex = tileGroupId.y * params.tileCountX + tileX;
	const device ComputeAATile &tile = tiles[tileIndex];
	float invGrid = 1.0 / float(QK_COMPUTE_AA_SAMPLE_GRID);

	// 一个 SIMD32 threadgroup 同时处理横向相邻的两个 tile。每组 16 个
	// 线程负责一个 tile，每个线程直接累计一整行的 coverage。delta 只
	// 属于当前线程和当前 Y sample 行，不需要 threadgroup 数组或 barrier。
	ushort covered[QK_COMPUTE_AA_TILE_SIZE] = {};
	for (uint sy = 0; sy < QK_COMPUTE_AA_SAMPLE_GRID; sy++) {
		uint localSampleY = pixelY * QK_COMPUTE_AA_SAMPLE_GRID + sy;
		float sampleY = float(tile.originY) + (float(localSampleY) + 0.5) * invGrid;
		int winding = backdrops[tileIndex * sampleCount + localSampleY];
		short windingDelta[QK_COMPUTE_AA_TILE_SIZE * QK_COMPUTE_AA_SAMPLE_GRID] = {};

		for (uint i = 0; i < tile.edgeCount; i++) {
			const device ComputeAATileEdge &tileEdge = tileEdges[tile.edgeOffset + i];
			if (localSampleY >= tileEdge.sampleBegin && localSampleY < tileEdge.sampleEnd) {
				const device ComputeAAEdge &edge = edges[tileEdge.edgeIndex];
				int crossX = int(ceil(
					(qk_aa_edge_cross_x(sampleY, edge) - float(tile.originX)) *
					float(QK_COMPUTE_AA_SAMPLE_GRID) - 0.5
				));
				crossX = clamp(crossX, 0, int(sampleCount));
				if (crossX < int(sampleCount)) {
					windingDelta[crossX] += short(edge.winding);
				}
			}
		}

		for (uint sampleX = 0; sampleX < sampleCount; sampleX++) {
			winding += int(windingDelta[sampleX]);
			if (qk_aa_inside(winding, params.fillRule)) {
				covered[sampleX / QK_COMPUTE_AA_SAMPLE_GRID]++;
			}
		}
	}

	for (uint pixelX = 0; pixelX < QK_COMPUTE_AA_TILE_SIZE; pixelX++) {
		uint2 pixel = uint2(tile.originX + pixelX, tile.originY + pixelY);
		if (pixel.x < params.width && pixel.y < params.height) {
			float coverage = float(covered[pixelX]) *
				(1.0 / float(QK_COMPUTE_AA_SAMPLE_GRID * QK_COMPUTE_AA_SAMPLE_GRID));
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
