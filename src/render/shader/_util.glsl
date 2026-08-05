#version 450

// global flags from 1u << 0 to 1u << 15, 0x0000FFFF
#define Qk_FLAG_CLIP (1u << 0)
#define Qk_FLAG_PMA (1u << 1)
#define Qk_FLAG_AASIDE_LINE (1u << 2)
#define Qk_FLAG_CGAA (1u << 3)

// view matrix linear part
#define vMatL mat2(vMat.value.xy, vMat.value.zw)

#define Qk_CONSTANT(block) layout(push_constant) uniform PcArgs {\
	block \
	uint flags; \
} pc

#vert
precision highp int;
precision mediump float;

layout(binding=1, set=0, std140) uniform RootMatrixBlock {
	highp mat4 value;
	highp mat4 noScale; // for non-scaling
	highp vec2 surfaceScale;
	highp vec2 _pad; // pad to 16-byte alignment
} rMat;
layout(binding=2, set=0, std140) uniform ViewMatrixBlock {
	highp vec4 value;
} vMat;
layout(location=0) in highp vec2  vertexIn;
layout(location=1) in highp float aaSideIn; // anti alias side
layout(location=0) out mediump float aaSide;

highp vec4 vPosition(highp vec2 pos) {
	return vec4(vMatL * vertexIn + pos, 0.0, 1.0);
}

#frag
precision highp int;
precision mediump float; // lowp/highp
precision mediump sampler2D;

layout(location=0) in  mediump float aaSide;
layout(location=0) out mediump vec4  fragColor;
layout(binding=0, set=0) uniform sampler2D clipTex; // clip texture buffer
layout(binding=3, set=0, std140) uniform ClipStatBlock {
	ivec4 bounds; // x:left, y:top, z:right, w:bottom
	// Clip sampling mode used by fragment shader:
	// 0: intersect  -> keep masked area
	// 1: difference -> reject masked area
	int op;
} clipStat;

// clipStat.op: 0 for intersect, 1 for difference
float clipCoverage(ivec2 fragCoord) {
	float coverage = texelFetch(clipTex, fragCoord - clipStat.bounds.xy, 0).r;
	if (clipStat.op == 1)
		coverage = 1.0 - coverage; /* difference mode: invert coverage*/
	return coverage;
}

// GLSL built-in functions:
// mix(a, b, x)  x:0->1 => a->b
// smoothstep(a, b, x) x:a->b => 0->1
// step(edge, x) x<edge then 0 else 1
// anti-aliasing coverage alpha for side AA, in range [0, 1]
float aaSideCoverage(const uint flags) {
#if 0 // debug: disable AA
	return 1.0;
#endif
	// line AA: coverage is 1 - abs(aaSide),
	// where aaSide is the distance to the edge (negative inside, positive outside)
	if ((flags & Qk_FLAG_AASIDE_LINE) != 0)
		return 1.0 - abs(aaSide);
	float w = fwidth(aaSide);
#if 1 // branch
	if (w == 0.0)
		return 1.0;
	return 1.0 - smoothstep(-w, w, aaSide * max(w, 1.0));
#else // branchless
	return mix(1.0 - smoothstep(-w, w, aaSide), 1.0, step(w, 0.0));
#endif
}

#define Qk_aaSideCoverage() fragColor *= aaSideCoverage(pc.flags)

#define Qk_CLIP() \
if ((pc.flags & Qk_FLAG_CLIP) != 0) \
	fragColor *= clipCoverage(ivec2(gl_FragCoord.xy))