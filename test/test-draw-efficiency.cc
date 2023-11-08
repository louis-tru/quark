
#include <quark/app.h>
#include <quark/window.h>
#include <quark/render/render.h>
#include <quark/layout/root.h>
#include <quark/screen.h>

using namespace qk;

constexpr unsigned int u32 = 1;

class TestDrawEfficiency: public Box {
public:

	void accept(ViewVisitor *visitor) override {
		auto canvas = pre_render()->render()->getCanvas();
		auto size = pre_render()->window()->size();

		Paint paint;
		paint.color = Color4f(0, 0, 0);
		//auto circle = Path::MakeCircle(size/2, 105, false);
		auto circle = Path::MakeArc({size/2-150,300}, Qk_PI_2_1 * 0.5f, Qk_PI + Qk_PI_2_1, true);
		//auto circle = Path::MakeArc({{500-50,400-50},{100,100}}, 0, -Qk_PI, 0, 0);
		circle.close();

		// paint.antiAlias = false;
		canvas->save();
		canvas->clipPath(Path::MakeCircle(size*0.5, 100), Canvas::kIntersect_ClipOp, 1);

		for (int i = 0; i < 100000; i++) {
			canvas->drawPath(circle, paint);
		}
		canvas->restore();
		mark_render();
	}
};

void test_draw_efficiency(int argc, char **argv) {
	App app;
	auto win = Window::Make({.fps=0x0, .frame={{0,0}, {400,400}}});
	win->activate();
	// layout
	auto t = New<TestDrawEfficiency>()->append_to<Box>(win->root());
	t->set_width({ 0, SizeKind::kMatch });
	t->set_height({ 0, SizeKind::kMatch });
	// layout end
	app.run();
}