#include "_util.glsl"

void main() {
	gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);
}
