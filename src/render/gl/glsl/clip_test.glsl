#vert
void main() {
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag

void main() {
#ifdef Qk_SHADER_IF_FLAGS_CLEAR_AA
	fragColor = vec4(1.0,1.0,1.0,1.0);
#elif defined(Qk_LINUX)
	// TODO: Maybe have writing the fragColor error if blend mode is not the kSrcOver_BlendMode
	fragColor = vec4(0,0,0,0); // empty color
#endif
}

// clip
// clip_fill
