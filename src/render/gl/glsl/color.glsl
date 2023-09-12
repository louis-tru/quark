#vert
void main() {
	// gl_InstanceID, gl_VertexID
	aafuzz = aafuzzIn;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
uniform lowp vec4 color;
void main() {
	// fuzz value range: 1 => 0, alpha range: 0 => 1
	lowp float aaalpha = 1.0 - abs(aafuzz);
	fragColor = color;

#ifdef Qk_SHAFER_AACLIP
	fragColor.a *= aaalpha * smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#else
	fragColor.a *= aaalpha;
#endif
}