#vert
#include "_util.glsl"

uniform vec4  tex_start_size;
uniform vec2  tex_ratio;

#define tex_ss tex_start_size

out float f_opacity;
out vec2  f_tex_coord;

#define tex_a_x tex_ss.x / tex_ss.z / tex_ratio.x
#define tex_a_y tex_ss.y / tex_ss.w / tex_ratio.y
#define tex_c_x (vertex_ac.z - vertex_ac.x + tex_ss.x) / tex_ss.z / tex_ratio.x
#define tex_c_y (vertex_ac.w - vertex_ac.y + tex_ss.y) / tex_ss.w / tex_ratio.y

void main() {
	vec2 v;
	
	if (gl_VertexID == 0) {
		v = vertex_ac.xy; f_tex_coord = vec2(tex_a_x, tex_a_y);
	}
	else if (gl_VertexID == 1) {
		v = vertex_ac.zy; f_tex_coord = vec2(tex_c_x, tex_a_y);
	}
	else if (gl_VertexID == 2) {
		v = vertex_ac.zw; f_tex_coord = vec2(tex_c_x, tex_c_y);
	}
	else {
		v = vertex_ac.xw; f_tex_coord = vec2(tex_a_x, tex_c_y);
	}

	f_opacity = opacity;
	
	gl_Position = r_matrix * v_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag
#include "_version.glsl"

uniform sampler2D s_tex0;

in  lowp float f_opacity;
in  lowp vec2  f_tex_coord;
out lowp vec4  FragColor;

void main() {
	// discard;
	FragColor = texture(s_tex0, f_tex_coord) * vec4(1.0, 1.0, 1.0, f_opacity);
}
