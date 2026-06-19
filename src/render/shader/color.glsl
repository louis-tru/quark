#define Qk_CONSTANT_Fields \
	vec4 color; \
	vec4 surfaceOffset;

#import "_cgaa.glsl"

#vert
layout(location=3) flat out vec4 color;
void main() {
#if Qk_SHADER_FLAGS_ENABLE_CGAA
	if ((pc.flags & Qk_FLAG_CGAA) != 0) {
		Qk_cgaaVertexSteps();
		gl_Position = rMat.noScale * vec4(cgaaPosition + pc.surfaceOffset.xy, 0.0, 1.0);
		color = path.color;
	} else
#endif
	{
		aaSide = aaSideIn;
		vec4 pos = vMat.value * vec4(vertexIn.xy, 0.0, 1.0);
		pos.xy += pc.surfaceOffset.xy * pc.surfaceOffset.zw;
		gl_Position = rMat.value * pos;
		color = pc.color;
	}
}

#frag
layout(location=3) flat in vec4 color;
#define Qk_FLAG_AASIDE_Inverted (1u << 16)

void main() {
	float coverage = aaCoverage();
	if ((pc.flags & Qk_FLAG_AASIDE_Inverted) != 0)
		coverage = 1.0 - coverage;
	fragColor = color * coverage;
	Qk_CLIP_FROM(clipStat.range.xy - pc.surfaceOffset.xy);
}
