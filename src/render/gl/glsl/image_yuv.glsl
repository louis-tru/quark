#vert
#import "_image.glsl"

#frag
smooth in lowp vec2      coord_f;
uniform   lowp float     opacity;
uniform        sampler2D image;   // y
uniform        sampler2D image_u; // 420p u or 420sp uv
uniform        sampler2D image_v; // 420p v
uniform   lowp int       format; // 0: YUV420SP, 1: YUV420P

void main() {
	lowp float aaalpha = 1.0 - abs(aafuzz);
	lowp float y = texture(image, coord_f).r;
	lowp vec2  uv = texture(image_u, coord_f).ra;
	lowp float u = uv.x;
	lowp float v = mix(uv.y, texture(image_v, coord_f).r, format);

	fragColor = vec4(	y + 1.4075 * (v - 0.5),
										y - 0.3455 * (u - 0.5) - 0.7169 * (v - 0.5),
										y + 1.779  * (u - 0.5),
										opacity * aaalpha);
}