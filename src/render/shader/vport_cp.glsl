// viewport image copy
Qk_CONSTANT(
	vec2 iResolution; // gl viewport resolution
	vec2 oResolution; // output image resolution of fragColor <= gl viewport
	// frag
	vec4 coord; // texture offset coord, vec4(offset,scale) Fully mapped viewport
	float imageLod; // input image lod level
);

#vert
void main() {
	gl_Position = rMat.value * vec4(vertexIn.xy * pc.oResolution / pc.iResolution, pc.depth, 1.0);
	gl_Position.y += (pc.oResolution.y / pc.iResolution.y - 1.0) * 2.0; // correct canvas offset
}

#frag
layout(binding=3) uniform sampler2D image; // input image

void main() {
	fragColor = textureLod(image, gl_FragCoord.xy / pc.oResolution * pc.coord.zw + pc.coord.xy, pc.imageLod);
}
