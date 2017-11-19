#include "es2-box_.glsl"

// 只能使用4顶点的实例绘制

varying mediump float f_opacity;
varying mediump vec2  f_tex_coord;

void main() {
  f_opacity = opacity;
  
  vec2 v;
  
  if ( VertexID == 0.0 ) {
    v = vertex_ac.xy; f_tex_coord = vec2(0.0, 0.0);
  }
  else if ( VertexID == 1.0 ) {
    v = vertex_ac.zy; f_tex_coord = vec2(1.0, 0.0);
  }
  else if ( VertexID == 2.0 ) {
    v = vertex_ac.zw; f_tex_coord = vec2(1.0, 1.0);
  }
  else {
    v = vertex_ac.xw; f_tex_coord = vec2(0.0, 1.0);
  }
  
  gl_Position = root_matrix * view_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag

uniform sampler2D   s_tex;
varying lowp float  f_opacity;
varying lowp vec2   f_tex_coord;

void main(void) {
  gl_FragColor = texture2D(s_tex, f_tex_coord) * vec4(1.0, 1.0, 1.0, f_opacity);
}
