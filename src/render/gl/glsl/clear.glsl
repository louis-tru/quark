#vert
void main() {
	gl_Position = vec4(vertex_in.xy, 0.0, 1.0);
}

#frag
uniform lowp vec4 color;
void main() {
	fragColor = color;
}