
uniform   vec4      texCoords;/*offset,scale*/
uniform   float     allScale; // surface scale * view matrix scale
out       vec2      coords;

void main() {
	vec4 pos = (viewMatrix * vec4(vertexIn.xy, depth, 1.0));
	// fix draw image tearing with round function
	// Align the image pixels exactly onto the drawing surface
	//gl_Position = rootMatrix * vec4(round(pos.xy * allScale) / allScale, pos.zw);
	gl_Position = rootMatrix * vec4(pos.xy, pos.zw);

	aafuzz = aafuzzIn;
	coords = (texCoords.xy + vertexIn.xy) / texCoords.zw;
}
