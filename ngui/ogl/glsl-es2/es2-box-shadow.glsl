#include "es2-box_.glsl"

// 只能使用4顶点的实例绘制

#define shadow  vec3(draw_data[39], draw_data[40], draw_data[41])
#define color   vec4(draw_data[42], draw_data[43], draw_data[44], draw_data[45])

varying mediump vec4 f_color;
varying mediump vec2 f_tex_coord;

void main() {
  vec2 v;
  
  if (VertexID == 0.0) {
    v = vertex_ac.xy; f_tex_coord = vec2(0.0, 0.0);
  }
  else if(VertexID == 1.0) {
    v = vertex_ac.zy; f_tex_coord = vec2(1.0, 0.0);
  }
  else if (VertexID == 2.0) {
    v = vertex_ac.zw; f_tex_coord = vec2(1.0, 1.0);
  }
  else {
    v = vertex_ac.xw; f_tex_coord = vec2(0.0, 1.0);
  }
  
  f_color = color * vec4(1.0, 1.0, 1.0, opacity);
  
  gl_Position = root_matrix * view_matrix * vec4(v.xy, 0.0, 1.0);
  
}

#frag

varying lowp vec4 f_color;
varying lowp vec2 f_tex_coord;

void main() {
  gl_FragColor = f_color;
}
