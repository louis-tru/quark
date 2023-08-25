
uniform   vec4      coord;/*offset,scale*/
out       vec2      coordF;
void main() {
	aafuzz = aafuzzIn;
	coordF = (coord.xy + vertexIn.xy) * coord.zw;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}