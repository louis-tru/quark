
#include <quark/app.h>
#include <quark/window.h>
#include <quark/render/render.h>
#include <quark/layout/root.h>
#include <quark/screen.h>

using namespace qk;

constexpr unsigned int u32 = 1;

class TestRRect: public Box {
public:
	void accept(Visitor *vv) override {
		if (vv->flags()) return;
		auto canvas = pre_render()->render()->getCanvas();
		canvas->save();
		canvas->translate(-115, 0);

		canvas->clearColor(Color4f(0,0,1));
		auto size = pre_render()->window()->size();

		Paint paint;

		paint.color = Color4f(0, 0, 0);
		//canvas->drawPath(Path::MakeRRect({ {180,150}, 200 }, {50, 80, 50, 80}), paint);

		paint.color = Color4f(0, 0.5, 0.5, 0.1);
		canvas->drawPath(Path::MakeRRectOutline({ {400,100}, 200 }, { {440,140}, 120 }, {50, 80, 50, 80}), paint);

		paint.color = Color4f(1, 0, 0, 0.3);
		//auto circle = Path::MakeCircle(size/2, 105, false);
		auto circle = Path::MakeArc({size/2-105,210}, Qk_PI_2_1 * 0.5f, Qk_PI + Qk_PI_2_1, true);
		//auto circle = Path::MakeArc({{500-50,400-50},{100,100}}, 0, -Qk_PI, 0, 0);
		circle.close();

		//float stage[] = {10.471975511965978,-31.41592653589793};
		float stage[] = {11,11};
		auto dash = circle.dashPath(stage, 2, -12);
		Path z(Vec2{});// = circle.dashPath(stage, 2, -12);

		z.lineTo(Vec2(100,0));
		z.lineTo(Vec2(0,100));
		z.lineTo(Vec2(100,100));
		z.lineTo(Vec2(50,150));
		z.lineTo(Vec2(100,200));
		z.lineTo(Vec2(100,200)); // test duplicates
		z.lineTo(Vec2(-50,200));
		//z.close();
		//z.transfrom(Mat(-1,0,0,0,1,0));
		z.transfrom(Mat(1,0,size.x()/2-100,0,1,size.y()/2-50));

		//auto stroke = z.strokePath(10, Paint::kRound_Cap, Paint::kRound_Join);
		//canvas->drawPath(stroke, paint);
		paint.style = Paint::kStroke_Style;
		paint.width = 10;
		//paint.cap = Paint::kRound_Cap;
		//paint.join = Paint::kRound_Join;
		// canvas->drawPath(z, paint);

		paint.style = Paint::kFill_Style;
		paint.color = Color4f(0, 0, 0);
		//paint.width = 1;
		//paint.style = Paint::kStroke_Style;
		//canvas->drawPath(dash, paint);
		canvas->translate(70, 0);
		// canvas->drawPath(circle, paint);

		// paint.antiAlias = false;
		paint.color = Color4f(0.5,0.5,0.5,0.5);
		// circle = Path::MakeRect({size/2-105,210});

		canvas->drawPath(circle, paint);
		canvas->restore();

		mark_render();
	}
};

void test_rrect(int argc, char **argv) {
	App app;
	auto win = Window::Make({.fps=0x0, .frame={{0,0}, {400,400}}});
	win->activate();
	// layout
	auto t = New<TestRRect>()->append_to<Box>(win->root());
	t->set_width({ 0, SizeKind::kMatch });
	t->set_height({ 0, SizeKind::kMatch });
	// layout end
	app.run();
}
