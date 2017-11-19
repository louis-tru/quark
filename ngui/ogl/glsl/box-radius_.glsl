#include "box_.glsl"

#define PIx0_5  1.5707963267948966   // PI * 0.5
#define PI      3.141592653589793      // PI

// 最精细的圆角参数:
// 使用84顶点的实例绘制
// PI/4圆角采样为20
// #define SAMPLEx2 42.0              // sample * 2 + 2
// #define PI_40 0.07853981633974483  // PI / 2 / 20

// 高性能的圆角参数:
// 使用64顶点的实例绘制
// PI/4圆角采样为15
// #define SAMPLEx2 32.0              // sample * 2 + 2
// #define PI_40 0.10471975511965977  // PI / 2 / 15

uniform float sample_x2;

vec4 vertex() {
  
  float PI_40 = PI / sample_x2;
  float Vertex = float(gl_VertexID) - sample_x2 - 2.0;
  
  float r_x, r_y, x, y;
  vec2 v;
  
  if ( Vertex < 0.0 ) { // 上半
    float index = float(gl_VertexID) / 2.0;
    
    if ( gl_VertexID % 2 == 0 ) { // 左上角
      r_x = max(radius_size.x - border_width.x, 0.0); // x轴半径
      r_y = max(radius_size.x - border_width.y, 0.0); // y轴半径
      x = (cos(PIx0_5 + PI_40 * index) + 1.0) * r_x;   // 距离原始顶点的x轴偏移
      y = (sin(PIx0_5 + PI_40 * index) - 1.0) * -r_y;  // 距离原始顶点的y轴偏移
      v = vec2(vertex_ac.x + x, vertex_ac.y + y); // 得到真实坐标
    } else { // 右上角
      r_x = max(radius_size.y - border_width.z, 0.0); // x轴半径
      r_y = max(radius_size.y - border_width.y, 0.0); // y轴半径
      x = (cos(PIx0_5 - PI_40 * index) - 1.0) * r_x;
      y = (sin(PIx0_5 - PI_40 * index) - 1.0) * -r_y;
      v = vec2(vertex_ac.z + x, vertex_ac.y + y);
    }
  } else { // 下半
    float index = Vertex / 2.0;
    
    if ( gl_VertexID % 2 == 0 ) { // 左下角
      r_x = max(radius_size.w - border_width.x, 0.0);
      r_y = max(radius_size.w - border_width.w, 0.0);
      x = (cos(PI + PI_40 * index) + 1.0) * r_x;
      y = (sin(PI + PI_40 * index) + 1.0) * -r_y;
      v = vec2(vertex_ac.x + x, vertex_ac.w + y);
    } else { // 右下角
      r_x = max(radius_size.z - border_width.z, 0.0);
      r_y = max(radius_size.z - border_width.w, 0.0);
      x = (cos(PI_40 * index) - 1.0) * r_x;
      y = (sin(PI_40 * index) - 1.0) * r_y;
      v = vec2(vertex_ac.z + x, vertex_ac.w + y);
    }
  }
  
  return vec4(v.xy, 0.0, 1.0);
}
