#vert
void main() {
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
void main() {
	// only stencil fill test
}
