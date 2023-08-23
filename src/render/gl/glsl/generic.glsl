
layout (std140) uniform optsBlock {
	struct Options {
		vec4  color; // color
		vec4  coord; // image coord
		float zDepth; // depth
		int   image; // sampler2D index
		int   type;  // 0=color, 1=image, 2=colorMask
	} opts[1024];
};
uniform int  optOffset;

#vert
in         int   optIdxIn; // options index form uniform optsBlock
flat   out int   optIdx;
smooth out vec2  coord;
// out float aafuzz;

void main() {
	aafuzz = aafuzzIn;
	optIdx = optOffset + optIdxIn;
	if (opts[optIdx].type != 0) {
		vec4 coord_ = opts[optIdx].coord;
		coord = (coord_.xy + vertexIn.xy) * coord_.zw;
	}
	gl_Position = matrix * vec4(vertexIn.xy, opts[optIdx].zDepth, 1.0);
}

#frag
flat   in lowp int  optIdx;
smooth in lowp vec2 coord;
uniform   sampler2D images[16];

void main() {
	lowp float aaalpha = 1.0 - abs(aafuzz);
	lowp int type = opts[optIdx].type;
	if (type == 0) { // color
		fragColor = opts[optIdx].color;
		fragColor.a *= aaalpha;
	} else if (type == 1) { //  image
		fragColor = texture(images[opts[optIdx].image], coord);
		fragColor.a *= opts[optIdx].color.a * aaalpha;
	} else { // color mask
		fragColor = opts[optIdx].color;
		fragColor.a *= texture(images[opts[optIdx].image], coord).a * aaalpha;
	}
}