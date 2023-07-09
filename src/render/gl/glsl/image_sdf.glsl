#vert
#import "_image_sdf.glsl"

#frag
in      lowp  vec2      coord_f;
in      lowp  float     sdf_f;
uniform lowp  float     opacity;
uniform       sampler2D image;

void main() {
	lowp float alpha = smoothstep(-0.5, 0.25, -abs(sdf_f));
	fragColor = texture(image, coord_f) * vec4(1.0, 1.0, 1.0, opacity * alpha);
}