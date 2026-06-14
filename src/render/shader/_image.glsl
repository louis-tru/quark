
Qk_CONSTANT(
	vec4  texCoords; /*offset,scale*/
	vec4  color; /* color */
	Qk_CONSTANT_Fields
);

#vert
layout(location=1) out vec2 coords; // texture coordinates uv for fragment shader

void main() {
	gl_Position = matrix * vec4(vertexIn.xy, 0.0, 1.0);

	aaSide = aaSideIn;
	// Qk uses screen-space coordinates internally.
	// Intermediate render targets keep the same memory orientation as uploaded images.
	// Do not flip Y here; backend-specific Y correction is applied only at final present.
	coords = (pc.texCoords.xy + vertexIn.xy) / pc.texCoords.zw; // coord uv
}

#frag
layout(binding=1,set=1)  uniform sampler2D image;
layout(location=1) in vec2 coords;
