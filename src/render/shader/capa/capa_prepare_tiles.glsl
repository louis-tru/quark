// CAPA prepare tiles pass.
// Allocate path-tile rows from prepared path bounds and publish path tile ranges
// only when the full row allocation fits the row budget.

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

layout(binding=3,set=0,std430) buffer CAPATileRows {
	CAPAPathTileRow values[];
} tileRows;

void capa_prepare_path_inverse_matrix(uint pathIndex) {
	vec4 mx = paths.values[pathIndex].matrixX;
	vec4 my = paths.values[pathIndex].matrixY;
	float det = mx.x * my.y - mx.y * my.x;
	if (abs(det) <= 1e-12) {
		paths.values[pathIndex].inverseMatrixX = vec4(1.0, 0.0, 0.0, 0.0);
		paths.values[pathIndex].inverseMatrixY = vec4(0.0, 1.0, 0.0, 0.0);
		return;
	}

	float invDet = 1.0 / det;
	paths.values[pathIndex].inverseMatrixX = vec4(
		my.y * invDet,
		-mx.y * invDet,
		(mx.y * my.z - my.y * mx.z) * invDet,
		0.0
	);
	paths.values[pathIndex].inverseMatrixY = vec4(
		-my.x * invDet,
		mx.x * invDet,
		(my.x * mx.z - mx.x * my.z) * invDet,
		0.0
	);
}

void main() {
	uint pathIndex = gl_GlobalInvocationID.x;
	if (pathIndex >= pc.pathCount)
		return;

	capa_prepare_path_inverse_matrix(pathIndex);

	ivec4 bounds = paths.values[pathIndex].bounds;
	ivec4 tileBounds = ivec4(
		bounds.x / CAPA_TILE_SIZE,
		bounds.y / CAPA_TILE_SIZE,
		(bounds.z + CAPA_TILE_SIZE - 1) / CAPA_TILE_SIZE,
		(bounds.w + CAPA_TILE_SIZE - 1) / CAPA_TILE_SIZE
	);
	// update global tile bounds
	capa_join_bounds_atomic(env.value.globalTileBounds, tileBounds);

	ivec2 tileSpan = tileBounds.zw - tileBounds.xy;
	if (tileSpan.x > 0 && tileSpan.y > 0) {
		// This is path-local staging space. The composite-facing z-linear
		// CAPAPathTile span is allocated later by capa_layer_plan.glsl.
		uint smallTileCount = tileSpan.x * tileSpan.y;
		uint tileOffset = atomicAdd(env.value.pathTileCount, smallTileCount);
		if (tileOffset + smallTileCount <= pc.maxPathTileCount) {
			// allocate tile rows for this path
			uint offset = atomicAdd(env.value.pathTileRowCount, tileSpan.y);
			int rows = min(tileSpan.y, int(pc.maxPathTileRowCount) - int(offset));
			for (int i = 0; i < rows; i++) {
				// write the index to the first tile of this row in the path tile row
				tileRows.values[offset+i].pathIndex = pathIndex;
				tileRows.values[offset+i].smallTileIndex = tileOffset + i * tileSpan.x;
				tileRows.values[offset+i].boundaryTileIndex = CAPA_NIL;
				tileRows.values[offset+i].boundaryTileCount = 0u;
			}
			// if all rows are allocated, write the tile offset and count to the path
			if (rows == tileSpan.y) {
				paths.values[pathIndex].tileOffset = tileOffset;
				paths.values[pathIndex].tileRowOffset = offset;
				paths.values[pathIndex].tileRect = ivec4(tileBounds.xy, tileSpan);
				paths.values[pathIndex].tileEnd = tileBounds.zw;
			}
		}
	}
}
