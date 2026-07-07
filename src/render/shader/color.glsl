Qk_CONSTANT(
	vec4 color;
	vec4 surfaceOffset;
);

#vert
layout(location=3) flat out vec4 color;
void main() {
	aaSide = aaSideIn;
	vec4 pos = vMat.value * vec4(vertexIn.xy, 0.0, 1.0);
	pos.xy += pc.surfaceOffset.xy * pc.surfaceOffset.zw;
	gl_Position = rMat.value * pos;
	color = pc.color;
}

#frag
layout(location=3) flat in vec4 color;
#define Qk_FLAG_AASIDE_Inverted (1u << 16)

void main() {
	// apply anti-aliasing coverage for aaside
	float coverage = aaSideCoverage(pc.flags);
	if ((pc.flags & Qk_FLAG_AASIDE_Inverted) != 0)
		coverage = 1.0 - coverage;

	// apply clipping if enabled
	if ((pc.flags & Qk_FLAG_CLIP) != 0)
		coverage *= clipCoverage(-pc.surfaceOffset.xy);

	fragColor = color * coverage;
}
