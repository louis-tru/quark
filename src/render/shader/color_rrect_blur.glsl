// Ref:
// https://raphlinus.github.io/graphics/2020/04/21/blurred-rounded-rects.html
// https://madebyevan.com/shaders/fast-rounded-rectangle-shadows/
// https://www.shadertoy.com/view/DsdfDN
// https://en.wikipedia.org/wiki/Squircle

Qk_CONSTANT(
	vec2  horn; // horn pos, left/top,right/top,right/bottom,left/bottom
	vec4  color;
	vec3  consts; // consts
	float min_edge; // rect min edge size
	float s_inv; // 1/s, blur size reciprocal
);

#vert
layout(location=1) out vec2 pos_f;
void main() {
	pos_f = vertexIn.xy;
	gl_Position = matrix * vec4(vertexIn.xy, pc.depth, 1.0);
}

#frag
layout(location=1) in vec2 pos_f;
// blur radius
#define r1 pc.consts.x
// squircle exponent
#define n pc.consts.y
// 1/n
#define n_inv pc.consts.z

float erf(float x) {
	float s = sign(x), a = abs(x);
	x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
	x *= x; // x^2
	return s - s / (x * x);
}

float sqLen(vec2 p) { // squircle length
	// https://en.wikipedia.org/wiki/Squircle
	vec2 q = max(p,0.0);
	return pow(pow(q.x,n) + pow(q.y,n), n_inv);
}

float sdf(vec2 p, float r) {
	vec2 q = p - pc.horn + r;
	return /*min(max(q.x,q.y),0.0) + */sqLen(p)-r;
}

void main() {
	float d = sdf(pos_f, r1);
	float z = (erf(pc.s_inv * (d + pc.min_edge)) - erf(pc.s_inv * d)) * 0.5;
	fragColor = pc.color;
	// fragColor.a *= z;
	fragColor *= z; // premultiplied alpha

// #ifdef Qk_SHADER_IF_FLAGS_AACLIP
	Qk_IF_AACLIP {
		// fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
		fragColor *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
	}
// #endif
}
