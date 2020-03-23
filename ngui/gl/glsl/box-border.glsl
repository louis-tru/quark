#vert
#include "_box.glsl"

#define vertex0 (vertex_ac.xy - border_width.xy)
#define vertex1 vec2(vertex_ac.z + border_width.z, vertex_ac.y - border_width.y)
#define vertex2 (vertex_ac.zw + border_width.zw)
#define vertex3 vec2(vertex_ac.x - border_width.x, vertex_ac.w + border_width.w)
#define vertex4 vec2(vertex_ac.x, vertex_ac.y)
#define vertex5 vec2(vertex_ac.z, vertex_ac.y)
#define vertex6 vec2(vertex_ac.z, vertex_ac.w)
#define vertex7 vec2(vertex_ac.x, vertex_ac.w)

uniform int direction;
uniform vec4 border_color;

out vec4 f_color;

// 使用4顶点的实例绘制, 一个实例绘制一条边

void main() {
	
	vec2 v;
	
	// TODO
	// 有某些android设备上运行在es3环境中时,
	// 直接在if条件中使用 uniform `direction` 会导致非常奇怪的无法绘制的错误,
	// 这可能是在编译glsl条件语句时导致的分支紊乱。

	int id = gl_VertexID;
	int d = direction;

	if (d == 0) { // left
		if      ( id == 0 ) v = vertex3;
		else if ( id == 1 ) v = vertex0;
		else if ( id == 2 ) v = vertex4;
		else  v = vertex7;
	} 
	else if (d == 1) {
		if      ( id == 0 ) v = vertex0;
		else if ( id == 1 ) v = vertex1;
		else if ( id == 2 ) v = vertex5;
		else  v = vertex4;
	} 
	else if (d == 2) {
		if      ( id == 0 ) v = vertex1;
		else if ( id == 1 ) v = vertex2;
		else if ( id == 2 ) v = vertex6;
		else  v = vertex5;
	} 
	else {
		if      ( id == 0 ) v = vertex2;
		else if ( id == 1 ) v = vertex3;
		else if ( id == 2 ) v = vertex7;
		else  v = vertex6;
	}
	
	f_color = vec4(border_color.rgb, border_color.a * opacity);
	
	gl_Position = r_matrix * v_matrix * vec4(v.xy, 0.0, 1.0);
}

#frag
#include "_version.glsl"

in  lowp vec4 f_color;
out lowp vec4 FragColor;

void main() {
	FragColor = f_color;
}

