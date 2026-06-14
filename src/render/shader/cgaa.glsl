// CGAA: Compute Grid Anti-Aliasing.
// 当前测试路径在 compute 中直接应用 coverage 和实色 SrcOver。

Qk_CONSTANT(
	uint tileCountX;
	uint tileCountY;
	uint fillRule;
	uint boundaryTiles; // boundaryTiles 长度
	uint originX;
	uint originY;
	uint targetWidth;
	uint targetHeight;
	vec4 color;
);

#comp
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#extension GL_ARB_gpu_shader_int64 : require

layout(local_size_x=64, local_size_y=1, local_size_z=1) in;

const uint CGAA_TILE_SIZE = 16;
const uint CGAA_SAMPLE_GRID = 4;
const uint CGAA_EVEN_ODD = 1;
const uint CGAA_POSITIVE = 2;
const uint CGAA_NEGATIVE = 3;

struct CgaaEdge {
	vec2 p0;
	vec2 p1;
	float dxdy;
	int winding;
	uvec2 _pad;
};

struct CgaaTileEdge {
	uint edgeIndex;
	uint16_t sampleBegin;
	uint16_t sampleEnd;
};

struct CgaaBackdropEvent {
	uint firstTileX;
	uint16_t sampleBegin;
	uint16_t sampleEnd;
	int winding;
};

struct CgaaBackdropRow {
	uint eventOffset;
	uint eventCount;
};

struct CgaaTile {
	uint originX;
	uint originY;
	uint edgeOffset;
	uint edgeCount;
};

struct CgaaUniformTile {
	uint originX;
	uint originY;
	int winding;
	uint _pad;
};

layout(binding=1,set=0,std430) readonly buffer CgaaUniformTiles {
	CgaaUniformTile values[];
} uniformTiles;
layout(binding=2,set=0,std430) readonly buffer CgaaEdges {
	CgaaEdge values[];
} edges;
layout(binding=4,set=0,std430) readonly buffer CgaaTileEdges {
	CgaaTileEdge values[];
} tileEdges;
layout(binding=5,set=0,std430) readonly buffer CgaaBoundaryTiles {
	CgaaTile values[];
} boundaryTiles;
layout(binding=6,set=0,std430) readonly buffer CgaaBackdropEvents {
	CgaaBackdropEvent values[];
} backdropEvents;
layout(binding=7,set=0,std430) readonly buffer CgaaBackdropRows {
	CgaaBackdropRow values[];
} backdropRows;
layout(binding=1,set=1,rgba8) uniform image2D targetTex;

shared uint64_t insideMask[CGAA_TILE_SIZE * CGAA_SAMPLE_GRID];

bool cgaa_inside(int winding) {
	if (pc.fillRule == CGAA_EVEN_ODD)
		return (abs(winding) & 1) != 0;
	if (pc.fillRule == CGAA_POSITIVE)
		return winding > 0;
	if (pc.fillRule == CGAA_NEGATIVE)
		return winding < 0;
	return winding != 0;
}

void cgaa_write_color(uvec2 pixel, float coverage) {
	vec4 src = pc.color * coverage;
	vec4 dst = imageLoad(targetTex, ivec2(pixel));
	imageStore(targetTex, ivec2(pixel), src + dst * (1.0 - src.a));
}

float cgaa_edge_cross_x(float sampleY, CgaaEdge edge) {
	return fma(sampleY - edge.p0.y, edge.dxdy, edge.p0.x);
}

uint64_t cgaa_sample_range_mask(uint begin, uint end) {
	uint64_t beforeBegin = begin != 0 ? ((uint64_t(1) << begin) - 1): uint64_t(0);
	uint64_t throughEnd = end < 64 ? ((uint64_t(1) << end) - 1): ~uint64_t(0);
	return throughEnd & ~beforeBegin;
}

void cgaa_write_uniform_tile(uint tileIndex, uint threadIndex) {
	CgaaUniformTile tile = uniformTiles.values[tileIndex];
	float coverage = cgaa_inside(tile.winding) ? 1.0: 0.0;
	const uint pixelCount = CGAA_TILE_SIZE * CGAA_TILE_SIZE;
	const uint threadCount = CGAA_TILE_SIZE * CGAA_SAMPLE_GRID;
	for (uint pixelIndex = threadIndex; pixelIndex < pixelCount; pixelIndex += threadCount) {
		uvec2 pixel = uvec2(
			pc.originX + tile.originX + (pixelIndex & (CGAA_TILE_SIZE - 1)),
			pc.originY + tile.originY + (pixelIndex >> 4)
		);
		if (pixel.x < pc.targetWidth && pixel.y < pc.targetHeight)
			cgaa_write_color(pixel, coverage);
	}
}

