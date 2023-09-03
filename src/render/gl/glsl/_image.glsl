
uniform   vec4      coord;/*offset,scale*/
out       vec2      coord_f;
void main() {
	aafuzz = aafuzzIn;
	coord_f = (coord.xy + vertexIn.xy) * coord.zw;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}