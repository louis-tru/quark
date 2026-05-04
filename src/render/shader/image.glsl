
#define Qk_CONSTANT_Fields

#import "_image.glsl"

#frag
void main() {
	fragColor = texture(image, coords) * pc.color;

	// fragColor.a *= 1.0 - abs(aafuzz); // aalpha
	fragColor *= 1.0 - abs(aafuzz); // premultiplied alpha

// #ifdef Qk_SHADER_IF_FLAGS_AACLIP
	Qk_IF_AACLIP {
		// fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
		fragColor *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
	}
// #endif
}
