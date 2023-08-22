#vert
layout (std140) uniform colorOptsBlock {
	vec4 color1[4096]; // 64kb
};

const int type = 0;

void main() {
	if (type == 1) { // image or color mask
		// coord_f = (coord.xy + vertexIn.xy) * coord.zw;
	}
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
uniform lowp vec4 color;

void main() {
	fragColor = color;
}