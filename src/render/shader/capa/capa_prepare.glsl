// CAPA pass 1.
// CPU uploads only path-space flattened edges and path metadata. This pass
// applies the path matrix/surface scale on GPU and prepares surface-space edges.

Qk_CONSTANT(
	float shortEdgeLength;
	uint edgeCount;
	uint maxTaskCount;
);

#import "_capa.glsl"

#comp
layout(local_size_x=64, local_size_y=1, local_size_z=1) in;

layout(binding=1,set=0,std430) buffer CAPAEnvironments {
	CAPAEnvironment value;
} env;

layout(binding=2,set=0,std430) buffer CAPAPaths {
	CAPAPath values[];
} paths;

layout(binding=3,set=0,std430) buffer CAPAEdges {
	CAPAEdge values[];
} edges;

layout(binding=4,set=0,std430) buffer CAPAShortEdgeTasks {
	CAPAShortEdgeTask values[];
} shortEdgeTasks;

vec4 capa_bounds(vec2 p0, vec2 p1) {
	return vec4(min(p0, p1), max(p0, p1));
}

void capa_clip_bounds(inout vec4 bounds, vec4 clip) {
	bounds.xy = max(bounds.xy, clip.xy);
	bounds.zw = min(bounds.zw, clip.zw);
}

void capa_join_bounds(inout vec4 bounds, vec4 clip) {
	bounds.xy = min(bounds.xy, clip.xy);
	bounds.zw = max(bounds.zw, clip.zw);
}

vec2 capa_transform(uint pathIndex, vec2 p) {
	return vec2(
		dot(paths.values[pathIndex].matrixX.xyz, vec3(p, 1.0)),
		dot(paths.values[pathIndex].matrixY.xyz, vec3(p, 1.0))
	);
}

bool capa_is_edge_valid(vec2 p0, vec2 p1, vec4 clip) {
	// 整条边都在clip上方的无效区域，不生成任何数据
	if (p0.y < clip.y)
		if (p1.y < clip.y) return false;
	// 整条边都在clip下方的无效区域，不生成任何数据
	if (p0.y > clip.w)
		if (p1.y > clip.w) return false;
	// 整条边都在clip右侧的无效区域，不生成任何数据
	if (p0.x > clip.z)
		if (p1.x > clip.z) return false;
	return true;
}

void main() {
	uint edgeIndex = gl_GlobalInvocationID.x;
	if (edgeIndex >= pc.edgeCount)
		return;

	uint pathIndex = edges.values[edgeIndex].pathIndex;
	vec4 clip = paths.values[pathIndex].clip;
	vec2 p0 = capa_transform(pathIndex, edges.values[edgeIndex].p0);
	vec2 p1 = capa_transform(pathIndex, edges.values[edgeIndex].p1);

	// 计算边包围盒
	vec4 bounds = capa_bounds(p0, p1);
	// 裁剪包围盒
	capa_clip_bounds(bounds, clip);
	// 并集path的bounds
	ivec4 ibounds = ivec4(floor(bounds.xy), ceil(bounds.zw));
	// join bounds atomic
	capa_join_bounds_atomic(paths.values[pathIndex].bounds, ibounds);

	if (!capa_is_edge_valid(p0, p1, clip)) {
		return;
	}
	vec2 delta = p1 - p0;
	float edgeLength = length(delta);
	// vec2 unit = edgeLength > 1e-6 ? delta / edgeLength : vec2(0.0);
	float winding = p1.y > p0.y ? 1 : p1.y < p0.y ? -1 : 0;
	float dy = p1.y - p0.y;
	float dxdy = winding != 0 && dy != 0.0 ? (p1.x - p0.x) / dy : 0.0;

	// overwrite with transformed edge
	edges.values[edgeIndex] = CAPAEdge(
		p0,
		p1,
		edgeLength,
		dxdy,
		winding,
		pathIndex
	);

	if (edgeLength < 1e-6)
		return;

	// write short edge tasks
	uint taskCount = uint(ceil(edgeLength / pc.shortEdgeLength));
	float invTaskCount = 1.0 / float(taskCount);
	uint offset = atomicAdd(env.value.taskCount, taskCount);
	if (offset >= pc.maxTaskCount)
		return;
	taskCount = min(taskCount, pc.maxTaskCount - offset);
	for (int i = 0; i < taskCount; i++) {
		shortEdgeTasks.values[offset + i] = CAPAShortEdgeTask(
			edgeIndex,
			pathIndex,
			float(i) * invTaskCount,
			float(i + 1) * invTaskCount
		);
	}
}
