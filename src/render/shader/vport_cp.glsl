#vert
const vec2 verts[3] = vec2[3](
	vec2(-1.0, -1.0),
	vec2( 3.0, -1.0),
	vec2(-1.0,  3.0)
);
layout(location=1) out vec2 coords;

void main() {
	gl_Position = vec4(verts[gl_VertexIndex], 1.0, 1.0);
	coords = (gl_Position.xy * 0.5) + 0.5; // -1=>1 to 0-1 of uv coords
	coords.y = 1.0 - coords.y; // flip y
}

#frag
layout(binding=1,set=1) uniform sampler2D image;
layout(location=1) in vec2 coords;

void main() {
	fragColor = texture(image, coords);
	// fragColor = vec4(1.0,0.0,0.0,1.0); // debug red
}
