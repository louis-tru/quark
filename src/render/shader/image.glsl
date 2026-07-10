Qk_CONSTANT(
	highp vec4 texCoords; /* offset, scale */
	vec4 color;
	vec4 strokeColor;
	float strokeWidth;
	int alphaIndex;
);

#define Qk_FLAG_IMAGE_MASK (1u << 16)
#define Qk_FLAG_IMAGE_SDF_MASK (1u << 17)
#define Qk_FLAG_IMAGE_CLAMP_TO_ZERO_X (1u << 18)
#define Qk_FLAG_IMAGE_CLAMP_TO_ZERO_Y (1u << 19)

#vert
layout(location=3) out vec2 coords; // texture coordinates uv for fragment shader

void main() {
	aaSide = aaSideIn;
	gl_Position = matrix * vec4(vertexIn.xy, 0.0, 1.0);
	// Qk uses screen-space coordinates internally.
	// Intermediate render targets keep the same memory orientation as uploaded images.
	// Do not flip Y here; backend-specific Y correction is applied only at final present.
	coords = (pc.texCoords.xy + vertexIn.xy) / pc.texCoords.zw; // coord uv
}

#frag
layout(binding=2,set=1) uniform sampler2D image;
layout(location=3) in vec2 coords;

void main() {
	if ((pc.flags & Qk_FLAG_IMAGE_SDF_MASK) != 0) {
		float dist = texture(image, coords).r;
		float width = max(fwidth(dist), 1e-4);
		float alpha = smoothstep(pc.strokeWidth + width, pc.strokeWidth, dist);
		fragColor = mix(pc.color, pc.strokeColor, dist) * alpha;
	} else if ((pc.flags & Qk_FLAG_IMAGE_MASK) != 0) {
		fragColor = pc.color * texture(image, coords)[pc.alphaIndex];
	} else {
		fragColor = texture(image, coords) * pc.color;
	}
#if Qk_SHADER_FLAGS_GLES300
	if ((pc.flags & Qk_FLAG_IMAGE_CLAMP_TO_ZERO_X) != 0) {
		if (coords.x < 0.0 || coords.x > 1.0)
			fragColor = vec4(0.0); // discard;
	}
	if ((pc.flags & Qk_FLAG_IMAGE_CLAMP_TO_ZERO_Y) != 0) {
		if (coords.y < 0.0 || coords.y > 1.0)
			fragColor = vec4(0.0);
	}
#endif
	Qk_aaSideCoverage(); // apply anti-aliasing coverage
	Qk_CLIP(); // apply clip mask if needed
}
