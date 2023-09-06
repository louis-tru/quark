#vert
void main() {
	gl_Position = vec4(vertexIn.xy, depth, 1.0);
}

#frag
uniform lowp vec4 color;
void main() {
	fragColor = vec4(1.0,0.0,0.0,1.0);// color;
}