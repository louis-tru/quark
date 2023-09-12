
layout (std140) uniform optsBlock {
	struct Option {
		int   flags; // reserve
		float depth;
		float m0,m1,m2,m3,m4,m5; // 2d mat2x3
		vec4  color; // color
	} opts[1024];
};

#vert
in           int   optidxIn; // options index form uniform optsBlock
smooth   out vec4  color;

#define _vmatrix mat4(\
	opt.m0, opt.m3, 0.0, 0.0, \
	opt.m1, opt.m4, 0.0, 0.0, \
	0.0,    0.0,    1.0, 0.0, \
	opt.m2, opt.m5, 0.0, 1.0)
#define _matrix (rootMatrix * _vmatrix)

void main() {
	Option opt = opts[optidxIn];
	aafuzz = aafuzzIn;
	color = opt.color;
	gl_Position = _matrix * vec4(vertexIn.xy, opt.depth, 1.0);
}

#frag
smooth in lowp vec4 color;

void main() {
	// fuzz value range: 1 => 0, alpha range: 0 => 1
	lowp float aaalpha = 1.0 - abs(aafuzz);
	fragColor = color;

#ifdef Qk_SHAFER_AACLIP
	fragColor.a *= aaalpha * smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#else
	fragColor.a *= aaalpha;
#endif
}