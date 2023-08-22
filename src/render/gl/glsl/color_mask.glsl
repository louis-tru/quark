#vert
#import "_image.glsl"

#frag
in      lowp  vec2      coord_f;
uniform lowp  float     opacity;
uniform       sampler2D image;
uniform lowp  vec4      color;

void main() {
	fragColor = vec4(color.rgb, color.a * texture(image, coord_f).a);
}
