
Qk_CONSTANT(
	// frag
	float aafuzzWeight;
	float aafuzzConst;
);

#vert
void main() {
	aafuzz = aafuzzIn;
	gl_Position = matrix * vec4(vertexIn.xy, pc.depth, 1.0);
}

#frag

#define Qk_FLAG_AACLIP_REVOKE (1u << 1)

void main() {
	//
	// Limit the range to 1.0-0.9, which can precisely achieve pruning rollback (division operation). 
	// The range of 1.0-0.9 is already accurate enough for aa
	//
	// lowp float alpha = (1.0 - abs(aafuzz)) * (pc.aafuzzWeight * 0.1) + 0.9;
	// F = (C + Fuzz) * W + C1;
	// F = (C.W + C1) + Fuzz.W
	// C'.W = (C.W + C1)
	// C'   = C + C1/W
	// F = C'.W + Fuzz.W
	// (-10 + 1) * -0.09
	float src = (pc.aafuzzConst + abs(aafuzz)) * pc.aafuzzWeight;

// #ifdef Qk_SHADER_IF_FLAGS_AACLIP_REVOKE
	if ((pc.flags & Qk_FLAG_AACLIP_REVOKE) != 0) {
		// aaclip revoke = dst/src, because src don't > 1.0 so use 1/src-1
		// modify gl blend mode to: src * dst + dst.
		// dst/src = dst(1/src)
		//				 = dst(1/src-1) + dst
		fragColor = vec4((1.0-src)/src, 1.0, 1.0, 1.0);
	} else {
// #else
		// aaclip = dst*src
		// modify gl blend mode to: src * dst
		fragColor = vec4(src, 1.0, 1.0, 1.0);
	}
// #endif
}

// clip_aa
// clip_aa_revoke
