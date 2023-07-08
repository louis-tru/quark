#include "_util.glsl"

in  float sdf_in; // signed distance field
out float sdf_f;
void main() {
	sdf_f = sdf_in;
	gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);
}
