
#include <stdio.h>
#include <skia/core/SkImage.h>
#include <skia/core/SkSurface.h>
#include <skia/core/SkCanvas.h>
#include <noug/app.h>
#include <noug/layout/root.h>
#include <noug/render/render.h>
#include <noug/render/skia/skia_render.h>
#include <noug/layout/flow.h>
#include <noug/layout/image.h>
#include <noug/layout/label.h>
#include <noug/layout/text.h>
#include <noug/effect.h>
#include <noug/display.h>
#include <noug/util/fs.h>
#include <noug/render/font/pool.h>
#include <vector>
#include <skia/core/SkFont.h>
#include <skia/core/SkMaskFilter.h>
#include <skia/effects/SkDashPathEffect.h>

using namespace noug;

namespace noug {
	SkImage* CastSkImage(ImageSource* img);
}

class ImageTest: public Image {
public:

	static SkRect MakeSkRectFrom(Box *host) {
		auto begin = host->transform_origin(); // begin
		auto end = host->client_size() - begin; // end
		SkRect _rect_inside = {-begin.x(), -begin.y(), end.x(), end.y()};
		return _rect_inside;
	}

	virtual void accept(ViewVisitor *visitor) override {
		if (visitor->flags() != 0) {
			return visitor->visitImage(this);
		}
		auto render = static_cast<SkiaRender*>(visitor);
		auto src = source();

		render->solveBox(this, src && src->ready() ? [](SkiaRender* render, Box* box) {
			Image* v = static_cast<Image*>(box);
			auto img = CastSkImage(v->source());
			auto canvas = render->getCanvas();
			auto rect = MakeSkRectFrom(box);
			SkPaint paint;
			paint.setAlpha(200);
			canvas->drawImageRect(img, rect, SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest), &paint);

			auto img2 = img;//img->makeTextureImage(render()->direct(), GrMipmapped::kYes);
			/*
			canvas->drawImageRect(img, SkRect::MakeXYWH(10, 10, 145, 110),
														SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone));
			canvas->drawImageRect(img, SkRect::MakeXYWH(160, 10, 145, 110),
														SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest));
			canvas->drawImageRect(img2, SkRect::MakeXYWH(10, 140, 145, 110),
														SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone));
			canvas->drawImageRect(img2, SkRect::MakeXYWH(160, 140, 145, 110),
														SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear));
			*/
		}: nullptr);
		
		auto r = app()->root();
		auto fill = static_cast<FillGradientLinear*>(static_cast<Box*>(r->first())->fill());
		fill->set_angle(fill->angle() + 2);
	}
};

void layout_text(FlowLayout* flow) {
	auto text = (TextLayout*)New<TextLayout>()->append_to(flow);
	auto labe = (Label*)     New<Label>()     ->append_to(text);

	text->set_width({ 0, BoxSizeKind::MATCH });
	// text->set_height({ 0, BoxSizeKind::MATCH });
	text->set_text_size({ 12 });

	text->set_fill_color(Color(255,0,0,255));
	text->set_text_align(TextAlign::CENTER);
	//text->set_text_family({ app()->font_pool()->getFFID("Helvetica, PingFang SC") });

	labe->set_text_white_space(TextWhiteSpace::PRE_WRAP);
	labe->set_text_slant(TextSlant::ITALIC);
	labe->set_text_weight(TextWeight::BOLD);
	//labe->set_text_value("ABC  DEFG楚");
	//labe->set_text_value("Noug 1           abcdefghijkmln 禁忌");
	labe->set_text_value("Noug 1           abcdefghijkmln 禁忌");
	labe->set_text_color({ Color(0,255,255,255) });
}

