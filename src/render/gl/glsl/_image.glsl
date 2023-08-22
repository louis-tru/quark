
layout (std140) uniform imgOptsBlock {
	struct {
		vec4  coord;
		float opacity;
		uint  image; // sampler2D index
	} opts[2048];
};

uniform   vec4      coord;/*offset,scale*/
out       vec2      coord_f;
void main() {
	coord_f = (coord.xy + vertexIn.xy) * coord.zw;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}