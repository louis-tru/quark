#include "_util.glsl"

void main() {
	gl_Position = vec4(vertex_in.xy, 0.0, 1.0);
}