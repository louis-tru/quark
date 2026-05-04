#import "_blur.glsl"

#frag
// gaussian kernel function
// f(x,x') = exp(-|x-x'|^2 / 2σ^2)
#define gk(x) exp(-x*x*2.0)

void main() {
	vec2  coord = gl_FragCoord.xy / pc.oResolution;
	vec4  o = textureLod(image, coord, pc.imageLod);
	float g, x = -1.0, t = 0.0;

	do {
		g = gk(x);
		o += tex(coord, pc.size * x/*offset distance*/) * g;
		// o += tex(coord, vec2(0,0)/*offset distance*/) * g;
		t += g;
		x += pc.detail;
	} while(x < 0.0);

	fragColor = blend(o, t*2.0+1.0);
}
