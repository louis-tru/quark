Qk_CONSTANT(
	vec4 color;
	highp vec4 surfaceOffset;
	highp vec2 vPos;
);

#vert
void main() {
	aaSide = aaSideIn;
	gl_Position = rMat.value * vPosition(pc.vPos + pc.surfaceOffset.xy * pc.surfaceOffset.zw);
}

#frag
#define Qk_FLAG_AASIDE_Inverted (1u << 16)

void main() {
	// apply anti-aliasing coverage for aaside
	float coverage = aaSideCoverage(pc.flags);
	if ((pc.flags & Qk_FLAG_AASIDE_Inverted) != 0)
		coverage = 1.0 - coverage;

	// apply clipping if enabled
	if ((pc.flags & Qk_FLAG_CLIP) != 0)
		coverage *= clipCoverage(ivec2(gl_FragCoord.xy - pc.surfaceOffset.xy));

	fragColor = pc.color * coverage;
}