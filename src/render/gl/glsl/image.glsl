#vert
#import "_image.glsl"

#frag
in        lowp vec2      coord_f;
uniform   lowp float     alpha;
uniform        sampler2D image;

void main() {
	fragColor = texture(image, coord_f);

#ifdef Qk_SHADER_IF_FLAGS_AAFUZZ
	fragColor.a *= alpha * (1.0 - abs(aafuzz));
#else
	fragColor.a *= alpha;
#endif

#ifdef Qk_SHADER_IF_FLAGS_AACLIP
	fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#endif
}