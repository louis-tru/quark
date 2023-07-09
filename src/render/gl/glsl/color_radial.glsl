#vert
out       vec2      position_f;
void main() {
	position_f = vertex_in.xy;
	gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);
}

#frag
uniform lowp vec4   range;/*center/radius for circle*/
uniform      int    count;
uniform lowp vec4   colors[256];/*max 256 color points*/
uniform lowp float  positions[256];
in      lowp vec2   position_f;

void main() {
	lowp float indexed_f = length((position_f-range.xy)/range.zw);
	int s = 0;
	int e = count-1;
	while (s+1 < e) {/*dichotomy search color value*/
		int idx = (e - s) / 2 + s;
		if (indexed_f > positions[idx]) {
			s = idx;
		} else if (indexed_f < positions[idx]) {
			e = idx;
		} else { 
			s = idx; e = idx+1; break;
		}
	}
	lowp float w = (indexed_f - positions[s]) / (positions[e] - positions[s]);
	fragColor = mix(colors[s], colors[e], w);
}