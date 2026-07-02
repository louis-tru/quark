// CAPA composite pass.
// Ordered global-tile pure-color compositor.

Qk_CONSTANT(
	vec4 clearColor;
	uvec2 surfaceOffset;
);

#import "_capa.glsl"

#comp
layout(local_size_x=8, local_size_y=8, local_size_z=1) in;

layout(binding=1,set=0,std430) readonly buffer CAPAEnvironments {
	CAPAEnvironment value;
} env;

layout(binding=2,set=0,std430) readonly buffer CAPAPaths {
	CAPAPath values[];
} paths;

layout(binding=3,set=0,std430) readonly buffer CAPAGlobalTiles {
	CAPAGlobalTile values[];
} globalTiles;

layout(binding=4,set=0,std430) readonly buffer CAPAPathTiles {
	CAPAPathTile values[];
} pathTiles;

layout(binding=5,set=0,std430) readonly buffer CAPACoverageTiles {
	CAPACoverageTile values[];
} coverageTiles;

layout(binding=1,set=1,rgba8) uniform image2D dstImage;

#define CAPA_COMPOSITE_CLEAR_DST (1u << 16)
const uvec2 CAPA_COMPOSITE_SUBGROUPS = uvec2(2u, 2u);
const float eps = 1e-6;

float capa_load_boundary_coverage(uint coverageIndex, uint pixelIndex) {
	uint word = coverageTiles.values[coverageIndex].values[pixelIndex >> 2u];
	uint shift = (pixelIndex & 3u) << 3u;
	return float((word >> shift) & 255u) * (1.0 / 255.0);
}

float capa_path_tile_coverage(uint coverageIndex, uint pixelIndex) {
	if (coverageIndex == CAPA_NIL)
		return 0.0;
	if (coverageIndex == CAPA_FULL_TILE)
		return 1.0;
	return capa_load_boundary_coverage(coverageIndex, pixelIndex);
}

struct CAPACoverageGroup {
	bool hasValue;
	uint blendMode;
	float coverage;
	vec4 src;
};

CAPABlendFront front;
CAPACoverageGroup group;

CAPACoverageGroup capa_group_empty() {
	return CAPACoverageGroup(false, CAPA_BLEND_SRC_OVER, 0.0, vec4(0.0));
}

bool capa_front_ignores_bottom() {
	const vec4 epsVec4 = vec4(eps);
	return all(lessThan(abs(front.scale), epsVec4)) && all(lessThan(abs(front.alphaTo), epsVec4));
}

bool capa_group_flush() {
	if (!group.hasValue)
		return false;
	// A completed same-mode group is one weighted PMA source.
	capa_blend_front_append(front, group.src, group.blendMode);
	group = capa_group_empty();
	return capa_front_ignores_bottom();
}

bool capa_group_add(vec4 src, uint blendMode, float coverage) {
	vec4 coveredSrc = src * coverage;
	if (!group.hasValue) {
		group.hasValue = true;
		group.blendMode = blendMode;
		group.coverage = coverage;
		group.src = coveredSrc;
	} else {
		group.coverage += coverage;
		group.src += coveredSrc;
	}
	if (group.coverage >= 1.0 - eps) {
		return capa_group_flush();
	}
	return false; // return true if the front ignores the bottom, false otherwise
}

bool capa_group_add_layer(vec4 src, uint blendMode, float coverage) {
	if (coverage <= eps)
		return true;

	// A blend-mode change is a hard group boundary: different modes need their
	// own completed coverage/color before they are mixed into the front expression.
	if (group.hasValue && group.blendMode != blendMode) {
		if (capa_group_flush())
			return true;
	}

	do {
		// Split only by area overflow: 0.9 + 0.2 becomes 0.1 to finish the
		// current group, then 0.1 in the next group. This keeps group coverage
		// continuous while preserving the invariant that no group exceeds 1.0.
		float take = min(coverage, 1.0 - group.coverage);
		if (capa_group_add(src, blendMode, take))
			return true;
		coverage -= take;
	} while (coverage > eps);

	return false;
}

void main() {
	uvec2 tileSlot = gl_WorkGroupID.xy / CAPA_COMPOSITE_SUBGROUPS;
	uvec2 subTile = gl_WorkGroupID.xy - tileSlot * CAPA_COMPOSITE_SUBGROUPS;
	uint tileSpanX = gl_NumWorkGroups.x / CAPA_COMPOSITE_SUBGROUPS.x;
	uint globalTileIndex = tileSlot.y * tileSpanX + tileSlot.x;
	uint count = globalTiles.values[globalTileIndex].count;
	bool clearDst = (pc.flags & CAPA_COMPOSITE_CLEAR_DST) != 0u;

	// If there are no path tiles in this global tile,
	// and we are not clearing the destination, then we can skip this tile entirely.
	if (count == 0u && !clearDst) {
		return;
	}

	uvec2 tileCoord = tileSlot + uvec2(env.value.globalTileBounds.xy);
	uvec2 localPixel = subTile * gl_WorkGroupSize.xy + gl_LocalInvocationID.xy;
	uvec2 pixel = pc.surfaceOffset + tileCoord * CAPA_TILE_SIZE_U + localPixel;
	uint pixelIndex = localPixel.y * CAPA_TILE_SIZE_U + localPixel.x;

	// define a local front and coverage group for this pixel.
	front = capa_blend_front_identity();
	group = capa_group_empty();

	uint head = globalTiles.values[globalTileIndex].head;
	for (uint i = 0u; i < count; i++) {
		uint node = head + i;
		if (pathTiles.values[node].color != 0u) {
			vec4 src = capa_unpack_rgba8(pathTiles.values[node].color);
			if (capa_group_add_layer(src, CAPA_BLEND_SRC_OVER, 1.0))
				break;
			continue;
		}
		uint pathIndex = pathTiles.values[node].pathIndex;
		uint coverageTileIndex = pathTiles.values[node].coverageTileIndex;
		float coverage = capa_path_tile_coverage(coverageTileIndex, pixelIndex);
		if (coverage <= 0.0)
			continue;

		vec4 src = paths.values[pathIndex].color;
		if (capa_group_add_layer(src, paths.values[pathIndex].blendMode, coverage))
			break;
	}

	// If the group has a value, we need to flush it into the front before resolving the final color.
	if (group.hasValue)
		capa_blend_front_append(front, group.src, group.blendMode);

	vec4 bottom = clearDst ? pc.clearColor : imageLoad(dstImage, ivec2(pixel));
	imageStore(dstImage, ivec2(pixel), capa_blend_front_resolve(front, bottom));
}
