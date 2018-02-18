#vert
#include "_version.glsl"
out vec2  f_tex_coord;

void main() {
  vec2 v;
  
  if (gl_VertexID == 0) {
  	v = vec2(-1.0, 1.0); f_tex_coord = vec2(0.0, 1.0);
  }
  else if (gl_VertexID == 1) {
  	v = vec2(1.0, 1.0); f_tex_coord = vec2(1.0, 1.0);
  }
  else if (gl_VertexID == 2) {
  	v = vec2(1.0, -1.0); f_tex_coord = vec2(1.0, 0.0);
  }
  else {
  	v = vec2(-1.0, -1.0); f_tex_coord = vec2(0.0, 0.0);
  }
  gl_Position = vec4(v.xy, 0.0, 1.0);
}

#frag
#include "_version.glsl"

uniform sampler2D s_tex0;

in  lowp vec2  f_tex_coord;
out lowp vec4  FragColor;

void main(void) {
  FragColor = texture(s_tex0, f_tex_coord);
}
