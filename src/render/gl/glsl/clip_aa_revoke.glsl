#vert
void main() {
	aafuzz = aafuzzIn;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
uniform                float     aafuzzWeight;
uniform                float     aafuzzConst;
layout(location=1) out lowp vec4 aaclipOut; // output anti alias clip texture buffer

void main() {
	lowp float alpha = (aafuzzConst + abs(aafuzz)) * aafuzzWeight;
	lowp float clip = texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r;
	aaclipOut = vec4(clip / alpha);
}
