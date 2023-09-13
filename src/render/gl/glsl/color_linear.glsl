#vert
uniform   vec4      range;/*origin/end range for rect*/
out       float     indexed;

void main() {
	vec2 ao = range.zw    - range.xy;
	vec2 bo = vertexIn.xy - range.xy;
	/*indexed = clamp(dot(ao,bo) / dot(ao,ao), 0.0, 1.0);*/
	indexed = dot(ao,bo) / dot(ao,ao);
	aafuzz = aafuzzIn;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
in      lowp float     indexed;
uniform lowp int       count;
uniform lowp float     alpha;
uniform lowp vec4      colors[256];/*max 256 color points*/
uniform lowp float     positions[256];

void main() {
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
	fragColor = mix(colors[s], colors[e], w);

#ifdef Qk_SHAFER_IF_FLAGS_AAFUZZ
	fragColor.a *= alpha * (1.0 - abs(aafuzz));
#else
	fragColor.a *= alpha;
#endif

#ifdef Qk_SHAFER_IF_FLAGS_AACLIP
	fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#endif
}