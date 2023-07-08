#include "_util.glsl"

in      lowp  vec2      coord_f;
uniform lowp  float     opacity;
uniform       sampler2D image;
uniform       sampler2D image_uv;
void main() {
	lowp float y = texture(image, coord_f).r;
	lowp float u = texture(image_uv, coord_f).r;
	lowp float v = texture(image_uv, coord_f).a;
	fragColor = vec4( y + 1.4075 * (v - 0.5),
									y - 0.3455 * (u - 0.5) - 0.7169 * (v - 0.5),
									y + 1.779  * (u - 0.5),
									opacity);
}