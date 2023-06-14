#include "_util.glsl"

in      float girth_in;
out     float girth_f;
void main() {
	girth_f = girth_in;
	gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);
}