void cgaa_write_boundary_tile(uint boundaryTileIndex, uint threadIndex) {
	const uint sampleCount = CGAA_TILE_SIZE * CGAA_SAMPLE_GRID;
	CgaaTile tile = boundaryTiles.values[boundaryTileIndex];
	uint tileX = tile.originX >> 4;
	uint tileY = tile.originY >> 4;
	float y = float(tile.originY) +
		(float(threadIndex) + 0.5) / float(CGAA_SAMPLE_GRID);

	CgaaBackdropRow row = backdropRows.values[tileY];
	int winding = 0;
	for (uint i = 0; i < row.eventCount; i++) {
		CgaaBackdropEvent event = backdropEvents.values[row.eventOffset + i];
		if (event.firstTileX <= tileX &&
			threadIndex >= event.sampleBegin &&
			threadIndex < event.sampleEnd)
		{
			winding += event.winding;
		}
	}

	int16_t crossingDelta[sampleCount];
	uint64_t crossingMask = uint64_t(0);
	for (uint i = 0; i < tile.edgeCount; i++) {
		CgaaTileEdge tileEdge = tileEdges.values[tile.edgeOffset + i];
		if (threadIndex >= tileEdge.sampleBegin && threadIndex < tileEdge.sampleEnd) {
			CgaaEdge edge = edges.values[tileEdge.edgeIndex];
			float localCrossX = cgaa_edge_cross_x(y, edge) - float(tile.originX);
			int sampleGridX = int(ceil(localCrossX * float(CGAA_SAMPLE_GRID) - 0.5));
			sampleGridX = clamp(sampleGridX, 0, int(sampleCount));
			if (sampleGridX < int(sampleCount)) {
				uint64_t crossingBit = uint64_t(1) << uint(sampleGridX);
				if ((crossingMask & crossingBit) != uint64_t(0)) {
					crossingDelta[sampleGridX] += int16_t(edge.winding);
				} else {
					crossingDelta[sampleGridX] = int16_t(edge.winding);
					crossingMask |= crossingBit;
				}
			}
		}
	}

	uint64_t mask = uint64_t(0);
	uint intervalBegin = 0;
	while (crossingMask != uint64_t(0)) {
		uint crossingX = uint(findLSB(crossingMask));
		if (cgaa_inside(winding))
			mask |= cgaa_sample_range_mask(intervalBegin, crossingX);
		winding += int(crossingDelta[crossingX]);
		intervalBegin = crossingX;
		crossingMask &= crossingMask - uint64_t(1);
	}
	if (cgaa_inside(winding))
		mask |= cgaa_sample_range_mask(intervalBegin, sampleCount);
	insideMask[threadIndex] = mask;

	barrier();

	const uint pixelCount = CGAA_TILE_SIZE * CGAA_TILE_SIZE;
	const uint64_t sampleMask = (uint64_t(1) << CGAA_SAMPLE_GRID) - uint64_t(1);
	const float invSampleCount = 1.0 / float(CGAA_SAMPLE_GRID * CGAA_SAMPLE_GRID);
	for (uint pixelIndex = threadIndex; pixelIndex < pixelCount; pixelIndex += sampleCount) {
		uint pixelX = pixelIndex & (CGAA_TILE_SIZE - 1);
		uint pixelY = pixelIndex >> 4;
		uint sampleGridX = pixelX * CGAA_SAMPLE_GRID;
		uint64_t sampleBits = sampleMask << sampleGridX;
		uint covered = 0;
		uint sampleGridY = pixelY * CGAA_SAMPLE_GRID;
		for (uint sy = 0; sy < CGAA_SAMPLE_GRID; sy++)
			covered += uint(bitCount(insideMask[sampleGridY + sy] & sampleBits));
		uvec2 pixel = uvec2(
			pc.originX + tile.originX + pixelX,
			pc.originY + tile.originY + pixelY
		);
		if (pixel.x < pc.targetWidth && pixel.y < pc.targetHeight) {
			float coverage = float(covered) * invSampleCount;
			cgaa_write_color(pixel, coverage);
		}
	}
}

void main() {
	uint tileIndex = gl_WorkGroupID.x;
	uint threadIndex = gl_LocalInvocationIndex;
	if (tileIndex < pc.boundaryTiles)
		cgaa_write_boundary_tile(tileIndex, threadIndex);
	else
		cgaa_write_uniform_tile(tileIndex - pc.boundaryTiles, threadIndex);
}
