// CAPA tile coverage pass.
// This v1 pass consumes tile-local short-edge buckets from capa_bin. It still
// derives tile-left row area by scanning the full path edge list; later passes
// should replace that row scan with a prefix backdrop buffer.

Qk_CONSTANT(
	uint tileCount;
	uint atlasTileCountX;
	vec2 surfaceSize;
	uint maxTileRefs;
	uint aaMode; // 0=analytic area, 2=2x2 grid, 4=4x4 grid
	uint tileSpanX;
	uint tileOriginX;
	uint tileOriginY;
);

#comp

layout(local_size_x=256, local_size_y=1, local_size_z=1) in;

const int CAPA_TILE_SIZE = 16;
const int CAPA_TILE_SIZE_BITS = int(log2(CAPA_TILE_SIZE));

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

struct CAPAPath {
	vec4 matrixX; // path-to-surface x row: m0, m1, m2, 0
	vec4 matrixY; // path-to-surface y row: m3, m4, m5, 0
	vec4 color;
	uint edgeOffset;
	uint edgeCount;
	uvec2 _pad0;
};

layout(binding=1,set=0,std430) readonly buffer CAPAPreparedEdges {
	CAPAPreparedEdge values[];
} edges;

layout(binding=2,set=0,std430) readonly buffer CAPAPaths {
	CAPAPath values[];
} paths;

struct CAPAShortEdgeTask {
	uint edgeIndex;
	uint pathIndex;
	float t0;
	float t1;
};

layout(binding=3,set=0,std430) readonly buffer CAPAShortEdgeTasks {
	CAPAShortEdgeTask values[];
} shortEdgeTasks;

layout(binding=4,set=0,std430) readonly buffer CAPATileCounts {
	uint values[];
} tileCounts;

layout(binding=5,set=0,std430) readonly buffer CAPATileRefs {
	uint values[];
} tileRefs;

layout(binding=0,set=1,r8) uniform writeonly image2D atlasTex;

shared float tileRowArea[CAPA_TILE_SIZE];

float capa_edge_cross_x(float sampleY, CAPAPreparedEdge edge) {
	return fma(sampleY - edge.p0.y, edge.dxdy, edge.p0.x);
}

float capa_right_area_segment(float y0, float y1, float x0, float x1, float pixelX) {
	float localX0 = x0 - pixelX;
	float localX1 = x1 - pixelX;
	float mid = (localX0 + localX1) * 0.5;
	if (mid <= 0.0)
		return y1 - y0;
	if (mid >= 1.0)
		return 0.0;
	return (y1 - y0) * (1.0 - mid);
}

float capa_right_area(float y0, float y1, CAPAPreparedEdge edge, float pixelX) {
	float x0 = capa_edge_cross_x(y0, edge);
	float x1 = capa_edge_cross_x(y1, edge);
	float dy = y1 - y0;
	float dx = x1 - x0;
	if (dy <= 0.0)
		return 0.0;
	if (abs(dx) < 1e-6)
		return capa_right_area_segment(y0, y1, x0, x1, pixelX);

	float split0 = y0 + (pixelX - x0) * dy / dx;
	float split1 = y0 + (pixelX + 1.0 - x0) * dy / dx;
	if (split0 > split1) {
		float tmp = split0;
		split0 = split1;
		split1 = tmp;
	}
	split0 = clamp(split0, y0, y1);
	split1 = clamp(split1, y0, y1);

	float area = 0.0;
	float a = y0;
	float b = split0;
	if (b > a)
		area += capa_right_area_segment(a, b, capa_edge_cross_x(a, edge), capa_edge_cross_x(b, edge), pixelX);
	a = split0;
	b = split1;
	if (b > a)
		area += capa_right_area_segment(a, b, capa_edge_cross_x(a, edge), capa_edge_cross_x(b, edge), pixelX);
	a = split1;
	b = y1;
	if (b > a)
		area += capa_right_area_segment(a, b, capa_edge_cross_x(a, edge), capa_edge_cross_x(b, edge), pixelX);
	return area;
}

bool capa_inside_sample(float x, float y, CAPAPath path) {
	int winding = 0;
	for (uint i = 0; i < path.edgeCount; i++) {
		CAPAPreparedEdge edge = edges.values[path.edgeOffset + i];
		if (edge.winding == 0)
			continue;
		if ((edge.p0.y <= y && edge.p1.y > y) || (edge.p1.y <= y && edge.p0.y > y)) {
			float crossX = capa_edge_cross_x(y, edge);
			if (crossX <= x)
				winding += edge.winding;
		}
	}
	return winding != 0;
}

float capa_grid_coverage(float x0, float y0, CAPAPath path, uint grid) {
	uint covered = 0;
	for (uint sy = 0; sy < grid; sy++) {
		float y = y0 + (float(sy) + 0.5) / float(grid);
		for (uint sx = 0; sx < grid; sx++) {
			float x = x0 + (float(sx) + 0.5) / float(grid);
			if (capa_inside_sample(x, y, path))
				covered++;
		}
	}
	return float(covered) / float(grid * grid);
}

