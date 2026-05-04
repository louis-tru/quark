#version 450

#define matrix (rMat.value * vMat.value)

#define Qk_CONSTANT(block) layout(push_constant) uniform PcArgs {\
	block \
	float depth; \
	uint flags; \
} pc

#define Qk_FLAG_AACLIP (1u << 0)
#define Qk_IF_AACLIP if ((pc.flags & Qk_FLAG_AACLIP) != 0)

#vert
// global shared data
layout(binding=0) uniform RootMatrixBlock {
	mat4 value;
} rMat;
layout(binding=1) uniform ViewMatrixBlock {
	mat4 value;
} vMat;
layout(location=0) in      vec2  vertexIn;
layout(location=1) in      float aafuzzIn; // anti alias fuzz or z depth plus
layout(location=0) out     float aafuzz;

#frag
precision mediump float; // lowp/highp
precision mediump sampler2D;
layout(binding=2)  uniform sampler2D aaclip; // anti alias clip texture buffer @aaclipOut
layout(location=0) in      float aafuzz;
layout(location=0) out     vec4  fragColor;
// layout(location=1) out  vec4  aaclipOut; // aa clip output
