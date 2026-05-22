Qk_CONSTANT(
	vec4 color;
);

#vert
// in
layout(location=2) in  vec2 texCoordsIn;     //!< 8 bytes
layout(location=3) in  vec4 lightColorIn;    //!< {GL_UNSIGNED_BYTE} 4 bytes, RGBA
layout(location=4) in  vec4 darkColorIn;     //!< {GL_UNSIGNED_BYTE} 4 bytes, RGBA
// out
layout(location=1) out vec2 texCoords;
layout(location=2) out vec4 light;
layout(location=3) out vec4 dark;

void main() {
	gl_Position = matrix * vec4(vertexIn.xy, aafuzzIn + pc.depth, 1.0);
	// gl_Position.y *= -1.0; // Flip Y axis for GLSL
	texCoords = texCoordsIn;
	light = lightColorIn * pc.color;
	dark = darkColorIn;
	light.rgb *= light.a; // apply premultiplied alpha
	dark.rgb *= dark.a;
}

#frag
layout(binding=1,set=1) uniform sampler2D image;
layout(location=1) in vec2 texCoords;
layout(location=2) in vec4 light;
layout(location=3) in vec4 dark;

#define Qk_FLAGS_DARK_COLOR (1 << 1)

void main() {
	vec4 tex = texture(image, texCoords);

	if ((pc.flags & Qk_FLAGS_DARK_COLOR) != 0) {
		// if not premultiplied alpha, (1.0 - tex.rgb) is dark color of tex
		// if premultiplied alpha, (tex.a - tex.rgb) is dark color of tex
		// but we always use (1.0 - tex.rgb) here, do simple processing.
		// so the dark color will be a bit different when using premultiplied alpha
		// because the dark color is usually very small, the difference is not easy to see
		////
		// fragColor = light * tex + dark * vec4(mix(1.0, tex.a, premultipliedAlpha) - tex.rgb, 0.0);
		// always apply premultiplied alpha for dark color
		fragColor = light * tex + dark * vec4(tex.a - tex.rgb, 0.0);
	} else {
		fragColor = light * tex;
	}

	Qk_CLIP(); // apply clip mask if needed
}