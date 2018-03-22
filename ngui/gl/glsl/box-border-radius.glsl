#vert
#include "_box.glsl"

#define PI 3.141592653589793
#define PIx0_5 1.5707963267948966     // PI * 0.5

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

#define radius          args[0].x
#define start_ra        args[0].y
#define inverse         args[0].z == 0.0
#define mirror_x        args[1].x == 0.0
#define mirror_y        args[1].y == 0.0
#define border_main     args[1].z
#define border_adjacent args[2].x
#define border_x        args[2].y
#define border_y        args[2].z

uniform int direction;

out vec4 f_color;

void main() {
  mat3 args = mat3(0.0);

  // TODO 这里如果直接使用`radius_size`会导致异常,这很奇怪
  vec4 radius_size2 = vec4(radius_size.xyzw);
  vec4 border_width2 = vec4(border_width.xyzw);
  
  float index = float(gl_VertexID - SAMPLEx2);
  
  if ( direction == 0 ) { // 左边边框
    if ( index < 0.0 ) { // 左下角
      args = mat3(radius_size2.w, PIx0_5, 0.0, 1.0, 0.0, border_width2.xwxw); // 逆时钟, y轴镜像
    } else { // 左上角
      args = mat3(radius_size2.x, 0.0, 1.0, 1.0, 1.0, border_width2.xyxy);
    }
    f_color = vec4(border_left_color.rgb, border_left_color.a * opacity);
  }
  else if ( direction == 1 ) { // 上边
    if ( index < 0.0 ) { // 左上角
      args = mat3(radius_size2.x, PIx0_5, 1.0, 1.0, 1.0, border_width2.yxxy);
    } else { // 右上角
      args = mat3(radius_size2.y, 0.0, 0.0, 0.0, 1.0, border_width2.yzzy); // 逆时钟, x轴镜像
    }
    f_color = vec4(border_top_color.rgb, border_top_color.a * opacity);
  }
  else if ( direction == 2 ) { // 右边
    if ( index < 0.0 ) { // 右上角
      args = mat3(radius_size2.y, PIx0_5, 0.0, 0.0, 1.0, border_width2.zyzy); // 逆时钟, x轴镜像
    } else { // 右下角
      args = mat3(radius_size2.z, 0.0, 1.0, 0.0, 0.0, border_width2.zwzw);   // xy轴镜像
    }
    f_color = vec4(border_right_color.rgb, border_right_color.a * opacity);
  }
  else { // 下边
    if ( index < 0.0 ) { // 右下角
      args = mat3(radius_size2.z, PIx0_5, 1.0, 0.0, 0.0, border_width2.wzzw); // xy轴镜像
    } else { // 左下角
      args = mat3(radius_size2.w, 0.0, 0.0, 1.0, 0.0, border_width2.wxxw);    // 逆时钟, y轴镜像
    }
    f_color = vec4(border_bottom_color.rgb, border_bottom_color.a * opacity);
  }
  
  // --
  
  if ( index < 0.0 ) { // begin
    index = float(gl_VertexID) / 2.0;
  } else {
    index /= 2.0;
  }
  
  float x, y, ra, r_w; // r_w 圆角权重接近1时绘制1/4个圆弧
  vec2 v;
  
  r_w = border_main / (border_main + border_adjacent);
  
  if ( inverse ) { // 逆时钟
    ra = PIx0_5 + (start_ra * (1.0 - r_w)) + PI_40 * index * r_w;
  } else { // 顺时钟
    ra = PI -     (start_ra * (1.0 - r_w)) - PI_40 * index * r_w;
  }
  
  if ( mod(float(gl_VertexID), 2.0) == 1.0 ) { // 内环椭圆
    x = (1.0 + cos(ra)) * max(radius - border_x, 0.0);
    y = (1.0 - sin(ra)) * max(radius - border_y, 0.0);
    v = vec2(x, y);
  } else {
    x = (1.0 + cos(ra)) * radius;
    y = (1.0 - sin(ra)) * radius;
    v = vec2(x - border_x, y - border_y);
  }
  
  if ( mirror_x ) {
    if ( mirror_y ) {
      v = vertex_ac.zw - v;     // xy轴镜像
    } else {
      v.x = vertex_ac.z - v.x;  // x轴镜像
      v.y = vertex_ac.y + v.y;
    }
  } else if ( mirror_y ) {
    v.x = vertex_ac.x + v.x;
    v.y = vertex_ac.w - v.y; // y轴镜像
  } else {
    v = vertex_ac.xy + v;
  }
  
  gl_Position = r_matrix * v_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag
#include "_version.glsl"

in  lowp vec4 f_color;
out lowp vec4 FragColor;

void main() {
  FragColor = f_color;
}
