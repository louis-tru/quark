#vert
#import "_image.glsl"

#frag
uniform   sampler2D    image;
uniform   lowp vec4    color;
in        lowp vec2    coords;

void main() {
	fragColor = texture(image, coords) * color;

	lowp float aa = 1.0 - abs(aafuzz); // aalpha

#ifdef Qk_SHADER_IF_FLAGS_AACLIP
	fragColor.a *= aa * smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#else
	fragColor.a *= aa;
#endif
	//fragColor.a += 0.3;
}