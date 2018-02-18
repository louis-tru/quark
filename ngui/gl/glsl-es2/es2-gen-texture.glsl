#include "_es2-index-id.glsl"

// 只能使用4顶点的实例绘制

varying mediump vec2  f_tex_coord;

void main() {
  vec2 v;
  if ( VertexID == 0.0 ) {
    v = vec2(-1.0, 1.0); f_tex_coord = vec2(0.0, 1.0);
  } else if ( VertexID == 1.0 ) {
    v = vec2(1.0, 1.0); f_tex_coord = vec2(1.0, 1.0);
  } else if ( VertexID == 2.0 ) {
    v = vec2(1.0, -1.0); f_tex_coord = vec2(1.0, 0.0);
  } else {
    v = vec2(-1.0, -1.0); f_tex_coord = vec2(0.0, 0.0);
  }
  gl_Position = vec4(v.xy, 0.0, 1.0);
}

#frag

uniform sampler2D   s_tex;
varying lowp vec2   f_tex_coord;

void main(void) {
  gl_FragColor = texture2D(s_tex, f_tex_coord);
}
