
#define Qk_CONSTANT_Fields \
	vec4 strokeColor; \
	float strokeWidth; \
	int alphaIndex;

#define Qk_FLAG_IMAGE_MASK (1u << 3)
#define Qk_FLAG_IMAGE_SDF_MASK (1u << 4)

#import "_image.glsl"

#frag
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

	Qk_aaSideCoverage();
	Qk_CLIP(); // apply clip mask if needed
}
