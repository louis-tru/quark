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

layout(binding=3,set=0,std430) readonly buffer CAPAShortEdges {
	CAPAShortEdge values[];
} shortEdges;

layout(binding=4,set=0,std430) readonly buffer CAPAPathTiles {
	CAPAPathTile values[];
} pathTiles;

layout(binding=5,set=0,std430) buffer CAPABoundaryTiles {
	CAPABoundaryTile values[];
} boundaryTiles;

float capa_edge_cross_x(float sampleY, CAPAShortEdge edge) {
	return fma(sampleY - edge.p0.y, edge.dxdy, edge.p0.x);
}

float capa_left_dy(float y0, float y1, CAPAShortEdge edge, float x) {
	float x0 = capa_edge_cross_x(y0, edge);
	float x1 = capa_edge_cross_x(y1, edge);
	bool left0 = x0 <= x;
	bool left1 = x1 <= x;
	if (left0 && left1)
		return y1 - y0;
	if (!left0 && !left1)
		return 0.0;
	if (abs(edge.dxdy) < 1e-6)
		return left0 ? y1 - y0 : 0.0;

	float yCross = edge.p0.y + (x - edge.p0.x) / edge.dxdy;
	yCross = clamp(yCross, y0, y1);
	return left0 ? yCross - y0 : y1 - yCross;
}

void main() {
	uint boundaryIndex = gl_WorkGroupID.x * gl_WorkGroupSize.y + gl_LocalInvocationID.y + 3u;
	if (boundaryIndex >= env.value.realBoundaryTileCount)
		return;
	uint row = gl_LocalInvocationID.x;
	uint pathTileIndex = boundaryTiles.values[boundaryIndex].pathTileIndex;
	ivec2 tileCoord = boundaryTiles.values[boundaryIndex].tileCoord;
	uint pathIndex = pathTiles.values[pathTileIndex].pathIndex;

	float tileLeft = float(tileCoord.x) * CAPA_TILE_SIZE_F;
	float tileRight = tileLeft + CAPA_TILE_SIZE_F;
	float y0 = float(tileCoord.y) * CAPA_TILE_SIZE_F + float(row);
	float y1 = y0 + 1.0;
	bool isTileX0 = tileCoord.x <= paths.values[pathIndex].tileRect.x;
	float left = 0.0;
	float local = 0.0;

	for (uint head = pathTiles.values[pathTileIndex].shortEdgeHead;
			head != CAPA_NIL;
			head = shortEdges.values[head].next)
	{
		CAPAShortEdge edge = shortEdges.values[head];
		float edgeY0 = min(edge.p0.y, edge.p1.y);
		float edgeY1 = max(edge.p0.y, edge.p1.y);
		float beginY = max(y0, edgeY0);
		float endY = min(y1, edgeY1);
		if (beginY >= endY)
			continue;

		// float delta = edge.winding * (endY - beginY);
		// float edgeRight = max(edge.p0.x, edge.p1.x);
		// if (isTileX0 && edgeRight <= tileLeft) {
		// 	left += delta;
		// } else {
		// 	local += delta;
		// }
		float leftDy = capa_left_dy(beginY, endY, edge, tileLeft);
		float rightDy = capa_left_dy(beginY, endY, edge, tileRight);
		left += edge.winding * leftDy;
		local += edge.winding * (rightDy - leftDy);
	}
	if (isTileX0)
		// save prefix for this row in coverage as float bits
		boundaryTiles.values[boundaryIndex].coverage[row] = floatBitsToUint(left);
	boundaryTiles.values[boundaryIndex].backdrop[row] = local;
}
