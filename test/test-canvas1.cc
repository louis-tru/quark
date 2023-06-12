
#include <quark/app.h>
#include <quark/render/paint.h>
#include <quark/render/render.h>
#include <quark/layout/root.h>

using namespace qk;

class TestCanvas1: public Box {
public:
	TestCanvas1(App *host): Box(host) {}

	void accept(ViewVisitor *visitor) override {
		auto _canvas = static_cast<Render*>(visitor)->getCanvas();

		_canvas->clearColor(Color4f(1,1,1));

		Paint paint;

		paint.color = Color4f(0, 0, 0);
		//_canvas->drawPath(Path::MakeRRect({ {180,150}, 200 }, {50, 80, 50, 80}), paint);

		paint.color = Color4f(0, 1, 1);
		//_canvas->drawPath(Path::MakeRRectOutline({ {400,100}, 200 }, { {440,140}, 120 }, {50, 80, 50, 80}), paint);

		paint.color = Color4f(1, 0, 0);
		auto circle = Path::MakeCircle(Vec2(500,400), 100);
		//auto circle = Path::MakeArc({{500-50,400-50},{100,100}}, 0, -Qk_PI, 0, 0);
		
		float stage[] = {10.471975511965978,-31.41592653589793};
		auto dash = circle.dashPath(stage, 2);
		auto stroke = dash.strokePath(10, Paint::kButt_Cap);
		_canvas->drawPath(stroke, paint);

		paint.color = Color4f(0, 0, 0);
		//paint.width = 1;
		//paint.style = Paint::kStroke_Style;
		_canvas->drawPath(dash, paint);
	}
};

void test_canvas1(int argc, char **argv) {
	App app;
	// layout
	auto t = (new TestCanvas1(&app))->append_to<Box>(app.root());
	t->set_width({ 0, BoxSizeKind::MATCH });
	t->set_height({ 0, BoxSizeKind::MATCH });
	// layout end
	app.run();
}
