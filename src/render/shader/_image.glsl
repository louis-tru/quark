
#define _CONSTANT_IMAGE(block) Qk_CONSTANT(\
	vec4  texCoords; /*offset,scale*/\
	vec4  color; /* color */\
	block \
)

_CONSTANT_IMAGE(Qk_CONSTANT_Fields);

#vert
layout(location=1) out vec2 coords; // texture coordinates uv for fragment shader

void main() {
	vec4 pos = (vMat.value * vec4(vertexIn.xy, pc.depth, 1.0));
	// fix draw image tearing with round function
	// Align the image pixels exactly onto the drawing surface
		gl_Position = rMat.value * vec4(pos.xy, pos.zw);

	aaSide = aaSideIn;
	// Qk uses screen-space coordinates internally.
	// Intermediate render targets keep the same memory orientation as uploaded images.
	// Do not flip Y here; backend-specific Y correction is applied only at final present.
	coords = (pc.texCoords.xy + vertexIn.xy) / pc.texCoords.zw; // coord uv
}

#frag
layout(binding=1,set=1)  uniform sampler2D image;
layout(location=1) in vec2 coords;
