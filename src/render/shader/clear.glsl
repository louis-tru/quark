Qk_CONSTANT(
	vec4 color;
);

#vert
const vec2 verts[3] = vec2[3](
	vec2(-1.0, -1.0),
	vec2( 3.0, -1.0),
	vec2(-1.0,  3.0)
);

void main() {
	gl_Position = vec4(verts[gl_VertexIndex], pc.depth, 1.0);
}

#frag
void main() {
	fragColor = pc.color;
}
