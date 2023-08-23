#vert
void main() {
	gl_Position = vec4(vertexIn.xy, zDepth, 1.0);
}

#frag
uniform lowp vec4 color;
void main() {
	fragColor = color;
}