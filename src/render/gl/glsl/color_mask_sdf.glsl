#vert
#import "_image_sdf.glsl"

#frag
in      lowp  vec2      coord_f;
in      lowp  float     sdf_f;
uniform lowp  float     opacity;
uniform       sampler2D image;
uniform lowp  vec4      color;
uniform lowp  float     sdf_range[3]; // range -0.5 => 0, sdf increase

void main() {
	lowp float alpha = smoothstep(sdf_range[0], sdf_range[1], abs(sdf_range[2] + sdf_f));
	fragColor = color * vec4(1.0,1.0,1.0,texture(image, coord_f).a * alpha);
}