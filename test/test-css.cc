#include <quark/ui/app.h>
#include <quark/ui/window.h>
#include <quark/ui/view/root.h>
#include <quark/ui/css/css.h>

using namespace qk;

void test_css(int argc, char **argv) {
	App app;
	// init css
	//auto csss = app.styleSheets()->search(".div_cls.div_cls2 .aa.bb.cc, .div_cls.div_cls2:active .aa.bb.cc");
	auto div_cls = app.styleSheets()->search(".div_cls").front();
	div_cls->set_width({ 100, BoxSizeKind::Match });
	div_cls->set_height({ 0, BoxSizeKind::Match });
	div_cls->set_background_color({255,0,0,255});
	div_cls->set_background(new FillGradientRadial({1,0},{{1,0,0},{0,1,0}}));
	// div_cls->set_background(new FillGradientLinear(90, {0,1},{{1,0,0},{0,1,0}}));
	// init css end
	auto win = Window::Make({.frame={{0,0}, {500,500}}, .title="Test Css"});
	auto box = win->root()->append_new<Box>();
	box->cssclass()->add("div_cls");
	app.run();
}
