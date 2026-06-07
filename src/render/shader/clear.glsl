Qk_CONSTANT(
	vec4 color;
);

#vert
void main() {
	gl_Position = rMat.value * vec4(vertexIn.xy, 0.0, 1.0);
}

#frag
void main() {
	fragColor = pc.color;
}
