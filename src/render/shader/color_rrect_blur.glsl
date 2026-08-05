// Ref:
// https://raphlinus.github.io/graphics/2020/04/21/blurred-rounded-rects.html
// https://madebyevan.com/shaders/fast-rounded-rectangle-shadows/
// https://www.shadertoy.com/view/DsdfDN
// https://en.wikipedia.org/wiki/Squircle

Qk_CONSTANT(
	vec4  color;
	highp vec4 consts; // consts
	highp vec4 rect; // rect begin/size
	highp vec4 clipRect; // clip rect, begin/size
	highp vec4 clipRadii; // clip round
	highp vec2 vPos;
	highp float min_edge; // rect min edge size
);

#vert
layout(location=1) out highp vec2 pos_f;
void main() {
	pos_f = vertexIn;
	gl_Position = rMat.value * vPosition(pc.vPos);
}

#frag
#define Qk_FLAG_USE_DIFF_CLIP (1u << 16)

layout(location=1) in highp vec2 pos_f;
// blur radius
#define r1 pc.consts.x
// squircle exponent
#define n pc.consts.y
// 1/n
#define n_inv pc.consts.z
// 1/s blur size reciprocal
#define s_inv pc.consts.w
// min w or h
#define min_edge pc.min_edge

float erf(float x) {
	float s = sign(x), a = abs(x);
	x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
	x *= x; // x^2
	return s - s / (x * x);
}

highp float sqLen(highp vec2 p) { // squircle length
	// https://en.wikipedia.org/wiki/Squircle
	highp vec2 q = max(p,0.0);
	return pow(pow(q.x,n) + pow(q.y,n), n_inv);
}

highp float sdf(highp vec2 p, highp float r) {
	highp vec2 halfSize = pc.rect.zw * 0.5;
	highp vec2 q = abs(p - (pc.rect.xy + halfSize)) - halfSize;
	return sqLen(q+r) - r;
}

float clipRRectSdf(highp vec2 p) {
	highp vec2 halfSize = pc.clipRect.zw * 0.5;
	highp vec2 center = pc.clipRect.xy + halfSize;
	vec2 radii = mix(pc.clipRadii.xw, pc.clipRadii.yz, step(center.x, p.x));
	float radius = mix(radii.x, radii.y, step(center.y, p.y));
	radius = min(radius, min(halfSize.x, halfSize.y));
	vec2 q = abs(p - center) - halfSize + radius;
	return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - radius;
}

void main() {
	float d = sdf(pos_f, r1);
	float z = (erf(s_inv * (d + min_edge)) - erf(s_inv * d)) * 0.5;
	if ((pc.flags & Qk_FLAG_USE_DIFF_CLIP) != 0) {
		float clipDistance = clipRRectSdf(pos_f);
		float aa = max(fwidth(clipDistance), 1e-4);
		z *= smoothstep(-aa*1.5, aa*0.5, clipDistance);
	}
	fragColor = pc.color;
	fragColor *= z; // premultiplied alpha

	Qk_CLIP(); // apply clip mask if needed
}
