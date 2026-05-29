#version 450

#define matrix (rMat.value * vMat.value)

#define Qk_RootMatrixBlock \
layout(binding=1, set=0, std140) uniform RootMatrixBlock { \
	mat4 value; \
} rMat;

#define Qk_ViewMatrixBlock \
layout(binding=2, set=0, std140) uniform ViewMatrixBlock { \
	mat4 value;\
} vMat;

#define Qk_CONSTANT(block) layout(push_constant) uniform PcArgs {\
	block \
	float depth; \
	uint flags; \
} pc

#vert
Qk_RootMatrixBlock
Qk_ViewMatrixBlock
layout(location=0) in      vec2  vertexIn;
layout(location=1) in      float aaSideIn; // anti alias side or z depth plus
layout(location=0) out     float aaSide;

#frag
precision mediump float; // lowp/highp
precision mediump sampler2D;
layout(binding=3, set=0, std140) uniform ClipStatBlock {
	vec4 range; // x:left, y:top, z:right, w:bottom
	// Clip sampling mode used by fragment shader:
	// 0: intersect  -> keep masked area
	// 1: difference -> reject masked area
	int op;
} clipStat;
layout(binding=0, set=1)  uniform sampler2D clipTex; // clip texture buffer
layout(location=0) in     float     aaSide;
layout(location=0) out    vec4      fragColor;

#define Qk_FLAG_CLIP (1u << 0)
#define Qk_FLAG_PMA (1u << 1)

// GLSL built-in functions:
// mix(a, b, x)  x:0->1 => a->b
// smoothstep(a, b, x) x:a->b => 0->1
// step(edge, x) x<edge then 0 else 1

// anti-aliasing coverage alpha for side AA, in range [0, 1]
float aaSideCoverage() {
	float w = fwidth(aaSide);
#if 1 // branch
	if (w == 0.0) return 1.0;
	return smoothstep(0.5*w, -0.5*w, aaSide);
#else // branchless
	return mix(smoothstep(0.5*w, -0.5*w, aaSide), 1.0, step(w, 0.0));
#endif
}

#define Qk_CLIP_FROM(offset) \
if ((pc.flags & Qk_FLAG_CLIP) != 0) { \
	float mask = texelFetch(clipTex, ivec2(gl_FragCoord.xy - offset), 0).r; \
	if (clipStat.op == 1) \
		mask = 1.0 - mask; /* difference mode: invert mask*/ \
	/*apply premultiplied alpha \
	float premul = mix(1.0, alpha, premultipliedAlpha); \
	fragColor *= vec4(vec3(premul), alpha); \
	always apply premultiplied alpha*/ \
	fragColor *= mask; \
}
#define Qk_CLIP() Qk_CLIP_FROM(clipStat.range.xy)
