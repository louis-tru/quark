#version 300 es

#define matrix (rootMatrix * viewMatrix)

// global shared data
layout (std140/*std430?,binding=0*/) uniform matrixBlock {
	/*mediump*/
	mat4  rootMatrix;
	mat4  viewMatrix;
};

#ifndef Qk_SHAFER_FRAG
/*layout(location=0)*/in    vec2  vertexIn;
in                          float aafuzzIn; // anti alias fuzz
out                         float aafuzz;
uniform                     float zDepth;
#else
in                     lowp float aafuzz;
layout(location=0) out lowp vec4  fragColor;
#endif
