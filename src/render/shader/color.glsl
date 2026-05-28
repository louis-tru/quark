Qk_CONSTANT(
	vec4 color;
	vec4 surfaceOffset; // offset xy, surface rMat scale_inv zw: surface.zw = 1 / surfaceScale
);

#vert
void main() {
	// gl_InstanceID, gl_VertexID
	aaSide = aaSideIn;
	vec4 pos = vMat.value * vec4(vertexIn.xy, pc.depth, 1.0);
	pos.xy += pc.surfaceOffset.xy * pc.surfaceOffset.zw; // apply surface offset
	gl_Position = rMat.value * pos;
}

#frag
#define Qk_FLAG_AASIDE_Inverted (1u << 2)

void main() {
	fragColor = pc.color;

	if ((pc.flags & Qk_FLAG_AASIDE_Inverted) != 0) {
		// Inverted AA side coverage.
		//
		// Normal AA side:
		//   alpha = 1 - abs(aaSide)
		//
		// Inverted mode:
		//   alpha = abs(aaSide)
		//
		// Used by subtractive/difference clip masks to produce
		// smooth anti-aliased mask edges without directly drawing
		// solid black coverage.
		fragColor *= abs(aaSide);
	} else {
		// aaSide value range: 1 => 0, alpha range: 0 => 1
		fragColor *= (1.0 - abs(aaSide)); // premultiplied alpha
	}

	Qk_CLIP_(clipStat.range.xy - pc.surfaceOffset.xy); // apply clip mask if needed
}
