#vert
#import "_image.glsl"

#frag
uniform   sampler2D    image;
in        lowp vec2    coord_f;
uniform   lowp float   alpha;

void main() {
	fragColor = texture(image, coord_f);

	lowp float aaa = alpha * (1.0 - abs(aafuzz)); // aaalpha

#ifdef Qk_SHADER_IF_FLAGS_AACLIP
	fragColor.a *= aaa * smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#else
	fragColor.a *= aaa;
#endif
}