void layout(Event<>& evt, Application* app) {
	app->display()->set_status_bar_style(Display::STATUS_BAR_STYLE_BLACK);
	app->default_text_options()->set_text_family({ app->font_pool()->getFFID("Helvetica, PingFang SC") });

	auto r = Root::create();
	auto flex = (FlexLayout*)New<FlexLayout>()->append_to(r);
	auto flow = (FlowLayout*)New<FlowLayout>()->append_to(r);
	auto img  = (Image*)     New<Image>     ()->append_to(r);
	auto img2 = (Image*)     New<ImageTest> ()->append_to(r);
	
	layout_text(flow);

	flex->set_fill_color(Color(255,0,0,255));
	//flex->set_fill(New<FillImage>(fs_resources("bench/img/21.jpeg"), FillImage::Init{
	//	.size_x={100, FillSizeType::PIXEL},
	// 	.position_x={0, FillPositionType::CENTER},
	// 	.position_y={0, FillPositionType::CENTER},
	//}));
	//flex->set_fill(new FillGradientRadial({ 0, 0.5, 1 }, { Color(255, 0, 0, 255), Color(0, 0, 255, 255), Color(0, 255, 255, 255) }));
	flex->set_fill(new FillGradientLinear(0, { 0, 0.5, 1 }, { Color(255, 0, 0, 255), Color(0, 255, 0, 255), Color(0, 0, 255, 255) }));
	flex->set_effect(New<BoxShadow>(10, 10, 5, Color(0,0,0,255)));
	//flex->set_effect(New<BoxShadow>(10, 10, 5, Color(255,0,0,255)))
	flex->set_width({ 0, BoxSizeKind::MATCH });
	flex->set_height({ 180, BoxSizeKind::PIXEL });
	flex->set_margin_left(10);
	flex->set_margin_top(10);
	flex->set_margin_right(10);
	flex->set_margin_bottom(20);
	flex->set_padding_left(20);
	flex->set_padding_top(20);
	flex->set_padding_right(20);
	flex->set_padding_bottom(20);
	flex->set_radius_left_top(40);
	flex->set_radius_right_top(20);
	flex->set_radius_right_bottom(40);
	flex->set_radius_left_bottom(40);
	flex->set_border_width_top(10);
	flex->set_border_width_right(20);
	flex->set_border_width_bottom(10);
	flex->set_border_width_left(20);
	flex->set_border_color_top(Color(0,0,255,255));
	flex->set_border_color_right(Color(0,0,255,255));
	flex->set_border_color_bottom(Color(0,0,255,255));
	flex->set_border_color_left(Color(0,0,255,255));
	//flex->set_opacity(0.5);
	//flex->set_rotate(10);
	//flex->set_skew(Vec2(0,1));
	//flex->set_translate(Vec2(100, 0));
	//
	flow->set_width({ 50, BoxSizeKind::PIXEL });
	flow->set_height({ 50, BoxSizeKind::PIXEL });
	flow->set_fill_color(Color(0,0,255,255));
	flow->set_layout_align(Align::LEFT_BOTTOM);
	flow->set_margin_left(10);
	flow->set_margin_top(10);
	flow->set_margin_right(10);
	flow->set_margin_bottom(10);
	flow->set_padding_left(50);
	//
	img->set_height({ 50, BoxSizeKind::PIXEL });
	img->set_layout_align(Align::RIGHT_BOTTOM);
	img->set_src(fs_resources("bench/img2/21.jpeg"));
	img->set_fill_color(Color(255,0,0,255));
	img->set_margin_left(10);
	img->set_margin_top(10);
	img->set_margin_right(10);
	img->set_margin_bottom(10);
	img->set_padding_left(50);
	//img->set_rotate(45);
	//
	//img2->set_src(fs_resources("bench/img/99.jpeg"));
	img2->set_width({0, BoxSizeKind::MATCH });
	img2->set_layout_align(Align::CENTER);
	
	N_DEBUG("%s, %p\n", "ok skia", app);
	N_DEBUG("Object size %d", sizeof(Object));
	N_DEBUG("Reference size %d", sizeof(Reference));
	N_DEBUG("Layout size %d", sizeof(Layout));
	N_DEBUG("Notification<UIEvent, UIEventName, Layout> size %d", sizeof(Notification<UIEvent, UIEventName, Layout>));
	N_DEBUG("View size %d", sizeof(View));
	N_DEBUG("Box size %d", sizeof(Box));
	N_DEBUG("FlowLayout size %d", sizeof(FlowLayout));
	N_DEBUG("FlexLayout size %d", sizeof(FlexLayout));
	N_DEBUG("Root size %d", sizeof(Root));
}

void test_layout(int argc, char **argv) {
	Application app;
	app.N_On(Load, layout, &app);
	app.run(true);
}
