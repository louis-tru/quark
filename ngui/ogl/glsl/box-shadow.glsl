#version 300 es
#include "box_.glsl"

// 只能使用4顶点的实例绘制

#define shadow  vec3(draw_data[39], draw_data[40], draw_data[41])
#define color   vec4(draw_data[42], draw_data[43], draw_data[44], draw_data[45])

out vec4 f_color;
out vec2 f_tex_coord;

void main() {
  vec2 v;
  
  switch ( gl_VertexID ) {
    case 0:
      v = vertex_ac.xy; f_tex_coord = vec2(0.0);
      break;
    case 1:
      v = vertex_ac.zy; f_tex_coord = vec2(1.0, 0.0);
      break;
    case 2:
      v = vertex_ac.zw; f_tex_coord = vec2(1.0);
      break;
    default:
      v = vertex_ac.xw; f_tex_coord = vec2(0.0, 1.0);
      break;
  }
  
  f_color = color * vec4(1., 1., 1., opacity);
  
  gl_Position = root_matrix * view_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag
#version 300 es

in  lowp vec4 f_color;
in  lowp vec2 f_tex_coord;
out lowp vec4 FragColor;

void main() {
  FragColor = f_color;
}
