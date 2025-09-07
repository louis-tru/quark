#vert
in  vec2     texCoordsIn;     // 8 bytes
in  vec4     colorIn;         //!< {GL_UNSIGNED_BYTE} 4 bytes
in  vec4     color2In;        //!< {GL_UNSIGNED_BYTE} 4 bytes

out vec2     texCoords;
out vec4     color;
out vec4     color2;

void main() {
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
	// aafuzz = aafuzzIn;
	texCoords = texCoordsIn;
	color = colorIn;
	color2 = color2In;
}

#frag
uniform   sampler2D      image;
in        lowp vec2      texCoords;
in        lowp vec4      color;
in        lowp vec4      color2;

void main() {
	// Spine 的 two color tint，一般是颜色 * tex 的混合
	// 这里可以用 vColor 作为乘色，用 vColor2 作为加色 (Spine 的标准做法)
	fragColor = texture(image, texCoords) * color + color2;

#ifdef Qk_SHADER_IF_FLAGS_AACLIP
	fragColor.a *= smoothstep(0.9, 1.0, texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).r);
#endif
}