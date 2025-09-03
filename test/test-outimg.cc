
#include <src/ui/app.h>
#include <src/ui/window.h>
#include <src/ui/screen.h>
#include <src/ui/view/root.h>
#include <src/render/render.h>
#include <src/render/canvas.h>
#include "./test.h"

using namespace qk;

class TestOutImg: public Box {
public:

	void draw(Painter *r) override {
		auto canvas = window()->render()->getCanvas();
		auto size = canvas->size();
		float width = 300;

		Paint paint;
		paint.color = Color4f(0, 0, 1);
		Rect rect{-width*0.5,width};
		auto path = Path::MakeArc(rect, Qk_PI_2_1 * 0.5f, Qk_PI + Qk_PI_2_1, true);

		canvas->save();
		auto img = canvas->outputImage(nullptr, false);
		canvas->drawPath(path, paint);
		canvas->restore();

		ImagePaint ipaint;
		ipaint.setImage(*img, {size*-0.5,size});
		paint.image = &ipaint;
		paint.type = Paint::kBitmap_Type;
		canvas->drawRect({size*-0.5,size}, paint);

		mark(kLayout_None,true);
	}
};

Qk_TEST_Func(outimg) {
	App app;
	auto win = Window::Make({.frame={{0,0}, {500,500}}});
	auto r = win->root();
	auto t = r->append_new<TestOutImg>();
	r->set_background_color({255,255,255,0});
	t->set_width({ 0, BoxSizeKind::Match });
	t->set_height({ 0, BoxSizeKind::Match });
	app.run();
}
