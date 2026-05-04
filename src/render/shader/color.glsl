Qk_CONSTANT(
	vec4 color;
);

#vert
void main() {
	// gl_InstanceID, gl_VertexID
	aafuzz = aafuzzIn;
	gl_Position = matrix * vec4(vertexIn.xy, pc.depth, 1.0);
}

#frag

#define Qk_FLAG_AAFUZZ (1u << 1)

void main() {
	fragColor = pc.color;

	if ((pc.flags & Qk_FLAG_AAFUZZ) != 0) {
		// fuzz value range: 1 => 0, alpha range: 0 => 1
		// fragColor.a *= (1.0 - abs(aafuzz));
		fragColor *= (1.0 - abs(aafuzz)); // premultiplied alpha
	} else {
		// only use aaclip in not aafuzz macros, if both aafuzz and aaclip macros enabled then
		// aafuzz will fail, maybe this is not get aaclip value correctly in some GPU,
		// for example macOS, it's very strange that may this a BUG.
		// so only use a clip when not aafuzz, because aafuzz is more less dependent on a aaclip.
		Qk_IF_AACLIP {
			// fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
			fragColor *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
		}
	}
}
