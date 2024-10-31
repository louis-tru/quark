// viewport image copy
uniform lowp vec2           iResolution; // viewport resolution
uniform lowp vec2           oResolution; // output image resolution of fragColor
#vert
void main() {
	gl_Position = rootMatrix * vec4(vertexIn.xy * oResolution / iResolution, depth, 1.0);
	gl_Position.y += (oResolution.y / iResolution.y - 1.0) * 2.0; // correct canvas offset
}

#frag
uniform lowp vec4           coord; // texture offset coord, vec4(offset,scale) Fully mapped viewport
uniform lowp float          imageLod; // input image lod level
uniform sampler2D           image; // input image

void main() {
	fragColor = textureLod(image, gl_FragCoord.xy / oResolution * coord.zw + coord.xy, imageLod);
}
