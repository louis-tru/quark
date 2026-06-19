Qk_CONSTANT(
	vec4  texCoords; /*offset,scale*/
	vec4  color; /* color */
	int   format; /* 0: YUV420SP, 1: YUV420P*/
);

#vert
layout(location=1) out vec2 coords; // texture coordinates uv for fragment shader
void main() {
	aaSide = aaSideIn;
	gl_Position = matrix * vec4(vertexIn.xy, 0.0, 1.0);
	coords = (pc.texCoords.xy + vertexIn.xy) / pc.texCoords.zw; // coord uv
}

#frag
layout(location=1) in vec2 coords;
layout(binding=1,set=1) uniform sampler2D image;   // y of yuv
layout(binding=2,set=1) uniform sampler2D image_uv; // 420p u or 420sp uv
layout(binding=3,set=1) uniform sampler2D image_v; // 420p v

void main() {
	float y = texture(image, coords).r;
	vec2  uv = texture(image_uv, coords).rg; // GL_RG
	float u = uv.r;
	float v = mix(uv.g, texture(image_v, coords).r, float(pc.format));

	fragColor = vec4(	y + 1.4075 * (v - 0.5),
										y - 0.3455 * (u - 0.5) - 0.7169 * (v - 0.5),
										y + 1.779  * (u - 0.5), 1.0) * pc.color;

	Qk_aaSideCoverage();
	Qk_CLIP();
}