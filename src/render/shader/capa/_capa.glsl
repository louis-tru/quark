// #extension GL_EXT_shader_8bit_storage : require
// #extension GL_EXT_shader_explicit_arithmetic_types : require

const int CAPA_TILE_SIZE = 16;
const uint CAPA_TILE_SIZE_U = 16u;
const float CAPA_TILE_SIZE_F = 16.0;
const int CAPA_TILE_SIZE_BITS = 4;
const uint CAPA_NIL = 0xffffffffu;
const uint CAPA_SHORT_EDGE_CHUNK_SIZE = 4u;
// CAPA fill rules
const uint CAPA_NON_ZERO_RULE = 0u;
const uint CAPA_EVEN_ODD_RULE = 1u;
const uint CAPA_POSITIVE_RULE = 2u;
const uint CAPA_NEGATIVE_RULE = 3u;
// blend modes
const uint CAPA_BLEND_CLEAR = 1u;   //!< r = 0
const uint CAPA_BLEND_SRC = 2u;     //!< r = s
const uint CAPA_BLEND_SRC_OVER = 3u; //!< r = s + (1-sa)*d
const uint CAPA_BLEND_DST = 4u;     //!< r = d
const uint CAPA_BLEND_DST_OVER = 5u; //!< r = (1-da)*s + d
const uint CAPA_BLEND_SRC_IN = 6u;   //!< r = da*s
const uint CAPA_BLEND_DST_IN = 7u;   //!< r = sa*d
const uint CAPA_BLEND_SRC_OUT = 8u;  //!< r = (1-da)*s
const uint CAPA_BLEND_DST_OUT = 9u;  //!< r = (1-sa)*d
const uint CAPA_BLEND_SRC_ATOP = 10u; //!< r = da*s + (1-sa)*d
const uint CAPA_BLEND_DST_ATOP = 11u; //!< r = (1-da)*s + sa*d
const uint CAPA_BLEND_XOR = 12u;     //!< r = (1-da)*s + (1-sa)*d
const uint CAPA_BLEND_PLUS = 13u;    //!< r = s + d
// Legacy straight-alpha artistic blend modes.
// These are approximate under the PMA framebuffer pipeline.
const uint CAPA_BLEND_SRC_OVER_LEGACY = 14u;  //!< r = sa*s + (1-sa)*d
const uint CAPA_BLEND_PLUS_LEGACY = 15u;     //!< r = sa*s + d
// These modes are not mathematically exact under fixed-function blending
// and may produce incorrect edge blending results when combined with coverage-based antialiasing.
const uint CAPA_BLEND_MODULATE = 16u; //!< r = s*d
const uint CAPA_BLEND_SCREEN = 17u;   //!< r = s + (1-s)*d
const uint CAPA_BLEND_MULTIPLY = 18u; //!< r = d*s + (1-sa)*d
struct CAPAEnvironment {
	uvec4 tilePassGroups_Size32; // number of 32-wide dispatch groups for path tile init pass
	uvec4 orderPassGroups_Size32; // number of 32-wide dispatch groups for order pass
	uvec4 binPassGroups_Size32; // number of 32-wide dispatch groups for bin pass
	uvec4 backdropPassGroups_Size16_2; // number of 16x2-wide dispatch groups for backdrop pass
	uvec4 prefixPrePassGroups_Size32; // number of 32-wide dispatch groups for prefix row-chain pass
	uvec4 prefixPassGroups_Size16_2; // number of 16x2-wide dispatch groups for prefix pass
	uvec4 compositePassGroups_Size16_16; // number of 16x16-wide dispatch groups for composite pass
	ivec4 globalTileBounds; // global tile bounds begin, end
	ivec2 globalTileSpan; // global tile spanX, spanY
	uint globalTileCount; // number of CAPAGlobalTile generated
	uint taskCount; // number of short-edge tasks generated
	uint realTaskCount; // number of short-edge tasks generated for non-overflow edges
	uint pathTileCount; // number of CAPAPathTile generated
	uint realPathTileCount; // number of CAPAPathTile generated, non-overflow
	uint pathTileRowCount; // number of CAPATileRows generated
	uint realPathTileRowCount; // number of CAPATileRows generated, non-overflow
	uint boundaryTileCount; // number of CAPABoundaryTile allocated, starts at 3
	uint realBoundaryTileCount; // number of CAPABoundaryTile allocated, non-overflow, starts at 0
};

struct CAPAGlobalTile {
	uint head; // index to CAPAPathTile
};

struct CAPAPath {
	vec4 matrixX; // 2x3 transform matrix for path coordinates
	vec4 matrixY; // 2x3 transform matrix for path coordinates
	vec4 clip; // clip begin,end
	ivec4 bounds; // path begin,end
	ivec4 tileRect; // path tile begin, size
	vec4 color; // fill color
	uint fillRule; // 0: non-zero, 1: even-odd, 2: positive, 3: negative
	uint blendMode; // color blend mode
	uint tileOffset; // index to CAPAPathTile
	uint tileCount; // number of CAPAPathTile for this path
	uint edgeOffset; // index to CAPAEdge
	uint edgeCount; // number of CAPAEdge for this path
	uint _pad0[2];
};

