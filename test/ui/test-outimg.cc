
#include <quark/ui/app.h>
#include <quark/ui/window.h>
#include <quark/ui/screen.h>
#include <quark/ui/view/root.h>
#include <quark/render/render.h>
#include <quark/render/canvas.h>

using namespace qk;

class TestOutImg: public Box {
public:

	void draw(UIRender *r) override {
		auto canvas = window()->render()->getCanvas();
		auto size = canvas->size();
		float width = 300;

		Paint paint;
		paint.color = Color4f(0, 0, 1);
		Rect rect{size/2-width*0.5,width};
		auto path = Path::MakeArc(rect, Qk_PI_2_1 * 0.5f, Qk_PI + Qk_PI_2_1, true);

		canvas->save();
		auto img = canvas->outputImage(nullptr, false);
		canvas->drawPath(path, paint);
		canvas->restore();

		ImagePaint ipaint;
		ipaint.setImage(*img, {{0},size});
		paint.image = &ipaint;
		paint.type = Paint::kBitmap_Type;
		canvas->drawRect({{0},{width}}, paint);

		mark(kLayout_None,true);
	}
};

void test_outimg(int argc, char **argv) {
	App app;
	auto win = Window::Make({.frame={{0,0}, {500,500}}});
	auto r = win->root();
	auto t = r->append_new<TestOutImg>();
	r->set_background_color({255,255,255,0});
	t->set_width({ 0, BoxSizeKind::Match });
	t->set_height({ 0, BoxSizeKind::Match });
	app.run();
}
