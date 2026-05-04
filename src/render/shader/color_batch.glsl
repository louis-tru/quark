// Batch drawing color sets

struct Option {
	int   flags; // reserve
	float depth;
	float m0,m1,m2,m3,m4,m5; // 2d mat2x3
	vec4  color; // color
};

layout(binding=3) uniform OptsBlock {
	Option opts[256];
};

#vert
layout(location=2) in int optidxIn; // options index form uniform optsBlock
// flat
// smooth
layout(location=1) out vec4  color;

#define _vmatrix mat4(\
	opt.m0, opt.m3, 0.0, 0.0, \
	opt.m1, opt.m4, 0.0, 0.0, \
	0.0,    0.0,    1.0, 0.0, \
	opt.m2, opt.m5, 0.0, 1.0)
#define _matrix (rMat.value * _vmatrix)

void main() {
	Option opt = opts[optidxIn];
	aafuzz = aafuzzIn;
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
	// fuzz value range: 1 => 0, alpha range: 0 => 1
	// fragColor.a *= 1.0 - abs(aafuzz);
	fragColor *= 1.0 - abs(aafuzz); // premultiplied alpha

// #ifdef Qk_SHADER_IF_FLAGS_AACLIP
	Qk_IF_AACLIP {
		// fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
		fragColor *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
	}
// #endif
}
