
#include <quark/app.h>
#include <quark/layout/root.h>
#include <quark/render/render.h>
#include <quark/layout/flow.h>
#include <quark/layout/image.h>
#include <quark/layout/label.h>
#include <quark/layout/text.h>
#include <quark/layout/input.h>
#include <quark/layout/textarea.h>
#include <quark/filter.h>
#include <quark/display.h>
#include <quark/util/fs.h>
#include <quark/render/font/pool.h>

using namespace qk;

class TestImage: public Image {
public:
	TestImage(App *host): Image(host) {}

//	virtual void accept(ViewVisitor *visitor) override {
//		auto render = static_cast<SkiaRender*>(visitor);
//		auto src = source();
//
//		render->solveBox(this, src && src->ready() ? [](SkiaRender* render, Box* box, int &clip) {
//			Image* v = static_cast<Image*>(box);
//			auto img = CastSkImage(v->source());
//			auto canvas = render->getCanvas();
//			auto rect = MakeSkRectFrom(box);
//			SkPaint paint;
//			paint.setAlpha(200);
//			canvas->drawImageRect(img, rect, SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest), &paint);
//
//			auto img2 = img;//img->makeTextureImage(render()->direct(), GrMipmapped::kYes);
//			/*
//			canvas->drawImageRect(img, SkRect::MakeXYWH(10, 10, 145, 110),
//														SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone));
//			canvas->drawImageRect(img, SkRect::MakeXYWH(160, 10, 145, 110),
//														SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest));
//			canvas->drawImageRect(img2, SkRect::MakeXYWH(10, 140, 145, 110),
//														SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone));
//			canvas->drawImageRect(img2, SkRect::MakeXYWH(160, 140, 145, 110),
//														SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear));
//			*/
//		}: nullptr);
//
//		auto r = app()->root();
//		auto fill = static_cast<FillGradientLinear*>(static_cast<Box*>(r->first())->fill());
//		fill->set_angle(fill->angle() + 2);
//	}
};

void layout_text(Box* box) {
	auto text = box->append_new<TextLayout>();
	auto labe = text->append_new<Label>();

	text->set_width({ 0, SizeKind::kMatch });
	text->set_height({ 0, SizeKind::kMatch });
	text->set_text_size({ 80 });
	text->set_origin_x({ 0, BoxOriginKind::kAuto });
	text->set_origin_y({ 0, BoxOriginKind::kAuto });
	//text->set_rotate(45);
	//text->set_text_line_height({16});
	text->set_text_background_color({ Color(0,255,0) });

	text->set_background_color(Color(255,0,0,255));
	text->set_text_align(TextAlign::kCenter);
	//text->set_text_family({ app()->font_pool()->getFFID("Helvetica, PingFang SC") });
	text->set_padding_top(20);

	labe->set_text_white_space(TextWhiteSpace::kPreWrap);
	labe->set_text_slant(TextSlant::kItalic);
	labe->set_text_weight(TextWeight::kBold);
  //labe->set_text_value("楚文学");
	labe->set_text_value("BAC");
	//labe->set_text_value("ABC  DEFG楚");
	//labe->set_text_value("Quark 1           abcdefghijkmln 禁忌");
	//labe->set_text_value("Quark 1           abcdefghijkmln 禁忌学");
	labe->set_text_color({ Color(0,0,0) });
}

void layout_scroll(Box *box) {
	auto v = box->append_new<Scroll>();
	//v->set_is_clip(false);
	
	v->set_width({ 200 });
	v->set_height({ 150 });
	v->set_padding_left(10);
	v->set_padding_right(10);
	v->set_padding_bottom(10);
	v->set_background_color(Color(255,255,255));
	v->set_radius_left_top(5);
	v->set_radius_right_top(5);
	v->set_radius_left_bottom(5);
	v->set_radius_right_bottom(5);

	auto a = v->append_new<Box>();
	a->set_margin_top(10);
	a->set_width({ 0, SizeKind::kMatch });
	a->set_height({ 100 });
	a->set_background_color(Color(255,0,0));

	auto b = v->append_new<Box>();
	b->set_margin_top(10);
	b->set_width({ 0, SizeKind::kMatch });
	b->set_height({ 100 });
	b->set_background_color(Color(0,255,0));

	auto c = v->append_new<Box>();
	c->set_margin_top(10);
	c->set_width({ 0.5, SizeKind::kRatio });
	c->set_height({ 100 });
	c->set_background_color(Color(0,0,255));

	auto d = v->append_new<Box>();
	d->set_margin_top(10);
	d->set_width({ 0.5, SizeKind::kRatio });
	d->set_height({ 100 });
	d->set_background_color(Color(0,255,255));

	auto e = v->append_new<Box>();
	e->set_margin_top(10);
	e->set_width({ 0, SizeKind::kMatch });
	e->set_height({ 100 });
	e->set_background_color(Color(0,255,0));

	auto f = v->append_new<Box>();
	f->set_margin_top(10);
	f->set_width({ 0, SizeKind::kMatch });
	f->set_height({ 100 });
	f->set_background_color(Color(0,0,255));
	
	auto g = v->append_new<Box>();
	g->set_margin_top(10);
	g->set_width({ 0, SizeKind::kMatch });
	g->set_height({ 100 });
	g->set_background_color(Color(255,0,255));
	
}

