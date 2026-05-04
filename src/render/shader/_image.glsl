
#define _CONSTANT_IMAGE(block) Qk_CONSTANT(\
	vec4  texCoords; /*offset,scale*/\
	vec4  color; /* color */\
	block \
	float allScale; /* surface scale * view matrix scale*/\
)

_CONSTANT_IMAGE(Qk_CONSTANT_Fields);

#vert
layout(location=1) out vec2 coords; // texture coordinates for fragment shader

void main() {
	vec4 pos = (vMat.value * vec4(vertexIn.xy, pc.depth, 1.0));
	// fix draw image tearing with round function
	// Align the image pixels exactly onto the drawing surface
	//gl_Position = rMat.value * vec4(round(pos.xy * pd.allScale) / pd.allScale, pos.zw);
	gl_Position = rMat.value * vec4(pos.xy, pos.zw);

	aafuzz = aafuzzIn;
	coords = (pc.texCoords.xy + vertexIn.xy) / pc.texCoords.zw;
}

#frag
layout(binding=3)  uniform sampler2D image;
layout(location=1) in vec2 coords;