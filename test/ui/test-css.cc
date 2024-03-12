
#include <quark/ui/app.h>
#include <quark/ui/window.h>
#include <quark/ui/screen.h>
#include <quark/ui/layout/root.h>
#include <quark/render/render.h>
#include <quark/render/canvas.h>
#include <quark/ui/css/css.h>

using namespace qk;

void test_css(int argc, char **argv) {
	App app;
	// init css
	//auto csss = app.styleSheets()->search(".div_cls.div_cls2 .aa.bb.cc, .div_cls.div_cls2:active .aa.bb.cc");
	auto div_cls = app.styleSheets()->search(".div_cls").front();
	auto div_cls1 = app.styleSheets()->search(".div_cls1").front();
	div_cls->set_width({ 100, SizeKind::kMatch });
	div_cls->set_height({ 0, SizeKind::kMatch });
	div_cls->set_background_color({255,0,0,255});
	div_cls1->set_width({ 200 });
	// init css end
	auto win = Window::Make({.frame={{0,0}, {500,500}}, .title="Test Css"});
	auto box = win->root()->append_new<Box>();
	box->cssclass()->add("div_cls");
	// box->cssclass()->add("div_cls1");
	app.run();
}
