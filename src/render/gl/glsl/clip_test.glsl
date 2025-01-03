#vert
void main() {
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
layout(location=1) out lowp vec4 aaclipOut; // output anti alias clip texture buffer

void main() {
#ifdef Qk_SHADER_IF_FLAGS_CLIP_FILL
	aaclipOut = vec4(1.0,1.0,1.0,1.0);
#endif
}

// clip
// clip_fill