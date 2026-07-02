// CAPA layer plan pass.
// Build each global tile's contiguous ordered path-tile layer span after
// classification, and allocate z-linear coverage tiles for retained boundary tiles.

Qk_CONSTANT(
	uint pathCount;
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

layout(binding=6,set=0,std430) buffer CAPACoverageTiles {
	CAPACoverageTile values[];
} coverageTiles;

uint pathTileIndex;
uint emittedCount = 0u;
uint emittedBoundaryTileCount = 0u;
bool hasPendingSrcOverFull = false;
uint pendingPathIndex;
vec4 pendingSrcOverColor;

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

void capa_write_order_node(uint pathIndex, uint boundaryIndex, uint color) {
	uint storeIndex = pathTileIndex + emittedCount;
	pathTiles.values[storeIndex].pathIndex = pathIndex;
	// Temporarily stores the source CAPABoundaryTile index; after this tile's
	// z-linear coverage slots are allocated, it is replaced with CAPACoverageTile.
	pathTiles.values[storeIndex].coverageTileIndex = boundaryIndex;
	pathTiles.values[storeIndex].color = color;
	if (boundaryIndex < CAPA_FULL_TILE)
		emittedBoundaryTileCount++;
	emittedCount++;
}

void capa_flush_pending_src_over_full() {
	if (!hasPendingSrcOverFull)
		return;
	// Consecutive full SrcOver color tiles can be represented by one packed
	// PMA color node. Boundary/image/other blend modes flush this run first.
	uint packedColor = capa_pack_rgba8(pendingSrcOverColor);
	if (packedColor != 0u)
		capa_write_order_node(pendingPathIndex, CAPA_FULL_TILE, packedColor);
	hasPendingSrcOverFull = false;
}

void capa_allocate_coverage_tiles() {
	if (emittedBoundaryTileCount == 0u)
		return;
	// Allocate one z-linear coverage segment per global tile so composite walks
	// coverage in the same order as the retained path-tile span.
	uint cursor = atomicAdd(env.value.coveragePassGroups_Size16_2.w, emittedBoundaryTileCount);
	for (uint i = 0u; i < emittedCount; i++) {
		uint node = pathTileIndex + i;
		uint boundaryIndex = pathTiles.values[node].coverageTileIndex;
		if (boundaryIndex < CAPA_FULL_TILE) {
			coverageTiles.values[cursor].boundaryTileIndex = boundaryIndex;
			pathTiles.values[node].coverageTileIndex = cursor++;
		}
	}
	// Coverage runs 16x2 rows per group, so two coverage tiles share one group.
	atomicMax(env.value.coveragePassGroups_Size16_2.x, (cursor + 1u) / 2u);
}

bool capa_emit_path(ivec2 tileCoord, uint pathIndex) {
	uint boundaryIndex = capa_small_tile_value(pathIndex, tileCoord);
	if (boundaryIndex == CAPA_NIL)
		return true;

	uint blendMode = paths.values[pathIndex].blendMode;
	uint paintType = paths.values[pathIndex].paintType;
	uint flags = paths.values[pathIndex].flags;
	if (boundaryIndex == CAPA_FULL_TILE && blendMode == CAPA_BLEND_SRC_OVER &&
			paintType == CAPA_PAINT_SOLID) {
		vec4 src = paths.values[pathIndex].color;
		if (!hasPendingSrcOverFull) {
			hasPendingSrcOverFull = true;
			pendingPathIndex = pathIndex;
			pendingSrcOverColor = src;
		} else {
			pendingSrcOverColor = capa_blend(pendingSrcOverColor, src, CAPA_BLEND_SRC_OVER);
		}
		if ((flags & CAPA_FLAG_PAINT_OPAQUE) != 0u) {
			// A fully opaque front SrcOver full tile hides all remaining lower
			// full/boundary layers for this global tile.
			return false;
		}
	} else {
		capa_flush_pending_src_over_full();
		capa_write_order_node(pathIndex, boundaryIndex, 0u);
		if (boundaryIndex == CAPA_FULL_TILE && blendMode == CAPA_BLEND_SRC_OVER &&
				(flags & CAPA_FLAG_PAINT_OPAQUE) != 0u) {
			if (paintType == CAPA_PAINT_IMAGE) {
				// TODO ...
			} else {
				return false;
			}
		}
	}
	return true;
}

void main() {
	uint tileIndex = gl_GlobalInvocationID.x;
	if (tileIndex >= env.value.globalTileCount)
		return;

	ivec2 globalTileSpan = env.value.globalTileSpan;
	ivec2 tileCoord = env.value.globalTileBounds.xy +
										ivec2(tileIndex % globalTileSpan.x, tileIndex / globalTileSpan.x);
	uint layerPathCount = 0u;

	// Count conservatively by path tileRect first. Some hits become NIL/full-run
	// skips during emission, so the final globalTile.count is adjusted afterward.
	for (uint i = 0u; i < pc.pathCount; i++) {
		if (capa_path_hits_tile(i, tileCoord))
			layerPathCount++;
	}

	if (layerPathCount == 0u) {
		globalTiles.values[tileIndex] = CAPAGlobalTile(CAPA_NIL, 0u);
		return;
	}

	pathTileIndex = atomicAdd(env.value.layerPlanPathTileCount, layerPathCount);

	// Emit from front to back. CAPAGlobalTile.head therefore points at the
	// topmost retained layer, which lets composite stop when front ignores bottom.
	for (uint i = 0u; i < pc.pathCount; i++) {
		uint pathIndex = pc.pathCount - 1u - i;
		if (capa_path_hits_tile(pathIndex, tileCoord)) {
			if (!capa_emit_path(tileCoord, pathIndex))
				break;
		}
	}
	capa_flush_pending_src_over_full();

	if (emittedCount == 0u) {
		globalTiles.values[tileIndex] = CAPAGlobalTile(CAPA_NIL, 0u);
		return;
	}
	capa_allocate_coverage_tiles();

	// write the contiguous span for this global tile
	globalTiles.values[tileIndex] = CAPAGlobalTile(pathTileIndex, emittedCount);
}
