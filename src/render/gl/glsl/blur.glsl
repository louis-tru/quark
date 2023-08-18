#vert
// The solution is based on Wojciech Jarosz's Fast Image Convolution
// https://blog.ivank.net/fastest-gaussian-blur.html
// https://elynxsdk.free.fr/ext-docs/Blur/Fast_box_blur.pdf
// https://www.peterkovesi.com/papers/FastGaussianSmoothing.pdf
// https://www.shadertoy.com/view/WtKfD3

void main() {
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
uniform lowp vec4 color;
void main() {
	fragColor = color;
}