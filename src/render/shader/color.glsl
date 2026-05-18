Qk_CONSTANT(
	vec4 color;
);

#vert
void main() {
	// gl_InstanceID, gl_VertexID
	aafuzz = aafuzzIn;
	gl_Position = matrix * vec4(vertexIn.xy, pc.depth, 1.0);
}

#frag

void main() {
	fragColor = pc.color;

	// fuzz value range: 1 => 0, alpha range: 0 => 1
	fragColor *= (1.0 - abs(aafuzz)); // premultiplied alpha

	Qk_IF_AACLIP {
		fragColor *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
	}
}
