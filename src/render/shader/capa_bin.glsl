// CAPA pass 2.
// Assign short-edge tasks to fixed-capacity tile-local lists. This is the
// first GPU-side tile bucketing step; later versions should replace the fixed
// capacity list with count + scan buffers.

Qk_CONSTANT(
	uint maxTaskCount;
	uint atlasTileCountX;
	uint atlasTileCountY;
	uint maxTileRefs;
	uint tileSpanX;
	uint tileSpanY;
	uint tileOriginX;
	uint tileOriginY;
);

#comp
layout(local_size_x=64, local_size_y=1, local_size_z=1) in;

const float CAPA_TILE_SIZE = 16.0;

struct CAPAPreparedEdge {
	vec2 p0;
	vec2 p1;
	vec2 unit;
	float length;
	float dxdy;
	int winding;
	uint pathIndex;
	uvec2 _pad;
};

struct CAPAShortEdgeTask {
	uint edgeIndex;
	uint pathIndex;
	float t0;
	float t1;
};

layout(binding=1,set=0,std430) readonly buffer CAPAPreparedEdges {
	CAPAPreparedEdge values[];
} edges;

layout(binding=2,set=0,std430) readonly buffer CAPAShortEdgeTasks {
	CAPAShortEdgeTask values[];
} shortEdgeTasks;

layout(binding=3,set=0,std430) readonly buffer CAPACounters {
	uint taskCount;
	uint overflow;
} counters;

layout(binding=4,set=0,std430) buffer CAPATileCounts {
	uint values[];
} tileCounts;

layout(binding=5,set=0,std430) writeonly buffer CAPATileRefs {
	uint values[];
} tileRefs;

void main() {
	uint taskIndex = gl_GlobalInvocationID.x;
	if (taskIndex >= pc.maxTaskCount || taskIndex >= counters.taskCount)
		return;

	CAPAShortEdgeTask task = shortEdgeTasks.values[taskIndex];
	CAPAPreparedEdge edge = edges.values[task.edgeIndex];
	if (edge.winding == 0)
		return;
	vec2 p0 = mix(edge.p0, edge.p1, task.t0);
	vec2 p1 = mix(edge.p0, edge.p1, task.t1);
	vec2 minP = min(p0, p1) - vec2(CAPA_TILE_SIZE + 1.0, 1.0);
	vec2 maxP = max(p0, p1) + vec2(1.0, 1.0);
	ivec2 tileMin = ivec2(floor(minP / CAPA_TILE_SIZE));
	ivec2 tileMax = ivec2(floor(maxP / CAPA_TILE_SIZE));
	ivec2 drawTileMin = ivec2(int(pc.tileOriginX), int(pc.tileOriginY));
	ivec2 drawTileMax = drawTileMin + ivec2(int(pc.tileSpanX), int(pc.tileSpanY)) - ivec2(1);
	if (tileMax.x < drawTileMin.x || tileMax.y < drawTileMin.y ||
		tileMin.x > drawTileMax.x || tileMin.y > drawTileMax.y)
		return;
	tileMin = clamp(tileMin, drawTileMin, drawTileMax);
	tileMax = clamp(tileMax, drawTileMin, drawTileMax);

	for (int tileY = tileMin.y; tileY <= tileMax.y; tileY++) {
		for (int tileX = tileMin.x; tileX <= tileMax.x; tileX++) {
			uint tileIndex =
				(uint(tileY) - pc.tileOriginY) * pc.tileSpanX +
				(uint(tileX) - pc.tileOriginX);
			uint slot = atomicAdd(tileCounts.values[tileIndex], 1u);
			if (slot < pc.maxTileRefs) {
				tileRefs.values[tileIndex * pc.maxTileRefs + slot] = taskIndex;
			}
		}
	}
}
