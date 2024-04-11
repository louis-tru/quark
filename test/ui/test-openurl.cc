#include <quark/ui/app.h>
#include <quark/ui/window.h>
#include <quark/ui/view/root.h>
#include <quark/ui/css/css.h>
#include <quark/ui/action/keyframe.h>

using namespace qk;

void test_openurl(int argc, char **argv) {
	App app;
	auto win = Window::Make({.frame={{0,0}, {500,500}}, .title="Test OpenURL"});
	auto box = win->root()->append_new<Box>();
	box->set_width({ 100, SizeKind::kMatch });
	box->set_height({ 0, SizeKind::kMatch });
	box->set_background_color({255,0,0,255});

	app.Qk_On(Load, [](auto &e) {
		auto app = static_cast<Application*>(e.sender());
		app->openURL("https://163.com");
		app->sendEmail("louistru@live.com", "Test", "louis.tru@gmail.com", "jfm.s@163.com", "Test Body asd https://baidu.com 枯工棋基本面");
	});

	app.run();
}
