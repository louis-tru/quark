
#include <src/ui/app.h>
#include <src/ui/view/view.h>
#include <src/ui/window.h>
#include <src/ui/screen.h>
#include <src/ui/view/root.h>
#include <src/render/render.h>
#include <src/render/canvas.h>
#include "./test.h"

using namespace qk;

constexpr unsigned int u32 = 1;

class TestRRect: public Box {
public:

	void draw(Painter *r) override {
		auto canvas = window()->render()->getCanvas();
		canvas->save();
		canvas->setMatrix(Mat().translate({-115, 0})); // reset mat

		//canvas->clearColor(Color4f(0,0,0));
		auto size = window()->size();

		Paint paint;
		//paint.antiAlias = false;

		//canvas->drawPath(Path::MakeRRect({ {180,150}, 200 }, {50, 80, 50, 80}), paint);
		//canvas->drawPath(Path::MakeRRect({ {180,50}, 200 }, {100, 100, 100, 100}), paint);
		//canvas->drawPathv(RectPath::MakeRRect({ {180,50}, 180 }, {100, 100, 100, 100}), paint);
		float border[4] = { 20,20,20,20 };
		float radius = 100;
		RectOutlinePath outline = RectOutlinePath::MakeRRectOutline({ {180,50}, 200 }, border, {radius, radius, radius, radius});
		paint.fill.color = Color4f(0, 0, 0, 0.5);
		canvas->drawPathv(outline.left, paint);
		paint.fill.color = Color4f(0, 1, 0, 0.5);
		canvas->drawPathv(outline.top, paint);
		paint.fill.color = Color4f(0, 0, 1, 0.5);
		canvas->drawPathv(outline.right, paint);
		paint.fill.color = Color4f(1, 0, 0, 0.5);
		canvas->drawPathv(outline.bottom, paint);

		paint.fill.color = Color4f(0, 0.5, 0.5, 0.4);
		//canvas->drawPath(Path::MakeRRectOutline({ {400,100}, 200 }, { {440,140}, 120 }, {50, 80, 50, 80}), paint);

		paint.fill.color = Color4f(1, 0, 0, 0.3);
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
		paint.stroke.color = Color4f(1, 0, 0, 0.3);
		paint.strokeWidth = 10;
		//paint.cap = Paint::kRound_Cap;
		//paint.join = Paint::kRound_Join;
		// canvas->drawPath(z, paint);

		paint.style = Paint::kFill_Style;
		paint.fill.color = Color4f(0, 0, 0);
		//paint.width = 1;
		//paint.style = Paint::kStroke_Style;
		//canvas->drawPath(dash, paint);
		canvas->translate({115, 0});
		// canvas->drawPath(circle, paint);

		// paint.antiAlias = false;
		paint.fill.color = Color4f(0.5,0.5,0.5,0.5);
		// circle = Path::MakeRect({size/2-105,210});

		//canvas->drawPath(circle, paint);
		canvas->restore();

		mark(kLayout_None,true);
	}
};

Qk_TEST_Func(rrect) {
	App app;
	auto win = Window::Make({.fps=0x0, .frame={{0,0}, {400,400}}});
	win->activate();
	auto t = win->root()->append_new<TestRRect>();
	t->set_width({ 0, BoxSizeKind::Match });
	t->set_height({ 0, BoxSizeKind::Match });
	// layout end
	app.run();
}
