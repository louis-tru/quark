#import "_blur.glsl"
#frag
const lowp int N = 19;
const lowp float step = 0.055556;
const lowp float gk_t = 10.331754;

void main() {
	lowp vec2  coord = gl_FragCoord.xy / oResolution;
	lowp vec4  o = textureLod(image, coord, imageLod);
	lowp float x = step;
	o += tex(coord, size * x) * 0.993846; x += step;
	o += tex(coord, size * x) * 0.975611; x += step;
	o += tex(coord, size * x) * 0.945959; x += step;
	o += tex(coord, size * x) * 0.905955; x += step;
	o += tex(coord, size * x) * 0.856997; x += step;
	o += tex(coord, size * x) * 0.800737; x += step;
	o += tex(coord, size * x) * 0.738991; x += step;
	o += tex(coord, size * x) * 0.673638; x += step;
	o += tex(coord, size * x) * 0.606531; x += step;
	o += tex(coord, size * x) * 0.539408; x += step;
	o += tex(coord, size * x) * 0.473827; x += step;
	o += tex(coord, size * x) * 0.411112; x += step;
	o += tex(coord, size * x) * 0.352322; x += step;
	o += tex(coord, size * x) * 0.298234; x += step;
	o += tex(coord, size * x) * 0.249352; x += step;
	o += tex(coord, size * x) * 0.205924; x += step;
	o += tex(coord, size * x) * 0.167973; x += step;
	o += tex(coord, size * x) * 0.135335; x += step;
	fragColor = blend(o, gk_t*2.0+1.0);
}