// Ref:
// https://raphlinus.github.io/graphics/2020/04/21/blurred-rounded-rects.html
// https://madebyevan.com/shaders/fast-rounded-rectangle-shadows/
// https://www.shadertoy.com/view/DsdfDN
// https://en.wikipedia.org/wiki/Squircle

#vert
out     vec2      pos_f;
void main() {
	pos_f = vertexIn.xy;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
in      lowp vec2  pos_f;
uniform lowp vec2  horn; // horn pos, left/top,right/top,right/bottom,left/bottom
uniform lowp vec4  color;
uniform lowp float min_edge; // rect min edge size
uniform lowp float s_inv; // 1/s, blur size reciprocal
uniform lowp vec3  consts; // consts

// blur radius
#define r1 consts.x
// squircle exponent
#define n consts.y
// 1/n
#define n_inv consts.z

lowp float erf(lowp float x) {
	lowp float s = sign(x), a = abs(x);
	x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
	x *= x; // x^2
	return s - s / (x * x);
}

lowp float sqLen(lowp vec2 p) { // squircle length
	// https://en.wikipedia.org/wiki/Squircle
	lowp vec2 q = max(p,0.0);
	return pow(pow(q.x,n) + pow(q.y,n), n_inv);
}

lowp float sdf(lowp vec2 p, lowp float r) {
	lowp vec2 q = p - horn + r;
	return /*min(max(q.x,q.y),0.0) + */sqLen(p)-r;
}

void main() {
	lowp float d = sdf(pos_f, r1);
	lowp float z = (erf(s_inv * (d + min_edge)) - erf(s_inv * d)) * 0.5;
	fragColor = color;
	// fragColor.a *= z;
	fragColor *= z; // premultiplied alpha

#ifdef Qk_SHADER_IF_FLAGS_AACLIP
	// fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
	fragColor *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#endif
}
