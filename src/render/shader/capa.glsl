// CAPA: Compute Grid Anti-Aliasing.
// 当前测试路径在 compute 中直接应用 coverage 和实色 SrcOver。

Qk_CONSTANT(
	uint atlasTileCountX; // atlas tile count in X direction, used for calculating atlas-space tile origin.
	uint atlasTileCountY;
);

#comp
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#extension GL_ARB_gpu_shader_int64 : require

layout(local_size_x=64, local_size_y=1, local_size_z=1) in;

const int CAPA_TILE_SIZE = 16;
const int CAPA_SAMPLE_GRID = 4;
const int CAPA_SAMPLE_COUNT = CAPA_TILE_SIZE * CAPA_SAMPLE_GRID;
const int CAPA_EVEN_ODD = 1;
const int CAPA_POSITIVE = 2;
const int CAPA_NEGATIVE = 3;
const int CAPA_TILE_SIZE_BITS = int(log2(CAPA_TILE_SIZE));
const uint16_t UINT16_MAX = uint16_t(65535);

struct CAPAEdge {
	vec2 p0; // atlas-space start point, y-down
	vec2 p1; // atlas-space end point, y-down
	float dxdy; // cached dx/dy for GPU X intersection
	int winding; // +1 for downward edge, -1 for upward edge
	// uvec2 _pad;
};

struct CAPATileEdge {
	uint edgeIndex;
	// tileX索引如果小于0则表示此边完全在tile内，不需要做精确交点测试，
	// 但此处小于0可用于统计tileX+1的初始winding。
	int16_t tileX;
	// 当前 tile 内需要使用原始边做精确交点测试的 Y sample 范围。
	uint16_t sampleBegin; // local Y sample range [begin, end)
	uint16_t sampleEnd; // local Y sample range [begin, end)
	int16_t winding; // CAPAEdge.winding 的副本，避免 GPU 访问 CAPAEdge 时的间接寻址性能问题
};

struct CAPATile {
	uint16_t tileX; // atlas-space tile origin, in tiles space
	uint16_t tileY;
	uint16_t fillRule; // index of the path that owns this tile
	uint16_t prevTileX; // tileX-1的tile索引，用于快速查找 backdrop 事件。
	uint edgeOffset; // offset into CAPADrawData::tileEdges
	uint edgeCount;
};

layout(binding=1,set=0,std430) readonly buffer CAPAEdges {
	CAPAEdge values[];
} edges;
layout(binding=2,set=0,std430) readonly buffer CAPATileEdges {
	CAPATileEdge values[];
} tileEdges;
layout(binding=3,set=0,std430) readonly buffer CAPABoundaryTiles {
	CAPATile values[];
} tiles;

layout(binding=0,set=1,r8) uniform writeonly image2D atlasTex;

shared uint64_t insideMask[CAPA_TILE_SIZE * CAPA_SAMPLE_GRID];

bool capa_inside(int fillRule, int winding) {
	if (fillRule == CAPA_EVEN_ODD)
		return (abs(winding) & 1) != 0;
	if (fillRule == CAPA_POSITIVE)
		return winding > 0;
	if (fillRule == CAPA_NEGATIVE)
		return winding < 0;
	return winding != 0;
}

float capa_edge_cross_x(float sampleY, CAPAEdge edge) {
	return fma(sampleY - edge.p0.y, edge.dxdy, edge.p0.x);
}

uint64_t capa_sample_range_mask(uint begin, uint end) {
	uint64_t beforeBegin = begin != 0 ? ((uint64_t(1) << begin) - 1): uint64_t(0);
	uint64_t throughEnd = end < 64 ? ((uint64_t(1) << end) - 1): ~uint64_t(0);
	return throughEnd & ~beforeBegin;
}

