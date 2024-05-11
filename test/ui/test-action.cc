#include <quark/ui/app.h>
#include <quark/ui/window.h>
#include <quark/ui/view/root.h>
#include <quark/ui/css/css.h>
#include <quark/ui/action/keyframe.h>

using namespace qk;

void test_action(int argc, char **argv) {
	App app;
	auto div_cls = app.styleSheets()->search(".div_cls").front();
	div_cls->set_width({ 100, BoxSizeKind::Match });
	div_cls->set_height({ 0, BoxSizeKind::Match });
	div_cls->set_background_color({255,0,0,255});
	auto win = Window::Make({.frame={{0,0}, {500,500}}, .title="Test Action"});
	auto box = win->root()->append_new<Box>();
	box->cssclass()->add("div_cls");

	// auto act0 = new SpawnAction(win);
	auto act0 = new SequenceAction(win);
	auto act1 = new KeyframeAction(win);
	auto act2 = new KeyframeAction(win);

	act0->append(act1);
	act0->append(act2);
	act0->set_loop(0xffff);
	act2->set_loop(0x1);

	act1->add(0,   LINEAR)->set_background_color({0,128,0});
	act1->add(2e3, LINEAR)->set_background_color({0,0,255});
	act1->add(4e3, LINEAR)->set_background_color({0,128,0});
	act2->add(0,   LINEAR)->set_background(new FillGradientLinear({1,0},{{1,0,0,0.5},{0,1,0,0.5}},0));
	act2->add(4e3, LINEAR)->set_background(new FillGradientLinear({1,0},{{1,0,0,0.5},{0,1,0,0.5}},360));

	box->set_action(act0);
	act0->play();
	app.run();
}
