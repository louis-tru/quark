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
lowp vec4 blend(in lowp vec4 o, in lowp float t) {
	// This is the blur normalization step.
	//
	// The mathematically "correct" version would be just `o / t`,
	// but that makes images with transparent edges (like circles or SDF text)
	// look darker — the fully transparent area (RGB=0) gets averaged in
	// and pulls down the brightness near the boundary.
	//
	// The alternative `vec4(o.rgb / o.a, o.a / t)` fixes that by
	// boosting the brightness where alpha is small, making edges look
	// more natural and preventing the dark fringe.
	//
	// Be aware: if the transparent pixels are *not* black (e.g. white+alpha=0),
	// this compensation will over-brighten them. Normally it's fine since
	// most textures have RGB≈0 in transparent regions.
	//
	// If you ever wonder later why this isn’t just `o / t`,
	// it’s because this version avoids dark edges caused by black transparency.
	// return o / t;
	return vec4(o.rgb / o.a, o.a / t); // brightness compensation
}
lowp vec4 tex(in lowp vec2 coord, in lowp vec2 d) {
	return textureLod(image, coord + d, imageLod) + textureLod(image, coord - d, imageLod);
}
#endif
