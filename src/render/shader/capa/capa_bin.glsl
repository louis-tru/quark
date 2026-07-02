// CAPA short-edge bin pass.
// Bin each short-edge task into the path tiles touched by the short segment.
// The prepare pass keeps short edges shorter than a tile, so a task can touch at most
// three tiles: start, optional crossed neighbor, and end.

Qk_CONSTANT(
	uint maxTaskCount;
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

layout(binding=3,set=0,std430) readonly buffer CAPAEdges {
	CAPAEdge values[];
} edges;

layout(binding=4,set=0,std430) buffer CAPAShortEdges {
	CAPAShortEdgeNode values[];
} shortEdges;

layout(binding=5,set=0,std430) readonly buffer CAPAShortEdgeTasks {
	CAPAShortEdgeTask values[];
} shortEdgeTasks;

layout(binding=6,set=0,std430) buffer CAPASmallTiles {
	CAPASmallTile values[];
} smallTiles;

// Left/top Closed, Right/bottom Open
int capa_tile_coord_half_open(bool isMin, float value) {
	return isMin
		? int(floor(value / CAPA_TILE_SIZE_F))
		: int(ceil(value / CAPA_TILE_SIZE_F)) - 1;
}

CAPAShortEdge capa_short_edge(uint edgeIndex, float t0, float t1) {
	vec2 p0 = edges.values[edgeIndex].p0;
	vec2 p1 = edges.values[edgeIndex].p1;
	return CAPAShortEdge(
		mix(p0, p1, t0),
		mix(p0, p1, t1)
	);
}

void capa_emit_edge(ivec2 tileCoord, CAPAShortEdge edge, float winding, uint pathIndex, uint shortEdgeIndex) {
	ivec4 tileRect = paths.values[pathIndex].tileRect;
	ivec2 local = tileCoord - tileRect.xy;

	// The left side is clamped into the first path tile because left-of-row
	// edges still affect backdrop/prefix. Other out-of-range sides are outside
	// this path's allocated staging row and can be ignored.
	if (local.y < 0 || local.y >= tileRect.w || local.x >= tileRect.z)
		return;

	if (local.x < 0) {
		local.x = 0; // clamp to the left edge of the path tile rect
	}

	// if the edge is almost horizontal
	// Nearly-horizontal edges do not contribute area/backdrop after prepare
	// clears their winding, but the touched tile still needs a boundary tile.
	// Move the marker outside all real rows so backdrop/coverage skip it while
	// the tile remains non-empty for boundary allocation.
	if (winding == 0.0) {
		edge.p0.y = -1.0e20;
		edge.p1.y = -1.0e20;
	}
	// Link a fully written per-task node into the small tile. Each task owns
	// taskIndex*3 + {0,1,2}, so this avoids same-bucket append repair logic.
	uint tileIndex = paths.values[pathIndex].tileOffset + local.y * tileRect.z + local.x;
	uint next = atomicExchange(smallTiles.values[tileIndex].value, shortEdgeIndex);
	shortEdges.values[shortEdgeIndex] = CAPAShortEdgeNode(edge, next, 0u);
}

void main() {
	uint taskIndex = gl_GlobalInvocationID.x;
	if (taskIndex >= env.value.realTaskCount)
		return;

	CAPAShortEdgeTask task = shortEdgeTasks.values[taskIndex];
	CAPAShortEdge edge = capa_short_edge(task.edgeIndex, task.t0, task.t1);
	float dxdy = edges.values[task.edgeIndex].dxdy;
	float winding = edges.values[task.edgeIndex].winding;

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
	// An edge exactly on a vertical tile boundary belongs to the tile on the
	// right. This keeps half-open tile ownership stable and avoids double links.
	if (tile0.x > tile1.x) {
		tile0.x = tile1.x;
	}

	// short edge store index,
	// each short edge task can emit at most 3 short edges to the path tile
	uint shortEdgeIndex = taskIndex * 3;

	capa_emit_edge(tile0, edge, winding, task.pathIndex, shortEdgeIndex);

	if (tile0 == tile1)
		return;

	// Diagonal short edges can touch one intermediate neighbor before reaching
	// tile1. The short-edge length bound keeps this branch to at most one tile.
	if (tile0.x != tile1.x && tile0.y != tile1.y) {
		float dx = (tile0.x + 1) * CAPA_TILE_SIZE_F - p0.x;
		float dy = abs((tile0.y + (p0yIsMin ? 1 : 0)) * CAPA_TILE_SIZE_F - p0.y);
		float dy2 = abs(dx / dxdy);

		if (dy2 < dy) {
			capa_emit_edge(tile0 + ivec2(1, 0), edge, winding, task.pathIndex, shortEdgeIndex + 1);
		} else if (dy2 > dy) {
			capa_emit_edge(tile0 + ivec2(0, p0yIsMin ? 1 : -1), edge, winding, task.pathIndex, shortEdgeIndex + 1);
		}
	}

	capa_emit_edge(tile1, edge, winding, task.pathIndex, shortEdgeIndex + 2);
}
