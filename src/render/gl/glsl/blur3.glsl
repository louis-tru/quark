#import "_blur.glsl"
#frag
const lowp int N = 3;
const lowp float step = 0.500000;
const lowp float gk_t = 0.741866;

void main() {
	lowp vec2  coord = gl_FragCoord.xy / oResolution;
	lowp vec4  o = textureLod(image, coord, imageLod);
	lowp float x = step;
	o += tex(coord, size * x) * 0.606531; x += step;
	o += tex(coord, size * x) * 0.135335; x += step;
	fragColor = blend(o, gk_t*2.0+1.0);
}