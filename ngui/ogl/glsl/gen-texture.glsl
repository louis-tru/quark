#version 300 es

out vec2  f_tex_coord;

void main() {
  vec2 v;
  switch ( gl_VertexID ) {
    case 0: v = vec2(-1.0, 1.0); f_tex_coord = vec2(0.0, 1.0); break;
    case 1: v = vec2(1.0, 1.0); f_tex_coord = vec2(1.0, 1.0); break;
    case 2:  v = vec2(1.0, -1.0); f_tex_coord = vec2(1.0, 0.0); break;
    default: v = vec2(-1.0, -1.0); f_tex_coord = vec2(0.0, 0.0); break;
  }
  gl_Position = vec4(v.xy, 0.0, 1.0);
}

#frag
#version 300 es

uniform sampler2D s_tex0;

in  lowp vec2  f_tex_coord;
out lowp vec4  FragColor;

void main(void) {
  FragColor = texture(s_tex0, f_tex_coord);
}
