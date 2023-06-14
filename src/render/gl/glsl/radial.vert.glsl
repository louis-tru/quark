#include "_util.glsl"

out       vec2     position_f;
void main() {
	position_f = vertex_in.xy;
	gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);
}