#vert
#include "_box-radius.glsl"

// 只能使用84个顶点绘制一个圆角矩形

out float f_opacity;
out vec2  f_tex_coord;

void main() {
  
  vec4 v = vertex(); // 获取顶点
  
  f_opacity = opacity;
  
  float w = vertex_ac.z - vertex_ac.x;
  float h = vertex_ac.w - vertex_ac.y;
  
  f_tex_coord = vec2((v.x - vertex_ac.x) / w, (v.y - vertex_ac.y) / h );
  
  gl_Position = r_matrix * v_matrix * v;
}

#frag
#include "_version.glsl"

uniform sampler2D s_tex0;

in 	lowp float f_opacity;
in 	lowp vec2  f_tex_coord;
out lowp vec4  FragColor;

void main() {
  FragColor = texture(s_tex0, f_tex_coord) * vec4(1.0, 1.0, 1.0, f_opacity);
}

