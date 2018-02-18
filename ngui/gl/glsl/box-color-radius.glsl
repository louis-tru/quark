#vert
#include "_box-radius.glsl"

// 只能使用84个顶点1实例绘制,可绘制一个圆角矩形

out vec4 f_color;

void main() {
  
  f_color = background_color * vec4(1.0, 1.0, 1.0, opacity);
  
  gl_Position = r_matrix * v_matrix * vertex();
}

#frag
#include "_version.glsl"

in  lowp vec4 f_color;
out lowp vec4 FragColor;

void main() {
  FragColor = f_color;
}
