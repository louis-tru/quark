#vert
const vec2 verts[3] = vec2[3](
	vec2(-1.0, -1.0),
	vec2( 3.0, -1.0),
	vec2(-1.0,  3.0)
);
out vec2 coords;
void main() {
	gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
	coords = (gl_Position.xy * 0.5) + 0.5;
}

#frag
uniform sampler2D image;
in      lowp vec2 coords;

void main() {
	fragColor = texture(image, coords);
	//fragColor = vec4(1.0,0.0,0.0,1.0);
}
