#vert
void main() {
	aafuzz = aafuzzIn;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

// ssbo GL_SHADER_STORAGE_BUFFER
#frag
uniform                float     aafuzzWeight;
uniform                sampler2D aaclip;
layout(location=1) out lowp vec4 aaclipOut; // clip anti alias alpha

void main() {
	lowp float aaalpha = 1.0 - abs(aafuzz);
	lowp float clip = smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).a);
	lowp float alpha = aaalpha * clip * aafuzzWeight * 0.1 + 0.9;
	aaclipOut = vec4(1.0,1.0,1.0,alpha);
}
