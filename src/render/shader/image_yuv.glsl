
#define Qk_CONSTANT_Fields \
	int format; /* 0: YUV420SP, 1: YUV420P*/

#import "_image.glsl"

#frag
// layout(binding=3) uniform sampler2D image;   // y of yuv
layout(binding=4) uniform sampler2D image_u; // 420p u or 420sp uv
layout(binding=5) uniform sampler2D image_v; // 420p v

void main() {
	float y = texture(image, coords).r;
	vec2  uv = texture(image_u, coords).rg; // GL_RG
	float u = uv.r;
	float v = mix(uv.g, texture(image_v, coords).r, float(pc.format));

	fragColor = vec4(	y + 1.4075 * (v - 0.5),
										y - 0.3455 * (u - 0.5) - 0.7169 * (v - 0.5),
										y + 1.779  * (u - 0.5), 1.0) * pc.color;
	// fragColor.a *= 1.0 - abs(aafuzz);
	fragColor *= 1.0 - abs(aafuzz); // premultiplied alpha

// #ifdef Qk_SHADER_IF_FLAGS_AACLIP
	Qk_IF_AACLIP {
		// fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
		fragColor *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
	}
// #endif
}
