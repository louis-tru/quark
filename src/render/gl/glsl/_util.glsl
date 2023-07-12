#version 300 es

#define matrix (root_matrix * view_matrix)

layout (std140) uniform mat_ubo { // std430?
	/*mediump*/
	mat4  root_matrix;
	mat4  view_matrix;
};

#ifndef Qk_SHAFER_FRAG
in      vec2  vertex_in;
#else
layout(location=0) out lowp vec4 fragColor;
#endif
