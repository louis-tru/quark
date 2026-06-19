#define Qk_CONSTANT_Fields \
	vec4 range; /* linear: origin/end, radial: center/radius */ \
	vec4 color; \
	int count;

#define Qk_FLAG_GRADIENT_COUNT2 (1u << 16)
#define Qk_FLAG_RADIAL_GRADIENT (1u << 17)

#import "_cgaa.glsl"

#vert
layout(location=3) out vec2 gradientValue;

void main() {
	vec2 vertex = vertexIn.xy;
#if Qk_SHADER_FLAGS_ENABLE_CGAA
	if ((pc.flags & Qk_FLAG_CGAA) != 0) {
		Qk_cgaaVertexSteps();
		vertex = cgaaCanvasPosition();
		gl_Position = rMat.noScale * vec4(cgaaPosition, 0.0, 1.0);
	} else
#endif
	{
		aaSide = aaSideIn;
		gl_Position = matrix * vec4(vertex, 0.0, 1.0);
	}
	if ((pc.flags & Qk_FLAG_RADIAL_GRADIENT) != 0) {
		gradientValue = vertex;
	} else {
		vec2 axis = pc.range.zw - pc.range.xy;
		float weight = dot(axis, vertex - pc.range.xy) / dot(axis, axis);
		gradientValue = vec2(weight, 0.0);
	}
}

#frag
layout(location=3) in vec2 gradientValue;

layout(binding=4, set=0, std140) uniform Colors {
	vec4 colors[64];
};
layout(binding=5, set=0, std140) uniform Positions {
	vec4 positions[16]; // 16 * 4 = 64 floats, enough for 64 stops
};

float getPos(int i) {
	return positions[i >> 2][i & 3];
}

float getWeight() {
	if ((pc.flags & Qk_FLAG_RADIAL_GRADIENT) != 0)
		return length((gradientValue - pc.range.xy) / pc.range.zw);
	return gradientValue.x;
}

void main() {
	float weight = getWeight();
	if ((pc.flags & Qk_FLAG_GRADIENT_COUNT2) != 0) {
		float w = (weight - getPos(0)) / (getPos(1) - getPos(0));
		fragColor = mix(colors[0], colors[1], w);
	} else {
		int s = 0;
		int e = pc.count - 1;
		while (s + 1 < e) {/*dichotomy search color value*/
			int idx = (e - s) / 2 + s;
			if (weight > getPos(idx)) {
				s = idx;
			} else if (weight < getPos(idx)) {
				e = idx;
			} else {
				s = idx;
				e = idx + 1;
				break;
			}
		}
		float w = (weight - getPos(s)) / (getPos(e) - getPos(s));
		fragColor = mix(colors[s], colors[e], w);
	}
	fragColor *= pc.color;

	Qk_aaCoverage();
	Qk_CLIP();
}
