// CAPA pass 5.
// Build the boundary-tile linked list for each path-tile row, 
// and compute the prefix coverage for first row.

Qk_CONSTANT(
	uint maxPathTileRowCount;
);

#import "_capa.glsl"

#comp
layout(local_size_x=32, local_size_y=1, local_size_z=1) in;

layout(binding=1,set=0,std430) readonly buffer CAPAEnvironments {
	CAPAEnvironment value;
} env;

layout(binding=2,set=0,std430) readonly buffer CAPAPaths {
	CAPAPath values[];
} paths;

layout(binding=3,set=0,std430) buffer CAPAPathTiles {
	CAPAPathTile values[];
} pathTiles;

layout(binding=4,set=0,std430) buffer CAPABoundaryTiles {
	CAPABoundaryTile values[];
} boundaryTiles;

layout(binding=5,set=0,std430) buffer CAPATileRows {
	CAPAPathTileRow values[];
} tileRows;

bool capa_is_full_backdrop(float area, uint fillRule) {
	switch (fillRule) {
		case CAPA_EVEN_ODD_RULE:
			return (uint(floor(abs(area) + 0.5)) & 1u) != 0u;
		case CAPA_POSITIVE_RULE:
			return area >= 0.999;
		case CAPA_NEGATIVE_RULE:
			return area <= -0.999;
		default:
			return abs(area) >= 0.999;
	}
}

void main() {
	uint tileRow = gl_GlobalInvocationID.x;
	if (tileRow >= env.value.realPathTileRowCount)
		return;

	uint pathTileX0Index = tileRows.values[tileRow].pathTileIndex;
	uint pathIndex = pathTiles.values[pathTileX0Index].pathIndex;
	uint tileSpanX = paths.values[pathIndex].tileRect.z;
	uint fillRule = paths.values[pathIndex].fillRule;
	uint prevBoundaryIndex = CAPA_NIL;
	float prefix = 0.0;
	bool is_full = false;
	
	// initialize prefix with the first boundary tile's coverage if it exists
	uint boundaryIndex = pathTiles.values[pathTileX0Index].boundaryTileIndex;
	if (boundaryIndex >= 3u) {
		prefix = uintBitsToFloat(boundaryTiles.values[boundaryIndex].coverage[0]);
	}

	for (uint tileX = 0u; tileX < tileSpanX; tileX++) {
		uint pathTileIndex = pathTileX0Index + tileX;
		uint boundaryIndex = pathTiles.values[pathTileIndex].boundaryTileIndex;
		if (boundaryIndex >= 3u) {
			if (prevBoundaryIndex == CAPA_NIL) {
				tileRows.values[tileRow].boundaryTileIndex = boundaryIndex;
			} else {
				boundaryTiles.values[prevBoundaryIndex].nextBoundaryTileX = boundaryIndex;
			}
			prefix += boundaryTiles.values[boundaryIndex].backdrop[0];
			prevBoundaryIndex = boundaryIndex;
			is_full = capa_is_full_backdrop(prefix, fillRule);
		}
		else if (boundaryIndex == 0u) {
			// store is_full as 0 or 1 in boundaryTileIndex for empty tiles
			if (is_full)
				pathTiles.values[pathTileIndex].boundaryTileIndex = 1u; // full backdrop
		}
	}
}
