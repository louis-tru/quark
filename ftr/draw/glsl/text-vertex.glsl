#vert
#include "_util.glsl"

uniform float text_size;                  // 字体尺寸
uniform vec4  color;                      // 文字颜色
//
uniform float hori_baseline;              // 文字水平基线
uniform float offset_x;                   // offset_x

in	vec2 	vertex;                         // 64pt 26.6 顶点数据

out	vec4  f_color;        // 文字颜色

void main() {
	
	f_color = color * vec4(1.0, 1.0, 1.0, opacity);
	
	vec2 v = vertex.xy * text_size / (4096.0);
	
	v = vec2(v.x + offset_x, v.y + hori_baseline);
	
	gl_Position = r_matrix * v_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag
#include "_version.glsl"

in 	lowp vec4  f_color;
out lowp vec4  FragColor;

void main() {
	FragColor = f_color;
	// discard;
}
