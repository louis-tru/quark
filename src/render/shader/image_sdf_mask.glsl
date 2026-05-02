#vert
#import "_image.glsl"

#frag
uniform   sampler2D       image;
uniform   lowp  vec4      color;
uniform   lowp  vec4      strokeColor;
uniform   lowp  float     strokeWidth;
in        lowp  vec2      coords;

void main() {
	lowp float dist = texture(image, coords).r;
	lowp float stroke = strokeWidth;
	lowp float alpha = smoothstep(1.2 + stroke, stroke, dist);

	fragColor = mix(color, strokeColor, dist);
	// fragColor.a *= alpha * (1.0 - abs(aafuzz));
	fragColor *= alpha * (1.0 - abs(aafuzz)); // premultiplied alpha

#ifdef Qk_SHADER_IF_FLAGS_AACLIP
	// fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
	fragColor *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#endif
}
