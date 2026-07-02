// #extension GL_EXT_shader_8bit_storage : require
// #extension GL_EXT_shader_explicit_arithmetic_types : require

const int CAPA_TILE_SIZE = 16;
const uint CAPA_TILE_SIZE_U = 16u;
const float CAPA_TILE_SIZE_F = 16.0;
const int CAPA_TILE_SIZE_BITS = 4;
// High sentinels keep real GPU-allocated indices starting at 0.
const uint CAPA_NIL = 0xffffffffu;
const uint CAPA_FULL_TILE = CAPA_NIL - 1u;
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
// CAPA paint types
const uint CAPA_PAINT_SOLID = 0u;
const uint CAPA_PAINT_GRADIENT = 1u;
const uint CAPA_PAINT_IMAGE = 2u;
// Gradient types
const uint CAPA_GRADIENT_LINEAR = 0u;
const uint CAPA_GRADIENT_RADIAL = 1u;
// CAPA image paint kinds
const uint CAPA_IMAGE_DEFAULT = 0;
const uint CAPA_IMAGE_MASK = 1;
const uint CAPA_IMAGE_SDF_MASK = 2;
// Paint source is guaranteed to produce alpha == 1 for every sampled point.
const uint CAPA_FLAG_PAINT_OPAQUE = 1u << 0;

struct CAPAEnvironment {
	// Indirect dispatch arguments are uvec4-aligned so Metal can dispatch from
	// the same storage without a CPU readback between CAPA passes.
	uvec4 tilePassGroups_Size32; // number of 32-wide dispatch groups for small tile init pass
	uvec4 layerPlanPassGroups_Size32; // number of 32-wide dispatch groups for layer plan pass
	uvec4 binPassGroups_Size32; // number of 32-wide dispatch groups for bin pass
	uvec4 backdropPassGroups_Size16_2; // number of 16x2-wide dispatch groups for backdrop pass
	uvec4 classifyPassGroups_Size32; // number of 32-wide dispatch groups for classify pass
	uvec4 prefixPassGroups_Size16_2; // number of 16x2-wide dispatch groups for prefix pass
	uvec4 coveragePassGroups_Size16_2; // x: 16x2 coverage groups, w: allocated CAPACoverageTile count
	uvec4 compositePassGroups_Size16_16; // number of dispatch groups for composite pass
	ivec4 globalTileBounds; // global tile bounds begin, end
	ivec2 globalTileSpan; // global tile spanX, spanY
	uint globalTileCount; // number of CAPAGlobalTile generated
	uint taskCount; // number of short-edge tasks generated
	uint realTaskCount; // number of short-edge tasks generated for non-overflow edges
	uint pathTileCount; // number of CAPASmallTile slots allocated by prepare_tiles
	uint realPathTileCount; // number of CAPASmallTile slots allocated, non-overflow
	uint pathTileRowCount; // number of CAPATileRows generated
	uint realPathTileRowCount; // number of CAPATileRows generated, non-overflow
	uint boundaryTileCount; // number of CAPABoundaryTile allocated, starts at 0
	uint realBoundaryTileCount; // number of CAPABoundaryTile allocated, non-overflow, starts at 0
	uint boundaryDoneCount; // completed tile rows in boundary allocation pass
	// Final CAPAPathTile span allocator. The initial path-local CAPASmallTile
	// staging space is separate from this z-ordered global-tile list.
	uint layerPlanPathTileCount; // number of CAPAPathTile nodes allocated by layer plan pass
};

struct CAPAGradientPaint {
	vec2 origin;        // Gradient origin position.
	vec2 endOrRadius;   // End point (linear) or radius (radial).
	uint type;          // Gradient type: 0: linear, 1: radial.
	uint count;         // Number of color stops.
	uint colors;        // index to vec4 color array.
	uint positions;     // index to float position array.
};

struct CAPAImagePaint {
	vec4 coord; // origin/scale
	vec4 strokeColor; // premultiplied stroke color for sdf mask
	uint textureIndex; // index to texture array
	uint samplerIndex; // index to sampler array
	float stroke; // sdf stroke width
	uint kind; // 0: image, 1: mask, 2: sdf mask
};

struct CAPAGlobalTile {
	uint head; // first contiguous CAPAPathTile index for this global tile
	uint count; // contiguous CAPAPathTile count for this global tile
};

