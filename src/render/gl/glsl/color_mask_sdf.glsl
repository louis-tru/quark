#vert
#import "_image_sdf.glsl"

#frag
in      lowp  vec2      coord_f;
in      lowp  float     sdf_f;
uniform lowp  float     sdf_range[2]; // -0.5 => 0.25
uniform lowp  float     opacity;
uniform       sampler2D image;
uniform lowp  vec4      color;

void main() {
	lowp float alpha = smoothstep(-0.5, 0.25, -abs(sdf_f));
	fragColor = color * vec4(1.0,1.0,1.0,texture(image, coord_f).a * alpha);
}