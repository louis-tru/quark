#version 300 es

#define matrix (rootMatrix * viewMatrix)

// global shared data
layout (std140/*std430?,binding=0*/) uniform rootMatrixBlock {
	mediump mat4 rootMatrix;
};
layout (std140) uniform viewMatrixBlock {
	mediump mat4 viewMatrix;
};

#ifdef Qk_SHADER_VERT
uniform                     float depth;
/*layout(location=0)*/in    vec2  vertexIn;
in                          float aafuzzIn; // anti alias fuzz or z depth plus
out                         float aafuzz;
#else
uniform                sampler2D  aaclip; // anti alias clip texture buffer @aaclipOut
in                     lowp float aafuzz;
layout(location=0) out lowp vec4  fragColor;
// layout(location=1) out lowp vec4  aaclipOut; // aa clip output
#endif
