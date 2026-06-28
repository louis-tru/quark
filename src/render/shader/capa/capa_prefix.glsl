// CAPA pass 5.
// Turn per-tile local row backdrop values into row-prefix backdrop.

Qk_CONSTANT(
	uint maxPathTileRowCount;
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

layout(binding=3,set=0,std430) buffer CAPAPathTiles {
	CAPAPathTile values[];
} pathTiles;

layout(binding=4,set=0,std430) buffer CAPABoundaryTiles {
	CAPABoundaryTile values[];
} boundaryTiles;

layout(binding=5,set=0,std430) readonly buffer CAPATileRows {
	CAPAPathTileRow values[];
} tileRows;

bool capa_is_full_backdrop(float area, uint fillRule) {
	switch (fillRule) {
		case CAPA_EVEN_ODD_RULE: // EvenOdd
			return (uint(floor(abs(area) + 0.5)) & 1u) != 0u;
		case CAPA_POSITIVE_RULE: // Positive
			return area >= 0.999;
		case CAPA_NEGATIVE_RULE: // Negative
			return area <= -0.999;
		default: // NonZeroRule
			return abs(area) >= 0.999;
	}
}

void main() {
	uint tileRow = gl_WorkGroupID.x * gl_WorkGroupSize.y + gl_LocalInvocationID.y;
	uint row = gl_LocalInvocationID.x;
	uint tileRowCount = min(env.value.pathTileRowCount, pc.maxPathTileRowCount);
	if (tileRow >= tileRowCount)
		return;

	uint pathTileX0Index = tileRows.values[tileRow].pathTileIndex;
	uint pathIndex = pathTiles.values[pathTileX0Index].pathIndex;
	uint tileSpanX = uint(paths.values[pathIndex].tileRect.z);
	uint fillRule = paths.values[pathIndex].fillRule;
	float prefix = 0.0;

	for (uint tileX = 0u; tileX < tileSpanX; tileX++) {
		uint pathTileIndex = pathTileX0Index + tileX;
		uint boundaryIndex = pathTiles.values[pathTileIndex].boundaryTileIndex;
		if (boundaryIndex >= 3u) {
			if (tileX == 0u) {
				float left = boundaryTiles.values[boundaryIndex].backdrop[row];
				float local = uintBitsToFloat(boundaryTiles.values[boundaryIndex].coverage[row]);
				prefix = left + local;
			} else {
				float local = boundaryTiles.values[boundaryIndex].backdrop[row];
				boundaryTiles.values[boundaryIndex].backdrop[row] = prefix;
				prefix += local;
			}
		} else if (boundaryIndex == 0u && row == 0u) {
			if (capa_is_full_backdrop(prefix, fillRule))
				pathTiles.values[pathTileIndex].boundaryTileIndex = 1u; // full backdrop
		}
	}
}
