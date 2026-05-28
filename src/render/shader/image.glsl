
#define Qk_CONSTANT_Fields

#import "_image.glsl"

#frag
void main() {
	fragColor = texture(image, coords) * pc.color;

	fragColor *= 1.0 - abs(aaSide); // premultiplied alpha

	Qk_CLIP(); // apply clip mask if needed
}
