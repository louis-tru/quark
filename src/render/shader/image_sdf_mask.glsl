
#define Qk_CONSTANT_Fields \
	vec4  strokeColor;\
	float strokeWidth;

#import "_image.glsl"

#frag
void main() {
	float dist = texture(image, coords).r;
	float stroke = pc.strokeWidth;
	float alpha = smoothstep(1.2 + stroke, stroke, dist);

	fragColor = mix(pc.color, pc.strokeColor, dist);
	fragColor *= alpha * (1.0 - abs(aaSide)); // premultiplied alpha

	Qk_CLIP(); // apply clip mask if needed
}
