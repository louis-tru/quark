
#include <src/ui/app.h>
#include <src/ui/window.h>
#include <src/ui/screen.h>
#include <src/ui/view/root.h>
#include <src/render/render.h>
#include <src/render/canvas.h>
#include "./test.h"

using namespace qk;

class TestSubcanvas: public Box {
public:
	Sp<Canvas> _c;

	TestSubcanvas(Window *win) {
		_c = win->render()->newCanvas({.isMipmap=true});
		_c->setSurface({600},2);
	}

	void draw(Painter *vv) override {
		auto canvas = window()->render()->getCanvas();
		auto size = canvas->size();

		Paint paint;
		paint.fill.color = Color4f(0, 0, 1);
		auto path = Path::MakeArc({1,298}, Qk_PI_2_1 * 0.5f, Qk_PI + Qk_PI_2_1, true);

		_c->clearColor({0,0,0,0});
		_c->drawPath(path, paint);
		_c->swapBuffer();

		Rect rect{-150,300};
		PaintImage ipaint;
		ipaint.tileModeX = PaintImage::kDecal_TileMode;
		ipaint.tileModeY = PaintImage::kDecal_TileMode;
		ipaint.filterMode = PaintImage::kLinear_FilterMode;
		ipaint.mipmapMode = PaintImage::kLinearNearest_MipmapMode;
		ipaint.setCanvas(*_c, rect);
		paint.fill.image = &ipaint;
		canvas->drawRect({-150,300}, paint);

		mark(kLayout_None,true);
	}
};

Qk_TEST_Func(subcanvas) {
	App app;
	auto win = Window::Make({.fps=0x0, .frame={{0,0}, {500,500}}});
	win->activate();
	auto r = win->root();
	auto t = r->append_new<TestSubcanvas>(r->window());
	r->set_background_color({255,255,255,0});
	t->set_width({ 0, BoxSizeKind::Match });
	t->set_height({ 0, BoxSizeKind::Match });
	app.run();
}
