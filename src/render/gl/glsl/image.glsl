#vert
#import "_image.glsl"

#frag
in      lowp  vec2      coordF;
uniform lowp  float     opacity;
uniform       sampler2D image;

void main() {
	lowp float aaalpha = 1.0 - abs(aafuzz);
	fragColor = texture(image, coordF);
	fragColor.a = fragColor.a * opacity * aaalpha;
}
