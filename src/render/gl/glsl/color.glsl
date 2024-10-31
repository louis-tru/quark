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
#endif

#ifdef Qk_SHADER_IF_FLAGS_AACLIP
	fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#endif
}

// color
// color_aafuzz
// color_aaclip
// color_aafuzz_aaclip