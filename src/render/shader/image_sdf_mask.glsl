
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
	fragColor *= alpha * (1.0 - abs(aafuzz)); // premultiplied alpha

	Qk_IF_AACLIP {
		fragColor *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
	}
}
