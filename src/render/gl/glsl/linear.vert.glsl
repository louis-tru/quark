#include "_util.glsl"

uniform   vec4      range;/*start/end range for rect*/
out       float     indexed_f;
void main() {
	vec2 ao = range.zw     - range.xy;
	vec2 bo = vertex_in.xy - range.xy;
	/*indexed_f = clamp(dot(ao,bo) / dot(ao,ao), 0.0, 1.0);*/
	indexed_f = dot(ao,bo) / dot(ao,ao);
	gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);
}