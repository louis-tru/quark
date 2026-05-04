#import "_blur.glsl"
#frag
const int N = 3;
const float stepping = 0.500000;
const float gk_t = 0.741866;

void main() {
	vec2  coord = gl_FragCoord.xy / pc.oResolution;
	vec4  o = textureLod(image, coord, pc.imageLod);
	float x = stepping;
	o += tex(coord, pc.size * x) * 0.606531; x += stepping;
	o += tex(coord, pc.size * x) * 0.135335; x += stepping;
	fragColor = blend(o, gk_t*2.0+1.0);
}