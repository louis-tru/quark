
#include <stdio.h>
#include <skia/core/SkImage.h>
#include <skia/core/SkSurface.h>
#include <skia/core/SkCanvas.h>
#include <flare/app.h>
#include <flare/layout/root.h>
#include <flare/render/render.h>
#include <flare/render/skia/skia_render.h>
#include <flare/layout/flex.h>
#include <flare/layout/image.h>
#include <flare/effect.h>
#include <flare/display.h>
#include <flare/util/fs.h>
#include <vector>
#include <skia/core/SkFont.h>
#include <skia/core/SkMaskFilter.h>
#include <skia/effects/SkDashPathEffect.h>

using namespace flare;

namespace flare {
	SkImage* CastSkImage(ImageSource* img);
}

class ImageTest: public Image {
public:
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
			auto rect = render->MakeSkRectFrom(box);
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
	}
};

void layout(Application* app) {
	
	app->display()->set_status_bar_style(Display::STATUS_BAR_STYLE_BLACK);
	
	auto r = Root::create();
	auto flex = (FlexLayout*)New<FlexLayout>()->append_to(r);
	auto flow = (FlowLayout*)New<FlowLayout>()->append_to(r);
	auto img  = (Image*)     New<Image>     ()->append_to(r);
	auto img2 = (Image*)     New<ImageTest> ()->append_to(r);

	flex->set_fill_color(Color(255,0,0,255));
	flex->set_fill(New<FillImage>(Path::resources("bench/img/21.jpeg"), FillImage::Init{
		.size_x={200, FillSizeType::PIXEL}, //.size_y={100, FillSizeType::PIXEL},
		.position_x={0, FillPositionType::CENTER},
		.position_y={0, FillPositionType::CENTER},
		.repeat=Repeat::REPEAT,
	}));
	flex->set_width({ 0, BoxSizeType::MATCH });
	flex->set_height({ 180, BoxSizeType::PIXEL });
	flex->set_margin_left(10);
	flex->set_margin_top(20);
	flex->set_margin_right(10);
	flex->set_margin_bottom(20);
	flex->set_padding_left(20);
	flex->set_padding_top(20);
	flex->set_padding_right(20);
	flex->set_padding_bottom(20);
	flex->set_radius_left_top(40);
	flex->set_radius_right_top(10);
	flex->set_radius_right_bottom(40);
	flex->set_radius_left_bottom(10);
	flex->set_opacity(0.5);
	//
	flow->set_width({ 50, BoxSizeType::PIXEL });
	flow->set_height({ 50, BoxSizeType::PIXEL });
	flow->set_fill_color(Color(0,0,255,255));
	flow->set_layout_align(Align::LEFT_BOTTOM);
	flow->set_margin_left(10);
	flow->set_margin_top(10);
	flow->set_margin_right(10);
	flow->set_margin_bottom(10);
	flow->set_padding_left(50);
	//
	img->set_height({ 50, BoxSizeType::PIXEL });
	img->set_layout_align(Align::RIGHT_BOTTOM);
	img->set_src(Path::resources("bench/img2/21.jpeg"));
	img->set_fill_color(Color(255,0,0,255));
	img->set_margin_left(10);
	img->set_margin_top(10);
	img->set_margin_right(10);
	img->set_margin_bottom(10);
	img->set_padding_left(50);
	//
	//img2->set_src(Path::resources("bench/img/99.jpeg"));
	img2->set_width({0, BoxSizeType::MATCH });
	img2->set_layout_align(Align::CENTER);
	
	F_DEBUG("%s, %p\n", "ok skia", app);
	F_DEBUG("Object size %d", sizeof(Object));
	F_DEBUG("Reference size %d", sizeof(Reference));
	F_DEBUG("Layout size %d", sizeof(Layout));
	F_DEBUG("Notification<UIEvent, UIEventName, Layout> size %d", sizeof(Notification<UIEvent, UIEventName, Layout>));
	F_DEBUG("View size %d", sizeof(View));
	F_DEBUG("Box size %d", sizeof(Box));
	F_DEBUG("FlowLayout size %d", sizeof(FlowLayout));
	F_DEBUG("FlexLayout size %d", sizeof(FlexLayout));
	F_DEBUG("Root size %d", sizeof(Root));
}

void test_layout(int argc, char **argv) {
	Application app;
	layout(&app);
	app.run(true);
}
