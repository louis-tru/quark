// CAPA pass 4.
// Compute per-boundary-tile row backdrop values.
//
// tileX0 is special:
// - backdrop[row] keeps the accumulated backdrop from tiles left of tileX0.
// - coverage[row] temporarily stores tileX0's own local row value as float bits.
//
// For tileX > tileX0:
// - backdrop[row] stores this tile's own local row value.
//
// Pass5 will turn these local values into row-prefix backdrop.

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

layout(binding=5,set=0,std430) readonly buffer CAPAShortEdgeChunks {
	CAPAShortEdgeChunk values[];
} shortEdgeChunks;

void main() {
	uint boundaryIndex = gl_WorkGroupID.x * gl_WorkGroupSize.y + gl_LocalInvocationID.y + 3u;
	uint row = gl_LocalInvocationID.x;
	uint boundaryTileCount = min(env.value.boundaryTileCount, pc.maxBoundaryTileCount);
	if (boundaryIndex >= boundaryTileCount)
		return;

	uint pathIndex = boundaryTiles.values[boundaryIndex].pathIndex;
	uint pathTileIndex = boundaryTiles.values[boundaryIndex].pathTileIndex;
	ivec2 tileCoord = boundaryTiles.values[boundaryIndex].tileCoord;

	float tileLeft = float(tileCoord.x) * CAPA_TILE_SIZE_F;
	float y0 = float(tileCoord.y) * CAPA_TILE_SIZE_F + float(row);
	float y1 = y0 + 1.0;
	bool isTileX0 = tileCoord.x <= paths.values[pathIndex].tileRect.x;
	float local = 0.0;
	float left = 0.0;

	for (uint head = pathTiles.values[pathTileIndex].shortEdgeChunkHead;
			head != CAPA_NIL;
			head = shortEdgeChunks.values[head].next)
	{
		uint edgeCount = min(shortEdgeChunks.values[head].count, CAPA_SHORT_EDGE_CHUNK_SIZE);
		for (uint i = 0u; i < edgeCount; i++) {
			CAPAShortEdge edge = shortEdgeChunks.values[head].values[i];
			float edgeY0 = min(edge.p0.y, edge.p1.y);
			float edgeY1 = max(edge.p0.y, edge.p1.y);
			float beginY = max(y0, edgeY0);
			float endY = min(y1, edgeY1);
			if (beginY >= endY)
				continue;

			float delta = float(edge.winding) * (endY - beginY);
			float edgeRight = max(edge.p0.x, edge.p1.x);
			if (isTileX0 && edgeRight <= tileLeft) {
				left += delta;
			} else {
				local += delta;
			}
		}
	}

	if (isTileX0) {
		boundaryTiles.values[boundaryIndex].backdrop[row] = left;
		boundaryTiles.values[boundaryIndex].coverage[row] = floatBitsToUint(local);
	} else {
		boundaryTiles.values[boundaryIndex].backdrop[row] = local;
	}
}
