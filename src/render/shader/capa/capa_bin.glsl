// CAPA pass 3.
// Bin each short-edge task into the path tiles touched by the short segment.
// Pass1 keeps short edges shorter than a tile, so a task can touch at most
// three tiles: start, optional crossed neighbor, and end.

Qk_CONSTANT(
	uint maxTaskCount;
	uint maxShortEdgeChunkCount;
	uint maxBoundaryTileCount;
);

#import "_capa.glsl"

#comp
layout(local_size_x=64, local_size_y=1, local_size_z=1) in;

layout(binding=1,set=0,std430) buffer CAPAEnvironments {
	CAPAEnvironment value;
} env;

layout(binding=2,set=0,std430) readonly buffer CAPAPaths {
	CAPAPath values[];
} paths;

layout(binding=3,set=0,std430) readonly buffer CAPAEdges {
	CAPAEdge values[];
} edges;

layout(binding=4,set=0,std430) readonly buffer CAPAShortEdgeTasks {
	CAPAShortEdgeTask values[];
} shortEdgeTasks;

layout(binding=5,set=0,std430) buffer CAPAPathTiles {
	CAPAPathTile values[];
} pathTiles;

layout(binding=6,set=0,std430) buffer CAPABoundaryTiles {
	CAPABoundaryTile values[];
} boundaryTiles;

layout(binding=7,set=0,std430) buffer CAPAShortEdgeChunks {
	CAPAShortEdgeChunk values[];
} shortEdgeChunks;

// Left/top Closed, Right/bottom Open
int capa_tile_coord_half_open(bool isMin, float value) {
	return isMin
		? int(floor(value / CAPA_TILE_SIZE_F))
		: int(ceil(value / CAPA_TILE_SIZE_F)) - 1;
}

void capa_alloc_boundary_tile(ivec2 tileCoord, uint pathTileIndex, uint pathIndex, float winding) {
	if (pathTiles.values[pathTileIndex].boundaryTileIndex != 0u)
		return;

	// try atomic lock to allocate a boundary tile for this path tile
	if (atomicCompSwap(pathTiles.values[pathTileIndex].boundaryTileIndex, 0u, 2u) != 0u)
		return;

	uint boundaryIndex = atomicAdd(env.value.boundaryTileCount, 1u);
	if (boundaryIndex < pc.maxBoundaryTileCount) {
		if (winding != 0) {
			uint chunkIndex = atomicAdd(env.value.shortEdgeChunkCount, 1u);
			if (chunkIndex < pc.maxShortEdgeChunkCount) {
				shortEdgeChunks.values[chunkIndex].count = 0u;
				shortEdgeChunks.values[chunkIndex].next =
					atomicExchange(pathTiles.values[pathTileIndex].shortEdgeChunkHead, chunkIndex);
			}
		}
		boundaryTiles.values[boundaryIndex].pathTileIndex = pathTileIndex;
		boundaryTiles.values[boundaryIndex].pathIndex = pathIndex;
		boundaryTiles.values[boundaryIndex].tileCoord = tileCoord;
		pathTiles.values[pathTileIndex].boundaryTileIndex = boundaryIndex;
	}
}

