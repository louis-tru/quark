#import "_blur.glsl"
#frag
const int N = 7;
const float stepping = 0.166667;
const float gk_t = 3.149027;

void main() {
	vec2  coord = gl_FragCoord.xy / pc.oResolution;
	vec4  o = textureLod(image, coord, pc.imageLod);
	float x = stepping;
	o += tex(coord, pc.size * x) * 0.945959; x += stepping;
	o += tex(coord, pc.size * x) * 0.800737; x += stepping;
	o += tex(coord, pc.size * x) * 0.606531; x += stepping;
	o += tex(coord, pc.size * x) * 0.411112; x += stepping;
	o += tex(coord, pc.size * x) * 0.249352; x += stepping;
	o += tex(coord, pc.size * x) * 0.135335; x += stepping;
	fragColor = blend(o, gk_t*2.0+1.0);
}