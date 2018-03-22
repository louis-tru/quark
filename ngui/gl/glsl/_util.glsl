
#include "_version.glsl"

//precision mediump float;

in vec2 vertex_id;
uniform mat4  root_matrix;
uniform float view_matrix_op[7];  // 视图变换/透明度
uniform vec4  vertex_ac;
uniform float draw_data[48];

#define xx_VertexID int(vertex_id[0])
#define r_matrix root_matrix
#define v_matrix get_view_matrix()
#define d_data draw_data
#define _vmo view_matrix_op
#define opacity _vmo[6]

mat4 get_view_matrix() {
  return mat4(_vmo[0], _vmo[3], 0.0, 0.0,
              _vmo[1], _vmo[4], 0.0, 0.0,
              0.0,      0.0,    1.0, 0.0,
              _vmo[2], _vmo[5], 0.0, 1.0);
}
