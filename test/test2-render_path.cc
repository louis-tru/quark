
#include <stdio.h>
#include <quark/render/path.h>

using namespace qk;

int test2_render_path(int argc, char *argv[]) {

	// const Rect& rect, bool ccw = false

	Path path = Path::MakeOval({{0,0},{100,100}});

	float stage[] = { 3,-5 };

	auto dashPath = path.dashPath(stage,2);

	for (int i = 0; i < dashPath.verbsLen(); i++) {
		Qk_LOG("Path PathVerb: %s, point: %f, %f",
			dashPath.verbs()[i] == 0 ? "Move": "Line",
			dashPath.pts()[i].x(), dashPath.pts()[i].y());
	}

	Qk_LOG("test2_render_path, ptsLen: %d, verbsLen: %d", dashPath.ptsLen(), dashPath.verbsLen());

	return 0;
}