#vert
#include "_util.glsl"

uniform vec4  background_color; // 背景颜色
uniform vec2  origin; // 原点
out     vec4  f_color;  // 颜色

void main() {
  vec2 v;
  
  if (gl_VertexID == 0) {
    v = vertex_ac.xy + origin;
  }
  else if (gl_VertexID == 1) {
    v = vertex_ac.zy + origin;
  }
  else if (gl_VertexID == 2) {
    v = vertex_ac.zw + origin;
  }
  else {
    v = vertex_ac.xw + origin;
  }
  
  f_color = background_color * vec4(1.0, 1.0, 1.0, opacity);
  
  gl_Position = r_matrix * v_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag
#include "_version.glsl"

in  lowp vec4 f_color;
out lowp vec4 FragColor;

void main() {
  FragColor = f_color;
}
