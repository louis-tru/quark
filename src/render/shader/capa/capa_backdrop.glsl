// CAPA backdrop pass.
// Compute per-boundary-tile row backdrop values.
//
// tileX0 is special:
// - CAPAPathTileRow.backdrop[row] stores the accumulated backdrop from tiles
//   left of tileX0.
//
// For tileX > tileX0:
// - CAPABoundaryTile.backdrop[row] stores this tile's own local row delta.
//
// The prefix pass will turn backdrop[row] into row-prefix values.

Qk_CONSTANT(
	uint maxBoundaryTileCount;
);

#import "_capa.glsl"

#comp
layout(local_size_x=16, local_size_y=2, local_size_z=1) in;

layout(binding=1,set=0,std430) readonly buffer CAPAEnvironments {
	CAPAEnvironment value;
} env;

layout(binding=2,set=0,std430) buffer CAPATileRows {
	CAPAPathTileRow values[];
} tileRows;

layout(binding=3,set=0,std430) readonly buffer CAPAPaths {
	CAPAPath values[];
} paths;

layout(binding=4,set=0,std430) readonly buffer CAPAShortEdges {
	CAPAShortEdgeNode values[];
} shortEdges;

layout(binding=5,set=0,std430) buffer CAPABoundaryTiles {
	CAPABoundaryTile values[];
} boundaryTiles;

float capa_edge_cross_x(float sampleY, CAPAShortEdge edge, float dxdy) {
	return fma(sampleY - edge.p0.y, dxdy, edge.p0.x);
}

float capa_left_dy(float y0, float y1, CAPAShortEdge edge, float dxdy, float x) {
	float x0 = capa_edge_cross_x(y0, edge, dxdy);
	float x1 = capa_edge_cross_x(y1, edge, dxdy);
	bool left0 = x0 <= x;
	bool left1 = x1 <= x;
	if (left0 && left1)
		return y1 - y0;
	if (!left0 && !left1)
		return 0.0;
	if (abs(dxdy) < 1e-6)
		return left0 ? y1 - y0 : 0.0;

	float yCross = edge.p0.y + (x - edge.p0.x) / dxdy;
	yCross = clamp(yCross, y0, y1);
	return left0 ? yCross - y0 : y1 - yCross;
}

void main() {
	uint boundaryIndex = gl_WorkGroupID.x * gl_WorkGroupSize.y + gl_LocalInvocationID.y;
	if (boundaryIndex >= env.value.realBoundaryTileCount)
		return;
	uint row = gl_LocalInvocationID.x;
	uint pathIndex = boundaryTiles.values[boundaryIndex].pathIndex;
	ivec2 tileCoord = boundaryTiles.values[boundaryIndex].tileCoord;

	// compute the backdrop for this boundary tile row
	float tileLeft = float(tileCoord.x) * CAPA_TILE_SIZE_F;
	float tileRight = tileLeft + CAPA_TILE_SIZE_F;
	float y0 = float(tileCoord.y) * CAPA_TILE_SIZE_F + float(row);
	float y1 = y0 + 1.0;
	float left = 0.0;
	float local = 0.0;

	for (uint head = boundaryTiles.values[boundaryIndex].shortEdgeHead;
			head != CAPA_NIL;
			head = shortEdges.values[head].next)
	{
		CAPAShortEdge edge = shortEdges.values[head].edge;
		float edgeY0 = min(edge.p0.y, edge.p1.y);
		float edgeY1 = max(edge.p0.y, edge.p1.y);
		float beginY = max(y0, edgeY0);
		float endY = min(y1, edgeY1);
		if (beginY >= endY)
			continue;
		float dxdy = capa_edge_dxdy(edge);
		float winding = capa_edge_winding(edge);
		float leftDy = capa_left_dy(beginY, endY, edge, dxdy, tileLeft);
		float rightDy = capa_left_dy(beginY, endY, edge, dxdy, tileRight);
		left += winding * leftDy;
		local += winding * (rightDy - leftDy);
	}

	ivec2 pathCoord = paths.values[pathIndex].tileRect.xy;
	if (tileCoord.x <= pathCoord.x) { // is tileX0
		uint tileRowIndex = paths.values[pathIndex].tileRowOffset + tileCoord.y - pathCoord.y;
		// Save the initial row prefix in the row record; CAPABoundaryTile no
		// longer has an extra slot now that coverage storage is split out.
		tileRows.values[tileRowIndex].backdrop[row] = left;
	}
	boundaryTiles.values[boundaryIndex].backdrop[row] = local;
}
