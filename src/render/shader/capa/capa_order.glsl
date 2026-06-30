// CAPA ordered global-tile span pass.
// Build each global tile's contiguous ordered path-tile layer span after
// classification and coverage generation.

Qk_CONSTANT(
	uint pathCount;
	uint maxPathTileCount;
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

layout(binding=3,set=0,std430) buffer CAPAGlobalTiles {
	CAPAGlobalTile values[];
} globalTiles;

layout(binding=4,set=0,std430) buffer CAPAPathTiles {
	CAPAPathTile values[];
} pathTiles;

layout(binding=5,set=0,std430) readonly buffer CAPASmallTiles {
	CAPASmallTile values[];
} smallTiles;

const uint CAPA_ORDER_PATH_CACHE_SIZE = 16u;
uint orderPathCount = 0u;
uint emittedCount = 0u;
bool hasPendingFull = false;
uint pendingPathIndex = CAPA_NIL;
vec4 pendingColor = vec4(0.0);

bool capa_path_hits_tile(uint pathIndex, ivec2 tileCoord) {
	ivec2 tileBegin = paths.values[pathIndex].tileRect.xy;
	ivec2 tileEnd = paths.values[pathIndex].tileEnd;
	return tileCoord.x >= tileBegin.x && tileCoord.x < tileEnd.x &&
				 tileCoord.y >= tileBegin.y && tileCoord.y < tileEnd.y;
}

uint capa_small_tile_value(uint pathIndex, ivec2 tileCoord) {
	ivec4 tileRect = paths.values[pathIndex].tileRect;
	uint localOffset = capa_local_offset_row_major(tileCoord, tileRect);
	return smallTiles.values[paths.values[pathIndex].tileOffset + localOffset].value;
}

void capa_write_order_node(uint node, uint pathIndex, uint boundaryIndex, uint color) {
	pathTiles.values[node].pathIndex = pathIndex;
	pathTiles.values[node].boundaryTileIndex = boundaryIndex;
	pathTiles.values[node].color = color;
}

void capa_flush_pending_full(uint base) {
	if (!hasPendingFull)
		return;

	uint packedColor = capa_pack_rgba8(pendingColor);
	if (packedColor != 0u) {
		uint node = base + orderPathCount - emittedCount - 1u;
		capa_write_order_node(node, pendingPathIndex, CAPA_FULL_TILE, packedColor);
		emittedCount++;
	}
	hasPendingFull = false;
	pendingPathIndex = CAPA_NIL;
	pendingColor = vec4(0.0);
}

void capa_emit_path(ivec2 tileCoord, uint base, uint pathIndex) {
	uint boundaryIndex = capa_small_tile_value(pathIndex, tileCoord);
	if (boundaryIndex == CAPA_FULL_TILE && paths.values[pathIndex].blendMode == CAPA_BLEND_SRC_OVER) {
		vec4 src = paths.values[pathIndex].color;
		src.rgb *= src.a;
		if (!hasPendingFull) {
			hasPendingFull = true;
			pendingPathIndex = pathIndex;
			pendingColor = src;
		} else {
			pendingColor = capa_blend(pendingColor, src, CAPA_BLEND_SRC_OVER);
		}
	} else if (boundaryIndex != CAPA_NIL) {
		capa_flush_pending_full(base);
		uint node = base + orderPathCount - emittedCount - 1u;
		capa_write_order_node(node, pathIndex, boundaryIndex, 0u);
		emittedCount++;
	}
}

void main() {
	uint tileIndex = gl_GlobalInvocationID.x;
	if (tileIndex >= env.value.globalTileCount)
		return;

	ivec2 globalTileSpan = env.value.globalTileSpan;
	ivec2 tileCoord = env.value.globalTileBounds.xy +
										ivec2(tileIndex % globalTileSpan.x, tileIndex / globalTileSpan.x);

	// collect the number of paths that hit this global tile
	for (uint pathIndex = 0u; pathIndex < pc.pathCount; pathIndex++) {
		if (capa_path_hits_tile(pathIndex, tileCoord))
			orderPathCount++;
	}

	if (orderPathCount == 0u) {
		globalTiles.values[tileIndex] = CAPAGlobalTile(CAPA_NIL, 0u);
		return;
	}

	uint base = atomicAdd(env.value.orderedPathTileCount, orderPathCount);
	if (base + orderPathCount > pc.maxPathTileCount) {
		globalTiles.values[tileIndex] = CAPAGlobalTile(CAPA_NIL, 0u);
		return;
	}

	for (uint pathIndex = 0; pathIndex < pc.pathCount; pathIndex++) {
		if (capa_path_hits_tile(pathIndex, tileCoord))
			capa_emit_path(tileCoord, base, pathIndex);
	}

	capa_flush_pending_full(base);
	if (emittedCount == 0u) {
		globalTiles.values[tileIndex] = CAPAGlobalTile(CAPA_NIL, 0u);
		return;
	}

	// write the contiguous span for this global tile
	globalTiles.values[tileIndex] = CAPAGlobalTile(base + orderPathCount - emittedCount, emittedCount);
}
