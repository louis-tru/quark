// CAPA pass 5.1.
// Turn boundary-tile local row backdrop values into row-prefix backdrop.

Qk_CONSTANT(
	uint maxPathTileRowCount;
);

#import "_capa.glsl"

#comp
layout(local_size_x=16, local_size_y=2, local_size_z=1) in;

layout(binding=1,set=0,std430) readonly buffer CAPAEnvironments {
	CAPAEnvironment value;
} env;

layout(binding=2,set=0,std430) readonly buffer CAPATileRows {
	CAPAPathTileRow values[];
} tileRows;

layout(binding=3,set=0,std430) buffer CAPABoundaryTiles {
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
	uint pathTileX0Index = tileRows.values[tileRow].pathTileIndex;
	float prefix = 0.0;

	// tilex0 boundary tile's coverage is the prefix for this row
	if (boundaryTiles.values[boundaryIndex].pathTileIndex == pathTileX0Index) {
		prefix = uintBitsToFloat(boundaryTiles.values[boundaryIndex].coverage[row]);
	}
	do {
		float local = boundaryTiles.values[boundaryIndex].backdrop[row];
		// replace local to the prefix value for this row
		boundaryTiles.values[boundaryIndex].backdrop[row] = prefix;
		prefix += local;
		boundaryIndex = boundaryTiles.values[boundaryIndex].nextBoundaryTileX;
	} while (boundaryIndex != CAPA_NIL);
}
