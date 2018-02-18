#include "_es2-index-id.glsl"

uniform mat4  root_matrix;
uniform float other_data[64];

#define view_matrix get_view_matrix()
#define o_data other_data
#define opacity     o_data[6]
#define vertex_ac   vec4(o_data[7], o_data[8], o_data[9], o_data[10])

int Mod(int num, int mod) {
  return num - num / mod * mod;
}

mat4 get_view_matrix() {
  return mat4(o_data[0], o_data[3], 0.0, 0.0,
              o_data[1], o_data[4], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
              o_data[2], o_data[5], 0.0, 1.0);
}
