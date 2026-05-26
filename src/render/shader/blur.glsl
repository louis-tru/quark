#import "_blur.glsl"

#frag
// gaussian kernel function
// f(x,x') = exp(-|x-x'|^2 / 2σ^2)
#define gk(x) exp(-x*x*2.0)

void main() {
	vec2  uv = gl_FragCoord.xy / pc.oResolution + pc.uv_offset; // coord uv
	vec4  o = textureLod(image, uv, pc.imageLod); // original color
	float g, x = -1.0, t = 0.0;

	do {
		g = gk(x);
		o += tex(uv, pc.uv_radius * x/*offset distance*/) * g;
		t += g;
		x += pc.sample_inv;
	} while(x < -0.01); // x < -0.01 to avoid precision issue when x is close to 0

	fragColor = blend(o, t*2.0+1.0);

	// fragColor.a += 0.1; // debug
}
