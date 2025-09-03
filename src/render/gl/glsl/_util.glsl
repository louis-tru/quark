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
// in                          vec4  color; //!< {GL_UNSIGNED_BYTE}
/*layout(location=0)*/in    vec2  vertexIn;
in                          float aafuzzIn; // anti alias fuzz
out                         float aafuzz;
uniform                     float depth;
#else
in                     lowp float aafuzz;
uniform  sampler2D                aaclip; // anti alias clip texture buffer
layout(location=0) out lowp vec4  fragColor;
#endif
