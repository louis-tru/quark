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
uniform lowp float          step; // N target sampling rate, step = 1.0 / ((n-1)*0.5)

// gaussian kernel function
// f(x,x') = exp(-|x-x'|^2 / 2σ^2)
#define gk(x) exp(-x*x*2.0)

void main() {
	lowp vec2 coord = gl_FragCoord.xy;
	coord.y -= (iResolution.y - oResolution.y); // correct offset
	coord /= oResolution; // image texture coord

	lowp vec4  o = vec4(0);
	lowp float x = -1.0, t = 0.0, g;

	do {
		g = gk(x);
		o += g * textureLod(image, blurSize * x + coord, imageLod);
		t += g;
		x += step;
	} while(x <= 1.0);

	fragColor = o / t;
}