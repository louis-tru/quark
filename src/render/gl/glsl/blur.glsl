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
	gl_Position.y += (oResolution.y / iResolution.y - 1.0) * 2.0; // correct canvas offset
}

#frag
uniform lowp int            imageLod; // image image lod
uniform sampler2D           image; // image image input
uniform lowp vec2           size; // blur size resolution %
uniform lowp float          step; // N target sampling rate, step = 1.0 / ((n-1)*0.5)

// gaussian kernel function
// f(x,x') = exp(-|x-x'|^2 / 2σ^2)
#define gk(x) exp(-x*x*2.0)

void main() {
	lowp vec2 coord = gl_FragCoord.xy / oResolution;
	lowp vec4  o = textureLod(image, coord, imageLod), a, b;
	lowp float g, x = -1.0, t = 0.0;//, tc = o.a, ga, gb;
	lowp vec2  d; // offset distance

	do {
		g = gk(x);
		d = size * x;
		a = textureLod(image, coord + d, imageLod);
		b = textureLod(image, coord - d, imageLod);
		// ga = a.a * g;
		// gb = b.a * g;
		// o += vec4(a.rgb * ga + b.rgb * gb, ga+gb);
		o += (a + b) * g;
		t += g;
		// tc+= ga+gb;
		x += step;
	} while(x < 0.0);

	t = t*2.0+1.0;
	// fragColor = vec4(o.rgb/tc, o.a/(t*2.0+1.0));
	fragColor = vec4(o.rgb / o.a, o.a / t);
}
