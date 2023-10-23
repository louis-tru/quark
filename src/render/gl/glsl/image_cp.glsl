uniform vec2                iResolution; // viewport resolution
uniform vec2                oResolution; // output image resolution of fragColor
#vert
void main() {
	gl_Position = rootMatrix * vec4(vertexIn.xy * oResolution / iResolution, depth, 1.0);
	gl_Position.y += (iResolution.y - oResolution.y); // correct canvas offset
}

#frag
uniform lowp uint           imageLod; // input image lod level
uniform sampler2D           image; // input image

void main() {
	lowp vec2 coord = gl_FragCoord.xy;
	coord.y -= (iResolution.y - oResolution.y); // correct offset
	fragColor = textureLod(image, coord / oResolution, imageLod);
}