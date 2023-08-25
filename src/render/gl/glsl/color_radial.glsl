#vert
out       vec2      position;
void main() {
	aafuzz = aafuzzIn;
	position = vertexIn.xy;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
uniform lowp vec4   range;/*center/radius for circle*/
uniform lowp int    count;
uniform lowp float  opacity;
uniform lowp vec4   colors[256];/*max 256 color points*/
uniform lowp float  positions[256];
in      lowp vec2   position;

void main() {
	lowp float indexed = length((position-range.xy)/range.zw);
	lowp int s = 0;
	lowp int e = count-1;
	while (s+1 < e) {/*dichotomy search color value*/
		lowp int idx = (e - s) / 2 + s;
		if (indexed > positions[idx]) {
			s = idx;
		} else if (indexed < positions[idx]) {
			e = idx;
		} else { 
			s = idx; e = idx+1; break;
		}
	}
	lowp float w = (indexed - positions[s]) / (positions[e] - positions[s]);
	lowp vec4  color = mix(colors[s], colors[e], w);
	lowp float aaalpha = 1.0 - abs(aafuzz);
	fragColor = vec4(color.rgb, color.a * opacity * aaalpha);
}