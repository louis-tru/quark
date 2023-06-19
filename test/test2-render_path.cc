
#include <stdio.h>
#include <quark/render/path.h>

using namespace qk;

constexpr const char* Verb_str[] = {
	"Move","Line", "Quad","Cubic","Close"
};

int test2_render_path(int argc, char *argv[]) {

	Path path = Path::MakeOval({{0,0},{100,100}});

	auto strokePath = path.strokePath(5);

	for (int i = 0; i < strokePath.verbsLen(); i++) {
		Qk_LOG("Stroke Path PathVerb: %s, point: %f, %f",
			Verb_str[strokePath.verbs()[i]],
			strokePath.pts()[i].x(), strokePath.pts()[i].y());
	}

	Qk_LOG("----------------------------------------------------");

	float stage[] = { 3,5 };

	auto dashPath = path.dashPath(stage,2);

	for (int i = 0; i < dashPath.verbsLen(); i++) {
		Qk_LOG("Dash Path PathVerb: %s, point: %f, %f",
			Verb_str[dashPath.verbs()[i]],
			dashPath.pts()[i].x(), dashPath.pts()[i].y());
	}

	Qk_LOG("test2_render_path, ptsLen: %d, verbsLen: %d", dashPath.ptsLen(), dashPath.verbsLen());

	Qk_LOG("----------------------------------------------------");

	auto dashPath2 = dashPath.strokePath(2);

	for (int i = 0; i < dashPath2.verbsLen(); i++) {
		Qk_LOG("Dash Path PathVerb: %s, point: %f, %f",
			Verb_str[dashPath2.verbs()[i]],
			dashPath2.pts()[i].x(), dashPath2.pts()[i].y());
	}

	Qk_LOG("test2_render_path, ptsLen: %d, verbsLen: %d", dashPath2.ptsLen(), dashPath2.verbsLen());

	return 0;
}