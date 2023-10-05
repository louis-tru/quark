#vert
void main() {
	gl_Position = rootMatrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
uniform lowp vec4 color;
void main() {
	fragColor = color;
}