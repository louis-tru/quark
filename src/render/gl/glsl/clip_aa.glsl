#vert
void main() {
	aafuzz = aafuzzIn;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
uniform                lowp float aafuzzWeight;
uniform                lowp float aafuzzConst;
layout(location=1) out lowp vec4  aaclipOut; // output anti alias clip texture buffer

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
	//lowp float alpha = (1.0 - abs(aafuzz));// * (0.1) + 0.9;
	lowp float clip = texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r;

#ifdef Qk_SHADER_IF_FLAGS_AACLIP_REVOKE
	aaclipOut = vec4(clip / alpha, 1.0,1.0,1.0); // aaclip revoke
	//aaclipOut = vec4(0.0,1.0,1.0,1.0); // aaclip revoke
#else
	aaclipOut = vec4(clip * alpha, 1.0,1.0,1.0);
	//aaclipOut = vec4(alpha, 1.0,1.0,1.0);
#endif
}

// clip_aa
// clip_aa_revoke