float capa_area_coverage(float x0, float y0, CAPAPath path) {
	float y1 = y0 + 1.0;
	float area = 0.0;
	for (uint i = 0; i < path.edgeCount; i++) {
		CAPAPreparedEdge edge = edges.values[path.edgeOffset + i];
		if (edge.winding == 0)
			continue;

		float edgeY0 = min(edge.p0.y, edge.p1.y);
		float edgeY1 = max(edge.p0.y, edge.p1.y);
		float beginY = max(y0, edgeY0);
		float endY = min(y1, edgeY1);
		if (beginY >= endY)
			continue;

		area += float(edge.winding) * capa_right_area(beginY, endY, edge, x0);
	}
	return clamp(abs(area), 0.0, 1.0);
}

float capa_row_area_backdrop(float tileLeft, float y0, CAPAPath path) {
	float y1 = y0 + 1.0;
	float area = 0.0;
	for (uint i = 0; i < path.edgeCount; i++) {
		CAPAPreparedEdge edge = edges.values[path.edgeOffset + i];
		if (edge.winding == 0)
			continue;

		float edgeY0 = min(edge.p0.y, edge.p1.y);
		float edgeY1 = max(edge.p0.y, edge.p1.y);
		float beginY = max(y0, edgeY0);
		float endY = min(y1, edgeY1);
		if (beginY >= endY)
			continue;

		area += float(edge.winding) * capa_right_area(beginY, endY, edge, tileLeft);
	}
	return area;
}

CAPAPreparedEdge capa_short_edge(CAPAShortEdgeTask task) {
	CAPAPreparedEdge edge = edges.values[task.edgeIndex];
	vec2 p0 = mix(edge.p0, edge.p1, task.t0);
	vec2 p1 = mix(edge.p0, edge.p1, task.t1);
	int winding = p1.y > p0.y ? 1 : p1.y < p0.y ? -1 : 0;
	float dy = p1.y - p0.y;
	float dxdy = winding != 0 && dy != 0.0 ? (p1.x - p0.x) / dy : 0.0;
	edge.p0 = p0;
	edge.p1 = p1;
	edge.dxdy = dxdy;
	edge.winding = winding;
	return edge;
}

float capa_area_coverage_binned(float x0, float y0, float tileLeft, CAPAPath path, uint tileRefIndex) {
	uint rawCount = tileCounts.values[tileRefIndex];
	if (rawCount > 0u)
		return capa_area_coverage(x0, y0, path);

	float y1 = y0 + 1.0;
	float area = tileRowArea[int(y0) & (CAPA_TILE_SIZE - 1)];
	uint count = min(rawCount, pc.maxTileRefs);
	for (uint i = 0; i < count; i++) {
		uint taskIndex = tileRefs.values[tileRefIndex * pc.maxTileRefs + i];
		CAPAShortEdgeTask task = shortEdgeTasks.values[taskIndex];
		if (task.pathIndex != 0u)
			continue;
		CAPAPreparedEdge edge = capa_short_edge(task);
		if (edge.winding == 0)
			continue;

		float edgeY0 = min(edge.p0.y, edge.p1.y);
		float edgeY1 = max(edge.p0.y, edge.p1.y);
		float beginY = max(y0, edgeY0);
		float endY = min(y1, edgeY1);
		if (beginY >= endY)
			continue;

		float localArea =
			capa_right_area(beginY, endY, edge, x0) -
			capa_right_area(beginY, endY, edge, tileLeft);
		area += float(edge.winding) * localArea;
	}
	return clamp(abs(area), 0.0, 1.0);
}

void main() {
	uint localTileIndex = gl_WorkGroupID.x;
	uint pixelIndex = gl_LocalInvocationIndex;
	if (localTileIndex >= pc.tileCount || pixelIndex >= CAPA_TILE_SIZE * CAPA_TILE_SIZE)
		return;

	uint tileX = pc.tileOriginX + localTileIndex % pc.tileSpanX;
	uint tileY = pc.tileOriginY + localTileIndex / pc.tileSpanX;
	float tileLeft = float(tileX * CAPA_TILE_SIZE);
	CAPAPath path = paths.values[0];
	uint pixelX = pixelIndex & (CAPA_TILE_SIZE - 1);
	uint pixelY = pixelIndex >> CAPA_TILE_SIZE_BITS;
	if (pixelIndex < CAPA_TILE_SIZE) {
		float y0 = float(tileY * CAPA_TILE_SIZE + pixelIndex);
		tileRowArea[pixelIndex] = capa_row_area_backdrop(tileLeft, y0, path);
	}
	barrier();
	uint surfaceX = tileX * CAPA_TILE_SIZE + pixelX;
	uint surfaceY = tileY * CAPA_TILE_SIZE + pixelY;
	if (surfaceX >= uint(pc.surfaceSize.x) || surfaceY >= uint(pc.surfaceSize.y))
		return;
	float x0 = float(surfaceX);
	float y0 = float(surfaceY);

	float coverage = pc.aaMode == 1 ? capa_area_coverage(x0, y0, path):
		pc.aaMode == 2 ? capa_grid_coverage(x0, y0, path, 2):
		pc.aaMode == 4 ? capa_grid_coverage(x0, y0, path, 4):
		capa_area_coverage_binned(x0, y0, tileLeft, path, localTileIndex);
	imageStore(atlasTex, ivec2(surfaceX, surfaceY), vec4(coverage, 0.0, 0.0, 1.0));
}
