#vert
#include "_box-radius.glsl"

uniform vec4 tex_coord; // vec4(coord_start.xy, coord_scale.xy)
out float f_opacity;
out vec2  f_tex_coord;

void main() {
	vec2 v;
	if (is_radius) { // 使用84个顶点绘制一个圆角矩形
		v = vertex(); // 获取顶点
		f_tex_coord =                                             // coord coord_start -> coord_end
			vec2(v.x - vertex_ac.x, v.y - vertex_ac.y) /            // vertex min -> max
			vec2(vertex_ac.z - vertex_ac.x, vertex_ac.w - vertex_ac.y) *  // rect size
			tex_coord.zw + tex_coord.xy;
	} else {
		if ( gl_VertexID == 0) {
			v = vertex_ac.xy;
			f_tex_coord = tex_coord.xy;
		} else if ( gl_VertexID == 1) {
			v = vertex_ac.zy;
			f_tex_coord = vec2(tex_coord.x + tex_coord.z, tex_coord.y);
		} else if ( gl_VertexID == 2) {
			v = vertex_ac.zw;
			f_tex_coord = tex_coord.xy + tex_coord.zw;
		} else {
			v = vertex_ac.xw;
			f_tex_coord = vec2(tex_coord.x, tex_coord.y + tex_coord.w);
		}
	}
	f_opacity = opacity;
	
	gl_Position = r_matrix * v_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag
#include "_version.glsl"

uniform sampler2D s_tex0;
uniform bvec2 repeat;

in  lowp float f_opacity;
in  lowp vec2  f_tex_coord;
out lowp vec4  FragColor;

void main(void) {
	if (!repeat.x) { // no repeat x
		if (f_tex_coord.x < 0.0 || f_tex_coord.x > 1.0) {
			discard; return;
		}
	}
	if (!repeat.y) { // no repeat y
		if (f_tex_coord.y < 0.0 || f_tex_coord.y > 1.0) {
			discard; return;
		}
	}
	FragColor = texture(s_tex0, f_tex_coord) * vec4(1.0, 1.0, 1.0, f_opacity);
}
