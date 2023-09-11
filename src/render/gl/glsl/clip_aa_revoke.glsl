#vert
void main() {
	aafuzz = aafuzzIn;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
uniform                float     aafuzzWeight;
uniform                float     aafuzzConst;
uniform                sampler2D aaclip;
layout(location=1) out lowp vec4 aaclipOut; // clip anti alias alpha

void main() {
	lowp float alpha = (aafuzzConst + abs(aafuzz)) * aafuzzWeight;
	lowp float clip = texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).a;
	// aaclipOut = vec4(1.0,1.0,1.0, clip / alpha);
	aaclipOut = vec4(1.0,1.0,1.0,1.0);
}
