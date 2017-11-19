#include "es2-index-id_.glsl"

uniform mat4  root_matrix;
uniform float transf_opacity[7];         // 视图变换/透明度
uniform vec4  background_color;           // 背景颜色
uniform vec2  origin;                     // 原点
uniform vec4  vertex_ac;

varying mediump vec4  f_color;        // 颜色

mat4 get_view_matrix() {
  return mat4(transf_opacity[0], transf_opacity[3], 0.0, 0.0,
              transf_opacity[1], transf_opacity[4], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
              transf_opacity[2], transf_opacity[5], 0.0, 1.0);
}

#define view_matrix get_view_matrix()

void main() {
  vec2 v;
  
  f_color = background_color * vec4(1.0, 1.0, 1.0, transf_opacity[6]);
  
  if ( VertexID == 0.0 ) {
    v = vertex_ac.xy + origin;
  }
  else if ( VertexID == 1.0 ) {
    v = vertex_ac.zy + origin;
  }
  else if ( VertexID == 2.0 ) {
    v = vertex_ac.zw + origin;
  }
  else {
    v = vertex_ac.xw + origin;
  }
  
  gl_Position = root_matrix * view_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag

varying lowp vec4 f_color;

void main() {
  gl_FragColor = f_color;
}
