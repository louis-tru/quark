#include <src/ui/app.h>
#include <src/ui/window.h>
#include <src/ui/view/root.h>
#include <src/ui/css/css.h>
#include <src/ui/action/keyframe.h>
#include "./test.h"

using namespace qk;

Qk_TEST_Func(openurl) {
	App app;
	auto win = Window::Make({.frame={{0,0}, {500,500}}, .title="Test OpenURL"});
	auto box = win->root()->append_new<Box>();
	box->set_width({ 100, BoxSizeKind::Match });
	box->set_height({ 0, BoxSizeKind::Match });
	box->set_background_color({255,0,0,255});

	app.Qk_On(Load, [](auto &e) {
		auto app = static_cast<Application*>(e.sender());
		app->openURL("https://163.com");
		app->sendEmail("louistru@live.com", "Test", "Test Body asd https://baidu.com 枯工棋基本面", "louis.tru@gmail.com", "jfm.s@163.com");
	});

	app.run();
}
