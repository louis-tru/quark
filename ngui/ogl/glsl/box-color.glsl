#version 300 es
#include "box_.glsl"

// 只能使用4顶点的实例绘制

out vec4 f_color;

void main() {
  vec2 v;
  
  switch ( gl_VertexID ) {
    case 0:   v = vertex_ac.xy; break;
    case 1:   v = vertex_ac.zy; break;
    case 2:   v = vertex_ac.zw; break;
    default:  v = vertex_ac.xw; break;
  }
  
  f_color = background_color * vec4(1.0, 1.0, 1.0, opacity);
  
  gl_Position = root_matrix * view_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag
#version 300 es

in  lowp vec4 f_color;
out lowp vec4 FragColor;

void main() {
  FragColor = f_color;
}
