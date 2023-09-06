#vert
void main() {
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
uniform                vec4      color;
uniform                sampler2D aaalpha;
layout(location=1) out lowp vec4 aaalphaOut; // clip anti alias alpha

void main() {
	// only stencil test
	//aaalphaOut = color;
	//aaalphaOut.a *= texelFetch(aaalpha, ivec2(gl_FragCoord.xy), 0).a;
}
