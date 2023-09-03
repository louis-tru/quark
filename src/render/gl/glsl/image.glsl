#vert
#import "_image.glsl"

#frag
smooth in lowp vec2      coord_f;
uniform   lowp float     opacity;
uniform        sampler2D image;   // y

void main() {
	lowp float aaalpha = 1.0 - abs(aafuzz);
	fragColor = texture(image, coord_f);
	fragColor.a *= opacity * aaalpha;
}