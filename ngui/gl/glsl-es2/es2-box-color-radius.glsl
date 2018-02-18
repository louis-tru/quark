
#include "_es2-box-radius.glsl"

// 只能使用84个顶点1实例绘制,可绘制一个圆角矩形

varying mediump vec4 f_color;

void main() {
  
  f_color = background_color * vec4(1.0, 1.0, 1.0, opacity);
  
  gl_Position = root_matrix * view_matrix * vertex();
}

#frag

varying lowp vec4 f_color;

void main() {
  gl_FragColor = f_color;
}
