// Resolve CAPA coverage atlas into the active color target.

Qk_CONSTANT(
	vec4 color;
	vec2 surfaceSize;
	vec2 _pad;
);

#vert
void main() {
	gl_Position = rMat.noScale * vec4(vertexIn.xy, 0.0, 1.0);
}

#frag
layout(binding=1,set=1) uniform sampler2D atlasTex;

void main() {
	ivec2 pixel = ivec2(gl_FragCoord.xy);
	vec2 inside = step(vec2(0.0), gl_FragCoord.xy) * step(gl_FragCoord.xy, pc.surfaceSize);
	float coverage = texelFetch(atlasTex, pixel, 0).r * inside.x * inside.y;
	if ((pc.flags & Qk_FLAG_CLIP) != 0)
		coverage *= clipCoverage(vec2(0));
	fragColor = pc.color * coverage;
}
