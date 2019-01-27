#vert
#include "_util.glsl"

// 纹理文本着色器

uniform float display_port_scale; // 当前视口缩放
uniform float texture_scale;    // 纹理需要的缩放
uniform vec4  color;    // 文字颜色
//
uniform float hori_baseline;  // 文字水平基线
uniform vec4  tex_size;       // vec4(width, height, left, top)
uniform float offset_x;       // offset_x

out vec2  f_tex_coord;    // 纹理座标
out vec4  f_color;        // 文字颜色

void main() {

	f_color = color * vec4(1.0, 1.0, 1.0, opacity);
	
	vec4 tex_size2 = tex_size * texture_scale / display_port_scale;
	
	vec2 hori_bearing = tex_size2.zw;
	vec2 coord = vec2(offset_x + hori_bearing.x, hori_baseline - hori_bearing.y);
	
	vec4 v;
	
	if (gl_VertexID == 0) {
		f_tex_coord = vec2(0.0, 0.0); v.xy = coord;
	}
	else if (gl_VertexID == 1) {
		f_tex_coord = vec2(1., 0.0);  v.xy = vec2(coord.x + tex_size2.x, coord.y);
	}
	else if (gl_VertexID == 2) {
		f_tex_coord = vec2(1.0, 1.0); v.xy = (coord + tex_size2.xy);
	}
	else {
		f_tex_coord = vec2(0.0, 1.0); v.xy = vec2(coord.x, coord.y + tex_size2.y);
	}
	
	v = v_matrix * vec4(v.xy, 0.0, 1.0);
	
	/* 在这里做取整,使纹理都刚好落到频幕物理像素点上,这样能减少小号字体插值绘制失真。*/
	v.xy = round(v.xy * display_port_scale) / display_port_scale;
	// v.x = round(v.x * display_port_scale) / display_port_scale;
	// v.y = round(v.y * display_port_scale) / display_port_scale;
	
	gl_Position = r_matrix * v;
}

#frag
#include "_version.glsl"
// lowp/mediump/highp

uniform lowp sampler2D sampler_tex_1;

in  lowp vec2 f_tex_coord;
in  lowp vec4 f_color;
out lowp vec4 FragColor;

void main() {
	FragColor = f_color * vec4(1.0, 1.0, 1.0, texture(sampler_tex_1, f_tex_coord).a);
	// FragColor = vec4(0.0, 0.0, 0.0, texture(sampler_tex_1, f_tex_coord).a);
	//FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
