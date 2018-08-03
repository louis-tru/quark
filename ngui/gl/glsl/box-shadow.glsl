#vert
#include "_box.glsl"

uniform vec3 shadow; // offset_x, offset_y, size
uniform vec4 shadow_color;

out vec4 f_color;
out vec2 f_tex_coord;

void main() {
	vec2 v;
	
	if ( gl_VertexID == 0) {
		v = vertex_ac.xy; f_tex_coord = vec2(0.0, 0.0);
	}
	else if ( gl_VertexID == 1) {
		v = vertex_ac.zy; f_tex_coord = vec2(1.0, 0.0);
	}
	else if ( gl_VertexID == 2) {
		v = vertex_ac.zw; f_tex_coord = vec2(1.0, 1.0);
	}
	else {
		v = vertex_ac.xw; f_tex_coord = vec2(0.0, 1.0);
	}
	
	f_color = shadow_color * vec4(1., 1., 1., opacity);
	
	gl_Position = r_matrix * v_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag
#include "_version.glsl"

in  lowp vec4 f_color;
in  lowp vec2 f_tex_coord;
out lowp vec4 FragColor;

void main() {
	FragColor = f_color;
}
