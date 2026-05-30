Qk_CONSTANT(
	vec4 color;
);

#vert
void main() {
	aaSide = aaSideIn;
	gl_Position = matrix * vec4(vertexIn.xy, 0.0, 1.0);
}

#frag
void main() {
	fragColor = pc.color;
	fragColor *= aaSideCoverage();
	Qk_CLIP(); // apply clip mask if needed
}
