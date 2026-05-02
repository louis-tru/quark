#import "_blur.glsl"
#frag
const lowp int N = 19;
const lowp float stepping = 0.055556;
const lowp float gk_t = 10.331754;

void main() {
	lowp vec2  coord = gl_FragCoord.xy / oResolution;
	lowp vec4  o = textureLod(image, coord, imageLod);
	lowp float x = stepping;
	o += tex(coord, size * x) * 0.993846; x += stepping;
	o += tex(coord, size * x) * 0.975611; x += stepping;
	o += tex(coord, size * x) * 0.945959; x += stepping;
	o += tex(coord, size * x) * 0.905955; x += stepping;
	o += tex(coord, size * x) * 0.856997; x += stepping;
	o += tex(coord, size * x) * 0.800737; x += stepping;
	o += tex(coord, size * x) * 0.738991; x += stepping;
	o += tex(coord, size * x) * 0.673638; x += stepping;
	o += tex(coord, size * x) * 0.606531; x += stepping;
	o += tex(coord, size * x) * 0.539408; x += stepping;
	o += tex(coord, size * x) * 0.473827; x += stepping;
	o += tex(coord, size * x) * 0.411112; x += stepping;
	o += tex(coord, size * x) * 0.352322; x += stepping;
	o += tex(coord, size * x) * 0.298234; x += stepping;
	o += tex(coord, size * x) * 0.249352; x += stepping;
	o += tex(coord, size * x) * 0.205924; x += stepping;
	o += tex(coord, size * x) * 0.167973; x += stepping;
	o += tex(coord, size * x) * 0.135335; x += stepping;
	fragColor = blend(o, gk_t*2.0+1.0);
}