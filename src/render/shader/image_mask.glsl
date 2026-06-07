
#define Qk_CONSTANT_Fields \
	int  alphaIndex;

#import "_image.glsl"

#frag
void main() {
	fragColor = pc.color;

	float alpha = texture(image, coords)[pc.alphaIndex];
	// vec2 dx = dFdx(coords) * 0.5;
	// vec2 dy = dFdy(coords) * 0.5;
	// float alphaL = texture(image, coords - dx)[pc.alphaIndex];
	// float alphaR = texture(image, coords + dx)[pc.alphaIndex];
	// float alphaT = texture(image, coords - dy)[pc.alphaIndex];
	// float alphaB = texture(image, coords + dy)[pc.alphaIndex];
	// float filtered = (alpha * 4.0 + alphaL + alphaR + alphaT + alphaB) * 0.125;
	// float edge = max(max(abs(alphaL - alpha), abs(alphaR - alpha)),
	// 		max(abs(alphaT - alpha), abs(alphaB - alpha)));
	// alpha = mix(alpha, filtered, smoothstep(0.15, 0.65, edge) * 0.5);

	fragColor *= alpha;

	Qk_aaSideCoverage();
	Qk_CLIP(); // apply clip mask if needed
}
