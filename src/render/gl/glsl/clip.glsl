#vert
void main() {
	gl_Position = matrix * vec4(vertexIn.xy, zDepth, 1.0);
}

#frag
uniform                sampler2D aaalpha;
layout(location=1) out lowp vec4 aaalphaOut; // clip anti alias alpha

void main() {
	// only stencil test
	lowp float alpha = texelFetch(aaalpha, ivec2(gl_FragCoord.xy), 0).a;
	aaalphaOut = vec4(1.0,1.0,1.0,alpha);
}
