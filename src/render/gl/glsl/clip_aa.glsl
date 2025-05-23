#vert
void main() {
	aafuzz = aafuzzIn;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
uniform  lowp float aafuzzWeight;
uniform  lowp float aafuzzConst;
uniform  lowp float clearAA;

void main() {
	//
	// Limit the range to 1.0-0.9, which can precisely achieve pruning rollback (division operation). 
	// The range of 1.0-0.9 is already accurate enough for aa
	//
	// lowp float alpha = (1.0 - abs(aafuzz)) * (aafuzzWeight * 0.1) + 0.9;
	// F = (C + Fuzz) * W + C1;
	// F = (C.W + C1) + Fuzz.W
	// C'.W = (C.W + C1)
	// C'   = C + C1/W
	// F = C'.W + Fuzz.W
	// (-10 + 1) * -0.09
	lowp float alpha = (aafuzzConst + abs(aafuzz)) * aafuzzWeight;
	lowp float clip = mix(texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r, 1.0, clearAA);

#ifdef Qk_SHADER_IF_FLAGS_AACLIP_REVOKE
	fragColor = vec4(clip / alpha, 1.0,1.0,1.0); // aaclip revoke
#else
	fragColor = vec4(clip * alpha, 1.0,1.0,1.0);
#endif
}

// clip_aa
// clip_aa_revoke
