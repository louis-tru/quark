Qk_CONSTANT(
	vec4 color;
	vec4 surfaceOffset; // offset xy, surface rMat scale_inv zw: surface.zw = 1 / surfaceScale
);

#vert
void main() {
	aaSide = aaSideIn;
	vec4 pos = vMat.value * vec4(vertexIn.xy, pc.depth, 1.0);
	pos.xy += pc.surfaceOffset.xy * pc.surfaceOffset.zw;
	gl_Position = rMat.value * pos;
}

#frag
#define Qk_FLAG_AASIDE_Inverted (1u << 2)

void main() {
	fragColor = pc.color;
	float coverage = aaSideCoverage();
	if ((pc.flags & Qk_FLAG_AASIDE_Inverted) != 0)
		coverage = 1.0 - coverage; // inverted anti-aliasing side: coverage is inverted
	fragColor *= coverage;
	Qk_CLIP_FROM(clipStat.range.xy - pc.surfaceOffset.xy);
}
