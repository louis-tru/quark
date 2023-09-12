#vert
#import "_image.glsl"

#frag
smooth in lowp vec2      coord_f;
uniform   lowp float     alpha;
uniform        sampler2D image;   // y

void main() {
	lowp float aaalpha = 1.0 - abs(aafuzz);
	fragColor = texture(image, coord_f);

#ifdef Qk_SHAFER_AACLIP
	fragColor.a *= alpha * aaalpha * smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#else
	fragColor.a *= alpha * aaalpha;
#endif
}