#vert
void main() {
	gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);
}

#frag
void main() {
	// only stencil test
}
