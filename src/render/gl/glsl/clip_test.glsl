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

#ifdef Qk_LINUX
	// TODO: Maybe have writing the fragColor error if blend mode is not the kSrcOver_BlendMode
	fragColor = vec4(0,0,0,0); // empty color
#endif
}

// clip
// clip_fill