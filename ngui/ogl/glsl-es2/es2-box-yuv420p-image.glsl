#include "es2-box_.glsl"

// 只能使用4顶点的实例绘制

varying mediump float f_opacity;
varying mediump vec2  f_y_tex_coord;
varying mediump vec2  f_u_tex_coord;
varying mediump vec2  f_v_tex_coord;

void main() {
  f_opacity = opacity;
  
  vec2 v;
  
  if ( VertexID == 0.0 ) {
    v = vertex_ac.xy; f_y_tex_coord = vec2(0.0, 0.0);
  }
  else if ( VertexID == 1.0 ) {
    v = vertex_ac.zy; f_y_tex_coord = vec2(1.0, 0.0);
  }
  else if ( VertexID == 2.0 ) {
    v = vertex_ac.zw; f_y_tex_coord = vec2(1.0, 1.0);
  }
  else {
    v = vertex_ac.xw; f_y_tex_coord = vec2(0.0, 1.0);
  }
  
  f_u_tex_coord = vec2(f_y_tex_coord.x, f_y_tex_coord.y * 0.5);
  f_v_tex_coord = vec2(f_y_tex_coord.x, f_y_tex_coord.y * 0.5 + 0.5);
  
  gl_Position = root_matrix * view_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag

precision lowp float;

uniform sampler2D s_tex_y;
uniform sampler2D s_tex_uv;
varying float     f_opacity;
varying vec2      f_y_tex_coord;
varying vec2      f_u_tex_coord;
varying vec2      f_v_tex_coord;

void main(void) {
  float y = texture2D(s_tex_y,  f_y_tex_coord).r;
  float u = texture2D(s_tex_uv, f_u_tex_coord).r;
  float v = texture2D(s_tex_uv, f_v_tex_coord).r;
  /*
  // yuv to rgb
  
   r = y + 1.4075 * (v - 0.5)
   g = y – 0.3455 * (u – 0.5) – 0.7169 * (v – 0.5)
   b = y + 1.779  * (u – 0.5)
   */
  gl_FragColor = vec4(y + 1.4075 * (v - 0.5),
                      y - 0.3455 * (u - 0.5) - 0.7169 * (v - 0.5),
                      y + 1.779  * (u - 0.5),
                      f_opacity);
}
