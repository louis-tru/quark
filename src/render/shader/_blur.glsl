// Fast gaussian blur reference https://www.shadertoy.com/view/ctjcWR
// Relevant information reference:
//   https://www.shadertoy.com/view/WtKfD3
//   https://blog.ivank.net/fastest-gaussian-blur.html
//   https://elynxsdk.free.fr/ext-docs/Blur/Fast_box_blur.pdf
//   https://www.peterkovesi.com/papers/FastGaussianSmoothing.pdf
uniform lowp vec2           iResolution; // viewport resolution
uniform lowp vec2           oResolution; // generate image resolution
//frag
uniform lowp float          imageLod; // image image lod
uniform sampler2D           image; // image image input
uniform lowp vec2           size; // blur size resolution %
uniform lowp float          detail; // N target sampling rate, detail = 1.0 / ((n-1)*0.5)

#ifdef Qk_SHADER_VERT
void main() {
	gl_Position = rootMatrix * vec4(vertexIn.xy * oResolution / iResolution, depth, 1.0);
	gl_Position.y += (oResolution.y / iResolution.y - 1.0) * 2.0; // correct canvas offset
}
#else
// Gaussian blur normalization
//
// - For straight (non-premultiplied) alpha textures:
//     Transparent pixels (RGB=0, A<1) darken edges.
//     Use brightness compensation to normalize energy:
//
//         vec4(o.rgb / o.a, o.a / t)
//
// - For premultiplied alpha (PMA) textures:
//     RGB already encodes color * alpha.
//     The accumulation is energy-correct, so just:
//
//         o / t
//
// Using compensation on PMA images will double the correction
// and make edges overly bright.
//
lowp vec4 blend(in lowp vec4 o, in lowp float t) {
	// Assuming straight alpha input (non-premultiplied):
	// return vec4(o.rgb / o.a, o.a / t);

	// For PMA inputs:
	return o / t;
}
lowp vec4 tex(in lowp vec2 coord, in lowp vec2 d) {
	return textureLod(image, coord + d, imageLod) + textureLod(image, coord - d, imageLod);
}
#endif
