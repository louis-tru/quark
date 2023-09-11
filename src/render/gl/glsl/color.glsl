#vert
void main() {
	// gl_InstanceID, gl_VertexID
	aafuzz = aafuzzIn;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
uniform lowp vec4 color;
uniform sampler2D aaclip;
void main() {
	// fuzz value range: 1 => 0, alpha range: 0 => 1
	//lowp float clip = smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).a);
	// lowp float clip = texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).a;
	// lowp float aaalpha = 1.0 - abs(aafuzz);
	// fragColor = color;
	// fragColor.a *= aaalpha * clip;

	fragColor = texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0);
	// fragColor = vec4(1.0,0.0,0.0, texelFetch(aaclip, ivec2(1,1), 0).r);
	// fragColor = vec4(1.0,0.0,0.0, 1.0);
	// fragColor = texelFetch(aaclip, ivec2(1,1), 0);
	//fragColor = texture(aaclip, vec2(0.5,0.5));
}