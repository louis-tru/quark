#version 300 es
#include "box_.glsl"

#define PI 3.141592653589793
#define PIx1_5 4.71238898038469     // PI * 1.5
#define PIx2 6.283185307179586      // PI * 2
#define PIx0_5 1.5707963267948966   // PI * 0.5

// 最精细的圆角边线参数:
// 使用84顶点的实例绘制,1个实例绘制一条边,绘制4条边需要336个顶点
// PI/4圆角采样为20
// #define SAMPLEx2 42
// #define PI_40 0.07853981633974483   // PI / 2 / 20

// 高性能的圆角边线参数:
// 使用36顶点的实例绘制,1个实例绘制一条边,绘制4条边需要144个顶点
// PI/4圆角采样为8
#define SAMPLEx2 18
#define PI_40 0.19634954084936207   // PI / 2 / 8

uniform int direction;

out vec4 f_color;

float get_r_w(float a, float b) {
  return a / (a + b);
}

vec2 get_left_bottom(float ra) { // 左下角
  float x, y;
  vec2 v;
  if ( gl_VertexID % 2  == 1 ) { // 内环椭圆
    x = (1.0 + cos(ra) ) * max(radius_size.w - border_width.x, 0.0);
    y = (1.0 + sin(ra) ) * max(radius_size.w - border_width.w, 0.0);
    v = vec2(vertex_ac.x + x, vertex_ac.w - y);
  } else { // 外环正圆
    x = (1.0 + cos(ra)) * radius_size.w;
    y = (1.0 + sin(ra)) * radius_size.w;
    v = vec2(vertex_ac.x - border_width.x + x, vertex_ac.w + border_width.w - y);
  }
  return v;
}

vec2 get_left_top(float ra) { // 左上角
  vec2 v; float x, y;
  if ( gl_VertexID % 2 == 1 ) { // 内环椭圆
    x = (1.0 + cos(ra) ) * max(radius_size.x - border_width.x, 0.0);
    y = (1.0 - sin(ra) ) * max(radius_size.x - border_width.y, 0.0);
    v = vec2(vertex_ac.x + x, vertex_ac.y + y);
  } else {
    x = (1.0 + cos(ra)) * radius_size.x;
    y = (1.0 - sin(ra)) * radius_size.x;
    v = vec2(vertex_ac.x - border_width.x + x, vertex_ac.y - border_width.y + y);
  }
  return v;
}

vec2 get_right_top(float ra) { // 右上角
  vec2 v; float x, y;
  if ( gl_VertexID % 2  == 1 ) { // 内环椭圆
    x = (1.0 - cos(ra) ) * max(radius_size.y - border_width.z, 0.0);
    y = (1.0 - sin(ra) ) * max(radius_size.y - border_width.y, 0.0);
    v = vec2(vertex_ac.z - x, vertex_ac.y + y);
  } else {
    x = (1.0 - cos(ra)) * radius_size.y;
    y = (1.0 - sin(ra)) * radius_size.y;
    v = vec2(vertex_ac.z + border_width.z - x, vertex_ac.y - border_width.y + y);
  }
  return v;
}

vec2 get_right_bottom(float ra) { // 右下角
  vec2 v; float x, y;
  if ( gl_VertexID % 2  == 1 ) { // 内环椭圆
    x = (1.0 - cos(ra) ) * max(radius_size.z - border_width.z, 0.0);
    y = (1.0 + sin(ra) ) * max(radius_size.z - border_width.w, 0.0);
    v = vec2(vertex_ac.z - x, vertex_ac.w - y);
  } else { // 外环正圆
    x = (1.0 - cos(ra)) * radius_size.z;
    y = (1.0 + sin(ra)) * radius_size.z;
    v = vec2(vertex_ac.z + border_width.z - x, vertex_ac.w + border_width.w - y);
  }
  return v;
}

void main() {
  float index = float(gl_VertexID % SAMPLEx2 / 2);
  float r_w, ra; // r_w 圆角权重接近1时绘制1/4个圆弧
  vec2 v;
  
  switch ( direction ) {
    case 0: // 左边边框
      if ( gl_VertexID < SAMPLEx2 ) { // 左下角
        r_w = get_r_w(border_width.x, border_width.w);
        ra = PIx1_5 - PIx0_5 * (1.0 - r_w) - PI_40 * index * r_w; // 弧度
        v = get_left_bottom(ra);
      } else { // 左上角
        r_w = get_r_w(border_width.x, border_width.y);
        ra = PI - PI_40 * index * r_w; // 弧度
        v = get_left_top(ra);
      }
      f_color = vec4(border_left_color.rgb, border_left_color.a * opacity);
      break;
    case 1: // 上边
      if ( gl_VertexID < SAMPLEx2 ) { // 左上角
        r_w = get_r_w(border_width.y, border_width.x);
        ra = PI - PIx0_5 * (1.0 - r_w) - PI_40 * index * r_w; // 弧度
        v = get_left_top(ra);
      } else { // 右上角
        r_w = get_r_w(border_width.y, border_width.z);
        ra = PIx0_5 - PI_40 * index * r_w; // 弧度
        v = get_right_top(ra);
      }
      f_color = vec4(border_top_color.rgb, border_top_color.a * opacity);
      break;
    case 2: // 右边
      if ( gl_VertexID < SAMPLEx2 ) { // 右上角
        r_w = get_r_w(border_width.z, border_width.y);
        ra = PIx0_5 - PIx0_5 * (1.0 - r_w) - PI_40 * index * r_w; // 弧度
        v = get_right_top(ra);
      } else { // 右下角
        r_w = get_r_w(border_width.z, border_width.w);
        ra = PIx2 - PI_40 * index * r_w; // 弧度
        v = get_right_bottom(ra);
      }
      f_color = vec4(border_right_color.rgb, border_right_color.a * opacity);
      break;
    default: // 下边
      if ( gl_VertexID < SAMPLEx2 ) { // 右下角
        r_w = get_r_w(border_width.w, border_width.z);
        ra = PIx2 - PIx0_5 * (1.0 - r_w) - PI_40 * index * r_w; // 弧度
        v = get_right_bottom(ra);
      } else { // 左下角
        r_w = get_r_w(border_width.w, border_width.x);
        ra = PIx1_5 - PI_40 * index * r_w; // 弧度
        v = get_left_bottom(ra);
      }
      f_color = vec4(border_bottom_color.rgb, border_bottom_color.a * opacity);
      break;
  }
  
  gl_Position = root_matrix * view_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag
#version 300 es

in  lowp vec4 f_color;
out lowp vec4 FragColor;

void main() {
  FragColor = f_color;
}
