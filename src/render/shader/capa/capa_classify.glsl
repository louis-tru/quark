// CAPA classify pass.
// Walk each path-tile row's small-tile values after backdrop and mark edge-free
// full tiles as CAPA_FULL_TILE. Empty tiles remain CAPA_NIL.

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

layout(binding=3,set=0,std430) buffer CAPASmallTiles {
	CAPASmallTile values[];
} smallTiles;

layout(binding=4,set=0,std430) readonly buffer CAPABoundaryTiles {
	CAPABoundaryTile values[];
} boundaryTiles;

layout(binding=5,set=0,std430) readonly buffer CAPATileRows {
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

	uint pathIndex = tileRows.values[tileRow].pathIndex;
	uint pathTileIndex = tileRows.values[tileRow].pathTileIndex;
	uint tileSpanX = paths.values[pathIndex].tileRect.z;
	uint fillRule = paths.values[pathIndex].fillRule;
	float prefix = 0.0;
	bool is_full = false;

	for (uint tileX = 0u; tileX < tileSpanX; tileX++) {
		uint boundaryIndex = smallTiles.values[pathTileIndex + tileX].value;
		if (boundaryIndex != CAPA_NIL) {
			if (tileX == 0u)
				prefix = uintBitsToFloat(boundaryTiles.values[boundaryIndex].coverage[0]);
			prefix += boundaryTiles.values[boundaryIndex].backdrop[0];
			is_full = capa_is_full_backdrop(prefix, fillRule);
		}
		else {
			// store full backdrop as a high sentinel so real boundary index 0 remains usable
			if (is_full)
				smallTiles.values[pathTileIndex + tileX].value = CAPA_FULL_TILE;
		}
	}
}
