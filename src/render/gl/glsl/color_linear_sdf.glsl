#vert
uniform   vec4      range;/*start/end range for rect*/
in        float     sdf_in; // signed distance field
out       float     sdf_f;
out       float     indexed_f;
void main() {
	sdf_f = sdf_in;
	vec2 ao = range.zw     - range.xy;
	vec2 bo = vertex_in.xy - range.xy;
	/*indexed_f = clamp(dot(ao,bo) / dot(ao,ao), 0.0, 1.0);*/
	indexed_f = dot(ao,bo) / dot(ao,ao);
	gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);
}

#frag
in      lowp float     sdf_f;
in      lowp float     indexed_f;
uniform lowp float     sdf_range[2]; // -0.5 => 0.25
uniform      int       count;
uniform lowp vec4      colors[256];/*max 256 color points*/
uniform lowp float     positions[256];
void main() {
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
	lowp vec4  color = mix(colors[s], colors[e], w);
	lowp float alpha = smoothstep(-0.5, 0.25, -abs(sdf_f));
	fragColor = vec4(color.rgb, color.a * alpha);
}