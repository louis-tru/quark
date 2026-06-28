// CAPA pass 7.
// Ordered global-tile pure-color compositor.

Qk_CONSTANT(
	vec4 clearColor;
	uvec2 surfaceOffset;
);

#import "_capa.glsl"

#comp
layout(local_size_x=16, local_size_y=16, local_size_z=1) in;

layout(binding=1,set=0,std430) buffer CAPAEnvironments {
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

layout(binding=5,set=0,std430) readonly buffer CAPABoundaryTiles {
	CAPABoundaryTile values[];
} boundaryTiles;

layout(binding=1,set=1,rgba8) uniform image2D dstImage;

#define CAPA_COMPOSITE_CLEAR_DST (1u << 16)

float capa_load_boundary_coverage(uint boundaryIndex, uint pixelIndex) {
	uint word = boundaryTiles.values[boundaryIndex].coverage[pixelIndex >> 2u];
	uint shift = (pixelIndex & 3u) << 3u;
	return float((word >> shift) & 255u) * (1.0 / 255.0);
}

float capa_path_tile_coverage(uint boundaryIndex, uint pixelIndex) {
	if (boundaryIndex == 0u)
		return 0.0;
	if (boundaryIndex == 1u)
		return 1.0;
	return capa_load_boundary_coverage(boundaryIndex, pixelIndex);
}

void main() {
	uvec2 tileCoord = gl_WorkGroupID.xy;
	uint globalTileIndex = tileCoord.y * gl_NumWorkGroups.x + tileCoord.x;
	uint head = globalTiles.values[globalTileIndex].head;
	bool clearDst = (pc.flags & CAPA_COMPOSITE_CLEAR_DST) != 0u;

	// If there are no path tiles in this global tile,
	// and we are not clearing the destination, then we can skip this tile entirely.
	if (head == CAPA_NIL && !clearDst) {
		return;
	}

	tileCoord += env.value.globalTileBounds.xy;
	uvec2 localPixel = gl_LocalInvocationID.xy;
	uvec2 pixel = pc.surfaceOffset + tileCoord * CAPA_TILE_SIZE_U + localPixel;
	uint pixelIndex = localPixel.y * CAPA_TILE_SIZE_U + localPixel.x;
	vec4 dst = clearDst ? pc.clearColor : imageLoad(dstImage, ivec2(pixel));

	for (uint node = head;
			node != CAPA_NIL;
			node = pathTiles.values[node].next)
	{
		if (pathTiles.values[node].boundaryTileIndex == 1u && pathTiles.values[node].color != 0u) {
			vec4 src = capa_unpack_rgba8(pathTiles.values[node].color);
			dst = capa_blend(src, dst, CAPA_BLEND_SRC_OVER);
			continue;
		}

		uint pathIndex = pathTiles.values[node].pathIndex;
		uint boundaryIndex = pathTiles.values[node].boundaryTileIndex;
		float coverage = capa_path_tile_coverage(boundaryIndex, pixelIndex);
		if (coverage <= 0.0)
			continue;

		vec4 src = paths.values[pathIndex].color;
		src.rgb *= src.a;
		src *= coverage;
		dst = capa_blend(src, dst, paths.values[pathIndex].blendMode);
	}

	imageStore(dstImage, ivec2(pixel), dst);
}
