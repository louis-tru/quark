#version 300 es

// 纹理文本着色器

uniform mat4  root_matrix;            // 根变换矩阵
uniform float display_port_scale;     // 当前视口缩放
//
uniform float transf_opacity[7];      // 视图变换/透明度
uniform float texture_scale;          // 纹理需要的缩放
uniform vec4  color;                  // 文字颜色
//
uniform float hori_baseline;          // 文字水平基线
uniform vec4  tex_size;               // vec4(width, height, left, top)
uniform float offset_x;               // offset_x

out vec2  f_tex_coord;    // 纹理座标
out vec4  f_color;        // 文字颜色

mat4 get_view_matrix() {
  /* 将2*3的二维矩阵转换为4*4三维矩阵 */
  return mat4(transf_opacity[0], transf_opacity[3], 0.0, 0.0,
              transf_opacity[1], transf_opacity[4], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
              transf_opacity[2], transf_opacity[5], 0.0, 1.0);
}

#define view_matrix get_view_matrix()
#define opacity transf_opacity[6]

void main() {
  
  f_color = color * vec4(1.0, 1.0, 1.0, opacity);
  
  vec4 tex_size2 = tex_size * texture_scale / display_port_scale;
  
  vec2 hori_bearing = tex_size2.zw;
  vec2 coord = vec2(offset_x + hori_bearing.x, hori_baseline - hori_bearing.y);
  
  vec4 v;
  
  switch ( gl_VertexID ) {
    case 0:
      f_tex_coord = vec2(0.0, 0.0); v.xy = coord;
      break;
    case 1:
      f_tex_coord = vec2(1., 0.0);  v.xy = vec2(coord.x + tex_size2.x, coord.y);
      break;
    case 2:
      f_tex_coord = vec2(1.0, 1.0); v.xy = (coord + tex_size2.xy);
      break;
    default:
      f_tex_coord = vec2(0.0, 1.0); v.xy = vec2(coord.x, coord.y + tex_size2.y);
      break;
  }
  
  v = view_matrix * vec4(v.xy, 0.0, 1.0);
  
  /* 在这里做取整,使纹理都刚好落到频幕物理像素点上,这样能减少小号字体插值绘制失真。*/
  v.xyz = round(v.xyz * display_port_scale) / display_port_scale;
  
  gl_Position = root_matrix * v;
}

#frag
#version 300 es
// lowp/mediump/highp

uniform lowp sampler2D sampler_tex_1;

in  lowp vec2   f_tex_coord;
in  lowp vec4   f_color;
out lowp vec4   FragColor;

void main() {
  FragColor = f_color * vec4(1.0, 1.0, 1.0, texture(sampler_tex_1, f_tex_coord).a);
}

