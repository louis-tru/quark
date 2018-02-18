#include "_es2-util.glsl"

// texture start and size
#define tex_start_size  vec4(o_data[11], o_data[12], o_data[13], o_data[14])
#define tex_ratio  vec2(o_data[15], o_data[16])

#define tex_ss tex_start_size

varying  mediump  float f_opacity;
varying  mediump  vec2  f_tex_coord;

#define tex_a_x tex_ss.x / tex_ss.z / tex_ratio.x
#define tex_a_y tex_ss.y / tex_ss.w / tex_ratio.y
#define tex_c_x (vertex_ac.z - vertex_ac.x + tex_ss.x) / tex_ss.z / tex_ratio.x
#define tex_c_y (vertex_ac.w - vertex_ac.y + tex_ss.y) / tex_ss.w / tex_ratio.y

void main() {
  
  f_opacity = opacity;
  
  vec2 v;
  
  if ( VertexID == 0.0 ) {
    v = vertex_ac.xy; f_tex_coord = vec2(tex_a_x, tex_a_y);
  }
  else if ( VertexID == 1.0 ) {
    v = vertex_ac.zy; f_tex_coord = vec2(tex_c_x, tex_a_y);
  }
  else if ( VertexID == 2.0 ) {
    v = vertex_ac.zw; f_tex_coord = vec2(tex_c_x, tex_c_y);
  }
  else {
    v = vertex_ac.xw; f_tex_coord = vec2(tex_a_x, tex_c_y);
  }
  
  gl_Position = root_matrix * view_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag

uniform sampler2D  s_tex;
varying lowp float f_opacity;
varying lowp vec2  f_tex_coord;

void main() {
  gl_FragColor = texture2D(s_tex, f_tex_coord) * vec4(1.0, 1.0, 1.0, f_opacity);
}