struct CAPAPath {
	// CPU uploads flattened path-space edges; prepare transforms them into
	// surface-space CAPAEdge values and fills the tile ranges below.
	vec4 matrixX; // 2x3 transform matrix for path coordinates
	vec4 matrixY; // 2x3 transform matrix for path coordinates
	vec4 inverseMatrixX; // surface pixel -> path coordinates
	vec4 inverseMatrixY; // surface pixel -> path coordinates
	vec4 clip; // clip begin,end
	vec4 color; // premultiplied fill color
	ivec4 bounds; // path begin,end
	ivec4 tileRect; // path tile begin, end
	ivec2 tileEnd; // path tile end (exclusive)
	uint fillRule; // 0: non-zero, 1: even-odd, 2: positive, 3: negative
	uint blendMode; // color blend mode
	uint tileOffset; // index to CAPASmallTile
	uint tileRowOffset; // index to CAPATileRow
	uint edgeOffset; // index to CAPAEdge
	uint edgeCount; // number of CAPAEdge for this path
	uint paintIndex; // index to CAPAGradientPaint or CAPAImagePaint, or 0 for solid color
	uint paintType; // 0: solid, 1: gradient, 2: image
	uint flags; // CAPA_FLAG_PAINT_OPAQUE
	uint _pad; // padding to 16 bytes
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
};

struct CAPAShortEdgeNode {
	CAPAShortEdge edge;
	uint next; // index to next CAPAShortEdgeNode
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
	uint smallTileIndex; // SmallTileIndex of the first tile in this row
	uint boundaryTileIndex; // boundaryTileIndex of the first tile in this row
	uint boundaryTileCount; // number of boundary tiles in this row
	// Initial row prefix for the first boundary tile in this path-tile row.
	// This stays outside CAPABoundaryTile so coverage storage can be split out.
	float backdrop[16]; // row initial prefix
};

struct CAPAPathTile {
	uint pathIndex;
	// Final composite uses this as CAPACoverageTile. During layer planning it
	// temporarily carries the source CAPABoundaryTile until coverage slots exist.
	uint coverageTileIndex; // index to CAPACoverageTile
	uint color; // packed RGBA8 PMA color for preblended full tiles
};

struct CAPASmallTile {
	// Staging value evolves through the pipeline:
	// NIL after clear, short-edge head after bin, boundary index after boundary,
	// FULL_TILE after classify for solid edge-free tiles.
	uint value; // index to CAPABoundaryTile or CAPAShortEdgeNode head
};

struct CAPABoundaryTile {
	uint pathIndex; // index to CAPAPath
	uint shortEdgeHead; // index to CAPAShortEdgeNode
	ivec2 tileCoord;
	// Before prefix: local row delta for this tile. After prefix: tile-left row
	// prefix used by coverage integration.
	float backdrop[16]; // local row delta before prefix, tile-left row prefix after prefix pass
};

