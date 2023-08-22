#version 300 es

#define matrix (rootMatrix * viewMatrix)
#define depth zDepth

// global shared data
layout (std140/*std430?,binding=0*/) uniform matrixBlock {
	/*mediump*/
	mat4  rootMatrix;
	mat4  viewMatrix;
};

#ifndef Qk_SHAFER_FRAG
//layout(location=0) in      vec2  vertexIn;
in      vec2  vertexIn;
uniform float zDepth;
#else
layout(location=0) out lowp vec4 fragColor;
#endif
