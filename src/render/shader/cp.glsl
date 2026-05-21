Qk_CONSTANT(
	vec2 iResolution; // viewport resolution
	vec2 oResolution; // output image resolution, the <= iResolution
	// frag
	vec4 coord; // vec4(texture offset coord, scale coefficient)
	float imageLod; // input image lod level
);

#vert
void main() {
	vec2 scale = pc.oResolution / pc.iResolution;
	gl_Position = rMat.value * vec4(vertexIn.xy * scale, pc.depth, 1.0);
}

#frag
#define Qk_FLAG_CLAMP_TO_ZERO (1u << 2)
layout(binding=1,set=1) uniform sampler2D image; // input image

void main() {
	vec2 uv = gl_FragCoord.xy / pc.oResolution; // coord uv
	uv = uv * pc.coord.zw + pc.coord.xy; // apply offset and scale
	fragColor = textureLod(image, uv, pc.imageLod);

	if ((pc.flags & Qk_FLAG_CLAMP_TO_ZERO) != 0) {
		vec2 m = step(vec2(0.0), uv) * step(uv, vec2(1.0));
		fragColor *= m.x * m.y; // clamp to zero if out of range
	}
}