struct CAPACoverageTile {
	uint boundaryTileIndex; // index to CAPABoundaryTile
	// Packed R8 coverage. Each uint stores 4 pixels in row-major order.
	uint values[64]; // 16x16 coverage values, 4 pixels per value
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

struct CAPABlendFront {
	// Accumulates front-to-back blending as a function of the unknown bottom color:
	//   result = bias + scale * bottom + alphaTo * bottom.a
	// This lets order/coverage scan layers from front to back and still resolve the
	// exact bottom-up blend later, once the retained bottom color is known.
	vec4 bias;
	vec4 scale;
	vec4 alphaTo;
};

CAPABlendFront capa_blend_front_identity() {
	return CAPABlendFront(vec4(0.0), vec4(1.0), vec4(0.0));
}

void capa_blend_front_append(inout CAPABlendFront blend, vec4 src, uint mode) {
	// CAPAPath.color is already premultiplied before it reaches shaders. Do not
	// multiply src.rgb by src.a before calling this helper. The LEGACY modes below
	// intentionally keep capa_blend's historical straight-alpha approximation.
	if (mode == CAPA_BLEND_SRC_OVER) {
		blend.bias += blend.scale * src + blend.alphaTo * src.a;
		float trans = 1.0 - src.a;
		blend.scale *= trans;
		blend.alphaTo *= trans;
		return;
	}

	vec4 layerBias;
	vec4 layerScale;
	vec4 layerAlphaTo;
	switch (mode) {
		case CAPA_BLEND_CLEAR:
			layerBias = vec4(0.0);
			layerScale = vec4(0.0);
			layerAlphaTo = vec4(0.0);
			break;
		case CAPA_BLEND_SRC:
			layerBias = src;
			layerScale = vec4(0.0);
			layerAlphaTo = vec4(0.0);
			break;
		case CAPA_BLEND_DST:
			return;
		case CAPA_BLEND_DST_OVER:
			layerBias = src;
			layerScale = vec4(1.0, 1.0, 1.0, 0.0);
			layerAlphaTo = vec4(-src.rgb, 1.0 - src.a);
			break;
		case CAPA_BLEND_SRC_IN:
			layerBias = vec4(0.0);
			layerScale = vec4(0.0);
			layerAlphaTo = src;
			break;
		case CAPA_BLEND_DST_IN:
			layerBias = vec4(0.0);
			layerScale = vec4(src.a);
			layerAlphaTo = vec4(0.0);
			break;
		case CAPA_BLEND_SRC_OUT:
			layerBias = src;
			layerScale = vec4(0.0);
			layerAlphaTo = -src;
			break;
		case CAPA_BLEND_DST_OUT:
			layerBias = vec4(0.0);
			layerScale = vec4(1.0 - src.a);
			layerAlphaTo = vec4(0.0);
			break;
		case CAPA_BLEND_SRC_ATOP:
			layerBias = vec4(0.0);
			layerScale = vec4(1.0 - src.a, 1.0 - src.a, 1.0 - src.a, 0.0);
			layerAlphaTo = vec4(src.rgb, 1.0);
			break;
		case CAPA_BLEND_DST_ATOP:
			layerBias = src;
			layerScale = vec4(src.a, src.a, src.a, 0.0);
			layerAlphaTo = vec4(-src.rgb, 0.0);
			break;
		case CAPA_BLEND_XOR:
			layerBias = src;
			layerScale = vec4(1.0 - src.a, 1.0 - src.a, 1.0 - src.a, 0.0);
			layerAlphaTo = vec4(-src.rgb, 1.0 - 2.0 * src.a);
			break;
		case CAPA_BLEND_SRC_OVER_LEGACY:
			layerBias = vec4(src.rgb * src.a, src.a);
			layerScale = vec4(1.0 - src.a);
			layerAlphaTo = vec4(0.0);
			break;
		case CAPA_BLEND_PLUS:
			// PLUS is nonlinear only because capa_blend clamps after each layer.
			// Accumulate the add linearly and clamp in resolve. This is exact for
			// consecutive PLUS layers, but approximate if other modes are interleaved.
			layerBias = src;
			layerScale = vec4(1.0);
			layerAlphaTo = vec4(0.0);
			break;
		case CAPA_BLEND_PLUS_LEGACY:
			// Same delayed-saturation approximation as PLUS, with legacy src alpha.
			layerBias = vec4(src.rgb * src.a, src.a);
			layerScale = vec4(1.0);
			layerAlphaTo = vec4(0.0);
			break;
		case CAPA_BLEND_MODULATE:
			layerBias = vec4(0.0);
			layerScale = src;
			layerAlphaTo = vec4(0.0);
			break;
		case CAPA_BLEND_SCREEN:
			layerBias = vec4(src.rgb, src.a);
			layerScale = vec4(1.0 - src.rgb, 1.0 - src.a);
			layerAlphaTo = vec4(0.0);
			break;
		case CAPA_BLEND_MULTIPLY:
			layerBias = vec4(0.0, 0.0, 0.0, src.a);
			layerScale = vec4(src.rgb + vec3(1.0 - src.a), 1.0 - src.a);
			layerAlphaTo = vec4(0.0);
			break;
		default:
			layerBias = src;
			layerScale = vec4(1.0 - src.a);
			layerAlphaTo = vec4(0.0);
			break;
	}

	// Compose the accumulated front function with the new layer function:
	// front(layer(bottom)). layer alpha is b.a + (s.a + a.a) * bottom.a.
	blend.bias += blend.scale * layerBias + blend.alphaTo * layerBias.a;
	blend.alphaTo = blend.scale * layerAlphaTo + blend.alphaTo * (layerScale.a + layerAlphaTo.a);
	blend.scale *= layerScale;
}

vec4 capa_blend_front_resolve(CAPABlendFront blend, vec4 bottom) {
	vec4 dst = blend.bias + blend.scale * bottom + blend.alphaTo * bottom.a;
	// The final clamp is exact for PLUS-only chains and harmless for normalized
	// premultiplied colors produced by the other blend modes.
	return min(dst, vec4(1.0));
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

float capa_edge_dxdy(CAPAShortEdge edge) {
	float dy = edge.p1.y - edge.p0.y;
	return dy != 0.0 ? (edge.p1.x - edge.p0.x) / dy : 0.0;
}

float capa_edge_winding(CAPAShortEdge edge) {
	return edge.p1.y > edge.p0.y ? 1.0 : edge.p1.y < edge.p0.y ? -1.0 : 0.0;
}
