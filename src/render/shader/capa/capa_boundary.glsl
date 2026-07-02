// CAPA boundary tile allocation pass.
// Allocate contiguous boundary tiles for each path-tile row from small-tile
// short-edge heads, then write boundary-tile dispatch group counts.

Qk_CONSTANT(
	uint maxBoundaryTileCount;
);

#import "_capa.glsl"

#comp
layout(local_size_x=32, local_size_y=1, local_size_z=1) in;

layout(binding=1,set=0,std430) buffer CAPAEnvironments {
	CAPAEnvironment value;
} env;

layout(binding=2,set=0,std430) readonly buffer CAPAPaths {
	CAPAPath values[];
} paths;

layout(binding=3,set=0,std430) buffer CAPASmallTiles {
	CAPASmallTile values[];
} smallTiles;

layout(binding=4,set=0,std430) buffer CAPABoundaryTiles {
	CAPABoundaryTile values[];
} boundaryTiles;

layout(binding=5,set=0,std430) buffer CAPATileRows {
	CAPAPathTileRow values[];
} tileRows;

void capa_done() {
	uint done = atomicAdd(env.value.boundaryDoneCount, 1u) + 1u;
	if (done == env.value.realPathTileRowCount) {
		// Last row publishes the real boundary count and the indirect backdrop
		// dispatch size. Later passes consume only this capped range.
		uint realBoundaryTileCount = min(env.value.boundaryTileCount, pc.maxBoundaryTileCount);
		env.value.realBoundaryTileCount = realBoundaryTileCount;
		env.value.backdropPassGroups_Size16_2 = uvec4((realBoundaryTileCount + 1u) / 2u, 1u, 1u, 0u);
	}
}

void main() {
	uint tileRow = gl_GlobalInvocationID.x;
	if (tileRow >= env.value.realPathTileRowCount)
		return;

	uint pathIndex = tileRows.values[tileRow].pathIndex;
	uint tileIndex = tileRows.values[tileRow].smallTileIndex;
	uint count = 0u;
	ivec4 tileRect = paths.values[pathIndex].tileRect;

	for (uint tileX = 0u; tileX < tileRect.z; tileX++) {
		if (smallTiles.values[tileIndex + tileX].value != CAPA_NIL)
			count++;
	}

	if (count != 0u) {
		// Boundary tiles remain contiguous within a path-tile row. Prefix relies
		// on this horizontal order, while layer_plan later creates the z-linear
		// composite-facing coverage order.
		uint rowLocalY = (tileIndex - paths.values[pathIndex].tileOffset) / tileRect.z;
		ivec2 tileCoord = ivec2(tileRect.x, tileRect.y + rowLocalY);
		uint base = atomicAdd(env.value.boundaryTileCount, count);
		if (base < pc.maxBoundaryTileCount) {
			tileRows.values[tileRow].boundaryTileIndex = base;
			tileRows.values[tileRow].boundaryTileCount = min(count, pc.maxBoundaryTileCount - base);
		} else {
			tileRows.values[tileRow].boundaryTileCount = 0; // no boundary tiles allocated for this row
		}

		for (uint tileX = 0u; tileX < tileRect.z; tileX++) {
			uint smallTileIndex = tileIndex + tileX;
			uint shortEdgeHead = smallTiles.values[smallTileIndex].value;
			if (shortEdgeHead == CAPA_NIL)
				continue;
			if (base < pc.maxBoundaryTileCount) {
				boundaryTiles.values[base].pathIndex = pathIndex;
				boundaryTiles.values[base].shortEdgeHead = shortEdgeHead;
				boundaryTiles.values[base].tileCoord = tileCoord + ivec2(tileX, 0);
				smallTiles.values[smallTileIndex].value = base++;
			} else {
				smallTiles.values[smallTileIndex].value = CAPA_NIL;
			}
		}
	}

	capa_done();
}
