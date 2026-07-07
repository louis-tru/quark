// CAPA composite pass.
// Ordered global-tile pure-color compositor.

#extension GL_EXT_nonuniform_qualifier : require

Qk_CONSTANT(
	vec4 clearColor;
	ivec2 surfaceOffset;
);

#import "_capa.glsl"

#comp
layout(local_size_x=8, local_size_y=8, local_size_z=1) in;

layout(binding=1,set=0,std430) readonly buffer CAPAEnvironments {
	CAPAEnvironment value;
} env;

layout(binding=4,set=0,std430) readonly buffer CAPAPaths {
	CAPAPath values[];
} paths;

layout(binding=5,set=0,std430) readonly buffer CAPAGlobalTiles {
	CAPAGlobalTile values[];
} globalTiles;

layout(binding=6,set=0,std430) readonly buffer CAPAPathTiles {
	CAPAPathTile values[];
} pathTiles;

layout(binding=7,set=0,std430) readonly buffer CAPACoverageTiles {
	CAPACoverageTile values[];
} coverageTiles;

layout(binding=8,set=0,std430) readonly buffer CAPAGradientPaints {
	CAPAGradientPaint values[];
} gradientPaints;

layout(binding=9,set=0,std430) readonly buffer CAPAImagePaints {
	CAPAImagePaint values[];
} imagePaints;

layout(binding=10,set=0,std430) readonly buffer Colors {
	vec4 values[];
} colors;

layout(binding=11,set=0,std430) readonly buffer Positions {
	float values[];
} positions;

layout(binding=12,set=0,std430) readonly buffer ClipStatBlock {
	ivec2 begin; // x:left, y:top
	int op;
} clipStat;

layout(binding=0,set=1,r8) uniform readonly image2D clipTex; // clip texture buffer
layout(binding=1,set=1,rgba8) uniform image2D dstImage;
layout(binding=0,set=2) uniform texture2D images[];
layout(binding=0,set=3) uniform sampler samplers[];

const uvec2 CAPA_COMPOSITE_SUBGROUPS = uvec2(2u, 2u);
const float eps = 1e-6;
const float CAPA_GROUP_COVERAGE_QUANTIZE_STEPS = 8.0;

// clipStat.op: 0 for intersect, 1 for difference
float clipCoverage(ivec2 fragCoord) {
	float coverage = imageLoad(clipTex, fragCoord - clipStat.begin).r;
	if (clipStat.op == 1)
		coverage = 1.0 - coverage; /* difference mode: invert coverage*/
	return coverage;
}

