
#include <quark/app.h>
#include <quark/render/render.h>
#include <quark/layout/root.h>
#include <quark/display.h>

using namespace qk;

constexpr unsigned int u32 = 1;

class TestBlur: public Box {
public:
	TestBlur(App *host): Box(host) {}

	void accept(ViewVisitor *visitor) override {
		auto app = pre_render()->host();
		auto canvas = app->render()->getCanvas();
		auto size = canvas->size();

		Paint paint;
		paint.color = Color4f(0, 0, 0);
		// auto circle = Path::MakeArc({size/2-150,300}, Qk_PI_2_1 * 0.5f, Qk_PI + Qk_PI_2_1, true);
		auto circle = Path::MakeRect({size/2-150,300});
		circle.close();

		paint.antiAlias = false;

		// canvas->save();
		// canvas->clipPath(Path::MakeCircle(size*0.5, 100), Canvas::kIntersect_ClipOp, 0);

		canvas->drawPath(circle, paint);
		// canvas->restore();
		mark_none(kLayout_None);
	}
};

void test_blur(int argc, char **argv) {
	App app({.fps=0x0, .windowFrame={{0,0}, {400,400}}});
	// layout
	auto t = app.root()->append_new<TestBlur>();
	t->set_width({ 0, SizeKind::kMatch });
	t->set_height({ 0, SizeKind::kMatch });
	// layout end
	app.run();
}
