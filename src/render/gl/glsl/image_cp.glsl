uniform vec2                iResolution; // viewport resolution
uniform vec2                oResolution; // output image resolution of fragColor
#vert
void main() {
	gl_Position = rootMatrix * vec4(vertexIn.xy * oResolution / iResolution, depth, 1.0);
	gl_Position.y += (oResolution.y / iResolution.y - 1.0) * 2.0; // correct canvas offset
}

#frag
uniform lowp int            imageLod; // input image lod level
uniform sampler2D           image; // input image

void main() {
	fragColor = textureLod(image, gl_FragCoord.xy / oResolution, imageLod);
}