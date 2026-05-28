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
#define Qk_FLAG_CLIP (1u << 0)
#define Qk_FLAG_PMA (1u << 1)
#define Qk_CLIP_(offset) \
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
#define Qk_CLIP() Qk_CLIP_(clipStat.range.xy)

// #define Qk_BLEND() if ((pc.flags & Qk_FLAG_PMA) != 0) { \
// 	fragColor.rgb *= fragColor.a; \
// }

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
