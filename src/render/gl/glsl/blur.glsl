#import "_blur.glsl"
#frag
// gaussian kernel function
// f(x,x') = exp(-|x-x'|^2 / 2σ^2)
#define gk(x) exp(-x*x*2.0)

void main() {
	lowp vec2  coord = gl_FragCoord.xy / oResolution;
	lowp vec4  o = textureLod(image, coord, imageLod);
	lowp float g, x = -1.0, t = 0.0;

	do {
		g = gk(x);
		o += tex(coord, size * x/*offset distance*/) * g;
		t += g;
		x += detail;
	} while(x < 0.0);

	fragColor = blend(o, t*2.0+1.0);
}
