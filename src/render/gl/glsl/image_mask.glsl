#vert
#import "_image.glsl"

#frag
in      lowp  vec2      coord_f;
uniform       sampler2D image;
uniform lowp  vec4      color;

void main() {
	fragColor = color;

	fragColor.a *= texture(image, coord_f).a * (1.0 - abs(aafuzz));

#ifdef Qk_SHADER_IF_FLAGS_AACLIP
	fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#endif
}
