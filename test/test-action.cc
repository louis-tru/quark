#include <src/ui/app.h>
#include <src/ui/window.h>
#include <src/ui/view/root.h>
#include <src/ui/css/css.h>
#include <src/ui/action/keyframe.h>
#include "./test.h"

using namespace qk;

Qk_TEST_Func(action) {
	App app;
	auto div_cls = shared_root_styleSheets()->search(".div_cls", true).front();
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

	act1->addFrame(0,   LINEAR)->set_background_color({0,128,0});
	act1->addFrame(2e3, LINEAR)->set_background_color({0,0,255});
	act1->addFrame(4e3, LINEAR)->set_background_color({0,128,0});
	act2->addFrame(0,   LINEAR)->set_background(new FillGradientLinear({1,0},{{1,0,0,0.5},{0,1,0,0.5}},0));
	act2->addFrame(4e3, LINEAR)->set_background(new FillGradientLinear({1,0},{{1,0,0,0.5},{0,1,0,0.5}},360));

	box->set_action(act0);
	act0->play();
	app.run();
}
