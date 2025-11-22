// Batch drawing color sets

#ifndef Qk_OnlyEs2
struct Option {
	mediump int   flags; // reserve
	mediump float depth;
	mediump float m0,m1,m2,m3,m4,m5; // 2d mat2x3
	mediump vec4  color; // color
};

layout (std140) uniform optsBlock {
	Option opts[256];
};
#endif

#vert
in           int   optidxIn; // options index form uniform optsBlock
// flat
// smooth
out vec4  color;

#define _vmatrix mat4(\
	opt.m0, opt.m3, 0.0, 0.0, \
	opt.m1, opt.m4, 0.0, 0.0, \
	0.0,    0.0,    1.0, 0.0, \
	opt.m2, opt.m5, 0.0, 1.0)
#define _matrix (rootMatrix * _vmatrix)

void main() {
#ifndef Qk_OnlyEs2
	Option opt = opts[optidxIn];
	aafuzz = aafuzzIn;
	color = opt.color;
	gl_Position = _matrix * vec4(vertexIn.xy, opt.depth, 1.0);
#endif
}

#frag
// flat
// smooth
in lowp vec4 color;

void main() {
	fragColor = color;
	// fuzz value range: 1 => 0, alpha range: 0 => 1
	// fragColor.a *= 1.0 - abs(aafuzz);
	fragColor *= 1.0 - abs(aafuzz); // premultiplied alpha

#ifdef Qk_SHADER_IF_FLAGS_AACLIP
	// fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
	fragColor *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#endif
}
