
#include <quark/ui/app.h>
#include <quark/ui/window.h>
#include <quark/ui/view/root.h>
#include <quark/ui/filter.h>
#include <quark/ui/screen.h>
#include <quark/ui/view/flow.h>
#include <quark/ui/view/image.h>
#include <quark/ui/view/label.h>
#include <quark/ui/view/text.h>
#include <quark/ui/view/input.h>
#include <quark/ui/view/textarea.h>
#include <quark/render/render.h>
#include <quark/render/font/pool.h>
#include <quark/util/fs.h>

using namespace qk;

void layout_text(Window* win) {
	auto box = win->root();
	auto text = box->append_new<Text>();
	auto labe = text->append_new<Label>();

	text->set_width({ 0, BoxSizeKind::Match });
	text->set_height({ 0, BoxSizeKind::Match });
	text->set_text_size({ 80 });
	//text->set_origin_x({ 0, BoxOriginKind::kAuto });
	//text->set_origin_y({ 0, BoxOriginKind::kAuto });
	//text->set_rotate(45);
	//text->set_text_line_height({16});
	text->set_text_background_color({ Color(0,255,0) });

	text->set_background_color(Color(255,0,0,255));
	text->set_text_align(TextAlign::Center);
	//text->set_text_family({ app()->font_pool()->getFFID("Helvetica, PingFang SC") });
	text->set_padding_top(20);

	labe->set_text_white_space(TextWhiteSpace::PreWrap);
	labe->set_text_slant(TextSlant::Italic);
	labe->set_text_weight(TextWeight::Bold);
	//labe->set_value("BAC");
	labe->set_value("Quark 1           abcdefghijkmln 禁忌");
	labe->set_text_color({ Color(0,0,0) });
}

void layout_scroll(Window* win) {
	auto box = win->root();
	auto v = box->append_new<Scroll>();

	//v->set_clip(false);
	v->set_width({ 150, BoxSizeKind::Match });
	v->set_height({ 0.5, BoxSizeKind::Match });
	v->set_background_color(Color(255,255,255));
	v->set_border_radius({20});
	v->set_scrollbar_width(6);

	auto a = v->append_new<Box>();
	a->set_margin_top(10);
	a->set_width({ 0, BoxSizeKind::Match });
	a->set_height({ 100 });
	a->set_background_color(Color(255,0,0));

	auto b = v->append_new<Box>();
	b->set_margin_top(10);
	b->set_width({ 0, BoxSizeKind::Match });
	b->set_height({ 100 });
	b->set_background_color(Color(0,255,0));

	auto c = v->append_new<Box>();
	c->set_margin_top(10);
	c->set_width({ 0.5, BoxSizeKind::Ratio });
	c->set_height({ 100 });
	c->set_background_color(Color(0,0,255));

	auto d = v->append_new<Box>();
	d->set_margin_top(10);
	d->set_width({ 0.5, BoxSizeKind::Ratio });
	d->set_height({ 100 });
	d->set_background_color(Color(0,255,255));

	auto e = v->append_new<Box>();
	e->set_margin_top(10);
	e->set_width({ 0, BoxSizeKind::Match });
	e->set_height({ 100 });
	e->set_background_color(Color(0,255,0));

	auto f = v->append_new<Box>();
	f->set_margin_top(10);
	f->set_width({ 0, BoxSizeKind::Match });
	f->set_height({ 100 });
	f->set_background_color(Color(0,0,255));
	
	auto g = v->append_new<Box>();
	g->set_margin_top(10);
	g->set_width({ 0, BoxSizeKind::Match });
	g->set_height({ 100 });
	g->set_background_color(Color(255,0,255));
}

void layout_input(Window* win) {
	auto box = win->root();
	auto input = box->append_new<Textarea>();
	//auto input = (Input*)New<Input>()->append_to(box);

	input->set_width({ 200 });
	input->set_height({ 150 });
	input->set_border({ {1,Color(100,100,100,255)} });
	input->set_background_color(Color(255,255,255));
	// input->set_text_line_height({ 40 });
	input->set_text_align(TextAlign::Center);
	//input->set_text_align(TextAlign::RIGHT);
	input->set_padding_left(4);
	input->set_padding_right(4);
	input->set_placeholder("Placeholder..");
	input->set_text_background_color({Color(255,0,0)});
	input->set_text_color({Color(255,255,255)});
	input->set_text_line_height({20});
 	input->set_text_weight(TextWeight::Bold);
	input->set_scrollbar_width(5);
	//input->set_readonly(true);
	//input->set_text_value("ABCDEFG AA");
}

