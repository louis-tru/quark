#version 300 es
#include "box_.glsl"

// 只能使用4顶点的实例绘制

#define start vec2(draw_data[39], draw_data[40])
#define ratio vec2(draw_data[41], draw_data[42])

out float f_opacity;
out vec2  f_y_tex_coord;
out vec2  f_u_tex_coord;
out vec2  f_v_tex_coord;

void main() {
  vec2 v;
  f_opacity = opacity;
  
  switch ( gl_VertexID ) {
    case 0:
      v = vertex_ac.xy; f_y_tex_coord = vec2(0.0, 0.0);
      break;
    case 1:
      v = vertex_ac.zy; f_y_tex_coord = vec2(1.0, 0.0);
      break;
    case 2:
      v = vertex_ac.zw; f_y_tex_coord = vec2(1.0, 1.0);
      break;
    default:
      v = vertex_ac.xw; f_y_tex_coord = vec2(0.0, 1.0);
      break;
  }
  
  f_u_tex_coord = vec2(f_y_tex_coord.x, f_y_tex_coord.y * 0.5);
  f_v_tex_coord = vec2(f_y_tex_coord.x, f_y_tex_coord.y * 0.5 + 0.5);
  
  gl_Position = root_matrix * view_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag
#version 300 es

precision lowp float;

uniform sampler2D s_tex_y;
uniform sampler2D s_tex_uv;
in      float    f_opacity;
in      vec2     f_y_tex_coord;
in      vec2     f_u_tex_coord;
in      vec2     f_v_tex_coord;
out     vec4     FragColor;

void main(void) {
  float y = texture(s_tex_y,  f_y_tex_coord).r;
  float u = texture(s_tex_uv, f_u_tex_coord).r;
  float v = texture(s_tex_uv, f_v_tex_coord).r;
  
  // yuv to rgb
  /*
   r = y + 1.4075 * (v - 0.5)
   g = y – 0.3455 * (u – 0.5) – 0.7169 * (v – 128)
   b = y + 1.779  * (u – 0.5)
   */
  
  FragColor = vec4(y + 1.4075 * (v - 0.5),
                   y - 0.3455 * (u - 0.5) - 0.7169 * (v - 0.5),
                   y + 1.779  * (u - 0.5),
                   f_opacity);
}
