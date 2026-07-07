// CAPA prefix pass.
// Turn boundary-tile local row backdrop values into row-prefix backdrop.

#import "_capa.glsl"

#comp
layout(local_size_x=16, local_size_y=2, local_size_z=1) in;

layout(binding=1,set=0,std430) readonly buffer CAPAEnvironments {
	CAPAEnvironment value;
} env;

layout(binding=2,set=0,std430) readonly buffer CAPATileRows {
	CAPAPathTileRow values[];
} tileRows;

layout(binding=3,set=0,std430) readonly buffer CAPAPaths {
	CAPAPath values[];
} paths;

layout(binding=4,set=0,std430) buffer CAPABoundaryTiles {
	CAPABoundaryTile values[];
} boundaryTiles;

void main() {
	uint tileRow = gl_WorkGroupID.x * gl_WorkGroupSize.y + gl_LocalInvocationID.y;
	if (tileRow >= env.value.realPathTileRowCount)
		return;

	uint boundaryIndex = tileRows.values[tileRow].boundaryTileIndex;
	if (boundaryIndex == CAPA_NIL)
		return;

	uint row = gl_LocalInvocationID.x;
	float prefix = 0.0;

	// The row record carries the prefix to the first real boundary tile. Boundary
	// tiles themselves are then rewritten from local delta to tile-left prefix.
	uint pathIndex = tileRows.values[tileRow].pathIndex;
	if (boundaryTiles.values[boundaryIndex].tileCoord.x <= paths.values[pathIndex].tileRect.x) {
		prefix = tileRows.values[tileRow].backdrop[row];
	}
	uint end = boundaryIndex + tileRows.values[tileRow].boundaryTileCount;
	for (; boundaryIndex < end; boundaryIndex++) {
		float local = boundaryTiles.values[boundaryIndex].backdrop[row];
		// replace local to the prefix value for this row
		boundaryTiles.values[boundaryIndex].backdrop[row] = prefix;
		prefix += local;
	}
}
