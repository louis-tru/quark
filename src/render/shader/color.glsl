Qk_CONSTANT(
	vec4 color;
	vec4 surfaceOffset; // clip output offset xy, inverse surface scale zw
);

#vert
void main() {
	aaSide = aaSideIn;
	vec4 pos = vMat.value * vec4(vertexIn.xy, 0.0, 1.0);
	pos.xy += pc.surfaceOffset.xy * pc.surfaceOffset.zw;
	gl_Position = rMat.value * pos;
}

#frag
#define Qk_FLAG_AASIDE_Inverted (1u << 3)

void main() {
	fragColor = pc.color;
	float coverage = aaSideCoverage(pc.flags);
	if ((pc.flags & Qk_FLAG_AASIDE_Inverted) != 0)
		coverage = 1.0 - coverage;
	fragColor *= coverage;
	Qk_CLIP_FROM(clipStat.range.xy - pc.surfaceOffset.xy);
}
