#version 300 es
#include "util_.glsl"

// texture start and size
#define tex_start_size  vec4(draw_data[11], draw_data[12], draw_data[13], draw_data[14])
#define tex_ratio  vec2(draw_data[15], draw_data[16])

#define tex_ss tex_start_size

out float f_opacity;
out vec2  f_tex_coord;

#define tex_a_x tex_ss.x / tex_ss.z / tex_ratio.x
#define tex_a_y tex_ss.y / tex_ss.w / tex_ratio.y
#define tex_c_x (vertex_ac.z - vertex_ac.x + tex_ss.x) / tex_ss.z / tex_ratio.x
#define tex_c_y (vertex_ac.w - vertex_ac.y + tex_ss.y) / tex_ss.w / tex_ratio.y

void main() {
  vec2 v;
  f_opacity = opacity;
  
  switch ( gl_VertexID ) {
    case 0:
      v = vertex_ac.xy; f_tex_coord = vec2(tex_a_x, tex_a_y);
      break;
    case 1:
      v = vertex_ac.zy; f_tex_coord = vec2(tex_c_x, tex_a_y);
      break;
    case 2:
      v = vertex_ac.zw; f_tex_coord = vec2(tex_c_x, tex_c_y);
      break;
    default:
      v = vertex_ac.xw; f_tex_coord = vec2(tex_a_x, tex_c_y);
      break;
  }
  
  gl_Position = root_matrix * view_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag
#version 300 es

uniform sampler2D s_tex0;

in  lowp float f_opacity;
in  lowp vec2  f_tex_coord;
out lowp vec4  FragColor;

void main() {
  // discard;
  FragColor = texture(s_tex0, f_tex_coord) * vec4(1.0, 1.0, 1.0, f_opacity);
}
