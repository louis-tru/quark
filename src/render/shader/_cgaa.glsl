
Qk_CONSTANT(
	ivec2 cgaaAtlasTiles; // atlas tile count in x and y
	ivec2 _pad; // padding to 32 bytes avoid vec4 alignment issues
	Qk_CONSTANT_Fields
);

#vert
#if Qk_SHADER_FLAGS_ENABLE_CGAA
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

const int CGAA_TILE_SIZE = 16;
const int UNIFORM = 0;
const int BOUNDARY = 1;
const uint16_t UINT16_MAX = uint16_t(65535);

const vec2 quadCorners[4] = {
	vec2(0, 0), // left-top
	vec2(16, 0), // right-top
	vec2(0, 16), // left-bottom
	vec2(16, 16), // right-bottom
};

struct CGAAPath {
	uint originX; // path bounds origin, in screen space
	uint originY;
	uint fillRule;
	uint flags;
	vec4 color;
	uint tileOffset; // compositeTiles offset for this path, in CGAACompositeTile array
	uint tileCount;
	vec2 _pad; // padding to 32 bytes avoid vec4 alignment issues in array of CGAAPath
};

struct CGAACompositeTile {
	uint16_t originX;
	uint16_t originY;
	uint16_t spanX;
	uint16_t spanY;
	uint16_t atlasTileIndex;
	uint16_t pathIndex;
};

layout(binding=4,set=0,std430) readonly buffer CGAAPaths {
	CGAAPath values[];
} paths;
layout(binding=5,set=0,std430) readonly buffer CGAACompositeTiles {
	CGAACompositeTile values[];
} tiles;
layout(location=1) flat out int tileType;
layout(location=2) smooth out vec2 atlasCoord;
vec2 cgaaPosition;

#define Qk_cgaaVertexSteps() \
	uint tileIndex = gl_InstanceIndex; \
	CGAACompositeTile tile = tiles.values[tileIndex]; \
	CGAAPath path = paths.values[tile.pathIndex]; \
	vec2 localPos = quadCorners[gl_VertexIndex]; \
	cgaaPosition = vec2( \
		float(path.originX + tile.originX), \
		float(path.originY + tile.originY) \
	) + vec2(localPos.x * tile.spanX, localPos.y * tile.spanY); \
	atlasCoord = tile.atlasTileIndex == UINT16_MAX ? vec2(0): \
		atlasTileOrigin(tile.atlasTileIndex) + vec2(localPos); \
	tileType = tile.atlasTileIndex == UINT16_MAX ? UNIFORM : BOUNDARY

vec2 atlasTileOrigin(uint16_t atlasTileIndex) {
	return vec2(
		float(atlasTileIndex % pc.cgaaAtlasTiles.x * CGAA_TILE_SIZE),
		float(atlasTileIndex / pc.cgaaAtlasTiles.x * CGAA_TILE_SIZE)
	);
}

vec2 cgaaCanvasPosition() {
	mat4 mat = vMat.value;
	vec2 p = cgaaPosition / rMat.surfaceScale;
	float det = mat[0][0] * mat[1][1] - mat[1][0] * mat[0][1];
	if (det == 0.0)
		return p;
	det = 1.0 / det;
	p -= mat[3].xy;
	return vec2(
		(mat[1][1] * p.x - mat[1][0] * p.y) * det,
		(-mat[0][1] * p.x + mat[0][0] * p.y) * det
	);
}
#else
vec2 cgaaCanvasPosition() { return vertexIn.xy; }
#define path pc
#define cgaaPosition vec2(0)
#define Qk_cgaaVertexSteps()
#endif

#frag
#if Qk_SHADER_FLAGS_ENABLE_CGAA
const int BOUNDARY = 1;
layout(binding=1,set=1) uniform sampler2D atlasTex;
layout(location=1) in flat int tileType;
layout(location=2) in smooth vec2 atlasCoord;

float cgaaCoverage() {
	float coverage = 1.0;
	if (tileType == BOUNDARY) {
		coverage = texelFetch(atlasTex, ivec2(atlasCoord), 0).r;
		if (coverage == 0.0)
			discard;
	}
	return coverage;
}
float aaCoverage() {
	if ((pc.flags & Qk_FLAG_CGAA) != 0)
		return cgaaCoverage();
	else
		return aaSideCoverage(pc.flags);
}
#define Qk_aaCoverage() fragColor *= aaCoverage()
#else
float cgaaCoverage() { return 1.0; }
float aaCoverage() { return aaSideCoverage(pc.flags); }
#define Qk_aaCoverage Qk_aaSideCoverage
#endif
