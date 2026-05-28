Qk_CONSTANT(
	vec4 range; /*origin/end range for rect*/
	vec4 color;
	int count;
);

#vert
layout(location=1) out float weight;

void main() {
	vec2 ao = pc.range.zw    - pc.range.xy;
	vec2 bo = vertexIn.xy - pc.range.xy;
	/*indexed = clamp(dot(ao,bo) / dot(ao,ao), 0.0, 1.0);*/
	weight = dot(ao,bo) / dot(ao,ao);
	aaSide = aaSideIn;
	gl_Position = matrix * vec4(vertexIn.xy, pc.depth, 1.0);
}

#frag
#define Qk_FLAG_COUNT2 (1u << 2)

layout(location=1) in float weight;

layout(binding=4, set=0, std140) uniform Colors {
	vec4  colors[64];
};
layout(binding=5, set=0, std140) uniform Positions {
	vec4 positions[16]; // 16 * 4 = 64 floats, enough for 64 stops
};

float getPos(int i) {
	return positions[i >> 2][i & 3];
}

void main() {
	if ((pc.flags & Qk_FLAG_COUNT2) != 0) {
		float w = (weight - getPos(0)) / (getPos(1) - getPos(0));
		fragColor = mix(colors[0], colors[1], w);
	} else {
		int s = 0;
		int e = pc.count-1;
		while (s+1 < e) {/*dichotomy search color value*/
			int idx = (e - s) / 2 + s;
			if (weight > getPos(idx)) {
				s = idx;
			} else if (weight < getPos(idx)) {
				e = idx;
			} else {
				s = idx; e = idx+1; break;
			}
		}
		float w = (weight - getPos(s)) / (getPos(e) - getPos(s));
		fragColor = mix(colors[s], colors[e], w);
	}
	fragColor *= pc.color;
	fragColor *= 1.0 - abs(aaSide); // premultiplied alpha

	Qk_CLIP(); // apply clip mask if needed
}