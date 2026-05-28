Qk_CONSTANT(
	vec4 color;
	vec4 surfaceOffset; // offset xy, surface rMat scale_inv zw: surface.zw = 1 / surfaceScale
);

#vert
void main() {
	// gl_InstanceID, gl_VertexID
	aadist = aadistIn;
	vec4 pos = vMat.value * vec4(vertexIn.xy, pc.depth, 1.0);
	pos.xy += pc.surfaceOffset.xy * pc.surfaceOffset.zw; // apply surface offset
	gl_Position = rMat.value * pos;
}

#frag
#define Qk_FLAG_AADIST_Inverted (1u << 2)

void main() {
	fragColor = pc.color;

	if ((pc.flags & Qk_FLAG_AADIST_Inverted) != 0) {
		// Inverted AA dist coverage.
		//
		// Normal AA dist:
		//   alpha = 1 - abs(aadist)
		//
		// Inverted mode:
		//   alpha = abs(aadist)
		//
		// Used by subtractive/difference clip masks to produce
		// smooth anti-aliased mask edges without directly drawing
		// solid black coverage.
		fragColor *= abs(aadist);
	} else {
		// aadist value range: 1 => 0, alpha range: 0 => 1
		fragColor *= (1.0 - abs(aadist)); // premultiplied alpha
	}

	Qk_CLIP_(clipStat.range.xy - pc.surfaceOffset.xy); // apply clip mask if needed
}
