#include "_util.glsl"

in      lowp float sdf_f;
uniform lowp vec4  color;
void main() {
	fragColor = color;
}