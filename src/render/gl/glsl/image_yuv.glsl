#vert
#import "_image.glsl"

#frag
uniform   lowp vec4      color;
uniform   sampler2D      image;   // y of yuv
uniform   sampler2D      image_u; // 420p u or 420sp uv
uniform   sampler2D      image_v; // 420p v
uniform   lowp int       format; // 0: YUV420SP, 1: YUV420P
in        lowp vec2      coords;

void main() {
	lowp float y = texture(image, coords).r;
	lowp vec2  uv = texture(image_u, coords).rg; // GL_RG
	lowp float u = uv.r;
	lowp float v = mix(uv.g, texture(image_v, coords).r, float(format));

	fragColor = vec4(	y + 1.4075 * (v - 0.5),
										y - 0.3455 * (u - 0.5) - 0.7169 * (v - 0.5),
										y + 1.779  * (u - 0.5), 1.0) * color;
	// fragColor.a *= 1.0 - abs(aafuzz);
	fragColor *= 1.0 - abs(aafuzz); // premultiplied alpha

#ifdef Qk_SHADER_IF_FLAGS_AACLIP
	// fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
	fragColor *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#endif
}
