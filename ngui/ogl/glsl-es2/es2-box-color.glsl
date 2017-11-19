#include "es2-box_.glsl"

// 只能使用4顶点的实例绘制

varying mediump vec4 f_color;

void main() {
  vec2 v;
  
  if ( VertexID == 0.0 ) {
    v = vertex_ac.xy;
  }
  else if ( VertexID == 1.0 ) {
    v = vertex_ac.zy;
  }
  else if ( VertexID == 2.0 ) {
    v = vertex_ac.zw;
  }
  else {
    v = vertex_ac.xw;
  }
  
  f_color = background_color * vec4(1.0, 1.0, 1.0, opacity);
  
  gl_Position = root_matrix * view_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag

varying lowp vec4 f_color;

void main() {
  gl_FragColor = f_color;
}
