// Fast gaussian blur reference https://www.shadertoy.com/view/ctjcWR
// Relevant information reference:
//   https://www.shadertoy.com/view/WtKfD3
//   https://blog.ivank.net/fastest-gaussian-blur.html
//   https://elynxsdk.free.fr/ext-docs/Blur/Fast_box_blur.pdf
//   https://www.peterkovesi.com/papers/FastGaussianSmoothing.pdf

Qk_CONSTANT(
	vec2           iResolution; // viewport resolution
	vec2           oResolution; // generate image resolution
	// frag
	vec2           size; // blur size resolution %
	float          imageLod; // image image lod
	float          detail; // N target sampling rate, detail = 1.0 / ((n-1)*0.5)
);

#vert
void main() {
	gl_Position = rMat.value * vec4(vertexIn.xy * pc.oResolution / pc.iResolution, pc.depth, 1.0);
	gl_Position.y += (pc.oResolution.y / pc.iResolution.y - 1.0) * 2.0; // correct canvas offset
}

#frag
layout(binding=3) uniform sampler2D image; // image image input

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
vec4 blend(in vec4 o, in float t) {
	// Assuming straight alpha input (non-premultiplied):
	// return vec4(o.rgb / o.a, o.a / t);

	// For PMA inputs:
	return o / t;
}

vec4 tex(in vec2 coord, in vec2 d) {
	return textureLod(image, coord + d, pc.imageLod) + textureLod(image, coord - d, pc.imageLod);
}
