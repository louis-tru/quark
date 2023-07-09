#vert
in      float girth_in;
out     float girth_f;
void main() {
	girth_f = girth_in;
	gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);
}

#frag
in      lowp float girth_f;
uniform lowp vec4  color;
void main() {
	fragColor = color;
}