// Batch drawing color sets

struct Option {
	int   flags; // reserve
	float depth;
	float m0,m1,m2,m3,m4,m5; // 2d mat2x3
	vec4  color; // color
};

layout(binding=4, set=0, std140) uniform OptsBlock {
	Option opts[256];
};

#vert
layout(location=2) in int optidxIn; // options index form uniform optsBlock
layout(location=1) out vec4  color;

#define _vmatrix mat4(\
	opt.m0, opt.m3, 0.0, 0.0, \
	opt.m1, opt.m4, 0.0, 0.0, \
	0.0,    0.0,    1.0, 0.0, \
	opt.m2, opt.m5, 0.0, 1.0)
#define _matrix (rMat.value * _vmatrix)

void main() {
	Option opt = opts[optidxIn];
	aaSide = aaSideIn;
	color = opt.color;
	gl_Position = _matrix * vec4(vertexIn.xy, opt.depth, 1.0);
}

#frag
// flat
// smooth
layout(location=1) in vec4 color;

Qk_CONSTANT(
	int _;
);

void main() {
	fragColor = color;
	fragColor *= aaSideCoverage();

	Qk_CLIP(); // apply clip mask if needed
}
