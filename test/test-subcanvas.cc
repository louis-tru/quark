
#include <quark/ui/app.h>
#include <quark/ui/window.h>
#include <quark/ui/screen.h>
#include <quark/ui/layout/root.h>
#include <quark/render/render.h>
#include <quark/render/canvas.h>

using namespace qk;

class TestSubcanvas: public BoxLayout {
public:
	Sp<Canvas> _c;

	TestSubcanvas(Window *win): BoxLayout(win) {
		_c = window()->render()->newCanvas({.isMipmap=0});
		_c->setSurface({600},2);
	}

	void draw(UIRender *vv) override {
		auto canvas = window()->render()->getCanvas();
		auto size = canvas->size();

		Paint paint;
		paint.color = Color4f(0, 0, 1);
		auto path = Path::MakeArc({0,300}, Qk_PI_2_1 * 0.5f, Qk_PI + Qk_PI_2_1, true);

		_c->drawPath(path, paint);
		_c->swapBuffer();

		Rect rect{size/2-150,300};
		ImagePaint ipaint;
		paint.image = &ipaint;
		paint.type = Paint::kBitmap_Type;
		ipaint.tileModeX = ImagePaint::kDecal_TileMode;
		ipaint.tileModeY = ImagePaint::kDecal_TileMode;
		ipaint.setCanvas(*_c, rect);
		canvas->drawRect(rect, paint);

		mark_render();
	}
};

void test_subcanvas(int argc, char **argv) {
	App app;
	auto win = Window::Make({.fps=0x0, .frame={{0,0}, {500,500}}});
	win->activate();
	auto r = win->root();
	auto t = r->append_new<TestSubcanvas>()->layout<TestSubcanvas>();
	r->layout<BoxLayout>()->set_background_color({255,255,255,0});
	t->set_width({ 0, SizeKind::kMatch });
	t->set_height({ 0, SizeKind::kMatch });
	app.run();
}
