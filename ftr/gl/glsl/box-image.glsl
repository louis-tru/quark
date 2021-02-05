#vert
#include "_box-radius.glsl"

out float f_opacity;
out vec2  f_tex_coord;

void main() {
	vec2 v;
	if (is_radius) { // 使用84个顶点绘制一个圆角矩形
		v = vertex(); // 获取顶点
		f_tex_coord = // coord 0 -> 1
			vec2(v.x - vertex_ac.x, v.y - vertex_ac.y) /                // vertex min -> max
			vec2(vertex_ac.z - vertex_ac.x, vertex_ac.w - vertex_ac.y); // rect size
	} else { // 使用4个顶点
		if ( gl_VertexID == 0) {
			v = vertex_ac.xy;
			f_tex_coord = vec2(0.0, 0.0);
		} else if ( gl_VertexID == 1) {
			v = vertex_ac.zy;
			f_tex_coord = vec2(1.0, 0.0);
		} else if ( gl_VertexID == 2) {
			v = vertex_ac.zw;
			f_tex_coord = vec2(1.0, 1.0);
		} else {
			v = vertex_ac.xw;
			f_tex_coord = vec2(0.0, 1.0);
		}
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

void main(void) {
	FragColor = texture(s_tex0, f_tex_coord) * vec4(1.0, 1.0, 1.0, f_opacity);
}
