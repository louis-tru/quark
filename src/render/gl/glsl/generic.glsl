
layout (std140) uniform optsBlock {
	struct Option {
		int flags; // type: flags & 0xf + image: flags >> 4 sampler2D index
		float depth;
		float m0,m1,m2,m3,m4,m5; // 2d mat2x3
		vec4  color; // color
		vec4  coord; // image coord
	} opts[1024];
};

#vert
in         int   optidxIn; // options index form uniform optsBlock
flat   out int   type;
flat   out int   optidx;
flat   out int   image;
smooth out vec2  coord;

#define _vmatrix mat4(opt.m0, opt.m3, 0.0, 0.0, \
	opt.m1, opt.m4, 0.0, 0.0,   0.0, 0.0, 1.0, 0.0,   opt.m2, opt.m5, 0.0, 1.0)
#define _matrix (rootMatrix * _vmatrix)

void main() {
	Option opt = opts[optidxIn];
	aafuzz = aafuzzIn;
	optidx = optidxIn;
	type = opt.flags & 3;
	if (type != 0) {
		image = opt.flags >> 2;
		coord = (opt.coord.xy + vertexIn.xy) * opt.coord.zw;
	}
	gl_Position = _matrix * vec4(vertexIn.xy, opt.depth, 1.0);
}

#frag
flat   in lowp int  type;
flat   in lowp int  optidx;
flat   in lowp int  image; // sampler2D index
smooth in lowp vec2 coord;
uniform   sampler2D images[Qk_GL_MAX_TEXTURE_IMAGE_UNITS];

void main() {
	lowp float aaalpha = 1.0 - abs(aafuzz);
	if (type == 0) { // color
		fragColor = opts[optidx].color;
		fragColor.a *= aaalpha;
	} else if (type == 1) { //  color mask
		fragColor = opts[optidx].color;
		fragColor.a *= texture(images[image], coord).a * aaalpha;
	} else { // image
		fragColor = texture(images[image], coord);
		fragColor.a *= opts[optidx].color.a * aaalpha;
	}
}