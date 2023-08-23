#vert
void main() {
	// gl_InstanceID, gl_VertexID
	aafuzz = aafuzzIn;
	gl_Position = matrix * vec4(vertexIn.xy, zDepth, 1.0);
}

#frag
uniform lowp vec4 color;
void main() {
	// fuzz value range: 1 => 0, alpha range: 0 => 1
	// lowp float aaalpha = smoothstep(1.0, aaFuzzLimit, abs(aafuzz));
	lowp float aaalpha = 1.0 - abs(aafuzz);
	fragColor = vec4(color.rgb, color.a * aaalpha);
}