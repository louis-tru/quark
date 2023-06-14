#include "_util.glsl"

uniform   vec4      coord;/*offset,scale*/
out       vec2      coord_f;
void main() {
	coord_f = (coord.xy + vertex_in.xy) * coord.zw;
	gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);
}