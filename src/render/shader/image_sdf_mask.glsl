
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
	// fragColor.a *= alpha * (1.0 - abs(aafuzz));
	fragColor *= alpha * (1.0 - abs(aafuzz)); // premultiplied alpha

// #ifdef Qk_SHADER_IF_FLAGS_AACLIP
	Qk_IF_AACLIP {
		// fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
		fragColor *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
	}
// #endif
}
