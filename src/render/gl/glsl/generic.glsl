
layout (std140) uniform optsBlock {
	struct Options {
		vec4  color; // color
		vec4  coord; // image coord
		float zDepth; // depth
		uint  image; // sampler2D index
		int   type;  // 0=color, 1=image, 2=colorMask
	} opts[1024];
};
uniform int  optOffset;

#vert
in      int  optIdxIn; // options index form uniform optsBlock
out     float optIdx;
out     vec2  coord;
// out float aafuzz;

void main() {
	int idx = optOffset + optIdxIn;
	if (opts[idx].type != 0) {
		vec4 coord_ = opts[idx].coord;
		coord = (coord_.xy + vertexIn.xy) * coord_.zw;
	}
	aafuzz = aafuzzIn;
	optIdx = float(idx);
	gl_Position = matrix * vec4(vertexIn.xy, opts[idx].zDepth, 1.0);
}

#frag
in lowp  float optIdx;
in lowp  vec2 coord;
uniform  sampler2D images[16];

void main() {
	lowp float aaalpha = 1.0 - abs(aafuzz);
	int idx = int(optIdx);
	int type = opts[idx].type;
	if (type == 0) { // color
		fragColor = opts[idx].color;
		fragColor.a *= aaalpha;
	} else if (type == 1) { //  image
		fragColor = texture(images[opts[idx].image], coord);
		fragColor.a *= opts[idx].color.a * aaalpha;
	} else { // color mask
		fragColor = opts[idx].color;
		fragColor.a *= texture(images[opts[idx].image], coord).a * aaalpha;
	}
}