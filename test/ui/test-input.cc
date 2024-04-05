#include <quark/ui/app.h>
#include <quark/ui/window.h>
#include <quark/ui/layout/root.h>
#include <quark/ui/layout/input.h>
#include <quark/ui/layout/textarea.h>
#include <quark/ui/css/css.h>
#include <quark/ui/action/keyframe.h>

using namespace qk;

void test_input(int argc, char **argv) {
	App app;
	auto win = Window::Make({.frame={{0,0}, {500,500}}, .title="Test Input"});
	auto box = win->root()->append_new<Box>();
	auto input = box->append_new<Textarea>();
	//auto input = box->append_new<Input>();

	box->set_width({ 0, SizeKind::kMatch });
	box->set_height({ 0, SizeKind::kMatch });
	box->set_margin_left(20);
	box->set_margin_top(20);
	box->set_margin_right(20);
	box->set_margin_bottom(20);
	box->set_background_color({200,128,128});
	box->add_event_listener(UIEvent_Click, [](auto&e){
		Qk_DEBUG("------------------ Box Click ------------------");
	});

	input->set_width({ 0.5, SizeKind::kRatio });
	input->set_height({ 100 });
	input->set_background_color({255,255,255});
	input->set_align(Align::kCenterCenter);
	// input->set_padding_left(5);
	// input->set_padding_right(5);
	input->set_text_line_height({30});
	//input->set_text_background_color({{255,128,0}});
	input->set_text_shadow({{1,1,2,{0,0,0}}});

	app.run();
}