float capa_load_boundary_coverage(uint coverageIndex, uint pixelIndex) {
	// R8 coverage is packed as four consecutive row-major pixels per uint.
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

vec2 capa_path_local_position(uint pathIndex, ivec2 pixel) {
	return vec2(
		dot(paths.values[pathIndex].inverseMatrixX.xyz, vec3(pixel, 1.0)),
		dot(paths.values[pathIndex].inverseMatrixY.xyz, vec3(pixel, 1.0))
	);
}

float capa_gradient_weight(CAPAGradientPaint paint, vec2 local) {
	if (paint.type == CAPA_GRADIENT_RADIAL) {
		vec2 radius = max(paint.endOrRadius, vec2(eps));
		return length((local - paint.origin) / radius);
	}
	vec2 axis = paint.endOrRadius - paint.origin;
	float len2 = dot(axis, axis);
	if (len2 <= eps)
		return 0.0;
	return dot(axis, local - paint.origin) / len2;
}

vec4 capa_sample_gradient(uint paintIndex, vec2 local) {
	CAPAGradientPaint paint = gradientPaints.values[paintIndex];
	uint count = paint.count;
	if (count == 0u)
		return vec4(0.0);
	if (count == 1u)
		return colors.values[paint.colors];

	float weight = capa_gradient_weight(paint, local);
	uint s = 0u;
	uint e = count - 1u;
	while (s + 1u < e) {
		uint idx = (e - s) / 2u + s;
		float pos = positions.values[paint.positions + idx];
		if (weight > pos) {
			s = idx;
		} else if (weight < pos) {
			e = idx;
		} else {
			s = idx;
			e = idx + 1u;
			break;
		}
	}

	float p0 = positions.values[paint.positions + s];
	float p1 = positions.values[paint.positions + e];
	float w = (weight - p0) / (p1 - p0);
	vec4 c0 = colors.values[paint.colors + s];
	vec4 c1 = colors.values[paint.colors + e];
	return mix(c0, c1, w);
}

float capa_sdf_width(uint pathIndex, vec2 size, vec2 coordScale) {
	vec2 texScale = size / coordScale;
	vec2 inverseMatrixY = paths.values[pathIndex].inverseMatrixY.xy;
	vec2 inverseMatrixX = paths.values[pathIndex].inverseMatrixX.xy;
	vec2 dx = vec2(inverseMatrixX.x, inverseMatrixY.x) * texScale;
	vec2 dy = vec2(inverseMatrixX.y, inverseMatrixY.y) * texScale;
	float fwidth = sqrt(max(dot(dx, dx), dot(dy, dy)));
	return max(fwidth, 1e-4);
}

vec4 capa_sample_image(uint pathIndex, uint paintIndex, vec2 local, vec4 color) {
	CAPAImagePaint paint = imagePaints.values[paintIndex];
	vec2 uv = (paint.coord.xy + local) / paint.coord.zw;
	vec4 tex = textureLod(
		sampler2D(
			images[nonuniformEXT(paint.textureIndex)],
			samplers[nonuniformEXT(paint.samplerIndex)]
		),
		uv, paint.lod
	);
	if (paint.kind == CAPA_IMAGE_SDF_MASK) {
		float dist = tex.r;
		float width = capa_sdf_width(pathIndex, paint.size, paint.coord.zw);
		float alpha = smoothstep(paint.stroke + width, paint.stroke, dist);
		return mix(color, paint.strokeColor, dist) * alpha;
	} else if (paint.kind == CAPA_IMAGE_MASK) {
		return color * tex[paint.alphaIndex];
	} else {
		return color * tex;
	}
}

vec4 capa_sample_path(uint pathIndex, ivec2 pixel) {
	uint paintType = paths.values[pathIndex].paintType;
	if (paintType == CAPA_PAINT_SOLID)
		return paths.values[pathIndex].color;
	// For gradient and image paints, we need to compute the local position in the path's coordinate space.
	vec2 local = capa_path_local_position(pathIndex, pixel);
	uint paintIndex = paths.values[pathIndex].paintIndex;
	if (paintType == CAPA_PAINT_GRADIENT)
		return capa_sample_gradient(paintIndex, local);
	return capa_sample_image(pathIndex, paintIndex, local, paths.values[pathIndex].color);
}

struct CAPACoverageGroup {
	bool hasValue;
	uint blendMode;
	// Coverage groups approximate neighboring layers as complementary pieces of
	// one pixel until coverage reaches 1.0 or the blend mode changes.
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
	// If both bottom coefficients are zero, lower layers and the destination
	// image cannot change the resolved color for this pixel.
	return all(lessThan(abs(front.scale), epsVec4)) && all(lessThan(abs(front.alphaTo), epsVec4));
}

void capa_group_flush() {
	if (!group.hasValue)
		return;
	if ((pc.flags & CAPA_FLAG_COMPOSITE_QUANTIZE_COVERAGE) != 0) {
		float coverage = clamp(group.coverage, 0.0, 1.0);
		if (coverage > eps && coverage < 1.0 - eps) {
			float displayCoverage = floor(coverage * CAPA_GROUP_COVERAGE_QUANTIZE_STEPS + 0.5) /
				CAPA_GROUP_COVERAGE_QUANTIZE_STEPS;
			displayCoverage = clamp(displayCoverage, 0.0, 1.0);
			group.src *= displayCoverage / coverage;
		}
	}
	// A completed same-mode group is one weighted PMA source.
	capa_blend_front_append(front, group.src, group.blendMode);
	group = capa_group_empty();
}

bool capa_group_add(vec4 src, uint blendMode, float coverage) {
	vec4 coveredSrc = src * coverage;
	if (!group.hasValue) {
		group.hasValue = true;
		group.blendMode = blendMode;
		group.coverage = coverage;
		group.src = coveredSrc;
	} else {
		// Same-mode fragments inside one coverage group are converted into one
		// weighted PMA color before entering the front-to-back blend expression.
		group.coverage += coverage;
		group.src += coveredSrc;
	}
	if (group.coverage >= 1.0 - eps) {
		capa_blend_front_append(front, group.src, group.blendMode);
		group = capa_group_empty();
		return capa_front_ignores_bottom();
	}
	return false; // return true if the front ignores the bottom, false otherwise
}

bool capa_group_add_layer(vec4 src, uint blendMode, float coverage) {
	if (coverage <= eps)
		return true;

	// If the coverage is already full,
	// we can flush the current group and append the new layer directly.
	if (coverage >= 1.0 - eps) {
		capa_group_flush();
		capa_blend_front_append(front, src, blendMode);
		return capa_front_ignores_bottom();
	}

	// A blend-mode change is a hard group boundary: different modes need their
	// own completed coverage/color before they are mixed into the front expression.
	if (group.hasValue && group.blendMode != blendMode) {
		capa_group_flush();
		if (capa_front_ignores_bottom())
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
	bool clearDst = (pc.flags & CAPA_FLAG_COMPOSITE_CLEAR_DST) != 0u;

	// If there are no path tiles in this global tile,
	// and we are not clearing the destination, then we can skip this tile entirely.
	if (count == 0u && !clearDst) {
		return;
	}
	uvec2 tileCoord = tileSlot + env.value.globalTileBounds.xy;
	uvec2 localPixel = subTile * gl_WorkGroupSize.xy + gl_LocalInvocationID.xy;
	ivec2 pixel = ivec2(tileCoord * CAPA_TILE_SIZE_U + localPixel);
	uint pixelIndex = localPixel.y * CAPA_TILE_SIZE_U + localPixel.x;

	// define a local front and coverage group for this pixel.
	front = capa_blend_front_identity();
	group = capa_group_empty();

	uint head = globalTiles.values[globalTileIndex].head;
	for (uint i = 0u; i < count; i++) {
		uint node = head + i;
		if (pathTiles.values[node].color != 0u) {
			// Packed color nodes are preblended full SrcOver runs from layer_plan;
			// they do not need a coverage page lookup.
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

		vec4 src = capa_sample_path(pathIndex, pixel);
		if (capa_group_add_layer(src, paths.values[pathIndex].blendMode, coverage))
			break;
	}

	// The last group may be partially filled (< 1 coverage); it still contributes
	// before resolving against the destination/clear bottom.
	capa_group_flush();

	// If the clip flag is set, we need to apply the clip coverage to the front.
	if ((pc.flags & Qk_FLAG_CLIP) != 0) {
		capa_blend_front_clip(front, clipCoverage(pixel + pc.surfaceOffset));
	}

	vec4 bottom = clearDst ? pc.clearColor : imageLoad(dstImage, pixel);
	imageStore(dstImage, pixel, capa_blend_front_resolve(front, bottom));
}
