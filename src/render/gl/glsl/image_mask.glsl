#vert
#import "_image.glsl"

#frag
in      lowp  vec2      coord_f;
uniform       sampler2D image;
uniform lowp  vec4      color;

void main() {
	lowp float aaalpha = 1.0 - abs(aafuzz);
	fragColor = color;
	fragColor.a = texture(image, coord_f).a * aaalpha;
}
