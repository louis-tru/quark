
#define Qk_CONSTANT_Fields \
	vec4  strokeColor;\
	float strokeWidth;

#import "_image.glsl"

#frag
void main() {
	float dist = texture(image, coords).r;
	float stroke = pc.strokeWidth;
	float width = max(fwidth(dist), 1e-4);
	float alpha = smoothstep(stroke + width, stroke, dist);

	fragColor = mix(pc.color, pc.strokeColor, dist);
	fragColor *= alpha;

	Qk_aaSideCoverage();
	Qk_CLIP(); // apply clip mask if needed
}
