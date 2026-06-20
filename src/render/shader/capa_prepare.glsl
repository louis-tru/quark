// CAPA pass 1.
// CPU uploads only path-space flattened edges and path metadata. This pass
// applies the path matrix/surface scale on GPU and prepares surface-space edges
// for the current direct tile coverage prototype.

Qk_CONSTANT(
	uint edgeCount;
	uint maxTaskCount;
	float shortEdgeLength;
	float _pad0;
);

#comp
layout(local_size_x=64, local_size_y=1, local_size_z=1) in;

struct CAPAEdge {
	vec2 p0; // flattened path-space start point
	vec2 p1; // flattened path-space end point
	uint pathIndex;
	uint _pad;
};

struct CAPAPath {
	vec4 matrixX;
	vec4 matrixY;
	vec4 color;
	uint edgeOffset;
	uint edgeCount;
	uvec2 _pad0;
};

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

layout(binding=1,set=0,std430) readonly buffer CAPAEdges {
	CAPAEdge values[];
} edges;

layout(binding=2,set=0,std430) readonly buffer CAPAPaths {
	CAPAPath values[];
} paths;

layout(binding=3,set=0,std430) writeonly buffer CAPAPreparedEdges {
	CAPAPreparedEdge values[];
} preparedEdges;

layout(binding=4,set=0,std430) buffer CAPAShortEdgeTasks {
	CAPAShortEdgeTask values[];
} shortEdgeTasks;

layout(binding=5,set=0,std430) buffer CAPACounters {
	uint taskCount;
	uint overflow;
} counters;

vec2 capa_transform(uint pathIndex, vec2 p) {
	CAPAPath path = paths.values[pathIndex];
	return vec2(
		dot(path.matrixX.xyz, vec3(p, 1.0)),
		dot(path.matrixY.xyz, vec3(p, 1.0))
	);
}

void main() {
	uint edgeIndex = gl_GlobalInvocationID.x;
	if (edgeIndex >= pc.edgeCount)
		return;

	CAPAEdge edge = edges.values[edgeIndex];
	vec2 p0 = capa_transform(edge.pathIndex, edge.p0);
	vec2 p1 = capa_transform(edge.pathIndex, edge.p1);
	vec2 delta = p1 - p0;
	float edgeLength = length(delta);
	vec2 unit = edgeLength > 1e-6 ? delta / edgeLength : vec2(0.0);
	int winding = p1.y > p0.y ? 1 : p1.y < p0.y ? -1 : 0;
	float dy = p1.y - p0.y;
	float dxdy = winding != 0 && dy != 0.0 ? (p1.x - p0.x) / dy : 0.0;
	preparedEdges.values[edgeIndex] = CAPAPreparedEdge(
		p0,
		p1,
		unit,
		edgeLength,
		dxdy,
		winding,
		edge.pathIndex,
		uvec2(0)
	);

	if (edgeLength <= 1e-6)
		return;

	uint taskCount = max(1u, uint(ceil(edgeLength / pc.shortEdgeLength)));
	float invTaskCount = 1.0 / float(taskCount);
	uint taskOffset = atomicAdd(counters.taskCount, taskCount);
	if (taskOffset + taskCount > pc.maxTaskCount) {
		atomicAdd(counters.overflow, 1u);
		return;
	}
	for (uint i = 0; i < taskCount; i++) {
		shortEdgeTasks.values[taskOffset + i] = CAPAShortEdgeTask(
			edgeIndex,
			edge.pathIndex,
			float(i) * invTaskCount,
			float(i + 1u) * invTaskCount
		);
	}
}
