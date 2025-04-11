#include <src/ui/app.h>
#include <src/ui/window.h>
#include <src/ui/view/root.h>
#include <src/ui/view/input.h>
#include <src/ui/view/textarea.h>
#include <src/ui/css/css.h>
#include <src/ui/action/keyframe.h>
#include "./test.h"

using namespace qk;

Qk_TEST_Func(input) {
	App app;
	auto win = Window::Make({
		.frame={{0,0}, {500,500}},
		.title="Test Input",
		.backgroundColor={0,0,255}
	});
	// win->root()->set_background_color({0,0,0,0});

	auto box = win->root()->append_new<Box>();
	auto input = box->append_new<Textarea>();
	auto mat = box->append_new<Matrix>();

	box->set_width({ 0, BoxSizeKind::Match });
	box->set_height({ 0, BoxSizeKind::Match });
	box->set_margin_left(20);
	box->set_margin_top(20);
	box->set_margin_right(20);
	box->set_margin_bottom(20);
	box->set_background_color({200,128,128,128});
	box->add_event_listener(UIEvent_Click, [](auto& e) {
		Qk_DLog("------------------ Box Click ------------------");
	});
	// box->set_clip(true);

	// input->set_cursor(CursorStyle::ClosedHand);
	// input->set_cursor(CursorStyle::None);
	input->set_width({ 0.5, BoxSizeKind::Ratio });
	// input->set_width({ 300 });
	input->set_height({ 100 });
	input->set_background_color({255,0,0});
	input->set_align(Align::Center);
	// input->set_padding_left(5);
	// input->set_padding_right(5);
	input->set_text_line_height({30});
	input->set_value("ABCDEFGJ - abCcRdefgj");
	// input->set_text_background_color({{255,128,0}});
	// input->set_text_shadow({{1,1,1,{0,0,0}}});

	mat->set_width({100});
	mat->set_height({100});
	mat->set_y(50);
	mat->set_background_color({0,255,0});

	app.run();
}
