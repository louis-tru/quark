
uniform   vec4      coord;/*offset,scale*/
uniform   float     allScale; // surface scale * view matrix scale
out       vec2      coord_f;
void main() {
	aafuzz = aafuzzIn;
	coord_f = (coord.xy + vertexIn.xy) / coord.zw;
	vec4 pos = (viewMatrix * vec4(vertexIn.xy, depth, 1.0));
	// fix draw image tearing with round function
	// Align the image pixels exactly onto the drawing surface
	gl_Position = rootMatrix * vec4(round(pos.xy * allScale) / allScale, pos.zw);
	//gl_Position = rootMatrix * vec4(pos.xy, pos.zw);
}
