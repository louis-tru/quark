
layout (std140) uniform optsBlock {
	struct Option {
		// flags:
		// type = flags & 0xf
		// image = flags >> 2 & 0xff, image sampler2D index
		// optidx = flags >> 8
		int   flags;
		float depth;
		float m0,m1,m2,m3,m4,m5; // 2d mat2x3
		vec4  color; // color
		vec4  coord; // image coord
	} opts[1024];
};

#vert
in           int   optidxIn; // options index form uniform optsBlock
smooth   out float flags;
smooth   out vec2  coord;

#define _vmatrix mat4(\
	opt.m0, opt.m3, 0.0, 0.0, \
	opt.m1, opt.m4, 0.0, 0.0,   \
	0.0,    0.0,    1.0, 0.0,   \
	opt.m2, opt.m5, depth, 1.0)
#define _matrix (rootMatrix * _vmatrix)

void main() {
	Option opt = opts[optidxIn];
	aafuzz = aafuzzIn;
	flags = float(opt.flags);
	if ((opt.flags & 3) != 0) {
		coord = (opt.coord.xy + vertexIn.xy) * opt.coord.zw;
	}
	gl_Position = _matrix * vec4(vertexIn.xy, opt.depth, 1.0);
}

#frag
// flat   in lowp float  flags;
smooth in lowp float flags;
smooth in lowp vec2 coord;
uniform   sampler2D images[Qk_GL_MAX_TEXTURE_IMAGE_UNITS];

void main() {
	lowp float aaalpha = 1.0 - abs(aafuzz);
	lowp int _flags = int(flags);
	if ((_flags & 3) == 0) { // color
		fragColor = opts[_flags >> 8].color;
	} else { //  color mask or image
		fragColor = opts[_flags >> 8].color * texture(images[(_flags >> 2) & 0xff], coord);
	}
	fragColor.a = aaalpha;
}