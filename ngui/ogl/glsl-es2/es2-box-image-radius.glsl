
#include "es2-box-radius_.glsl"

varying mediump float f_opacity;
varying mediump vec2  f_tex_coord;

// 只能使用84个顶点1实例绘制,可绘制一个圆角矩形

void main() {
  
  vec4 v = vertex(); // 获取顶点
  
  f_opacity = opacity;
  
  float w = vertex_ac.z - vertex_ac.x;
  float h = vertex_ac.w - vertex_ac.y;
  
  f_tex_coord = vec2((v.x - vertex_ac.x) / w, (v.y - vertex_ac.y) / h );
  
  gl_Position = root_matrix * view_matrix * v;
}

#frag

uniform sampler2D s_tex0;
varying lowp float f_opacity;
varying lowp vec2  f_tex_coord;

void main() {
  gl_FragColor = texture2D(s_tex0, f_tex_coord) * vec4(1.0, 1.0, 1.0, f_opacity);
}
