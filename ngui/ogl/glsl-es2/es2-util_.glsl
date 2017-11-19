#include "es2-index-id_.glsl"

uniform mat4  root_matrix;
uniform float draw_data[64];

#define view_matrix get_view_matrix()
#define opacity     draw_data[6]
#define vertex_ac   vec4(draw_data[7], draw_data[8], draw_data[9], draw_data[10])

int Mod(int num, int mod) {
  return num - num / mod * mod;
}

mat4 get_view_matrix() {
  return mat4(draw_data[0], draw_data[3], 0.0, 0.0,
              draw_data[1], draw_data[4], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
              draw_data[2], draw_data[5], 0.0, 1.0);
}
