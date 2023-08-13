#vert
void main() {
	gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);
}

#frag
uniform lowp vec4 color;

void main() {
	fragColor = color;
}