#include "es2-index-id_.glsl"

uniform mat4  root_matrix;
//
uniform float transf_opacity[7];          // 视图变换/透明度
uniform float text_size;                  // 字体尺寸
uniform vec4  color;                      // 文字颜色
//
uniform float hori_baseline;              // 文字水平基线
uniform float offset_x;                   // offset_x

attribute vec2 vertex;                    // 64pt 26.6 顶点数据

varying mediump vec4  f_color;          // 文字颜色

mat4 get_view_matrix() {
  return mat4(transf_opacity[0], transf_opacity[3], 0.0, 0.0,
              transf_opacity[1], transf_opacity[4], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
              transf_opacity[2], transf_opacity[5], 0.0, 1.0);
}

#define view_matrix get_view_matrix()
#define opacity transf_opacity[6]

void main() {
  
  f_color = color * vec4(1.0, 1.0, 1.0, opacity);
  
  vec2 v = vertex.xy * text_size / (4096.0);
  
  v = vec2(v.x + offset_x, v.y + hori_baseline);
  
  gl_Position = root_matrix * view_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag

varying lowp vec4  f_color;

void main() {
  gl_FragColor = f_color;
}
