
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
	SkRect MakeSkRectFrom(Box *host);
}

class FillImageTest: public FillImage {
public:
	FillImageTest(cString& src): FillImage(src) {}
	// virtual void draw(Box* host, SkCanvas* canvas, uint8_t alpha, bool full) override {
	// 	if (full) {
	// 		auto src = source();
	// 		if (src && src->ready()) {
	// 			auto img = CastSkImage(src);
	// 			auto rect = MakeSkRectFrom(host);
	// 			SkSamplingOptions opts(SkFilterMode::kLinear, SkMipmapMode::kNearest);

	// 			// canvas->drawImageRect(img, rect, opts);

	// 			auto img2 = img;//img->makeTextureImage(render()->direct(), GrMipmapped::kYes);

	// 			// canvas->getSurface()->dirtyGenerationID()

	// 			canvas->drawImageRect(img, SkRect::MakeXYWH(0, 230, 145, 110),
	// 														SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone));
	// 			canvas->drawImageRect(img, SkRect::MakeXYWH(150, 230, 145, 110),
	// 														SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest));
	// 			canvas->drawImageRect(img2, SkRect::MakeXYWH(0, 345, 145, 110),
	// 														SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone));
	// 			canvas->drawImageRect(img2, SkRect::MakeXYWH(150, 345, 145, 110),
	// 														SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear));
	// 		}

	// 	}
	// 	if (_next)
	// 		_next->draw(host, canvas, alpha, full);
	// }
};

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
			auto begin = Vec2(v->padding_left(), v->padding_top()) - v->transform_origin(); // begin
			auto end = v->client_size() - begin; // end
			auto img = CastSkImage(v->source());
			SkRect rect = {begin.x(), begin.y(), end.x(), end.y()};
			SkSamplingOptions opts(SkFilterMode::kLinear, SkMipmapMode::kNearest);
			render->getCanvas()->drawImageRect(img, rect, opts);
		}: nullptr);
	}
};

void layout(Application* app) {
	
	app->display()->set_status_bar_style(Display::STATUS_BAR_STYLE_BLACK);
	
	auto r = Root::create();
	auto flex = (FlexLayout*)New<FlexLayout>()->append_to(r);
	auto flow = (FlowLayout*)New<FlowLayout>()->append_to(r);
	auto img  = (Image*)     New<ImageTest> ()->append_to(r);

	//	New<Box>()->append_to(flex);
	//	New<Box>()->append_to(flex);
	//	New<Box>()->append_to(flow);
	//	New<Box>()->append_to(flow);

	//
	flex->set_width({ 0, BoxSizeType::MATCH });
	flex->set_height({ 180, BoxSizeType::PIXEL });
	
	auto fill = New<FillImage>(Path::resources("bench/img/21.jpeg"));
	
	fill->set_position_x({0, FillPositionType::CENTER});
	fill->set_position_y({0, FillPositionType::CENTER});
	fill->set_size_x({200, FillSizeType::PIXEL});
	//fill->set_size_y({100, FillSizeType::PIXEL});
	//fill->set_repeat(Repeat::REPEAT_Y);
	//fill->set_next(New<FillImageTest>(Path::resources("bench/img/99.jpeg")));

	flex->set_fill_color(Color(255,0,0,255));
	flex->set_fill(fill);
	flex->set_margin_left(10);
	flex->set_margin_top(20);
	flex->set_margin_right(10);
	flex->set_margin_bottom(20);
	flex->set_padding_left(20);
	flex->set_padding_top(20);
	flex->set_padding_right(20);
	flex->set_padding_bottom(20);
	//
	flow->set_width({ 50, BoxSizeType::PIXEL });
	flow->set_height({ 50, BoxSizeType::PIXEL });
	flow->set_fill_color(Color(255,0,0,255));
	flow->set_layout_align(Align::LEFT_BOTTOM);
	flow->set_margin_left(10);
	flow->set_margin_top(10);
	flow->set_margin_right(10);
	flow->set_margin_bottom(10);
	// flow->set_padding_left(50);
	//
	img->set_height({ 50, BoxSizeType::PIXEL });
	img->set_layout_align(Align::RIGHT_BOTTOM);
	img->set_src(Path::resources("bench/img2/21.jpeg"));
	img->set_fill_color(Color(255,0,0,255));
	img->set_margin_left(10);
	img->set_margin_top(10);
	img->set_margin_right(10);
	img->set_margin_bottom(10);
	// img->set_padding_left(50);
	
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
