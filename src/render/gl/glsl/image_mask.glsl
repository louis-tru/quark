#vert
#import "_image.glsl"

#frag
in      lowp  vec2      coord_f;
uniform       sampler2D image;
uniform lowp  vec4      color;

void main() {
	lowp float alpha = texture(image, coord_f).a;
	fragColor = color;

#ifdef Qk_SHAFER_IF_FLAGS_AAFUZZ
	fragColor.a *= alpha * (1.0 - abs(aafuzz));
#else
	fragColor.a *= alpha;
#endif

#ifdef Qk_SHAFER_IF_FLAGS_AACLIP
	fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#endif
}
