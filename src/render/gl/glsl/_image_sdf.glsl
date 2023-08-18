
in        float     sdf_in; // signed distance field
uniform   vec4      coord; /*offset,scale*/
out       vec2      coord_f;
out       float     sdf_f;

void main() {
	coord_f = (coord.xy + vertexIn.xy) * coord.zw;
	sdf_f = sdf_in;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}