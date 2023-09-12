#vert
#import "_image.glsl"

#frag
in      lowp  vec2      coord_f;
uniform       sampler2D image;
uniform lowp  vec4      color;

void main() {
	lowp float aaalpha = 1.0 - abs(aafuzz);
	lowp float alpha = texture(image, coord_f).a;
	fragColor = color;

#ifdef Qk_SHAFER_AACLIP
	fragColor.a *= alpha * aaalpha * smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#else
	fragColor.a = alpha * aaalpha;
#endif
}
