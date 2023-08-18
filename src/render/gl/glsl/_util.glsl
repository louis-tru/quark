#version 300 es

#define matrix (rootMatrix * viewMatrix)
//#define depth (zDepth * 0.0000152587890625) // zDepth / 65536
#define depth zDepth

layout (std140) uniform uboData { // std430?, binding=0
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
