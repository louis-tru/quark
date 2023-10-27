#import "_blur.glsl"
#frag
const lowp int N = 7;
const lowp float step = 0.166667;
const lowp float gk_t = 3.149027;

void main() {
	lowp vec2  coord = gl_FragCoord.xy / oResolution;
	lowp vec4  o = textureLod(image, coord, imageLod);
	lowp float x = step;
	o += tex(coord, size * x) * 0.945959; x += step;
	o += tex(coord, size * x) * 0.800737; x += step;
	o += tex(coord, size * x) * 0.606531; x += step;
	o += tex(coord, size * x) * 0.411112; x += step;
	o += tex(coord, size * x) * 0.249352; x += step;
	o += tex(coord, size * x) * 0.135335; x += step;
	fragColor = blend(o, gk_t*2.0+1.0);
}