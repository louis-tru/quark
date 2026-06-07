
#define Qk_CONSTANT_Fields

#import "_image.glsl"

#frag
void main() {
	fragColor = texture(image, coords) * pc.color;

	Qk_aaSideCoverage();
	Qk_CLIP(); // apply clip mask if needed
}
