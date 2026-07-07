// Batch drawing color sets

#vert
struct Option {
	float m0,m1,m2,m3,m4,m5; // 2d mat2x3
	uint  flags; // flags maybe used for AA, etc
	int  _pad; // padding for std140
	vec4  color; // color
};

layout(binding=4, set=0, std140) uniform OptsBlock {
	Option opts[256];
};

layout(location=2) in int optidxIn; // options index form uniform optsBlock
layout(location=1) out flat vec4  color;
layout(location=2) out flat uint flags;

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
	flags = opt.flags;
	gl_Position = _matrix * vec4(vertexIn.xy, 0.0, 1.0);
}

#frag
// smooth
layout(location=1) in flat vec4 color;
layout(location=2) in flat uint flags;

void main() {
	fragColor = color;
	fragColor *= aaSideCoverage(flags);

	if ((flags & Qk_FLAG_CLIP) != 0)
		fragColor *= clipCoverage(vec2(0));
}
