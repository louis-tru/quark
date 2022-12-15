#include <stdio.h>
#include <math.h>

int test2_math_dot(int argc, char** argv) {

	struct vec2 {
		float x, y;
	};

	vec2 a = {200, 200};
	vec2 b = {100, 100};

	float a_b = a.x * b.x + a.y * b.y; // abcos = ab(d/a) = bd

	printf("%s,%f\n", "dot=", a_b);

	return 0;
}