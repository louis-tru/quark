#vert
#import "_image.glsl"

#frag
uniform   sampler2D    image;
uniform   lowp vec4    color;
in        lowp vec2    coords;

void main() {
	fragColor = texture(image, coords) * color;

	// fragColor *= 1.0 - abs(aafuzz); // aalpha
	fragColor *= 1.0 - abs(aafuzz); // premultiplied alpha

#ifdef Qk_SHADER_IF_FLAGS_AACLIP
	// fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
	fragColor *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#endif
}