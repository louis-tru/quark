#include <src/ui/app.h>
#include <src/ui/window.h>
#include <src/ui/view/root.h>
#include <src/ui/css/css.h>
#include "./test.h"

using namespace qk;

Qk_TEST_Func(css) {
	App app;
	// init css
	//auto csss = shared_root_styleSheets()->search(".div_cls.div_cls2 .aa.bb.cc, .div_cls.div_cls2:active .aa.bb.cc");
	auto div_cls = shared_root_styleSheets()->search(".div_cls", true).front();
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
