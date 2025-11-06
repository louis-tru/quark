#vert
uniform vec4     color;

in      vec2     texCoordsIn;     // 8 bytes
in      vec4     lightColorIn;    //!< {GL_UNSIGNED_BYTE} 4 bytes, RGBA
in      vec4     darkColorIn;     //!< {GL_UNSIGNED_BYTE} 4 bytes, RGBA

out     vec2     texCoords;
out     vec4     light;
out     vec4     dark;

void main() {
	gl_Position = matrix * vec4(vertexIn.xy, aafuzzIn + depth, 1.0);
	// gl_Position.y *= -1.0; // Flip Y axis for GLSL
	texCoords = texCoordsIn;
	light = lightColorIn * color;
	dark = darkColorIn;
}

#frag
uniform sampler2D      image;
uniform lowp float     premultipliedAlpha;
in      lowp vec2      texCoords;
in      lowp vec4      light;
in      lowp vec4      dark;

void main() {
	lowp vec4 tex = texture(image, texCoords);

#ifdef Qk_SHADER_IF_FLAGS_DARK_COLOR
	// if not premultiplied alpha, (1.0 - tex.rgb) is dark color of tex
	// if premultiplied alpha, (tex.a - tex.rgb) is dark color of tex
	// but we always use (1.0 - tex.rgb) here, do simple processing.
	// so the dark color will be a bit different when using premultiplied alpha
	// because the dark color is usually very small, the difference is not easy to see
	fragColor = light * tex + dark * vec4(mix(1.0, tex.a, premultipliedAlpha) - tex.rgb, 0.0);
#else
	fragColor = light * tex;
#endif

#ifdef Qk_SHADER_IF_FLAGS_AACLIP
	fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#endif
}