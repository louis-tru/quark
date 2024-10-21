#vert
#import "_image.glsl"

#frag
in        lowp vec2      coord_f;
uniform   lowp float     alpha;
uniform        sampler2D image;   // y of yuv
uniform        sampler2D image_u; // 420p u or 420sp uv
uniform        sampler2D image_v; // 420p v
uniform   lowp int       format; // 0: YUV420SP, 1: YUV420P

void main() {
	lowp float y = texture(image, coord_f).r;
	lowp vec2  uv = texture(image_u, coord_f).rg;
	lowp float u = uv.x;
	lowp float v = mix(uv.y, texture(image_v, coord_f).r, float(format));

	fragColor = vec4(	y + 1.4075 * (v - 0.5),
										y - 0.3455 * (u - 0.5) - 0.7169 * (v - 0.5),
										y + 1.779  * (u - 0.5),
										alpha);

#ifdef Qk_SHAFER_IF_FLAGS_AAFUZZ
	fragColor.a *= (1.0 - abs(aafuzz));
#endif

#ifdef Qk_SHAFER_IF_FLAGS_AACLIP
	fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#endif
}