#version 300 es

#include "box-radius_.glsl"

// 只能使用84个顶点1实例绘制,可绘制一个圆角矩形

out vec4 f_color;

void main() {
  
  f_color = background_color * vec4(1.0, 1.0, 1.0, opacity);
  
  gl_Position = root_matrix * view_matrix * vertex();
}

#frag
#version 300 es

in  lowp vec4 f_color;
out lowp vec4 FragColor;

void main() {
  FragColor = f_color;
}
