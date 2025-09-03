
#include <src/ui/app.h>
#include <src/ui/window.h>
#include <src/ui/screen.h>
#include <src/ui/view/root.h>
#include <src/render/render.h>
#include <src/render/canvas.h>
#include "./test.h"

using namespace qk;

constexpr unsigned int u32 = 1;

class TestDrawEfficiency: public Box {
public:

	void draw(Painter *r) override {
		auto canvas = window()->render()->getCanvas();
		auto size = window()->size();

		Paint paint;
		paint.color = Color4f(0, 0, 0);
		//auto circle = Path::MakeCircle(size/2, 105, false);
		auto circle = Path::MakeArc({size/2-150,300}, Qk_PI_2_1 * 0.5f, Qk_PI + Qk_PI_2_1, true);
		//auto circle = Path::MakeArc({{500-50,400-50},{100,100}}, 0, -Qk_PI, 0, 0);
		circle.close();

		paint.antiAlias = false;
		canvas->save();
		canvas->translate(size*-0.5);
		canvas->clipPath(Path::MakeCircle(size*0.5, 100), Canvas::kIntersect_ClipOp, 1);

		for (int i = 0; i < 10000; i++) {
			canvas->drawPath(circle, paint);
		}
		canvas->restore();
		mark(kLayout_None,true);
	}
};

Qk_TEST_Func(draw_efficiency) {
	App app;
	auto win = Window::Make({.fps=0x0, .frame={{0,0}, {400,400}}});
	win->activate();
	// layout
	auto t = win->root()->append_new<TestDrawEfficiency>();
	t->set_width({ 0, BoxSizeKind::Match });
	t->set_height({ 0, BoxSizeKind::Match });
	// layout end
	app.run();
}