void capa_emit_edge(ivec2 tileCoord, CAPAShortEdge edge, uint pathIndex) {
	ivec4 tileRect = paths.values[pathIndex].tileRect;
	ivec2 local = tileCoord - tileRect.xy;

	// check if the tileCoord is in the path tile rect, if not, ignore this edge
	if (local.y < 0 || local.y >= tileRect.w || local.x >= tileRect.z)
		return;

	if (local.x < 0)
		local.x = 0; // clamp to the left edge of the path tile rect

	// get the path tile index for this tile coordinate
	uint pathTileIndex = paths.values[pathIndex].tileOffset + local.y * tileRect.z + local.x;

	// allocate a boundary tile for this path tile
	capa_alloc_boundary_tile(tileCoord, pathTileIndex, pathIndex, edge.winding);

	// don't need to store it if is horizontal edge
	if (edge.winding == 0)
		return;

	uint head = pathTiles.values[pathTileIndex].shortEdgeChunkHead;
	if (head != CAPA_NIL) {
		uint offset = atomicAdd(shortEdgeChunks.values[head].count, 1u);
		if (offset < CAPA_SHORT_EDGE_CHUNK_SIZE) {
			// store the short edge in the chunk
			shortEdgeChunks.values[head].values[offset] = edge;
			return;
		}
	}
	uint chunkIndex = atomicAdd(env.value.shortEdgeChunkCount, 1u);
	if (chunkIndex >= pc.maxShortEdgeChunkCount)
		return; // overflow, ignore this short edge

	shortEdgeChunks.values[chunkIndex].count = 1u;
	shortEdgeChunks.values[chunkIndex].values[0] = edge;
	shortEdgeChunks.values[chunkIndex].next =
		atomicExchange(pathTiles.values[pathTileIndex].shortEdgeChunkHead, chunkIndex);
}

CAPAShortEdge capa_short_edge(uint edgeIndex, float t0, float t1) {
	vec2 p0 = edges.values[edgeIndex].p0;
	vec2 p1 = edges.values[edgeIndex].p1;
	return CAPAShortEdge(
		mix(p0, p1, t0),
		mix(p0, p1, t1),
		edges.values[edgeIndex].dxdy,
		float(edges.values[edgeIndex].winding)
	);
}

void main() {
	uint taskIndex = gl_GlobalInvocationID.x;
	uint taskCount = min(env.value.taskCount, pc.maxTaskCount);
	if (taskIndex >= taskCount)
		return;

	CAPAShortEdgeTask task = shortEdgeTasks.values[taskIndex];
	CAPAShortEdge edge = capa_short_edge(task.edgeIndex, task.t0, task.t1);

	vec2 p0 = edge.p0.x < edge.p1.x ? edge.p0: edge.p1;
	vec2 p1 = edge.p0.x < edge.p1.x ? edge.p1: edge.p0;
	bool p0yIsMin = p0.y < p1.y;
	int minTileY = int(floor(min(p0.y, p1.y) / CAPA_TILE_SIZE_F));
	int maxTileY = int(ceil(max(p0.y, p1.y) / CAPA_TILE_SIZE_F)) - 1;

	// compute the tile coordinates for the start and end points of the edge
	ivec2 tile0 = ivec2(
		floor(p0.x / CAPA_TILE_SIZE_F),
		p0yIsMin ? minTileY: maxTileY
	);
	ivec2 tile1 = ivec2(
		ceil(p1.x / CAPA_TILE_SIZE_F) - 1,
		p0yIsMin ? maxTileY: minTileY
	);

	// it's a horizontal tile boundary that don't need to emit edge
	if (minTileY > maxTileY) {
		return;
	}
	// it's a vertical tile boundary,
	// fix the tile0.x to tile1.x to avoid emitting edge twice
	if (tile0.x > tile1.x) {
		tile0.x = tile1.x;
	}

	capa_emit_edge(tile0, edge, task.pathIndex);

	if (tile0 == tile1)
		return;

	// if the edge crosses a tile boundary x/y
	if (tile0.x != tile1.x && tile0.y != tile1.y) {
		float dx = (tile0.x + 1) * CAPA_TILE_SIZE_F - p0.x;
		float dy = abs((tile0.y + (p0yIsMin ? 1 : 0)) * CAPA_TILE_SIZE_F - p0.y);
		float dy2 = abs(dx / edge.dxdy);

		if (dy2 < dy) {
			capa_emit_edge(tile0 + ivec2(1, 0), edge, task.pathIndex);
		} else if (dy2 > dy) {
			capa_emit_edge(tile0 + ivec2(0, p0yIsMin ? 1 : -1), edge, task.pathIndex);
		}
	}

	capa_emit_edge(tile1, edge, task.pathIndex);
}
