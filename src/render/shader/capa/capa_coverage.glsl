// CAPA pass 6.
// Integrate pathTile-local short edges with tile-left row backdrop and write R8
// coverage pages.

Qk_CONSTANT(
	uint maxBoundaryTileCount;
);

#import "_capa.glsl"

#comp
layout(local_size_x=16, local_size_y=2, local_size_z=1) in;

layout(binding=1,set=0,std430) readonly buffer CAPAEnvironments {
	CAPAEnvironment value;
} env;

layout(binding=2,set=0,std430) readonly buffer CAPAPaths {
	CAPAPath values[];
} paths;

layout(binding=3,set=0,std430) readonly buffer CAPAPathTiles {
	CAPAPathTile values[];
} pathTiles;

layout(binding=4,set=0,std430) buffer CAPABoundaryTiles {
	CAPABoundaryTile values[];
} boundaryTiles;

layout(binding=5,set=0,std430) buffer CAPAShortEdges {
	CAPAShortEdge values[];
} shortEdges;

float capa_area_to_coverage(float area, uint fillRule) {
	switch (fillRule) {
		case CAPA_EVEN_ODD_RULE: {
			float folded = mod(abs(area), 2.0);
			return folded <= 1.0 ? folded : 2.0 - folded;
		}
		case CAPA_POSITIVE_RULE:
			return clamp(area, 0.0, 1.0);
		case CAPA_NEGATIVE_RULE:
			return clamp(-area, 0.0, 1.0);
		default:
			return clamp(abs(area), 0.0, 1.0);
	}
}

uint capa_pack4(float c0, float c1, float c2, float c3) {
	uint b0 = uint(c0 * 255.0 + 0.5);
	uint b1 = uint(c1 * 255.0 + 0.5);
	uint b2 = uint(c2 * 255.0 + 0.5);
	uint b3 = uint(c3 * 255.0 + 0.5);
	return b0 | (b1 << 8u) | (b2 << 16u) | (b3 << 24u);
}

float capa_edge_cross_x(float sampleY, CAPAShortEdge edge) {
	return fma(sampleY - edge.p0.y, edge.dxdy, edge.p0.x);
}

bool capa_clip_right_of_x(inout float y0, inout float y1, CAPAShortEdge edge, float x) {
	float x0 = capa_edge_cross_x(y0, edge);
	float x1 = capa_edge_cross_x(y1, edge);
	bool left0 = x0 <= x;
	bool left1 = x1 <= x;
	if (left0 && left1)
		return false;
	if (!left0 && !left1)
		return true;
	if (abs(edge.dxdy) < 1e-6)
		return !left0;

	float yCross = edge.p0.y + (x - edge.p0.x) / edge.dxdy;
	yCross = clamp(yCross, y0, y1);
	if (left0)
		y0 = yCross;
	else
		y1 = yCross;
	return y0 < y1;
}

float capa_right_width_integral(float x) {
	if (x <= 0.0)
		return x;
	if (x >= 1.0)
		return 0.5;
	return x - 0.5 * x * x;
}

float capa_right_area_from_x(float dy, float x0, float x1, float pixelX) {
	x0 -= pixelX;
	x1 -= pixelX;
	float dx = x1 - x0;
	if (abs(dx) < 1e-6)
		return dy * clamp(1.0 - x0, 0.0, 1.0);
	return dy * (capa_right_width_integral(x1) - capa_right_width_integral(x0)) / dx;
}

float capa_right_area(float y0, float y1, CAPAShortEdge edge, float pixelX) {
	float x0 = capa_edge_cross_x(y0, edge);
	float x1 = capa_edge_cross_x(y1, edge);
	return capa_right_area_from_x(y1 - y0, x0, x1, pixelX);
}

void main() {
	uint boundaryIndex = gl_WorkGroupID.x * gl_WorkGroupSize.y + gl_LocalInvocationID.y + 3u;
	uint row = gl_LocalInvocationID.x;
	uint boundaryTileCount = min(env.value.boundaryTileCount, pc.maxBoundaryTileCount);
	if (boundaryIndex >= boundaryTileCount)
		return;

	uint pathTileIndex = boundaryTiles.values[boundaryIndex].pathTileIndex;
	uint pathIndex = boundaryTiles.values[boundaryIndex].pathIndex;
	ivec2 tileCoord = boundaryTiles.values[boundaryIndex].tileCoord;
	float originX = float(tileCoord.x * CAPA_TILE_SIZE);
	float y0 = float(tileCoord.y * CAPA_TILE_SIZE + row);
	float y1 = y0 + 1.0;
	float localArea[16];
	float crossingDelta[16];
	for (uint x = 0u; x < CAPA_TILE_SIZE_U; x++) {
		localArea[x] = 0.0;
		crossingDelta[x] = 0.0;
	}

	for (uint head = pathTiles.values[pathTileIndex].shortEdgeHead;
			head != CAPA_NIL;
			head = shortEdges.values[head].next)
	{
		CAPAShortEdge edge = shortEdges.values[head];
		float beginY = max(y0, min(edge.p0.y, edge.p1.y));
		float endY = min(y1, max(edge.p0.y, edge.p1.y));
		if (beginY >= endY)
			continue;
		if (!capa_clip_right_of_x(beginY, endY, edge, originX))
			continue;
		float dy = endY - beginY;
		float x0 = capa_edge_cross_x(beginY, edge) - originX;
		float x1 = capa_edge_cross_x(endY, edge) - originX;
		uint localBeginX = uint(clamp(floor(min(x0, x1)), 0.0, CAPA_TILE_SIZE_F));
		uint localEndX = uint(clamp(ceil(max(x0, x1)), 0.0, CAPA_TILE_SIZE_F));

		for (uint x = localBeginX; x < localEndX; x++) {
			localArea[x] += edge.winding * capa_right_area_from_x(dy, x0, x1, float(x));
		}
		if (localEndX < CAPA_TILE_SIZE) {
			crossingDelta[localEndX] += edge.winding * dy;
		}
	}

	float prefix = boundaryTiles.values[boundaryIndex].backdrop[row];
	uint fillRule = paths.values[pathIndex].fillRule;
	uint baseWord = row * 4u;
	for (uint word = 0u; word < 4u; word++) {
		uint x = word << 2u;
		float coverage[4];
		for (uint i = 0u; i < 4u; i++) {
			uint px = x + i;
			prefix += crossingDelta[px];
			coverage[i] = capa_area_to_coverage(prefix + localArea[px], fillRule);
		}
		boundaryTiles.values[boundaryIndex].coverage[baseWord + word] = capa_pack4(
			coverage[0],
			coverage[1],
			coverage[2],
			coverage[3]
		);
	}
}
