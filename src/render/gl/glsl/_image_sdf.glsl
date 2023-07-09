
in        float     sdf_in; // signed distance field
uniform   vec4      coord; /*offset,scale*/
out       vec2      coord_f;
out       float     sdf_f;

void main() {
	coord_f = (coord.xy + vertex_in.xy) * coord.zw;
	sdf_f = sdf_in;
	gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);
}