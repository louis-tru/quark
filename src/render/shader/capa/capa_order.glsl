// CAPA pass 2.
// Build the ordered global-tile layer chain. This first landing version uses
// the full pass tile span for every path; later prepare-path bounds can narrow
// each path's local tile rect without changing pass7's traversal contract.

Qk_CONSTANT(
	uint pathCount;
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

layout(binding=3,set=0,std430) buffer CAPAGlobalTiles {
	CAPAGlobalTile values[];
} globalTiles;

layout(binding=4,set=0,std430) buffer CAPAPathTiles {
	CAPAPathTile values[];
} pathTiles;

void main() {
	uint tileIndex = gl_GlobalInvocationID.x;
	if (tileIndex >= env.value.globalTileCount)
		return;

	uint head = CAPA_NIL;
	ivec2 globalTileSpan = env.value.globalTileSpan;
	ivec2 tileCoord = env.value.globalTileBounds.xy +
										ivec2(tileIndex % globalTileSpan.x, tileIndex / globalTileSpan.x);

	for (uint pathIndex = 0u; pathIndex < pc.pathCount; pathIndex++) {
		if (paths.values[pathIndex].tileCount != 0u) {
			ivec4 tileRect = paths.values[pathIndex].tileRect;
			if (capa_is_coord_in_rect(tileCoord, tileRect)) {
				CAPAPathTile tile = CAPAPathTile(pathIndex, 0u, CAPA_NIL, head);
				head = paths.values[pathIndex].tileOffset;
				head += capa_local_offset_row_major(tileCoord, tileRect);
				pathTiles.values[head] = tile;
			}
		}
	}

	// write the head of the tile chain to the global tile
	globalTiles.values[tileIndex] = CAPAGlobalTile(head);
}
