// CAPA ordered global-tile chain pass.
// Build the ordered global-tile layer chain after coverage classification.
// Empty path tiles are excluded here; full-tile merging can be added later.

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
	uint pendingFullNode = CAPA_NIL;
	vec4 pendingFullColor = vec4(0.0);
	ivec2 globalTileSpan = env.value.globalTileSpan;
	ivec2 tileCoord = env.value.globalTileBounds.xy +
										ivec2(tileIndex % globalTileSpan.x, tileIndex / globalTileSpan.x);

	#define CAPA_FLUSH_PENDING_FULL() \
		if (pendingFullNode != CAPA_NIL) { \
			uint packedColor = capa_pack_rgba8(pendingFullColor); \
			if (packedColor != 0u) { \
				pathTiles.values[pendingFullNode].boundaryTileIndex = 1u; \
				pathTiles.values[pendingFullNode].color = packedColor; \
				pathTiles.values[pendingFullNode].nextLevel = head; \
				head = pendingFullNode; \
			} \
			pendingFullNode = CAPA_NIL; \
			pendingFullColor = vec4(0.0); \
		}

	for (uint pathIndex = 0u; pathIndex < pc.pathCount; pathIndex++) {
		if (paths.values[pathIndex].tileCount != 0u) {
			ivec4 tileRect = paths.values[pathIndex].tileRect;
			if (capa_is_coord_in_rect(tileCoord, tileRect)) {
				uint pathTileIndex = paths.values[pathIndex].tileOffset;
				pathTileIndex += capa_local_offset_row_major(tileCoord, tileRect);
				uint boundaryIndex = pathTiles.values[pathTileIndex].boundaryTileIndex;
				if (boundaryIndex == 1u && paths.values[pathIndex].blendMode == CAPA_BLEND_SRC_OVER) {
					vec4 src = paths.values[pathIndex].color;
					src.rgb *= src.a;
					if (pendingFullNode == CAPA_NIL) {
						pendingFullNode = pathTileIndex;
						pendingFullColor = src;
					} else {
						pendingFullColor = capa_blend(pendingFullColor, src, CAPA_BLEND_SRC_OVER);
					}
				} else if (boundaryIndex != 0u) {
					CAPA_FLUSH_PENDING_FULL();
					pathTiles.values[pathTileIndex].pathIndex = pathIndex;
					pathTiles.values[pathTileIndex].nextLevel = head;
					head = pathTileIndex;
				}
			}
		}
	}
	CAPA_FLUSH_PENDING_FULL();
	#undef CAPA_FLUSH_PENDING_FULL

	// write the head of the tile chain to the global tile
	globalTiles.values[tileIndex] = CAPAGlobalTile(head);
}
