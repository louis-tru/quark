#import "_blur.glsl"
#frag
const lowp int N = 13;
const lowp float stepping = 0.083333;
const lowp float gk_t = 6.741638;

void main() {
	lowp vec2  coord = gl_FragCoord.xy / oResolution;
	lowp vec4  o = textureLod(image, coord, imageLod);
	lowp float x = stepping;
	o += tex(coord, size * x) * 0.986207; x += stepping;
	o += tex(coord, size * x) * 0.945959; x += stepping;
	o += tex(coord, size * x) * 0.882497; x += stepping;
	o += tex(coord, size * x) * 0.800737; x += stepping;
	o += tex(coord, size * x) * 0.706648; x += stepping;
	o += tex(coord, size * x) * 0.606531; x += stepping;
	o += tex(coord, size * x) * 0.506336; x += stepping;
	o += tex(coord, size * x) * 0.411112; x += stepping;
	o += tex(coord, size * x) * 0.324652; x += stepping;
	o += tex(coord, size * x) * 0.249352; x += stepping;
	o += tex(coord, size * x) * 0.186270; x += stepping;
	o += tex(coord, size * x) * 0.135335; x += stepping;
	fragColor = blend(o, gk_t*2.0+1.0);
}