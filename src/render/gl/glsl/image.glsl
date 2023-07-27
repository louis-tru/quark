#vert
#import "_image.glsl"

#frag
in      lowp  vec2      coord_f;
uniform lowp  float     opacity;
uniform       sampler2D image;

void main() {
	fragColor = texture(image, coord_f) * vec4(1.0, 1.0, 1.0, opacity);
	//fragColor = vec4(coord_f.x, coord_f.y, 1, opacity);
}
