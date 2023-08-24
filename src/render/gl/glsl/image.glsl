#vert
#import "_image.glsl"

#frag
#define Qk_RGB_Format 0 // 1 texure
#define Qk_YUV420SP_Format 1 // 2 texure
#define Qk_YUV420P_Format 2 // 3 texure

in      lowp  vec2      coordF;
uniform lowp  float     opacity;
uniform lowp  int       format;
uniform       sampler2D image;   // y
uniform       sampler2D image_u; // 420p u or 420sp uv
uniform       sampler2D image_v; // 420p v

void main() {
	lowp float aaalpha = 1.0 - abs(aafuzz);
	if (format != Qk_RGB_Format) {
		lowp float y = texture(image, coordF).r;
		lowp vec2  uv = texture(image_u, coordF).ra;
		lowp float u = uv.x;
		lowp float v = mix(uv.y, texture(image_v, coordF).r, format - 1);

		fragColor = vec4(y + 1.4075 * (v - 0.5),
										 y - 0.3455 * (u - 0.5) - 0.7169 * (v - 0.5),
										 y + 1.779  * (u - 0.5),
										opacity * aaalpha);
	} else {
		fragColor = texture(image, coordF);
		fragColor.a = fragColor.a * opacity * aaalpha;
	}
}