void layout_input(Box* box) {
	auto input = box->append_new<Textarea>();
	//auto input = (Input*)New<Input>()->append_to(box);

	input->set_width({ 200 });
	input->set_height({ 150 });
	input->set_background_color(Color(255,255,255));
	// input->set_text_line_height({ 40 });
	input->set_text_align(TextAlign::kCenter);
	//input->set_text_align(TextAlign::RIGHT);
	input->set_padding_left(4);
	input->set_padding_right(4);
	input->set_placeholder("Placeholder..");
	input->set_text_background_color({Color(255,0,0)});
	input->set_text_color({Color(255,255,255)});
	input->set_text_line_height({20});
 	input->set_text_weight(TextWeight::kBold);
	input->set_scrollbar_width(5);
	//input->set_readonly(true);
	//input->set_text_value("ABCDEFG AA");
}

void layout(Event<>& evt, Application* app) {
	app->display()->set_status_bar_style(Display::STATUS_BAR_STYLE_BLACK);
	app->default_text_options()->set_text_family({ app->font_pool()->getFFID("Helvetica, PingFang SC") });

	auto r = app->root();
	auto flex = r->append_new<FlexLayout>();
	auto flow = r->append_new<FlowLayout>();
	//auto img  = r->append_new<Image>();
	auto img2 = r->append_new<TestImage>();

	//layout_text(r);
	//layout_text(flow);
	//layout_input(flex);
	//layout_scroll(flex);

	flex->set_background(
		//(new FillImage(fs_resources("bench/img/99.jpeg"), {
		//	.size_x={100, FillSizeKind::kPixel},
		//	.position_x={0, FillPositionKind::kCenter},
		//	.position_y={0, FillPositionKind::kCenter},
		//}))
		//->set_next(
		// (new FillGradientRadial({0,0.5,1}, {{255,0,0,255},{0,0,255,255},{0,255,255,255}}))
		// ->set_next(
			new FillGradientLinear(0, {0,0.5,1}, {{255,0,0,255},{0,0,255,255},{0,255,255,255}})
			// 0
		// )
		// )
	);
	// flex->set_background_color({0,0,0,255});
	// flex->set_box_shadow(new BoxShadow(10, 10, 5, {0,0,0,255}));
	flex->set_width({ 0, SizeKind::kMatch });
	flex->set_height({ 180, SizeKind::kPixel });
	flex->set_margin_left(11.5);
	flex->set_margin_top(10);
	flex->set_margin_right(11.5);
	// flex->set_margin_bottom(20);

	flex->set_padding_left(20);
	flex->set_padding_top(20);
	flex->set_padding_right(20);
	flex->set_padding_bottom(20);

	flex->set_radius_left_top(80);
	flex->set_radius_right_top(10);
	flex->set_radius_right_bottom(80);
	flex->set_radius_left_bottom(40);

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
	flow->set_width({ 50, SizeKind::kPixel });
	flow->set_height({ 50, SizeKind::kPixel });
	flow->set_background_color({0,0,255,255});
	flow->set_layout_align(Align::kLeftBottom);
	flow->set_margin_left(10);
	flow->set_margin_top(340);
	flow->set_margin_right(10);
	flow->set_margin_bottom(10);
	flow->set_padding_left(50);
	// //
	// img->set_height({ 50, SizeKind::kPixel });
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
	img2->set_width({0, SizeKind::kMatch });
	//img2->set_height({0, SizeKind::kMatch });
	img2->set_layout_align(Align::kCenter);
	img2->set_margin_left(100);
	img2->set_margin_top(30);
	img2->set_margin_right(100);
	img2->set_rotate(2);
	img2->set_origin_x({0.5,OriginKind::kRatio});
	img2->set_origin_y({0.5,OriginKind::kRatio});
	//img2->set_border_width_right(10);
	//img2->set_border_width_left(10);
	//img2->set_border_width_top(10);
	//img2->set_border_width_bottom(10);
	img2->set_border_color_right({255,0,100,255});
	img2->set_border_color_left({255,0,255,255});
	img2->set_border_color_top({255,0,100,255});
	img2->set_border_color_bottom({255,0,255,255});
	// img2->set_radius_right_bottom(5);

	Qk_DEBUG("%s, %p\n", "ok test layout", app);
	Qk_DEBUG("Object size %d", sizeof(Object));
	Qk_DEBUG("Reference size %d", sizeof(Reference));
	Qk_DEBUG("Layout size %d", sizeof(Layout));
	Qk_DEBUG("Notification<UIEvent, UIEventName, Layout> size %d", sizeof(Notification<UIEvent, UIEventName, Layout>));
	Qk_DEBUG("View size %d", sizeof(View));
	Qk_DEBUG("Box size %d", sizeof(Box));
	Qk_DEBUG("FlowLayout size %d", sizeof(FlowLayout));
	Qk_DEBUG("FlexLayout size %d", sizeof(FlexLayout));
	Qk_DEBUG("Root size %d", sizeof(Root));
}

void test_layout(int argc, char **argv) {
	Application::Options opts{.msaaSampleCnt=1};
	Application app(opts);
	app.Qk_On(Load, layout, &app);
	app.run();
}
