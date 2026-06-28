// CAPA pass 1.1
// This pass allocates path tiles and tile rows.

Qk_CONSTANT(
	uint pathCount;
	uint maxPathTileCount;
	uint maxPathTileRowCount;
);

#import "_capa.glsl"

#comp
layout(local_size_x=32, local_size_y=1, local_size_z=1) in;

layout(binding=1,set=0,std430) buffer CAPAEnvironments {
	CAPAEnvironment value;
} env;

layout(binding=2,set=0,std430) buffer CAPAPaths {
	CAPAPath values[];
} paths;

layout(binding=7,set=0,std430) buffer CAPATileRows {
	CAPAPathTileRow values[];
} tileRows;

void main() {
	uint pathIndex = gl_GlobalInvocationID.x;
	if (pathIndex >= pc.pathCount)
		return;

	ivec4 bounds = paths.values[pathIndex].bounds;
	ivec4 tileBounds = ivec4(
		bounds.x / CAPA_TILE_SIZE,
		bounds.y / CAPA_TILE_SIZE,
		(bounds.z + CAPA_TILE_SIZE - 1) / CAPA_TILE_SIZE,
		(bounds.w + CAPA_TILE_SIZE - 1) / CAPA_TILE_SIZE
	);
	// update global tile bounds
	capa_join_bounds_atomic(env.value.globalTileBounds, tileBounds);

	// alloc path tiles
	ivec2 tileSpan = tileBounds.zw - tileBounds.xy;
	// only allocate path tiles if there are any tiles to allocate
	if (tileSpan.x > 0 && tileSpan.y > 0) {
		uint tileCount = tileSpan.x *  tileSpan.y;
		uint tileOffset = atomicAdd(env.value.pathTileCount, tileCount);
		if (tileOffset + tileCount <= pc.maxPathTileCount) {
			paths.values[pathIndex].tileOffset = tileOffset;
			paths.values[pathIndex].tileCount = tileCount;
			// allocate tile rows for this path
			uint offset = atomicAdd(env.value.pathTileRowCount, tileSpan.y);
			int rows = min(tileSpan.y, int(pc.maxPathTileRowCount) - int(offset));
			for (int i = 0; i < rows; i++) {
				// write the index to the first tile of this row in the path tile row
				tileRows.values[offset+i] = CAPAPathTileRow(tileOffset + i * tileSpan.x);
			}
		}
		paths.values[pathIndex].tileRect = ivec4(tileBounds.xy, tileSpan);
	}
}
