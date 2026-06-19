#version 450

// global flags from 1u << 0 to 1u << 15, 0x0000FFFF
#define Qk_FLAG_CLIP (1u << 0)
#define Qk_FLAG_PMA (1u << 1)
#define Qk_FLAG_AASIDE_LINE (1u << 2)
#define Qk_FLAG_CGAA (1u << 3)

#define matrix (rMat.value * vMat.value)

#define Qk_CONSTANT(block) layout(push_constant) uniform PcArgs {\
	block \
	uint flags; \
} pc

#vert
layout(binding=1, set=0, std140) uniform RootMatrixBlock {
	mat4 value;
	mat4 noScale; // for non-scaling
	vec2 surfaceScale;
	vec2 _pad; // pad to 16-byte alignment
} rMat;
layout(binding=2, set=0, std140) uniform ViewMatrixBlock {
	mat4 value;
} vMat;
layout(location=0) in      vec2  vertexIn;
layout(location=1) in      float aaSideIn; // anti alias side
layout(location=0) out     float aaSide;

#frag
precision mediump float; // lowp/highp
precision mediump sampler2D;
layout(location=0) in      float aaSide;
layout(location=0) out     vec4  fragColor;
layout(binding=3, set=0, std140) uniform ClipStatBlock {
	vec4 range; // x:left, y:top, z:right, w:bottom
	// Clip sampling mode used by fragment shader:
	// 0: intersect  -> keep masked area
	// 1: difference -> reject masked area
	int op;
} clipStat;
layout(binding=0, set=1)  uniform sampler2D clipTex; // clip texture buffer

// clipStat.op: 0 for intersect, 1 for difference
void clip(ivec2 coord, inout vec4 color) {
	float mask = texelFetch(clipTex, coord, 0).r;
	if (clipStat.op == 1)
		mask = 1.0 - mask; /* difference mode: invert mask*/
	color *= mask;
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
	return smoothstep(w, -w, aaSide * max(w, 1.0));
#else // branchless
	return mix(smoothstep(w, -w, aaSide), 1.0, step(w, 0.0));
#endif
}

#define Qk_aaSideCoverage() fragColor *= aaSideCoverage(pc.flags)

#define Qk_CLIP_FROM(offset) \
if ((pc.flags & Qk_FLAG_CLIP) != 0) { \
	clip(ivec2(gl_FragCoord.xy - offset), fragColor); \
}
#define Qk_CLIP() Qk_CLIP_FROM(clipStat.range.xy)
