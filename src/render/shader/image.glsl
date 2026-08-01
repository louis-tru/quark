Qk_CONSTANT(
	highp vec4 texCoords; /* offset, scale */
	vec4 color;
	vec4 strokeColor;
	highp vec2 vPos;
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
	gl_Position = rMat.value * vPosition(pc.vPos);
	// Qk uses screen-space coordinates internally.
	// Intermediate render targets keep the same memory orientation as uploaded images.
	// Do not flip Y here; backend-specific Y correction is applied only at final present.
	coords = (pc.texCoords.xy + vertexIn) / pc.texCoords.zw; // coord uv
}

#frag
layout(binding=1,set=1) uniform sampler2D image;
layout(location=3) in vec2 coords;

void main() {
	if ((pc.flags & Qk_FLAG_IMAGE_SDF_MASK) != 0) {
		float dist = texture(image, coords).r;
		float width = max(fwidth(dist), 1e-4);
		float alpha = 1.0 - smoothstep(pc.strokeWidth, pc.strokeWidth + width, dist);
		fragColor = mix(pc.color, pc.strokeColor, dist) * alpha;
	} else if ((pc.flags & Qk_FLAG_IMAGE_MASK) != 0) {
		fragColor = pc.color * texture(image, coords)[pc.alphaIndex];
	} else {
		fragColor = texture(image, coords) * pc.color;
	}
	if ((pc.flags & Qk_FLAG_IMAGE_CLAMP_TO_ZERO_X) != 0) {
		if (coords.x < 0.0 || coords.x > 1.0)
			fragColor = vec4(0.0); // discard;
	}
	if ((pc.flags & Qk_FLAG_IMAGE_CLAMP_TO_ZERO_Y) != 0) {
		if (coords.y < 0.0 || coords.y > 1.0)
			fragColor = vec4(0.0);
	}
	Qk_aaSideCoverage(); // apply anti-aliasing coverage
	Qk_CLIP(); // apply clip mask if needed
}
