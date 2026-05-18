// vportCp was originally written for GL path where viewport may stay as canvas size.
// In Metal path we set viewport to dst texture size and pass iResolution == oResolution,
// so the shader's viewport compensation becomes identity:
//
//   vertexIn * oResolution / iResolution == vertexIn
//   y offset correction == 0
//
// This lets us reuse the same shader without changing the GLSL/MSL source.

Qk_CONSTANT(
	vec2 iResolution; // viewport resolution
	vec2 oResolution; // output image resolution, the <= iResolution
	// frag
	vec4 coord; // vec4(texture offset coord, scale coefficient)
	float imageLod; // input image lod level
);

#vert
layout(location=1) out vec2 coord;

void main() {
	vec2 scale = pc.oResolution / pc.iResolution;
	gl_Position = rMat.value * vec4(vertexIn.xy * scale, pc.depth, 1.0);
}

#frag
layout(location=1) in vec2 coord;
layout(binding=4) uniform sampler2D image; // input image

void main() {
	vec2 coord = gl_FragCoord.xy / pc.oResolution; // coord uv
	coord = coord * pc.coord.zw + pc.coord.xy; // apply offset and scale
	fragColor = textureLod(image, coord, pc.imageLod);
}
