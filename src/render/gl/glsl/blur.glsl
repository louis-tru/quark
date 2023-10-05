// Fast gaussian blur reference https://www.shadertoy.com/view/ctjcWR
// Relevant information reference:
//   https://www.shadertoy.com/view/WtKfD3
//   https://blog.ivank.net/fastest-gaussian-blur.html
//   https://elynxsdk.free.fr/ext-docs/Blur/Fast_box_blur.pdf
//   https://www.peterkovesi.com/papers/FastGaussianSmoothing.pdf
uniform vec2                iResolution; // viewport resolution
uniform vec2                oResolution; // generate image resolution
#vert
void main() {
	gl_Position = rootMatrix * vec4(vertexIn.xy * oResolution / iResolution, depth, 1.0);
	gl_Position.y += (iResolution.y - oResolution.y); // correct canvas offset
}

#frag
uniform lowp uint           imageLod; // image image lod
uniform sampler2D           image; // image image input
uniform lowp vec2           blurSize; // blur size resolution %
uniform lowp vec2           dir; // blur direction, horizontal or vertical
uniform lowp int            n; // target sampling rate

// gaussian kernel function
// f(x,x') = exp(-|x-x'|^2 / 2σ^2)
#define gk(x) exp(-x*x*2.0)

void main() {
	lowp vec2 coord = gl_FragCoord.xy;
	coord.y -= (iResolution.y - oResolution.y); // correct offset
	coord /= oResolution; // image texture coord

	lowp vec4 O = vec4(0);
	lowp float x, g, r = float(n-1)*0.5, t=0.0;

	for (int k = 0; k < n; k++ ) {
		x = float(k)/r-1.0; // range: -1 -> 1
		g = gk(x);
		O += g * textureLod(image, blurSize * x + coord, imageLod);
		t += g;
	}

	fragColor = O / t;
}