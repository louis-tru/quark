
#define Qk_CONSTANT_Fields \
	int  alphaIndex;

#import "_image.glsl"

#frag
void main() {
	fragColor = pc.color;

	float alpha = texture(image, coords)[pc.alphaIndex];

	fragColor *= alpha * (1.0 - abs(aafuzz)); // premultiplied alpha

	Qk_CLIP(); // apply clip mask if needed
}