struct CAPAEdge {
	vec2 p0; // start point of the edge
	vec2 p1; // end point of the edge
	float len; // length of the edge
	float dxdy; // slope of the edge
	float winding; // winding number of the edge
	uint pathIndex; // index to the path this edge belongs to
};

struct CAPAShortEdge {
	vec2 p0;
	vec2 p1;
	float dxdy;
	float winding;
	uint next; // index to next CAPAShortEdge
	uint _pad;
};

struct CAPAShortEdgeTask {
	uint edgeIndex;
	uint pathIndex;
	float t0;
	float t1;
};

struct CAPAPathTileRow {
	uint pathIndex; // index to CAPAPath
	uint smallTileIndex; // smallTileIndex of the first tile in this row
	uint boundaryTileIndex; // boundaryTileIndex of the first tile in this row
	uint boundaryTileCount; // number of boundary tiles in this row
};

struct CAPAPathTile {
	uint pathIndex;
	uint boundaryTileIndex;
	uint shortEdgeHead; // index to CAPAShortEdge
	uint nextLevel; // index to next level of CAPAPathTile for this path
	uint color; // packed RGBA8 PMA color for preblended full tiles
};

struct CAPASmallTile {
	uint value; // index to CAPABoundaryTile or CAPAShortEdge head
};

struct CAPABoundaryTile {
	uint pathIndex; // index to CAPAPath
	uint shortEdgeHead; // index to CAPAShortEdge
	ivec2 tileCoord;
	float backdrop[16];
	uint coverage[64];
};

#define capa_join_bounds_atomic(bounds, b) \
	if (bounds.x > b.x) atomicMin(bounds.x, b.x); \
	if (bounds.y > b.y) atomicMin(bounds.y, b.y); \
	if (bounds.z < b.z) atomicMax(bounds.z, b.z); \
	if (bounds.w < b.w) atomicMax(bounds.w, b.w);

bool capa_is_coord_in_rect(ivec2 coord, ivec4 rect) {
	return coord.x >= rect.x && coord.x < rect.x + rect.z &&
				 coord.y >= rect.y && coord.y < rect.y + rect.w;
}

uint capa_local_offset_row_major(ivec2 coord, ivec4 rect) {
	ivec2 local = coord - rect.xy;
	return local.y * rect.z + local.x;
}

vec4 capa_blend(vec4 src, vec4 dst, uint mode) {
	if (mode == CAPA_BLEND_SRC_OVER)
		return src + dst * (1.0 - src.a);
	switch (mode) {
		case CAPA_BLEND_CLEAR:
			return vec4(0.0);
		case CAPA_BLEND_SRC:
			return src;
		case CAPA_BLEND_DST:
			return dst;
		case CAPA_BLEND_DST_OVER:
			return dst + src * (1.0 - dst.a);
		case CAPA_BLEND_SRC_IN:
			return src * dst.a;
		case CAPA_BLEND_DST_IN:
			return dst * src.a;
		case CAPA_BLEND_SRC_OUT:
			return src * (1.0 - dst.a);
		case CAPA_BLEND_DST_OUT:
			return dst * (1.0 - src.a);
		case CAPA_BLEND_SRC_ATOP:
			return src * dst.a + dst * (1.0 - src.a);
		case CAPA_BLEND_DST_ATOP:
			return src * (1.0 - dst.a) + dst * src.a;
		case CAPA_BLEND_XOR:
			return src * (1.0 - dst.a) + dst * (1.0 - src.a);
		case CAPA_BLEND_PLUS:
			return min(src + dst, vec4(1.0));
		case CAPA_BLEND_SRC_OVER_LEGACY:
			return vec4(src.rgb * src.a + dst.rgb * (1.0 - src.a), src.a + dst.a * (1.0 - src.a));
		case CAPA_BLEND_PLUS_LEGACY:
			return min(vec4(src.rgb * src.a + dst.rgb, src.a + dst.a), vec4(1.0));
		case CAPA_BLEND_MODULATE:
			return vec4(src.rgb * dst.rgb, src.a * dst.a);
		case CAPA_BLEND_SCREEN:
			return vec4(src.rgb + dst.rgb - src.rgb * dst.rgb, src.a + dst.a * (1.0 - src.a));
		case CAPA_BLEND_MULTIPLY:
			return vec4(src.rgb * dst.rgb + dst.rgb * (1.0 - src.a), src.a + dst.a * (1.0 - src.a));
		default:
			return src + dst * (1.0 - src.a); // fallback SrcOver
	}
}

uint capa_pack_rgba8(vec4 c) {
	vec4 v = clamp(c, vec4(0.0), vec4(1.0)) * 255.0 + 0.5;
	uvec4 b = uvec4(v);
	return b.r | (b.g << 8u) | (b.b << 16u) | (b.a << 24u);
}

vec4 capa_unpack_rgba8(uint c) {
	return vec4(
		float(c & 255u),
		float((c >> 8u) & 255u),
		float((c >> 16u) & 255u),
		float((c >> 24u) & 255u)
	) * (1.0 / 255.0);
}
