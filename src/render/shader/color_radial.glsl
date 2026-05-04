
Qk_CONSTANT(
	vec4 range; /*center/radius for circle*/
	vec4 color;
	int count;
);

#vert
layout(location=1) out vec2 position;

void main() {
	aafuzz = aafuzzIn;
	position = vertexIn.xy;
	gl_Position = matrix * vec4(vertexIn.xy, pc.depth, 1.0);
}

#frag
#define Qk_FLAG_COUNT2 (1u << 1)

layout(location=1) in vec2 position;

layout(std140, binding=4) uniform Colors {
	vec4  colors[64];
};
layout(std140, binding=5) uniform Positions {
	vec4 positions[16];
};

float getPos(int i) {
	return positions[i >> 2][i & 3];
}

void main() {
	float weight = length((position-pc.range.xy)/pc.range.zw); // 0 - 1
// #ifdef Qk_SHADER_IF_FLAGS_COUNT2
	if ((pc.flags & Qk_FLAG_COUNT2) != 0) {
		float w = (weight - getPos(0)) / (getPos(1) - getPos(0));
		fragColor = mix(colors[0], colors[1], w);
	} else {
// #else
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
// #endif
	fragColor *= pc.color;
	// fragColor.a *= 1.0 - abs(aafuzz);
	fragColor *= 1.0 - abs(aafuzz); // premultiplied alpha

// #ifdef Qk_SHADER_IF_FLAGS_AACLIP
	Qk_IF_AACLIP {
		//fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
		// premultiplied alpha
		fragColor *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
	}
// #endif
}