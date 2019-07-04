#include "_version.glsl"

//precision mediump float;

in vec2 vertex_id;
uniform mat4  root_matrix;
uniform float view_matrix[7];  // 视图变换/透明度
uniform vec4  vertex_ac;

#define xx_VertexID int(vertex_id[0])
#define r_matrix root_matrix
#define v_matrix get_view_matrix()
#define _vm view_matrix
#define opacity _vm[6]

mat4 get_view_matrix() {
	return mat4(_vm[0], _vm[3], 0.0, 0.0,
							_vm[1], _vm[4], 0.0, 0.0,
							0.0,      0.0,    1.0, 0.0,
							_vm[2], _vm[5], 0.0, 1.0);
}
