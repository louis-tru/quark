#vert
void main() {
	// gl_InstanceID, gl_VertexID
	aafuzz = aafuzzIn;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
uniform lowp vec4 color;
void main() {
	fragColor = color;

#ifdef Qk_SHADER_IF_FLAGS_AAFUZZ
	// fuzz value range: 1 => 0, alpha range: 0 => 1
	fragColor.a *= (1.0 - abs(aafuzz));
#else
// only use aaclip in not aafuzz macros, if both aafuzz and aaclip macros enabled then
// aafuzz will fail, maybe this is not get aaclip value correctly in some GPU,
// for example macOS, it's very strange that may this a BUG.
// so only use a clip when not aafuzz, because aafuzz is more less dependent on a aaclip.
#ifdef Qk_SHADER_IF_FLAGS_AACLIP
	fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#endif
#endif
}

// color
// color_aafuzz
// color_aaclip
// color_aafuzz_aaclip
