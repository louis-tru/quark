#vert
#import "_image.glsl"

#frag
in      lowp  vec2      coordF;
uniform       sampler2D image;
uniform lowp  vec4      color;

void main() {
	lowp float aaalpha = 1.0 - abs(aafuzz);
	fragColor = vec4(color.rgb, color.a * aaalpha * texture(image, coordF).a);
}