void main() {
	uint tileIndex = gl_WorkGroupID.x; // as tile index within atlas
	uint threadIndex = gl_LocalInvocationIndex; // as Y sample index within tile
	CAPATile tile = tiles.values[tileIndex];
	float originX = float(tile.tileX << CAPA_TILE_SIZE_BITS);
	float originY = float(tile.tileY << CAPA_TILE_SIZE_BITS);
	float y = originY + (float(threadIndex) + 0.5) / float(CAPA_SAMPLE_GRID);

	// 统计当前 tile 行的 backdrop winding，来自 tileX - 1 的边界事件。
	int winding = 0;
	if (tile.tileX == 0) {
		for (uint i = 0; i < tile.edgeCount; i++) {
			CAPATileEdge tileEdge = tileEdges.values[tile.edgeOffset + i];
			if (tileEdge.tileX < 0 &&
					threadIndex >= tileEdge.sampleBegin && threadIndex < tileEdge.sampleEnd) {
				winding += tileEdge.winding;
			}
		}
	} else {
		uint16_t prevTileX = tile.prevTileX;
		while(prevTileX != UINT16_MAX) {
			CAPATile tile = tiles.values[prevTileX];
			for (uint i = 0; i < tile.edgeCount; i++) {
				CAPATileEdge tileEdge = tileEdges.values[tile.edgeOffset + i];
				if (threadIndex >= tileEdge.sampleBegin &&  threadIndex < tileEdge.sampleEnd) {
					winding += tileEdge.winding;
				}
			}
			prevTileX = tile.prevTileX;
		}
	}

	int16_t crossingDelta[CAPA_SAMPLE_COUNT];
	uint64_t crossingMask = uint64_t(0);
	for (uint i = 0; i < tile.edgeCount; i++) {
		CAPATileEdge tileEdge = tileEdges.values[tile.edgeOffset + i];
		if (threadIndex >= tileEdge.sampleBegin && threadIndex < tileEdge.sampleEnd) {
			CAPAEdge edge = edges.values[tileEdge.edgeIndex];
			float localCrossX = capa_edge_cross_x(y, edge) - originX;
			int sampleGridX = int(ceil(localCrossX * float(CAPA_SAMPLE_GRID) - 0.5));
			if (sampleGridX >= 0 && sampleGridX < CAPA_SAMPLE_COUNT) {
				uint64_t crossingBit = uint64_t(1) << sampleGridX;
				if ((crossingMask & crossingBit) != uint64_t(0)) {
					crossingDelta[sampleGridX] += int16_t(edge.winding);
				} else {
					crossingDelta[sampleGridX] = int16_t(edge.winding);
					crossingMask |= crossingBit;
				}
			}
		}
	}

	int fillRule = tile.fillRule;
	uint64_t mask = uint64_t(0);
	uint intervalBegin = 0;
	while (crossingMask != uint64_t(0)) {
		uint crossingX = uint(findLSB(crossingMask));
		if (capa_inside(fillRule, winding))
			mask |= capa_sample_range_mask(intervalBegin, crossingX);
		winding += int(crossingDelta[crossingX]);
		intervalBegin = crossingX;
		crossingMask &= crossingMask - uint64_t(1);
	}
	if (capa_inside(fillRule, winding))
		mask |= capa_sample_range_mask(intervalBegin, CAPA_SAMPLE_COUNT);
	insideMask[threadIndex] = mask;

	barrier();

	const uint pixelCount = CAPA_TILE_SIZE * CAPA_TILE_SIZE;
	const uint64_t sampleMask = (uint64_t(1) << CAPA_SAMPLE_GRID) - uint64_t(1);
	const float invSampleCount = 1.0 / float(CAPA_SAMPLE_GRID * CAPA_SAMPLE_GRID);
	const uint atlasOriginX = (tileIndex % pc.atlasTileCountX) * CAPA_TILE_SIZE;
	const uint atlasOriginY = (tileIndex / pc.atlasTileCountX) * CAPA_TILE_SIZE;

	for (uint pixelIndex = threadIndex; pixelIndex < pixelCount; pixelIndex += CAPA_SAMPLE_COUNT) {
		uint pixelX = pixelIndex & (CAPA_TILE_SIZE - 1);
		uint pixelY = pixelIndex >> CAPA_TILE_SIZE_BITS;
		uint sampleGridX = pixelX * CAPA_SAMPLE_GRID;
		uint64_t sampleBits = sampleMask << sampleGridX;
		uint covered = 0;
		uint sampleGridY = pixelY * CAPA_SAMPLE_GRID;
		for (uint sy = 0; sy < CAPA_SAMPLE_GRID; sy++)
			covered += uint(bitCount(insideMask[sampleGridY + sy] & sampleBits));
		uvec2 pixel = uvec2(
			atlasOriginX + pixelX, atlasOriginY + pixelY
		);
		float coverage = float(covered) * invSampleCount;
		imageStore(atlasTex, ivec2(pixel), vec4(coverage, 0.0, 0.0, 1.0));
	}
}