void layout(Window* win) {
	auto r = win->root();
	auto flex = r->append_new<Flex>();
	auto flow = r->append_new<Flow>();
	auto img2 = r->append_new<Image>();

	flex->set_background(
		//(new FillImage(fs_resources("bench/img/99.jpeg"), {
		//	.size_x={100, FillSizeKind::kPixel},
		//	.position_x={0, FillPositionKind::kCenter},
		//	.position_y={0, FillPositionKind::kCenter},
		//}))
		//->set_next(
		// (new FillGradientRadial({0,0.5,1}, {{255,0,0,255},{0,0,255,255},{0,255,255,255}}))
		// ->set_next(
		//new FillGradientLinear(0, {0,0.5,1}, {{255,0,0,255},{0,0,255,255},{0,255,255,255}})
			0
		// )
		// )
	);
	flex->set_background_color({0,0,0,255});
	// flex->set_box_shadow(new BoxShadow(10, 10, 5, {0,0,0,255}));
	flex->set_width({ 0, BoxSizeKind::Match });
	flex->set_height({ 180, BoxSizeKind::Value });
	flex->set_margin_left(11.5);
	flex->set_margin_top(10);
	flex->set_margin_right(11.5);
	// flex->set_margin_bottom(20);

	flex->set_padding_left(20);
	flex->set_padding_top(20);
	flex->set_padding_right(20);
	flex->set_padding_bottom(20);

	flex->set_border_radius_left_top(80);
	flex->set_border_radius_right_top(10);
	flex->set_border_radius_right_bottom(80);
	flex->set_border_radius_left_bottom(40);

	flex->set_border_width_top(40);
	flex->set_border_width_right(10);
	flex->set_border_width_bottom(0);
	flex->set_border_width_left(40);

	flex->set_border_color_top({0,0,255,255});
	flex->set_border_color_right({255,0,100,255});
	flex->set_border_color_bottom({0,255,100,255});
	flex->set_border_color_left({255,0,255,255});

	//flex->set_opacity(0.8);
	//flex->set_rotate(10);
	//flex->set_skew(Vec2(0,1));
	//flex->set_translate(Vec2(100, 0));
	//
	flow->set_width({ 50, BoxSizeKind::Value });
	flow->set_height({ 50, BoxSizeKind::Value });
	flow->set_background_color({0,0,0,255});
	flow->set_align(Align::LeftBottom);
	flow->set_margin_left(30);
	flow->set_margin_top(340);
	flow->set_margin_right(10);
	flow->set_margin_bottom(30);
	flow->set_padding_left(50);
	// //
	// img->set_height({ 50, BoxSizeKind::kPixel });
	// img->set_layout_align(Align::kRightBottom);
	// //img->set_src(fs_resources("bench/img2/21.jpeg"));
	// img->set_background_color({255,0,0,255});
	// img->set_margin_left(10);
	// img->set_margin_top(10);
	// img->set_margin_right(10);
	// img->set_margin_bottom(10);
	// img->set_padding_left(50);
	// //img->set_rotate(45);
	//
	//img2->set_src(fs_resources("bench/img/21.jpeg"));
	img2->set_width({0, BoxSizeKind::Match });
	//img2->set_height({0, BoxSizeKind::Match });
	img2->set_align(Align::Center);
	img2->set_margin_left(100);
	img2->set_margin_top(30);
	img2->set_margin_right(100);
	//img2->set_rotate(2);
	//img2->set_origin_x({0.5,OriginKind::kRatio});
	//img2->set_origin_y({0.5,OriginKind::kRatio});
	//img2->set_border_width_right(10);
	//img2->set_border_width_left(10);
	//img2->set_border_width_top(10);
	//img2->set_border_width_bottom(10);
	img2->set_border_color_right({255,0,100,255});
	img2->set_border_color_left({255,0,255,255});
	img2->set_border_color_top({255,0,100,255});
	img2->set_border_color_bottom({255,0,255,255});
	// img2->set_radius_right_bottom(5);

	Qk_DLog("%s, %p\n", "ok test layout", win);
	Qk_DLog("Object size %d", sizeof(Object));
	Qk_DLog("Reference size %d", sizeof(Reference));
	
	Qk_DLog("Notification<UIEvent, UIEventName, Layout> size %d", sizeof(Notification<UIEvent, UIEventName, Reference>));
	Qk_DLog("View size %d", sizeof(View));
	Qk_DLog("Box size %d", sizeof(Box));
	Qk_DLog("Flow size %d", sizeof(Flow));
	Qk_DLog("Flex size %d", sizeof(Flex));
	Qk_DLog("Root size %d", sizeof(Root));
}

void test_layout(int argc, char **argv) {
	App app;
	auto win = Window::Make({.msaa=1});
	app.Qk_On(Load, [](auto e) {
		Qk_Log("Applicatio::onLoad");
	});
	win->activate();
	app.screen()->set_status_bar_style(Screen::kBlack);
	app.defaultTextOptions()->set_text_family({
		app.fontPool()->getFontFamilys("Helvetica, PingFang SC")
	});
	//layout_text(win);
	//layout_scroll(win);
	layout_input(win);
	//layout(win);

	app.run();
}
