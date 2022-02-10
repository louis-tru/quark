
#include <stdio.h>
#include <skia/core/SkICC.h>
#include <flare/app.h>
#include <flare/layout/root.h>
#include <flare/render/render.h>
#include <flare/layout/flex.h>
#include <flare/layout/image.h>
#include <flare/fill.h>
#include <flare/display.h>
#include <flare/util/fs.h>

using namespace flare;

namespace flare {
	SkImage* CastSkImage(ImageSource* img);
	SkRect MakeSkRectFrom(Box *host);
}

void draw_skia(SkCanvas* canvas) {
	canvas->clear(SK_ColorWHITE);

	SkPaint paint;
	paint.setStyle(SkPaint::kFill_Style);
	paint.setAntiAlias(true);
	paint.setStrokeWidth(4);
	paint.setColor(0xFFFF0000);

	// ------------------------- drawRect -------------------------
	SkRect rect = SkRect::MakeXYWH(10, 10, 300, 300);
	// canvas->drawRect(rect, paint);

	// ------------------------- drawRRect -------------------------
	SkRRect rrect;// = SKRRect::MakeRect(rect);
	SkVector radii[4] = {{80,80},{80,80},{80,80},{20,20}/*left-bottom*/};
	rrect.setRectRadii(rect, radii);
	paint.setColor(0xFFFF0000);
	// paint.setStyle(SkPaint::kStroke_Style);
	paint.setStrokeWidth(20);
	canvas->drawRRect(rrect, paint);

	// ------------------------- drawDRRect -------------------------
	SkRRect rrect0, rrect1;
	SkVector radii0[4] = {{50,50},{50,50},{50,50},{50,50}};
	SkVector radii1[4] = {{40,40},{40,40},{40,40},{40,40}};
	rrect0.setRectRadii(SkRect::MakeXYWH(450, 10, 240, 260), radii0);
	rrect1.setRectRadii(SkRect::MakeXYWH(460, 20, 220, 240), radii1);
	paint.setColor(0xff0000ff);
	paint.setStyle(SkPaint::kFill_Style);
	//paint.setStyle(SkPaint::kStroke_Style);
	//paint.setStrokeWidth(1);
	canvas->drawDRRect(rrect0, rrect1, paint);

	// ------------------------- drawCircle -------------------------
	paint.setColor(0xff0000ff);
	canvas->drawCircle(180, 440, 80, paint);

	// ------------------------- drawArc -------------------------
	paint.setColor(0xff000000);
	paint.setStyle(SkPaint::kStroke_Style);
	canvas->drawArc(SkRect::MakeXYWH(480, 360, 160, 200), 0, 220, true, paint);
	
}

class FillImageTest: public FillImage {
	public:
	FillImageTest(cString& src): FillImage(src) {}
	virtual void draw(Box* host, Canvas* canvas, uint8_t alpha, bool full) override {
		if (full) {
			auto src = source();
			if (src && src->ready()) {
				auto img = CastSkImage(src);
				auto rect = MakeSkRectFrom(host);
				SkSamplingOptions opts(SkFilterMode::kLinear, SkMipmapMode::kNearest);

				// canvas->drawImageRect(img, rect, opts);

				auto img2 = img->makeTextureImage(render()->direct(), GrMipmapped::kYes);

				canvas->drawImageRect(img, SkRect::MakeXYWH(0, 230, 145, 110),
															SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone));
				canvas->drawImageRect(img, SkRect::MakeXYWH(150, 230, 145, 110),
															SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest));
				canvas->drawImageRect(img2, SkRect::MakeXYWH(0, 345, 145, 110),
															SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone));
				canvas->drawImageRect(img2, SkRect::MakeXYWH(150, 345, 145, 110),
															SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear));
			}
		}
		if (_next)
			_next->draw(host, canvas, alpha, full);
	}
};

class ImageTest: public Image {
	public:
	virtual void draw(Canvas* canvas, uint8_t alpha) override {
		auto src = source();
		if (src && src->ready()) {
			canvas->setMatrix(matrix());

			auto begin = Vec2(padding_left(), padding_top()) - transform_origin(); // begin
			auto end = client_size() - begin; // end
			
			auto img = CastSkImage(src);
			SkRect rect = {begin.x(), begin.y(), end.x(), end.y()};
			SkSamplingOptions opts(SkFilterMode::kLinear, SkMipmapMode::kNearest);

			canvas->drawImageRect(img, rect, opts);

			if (fill()) {
				fill()->draw(this, canvas, alpha, false);
			}
			View::draw(canvas, alpha);
		} else {
			Box::draw(canvas, alpha);
		}
	}
};

void layout(Application* app) {
	
	app->display()->set_status_bar_style(Display::STATUS_BAR_STYLE_BLACK);
	
	auto r = Root::create();
	auto flex = (FlexLayout*)New<FlexLayout>()->append_to(r);
	auto flow = (FlowLayout*)New<FlowLayout>()->append_to(r);
	auto img  = (Image*)New<ImageTest>()->append_to(r);

	New<Box>()->append_to(flex);
	New<Box>()->append_to(flex);
	New<Box>()->append_to(flow);
	New<Box>()->append_to(flow);

	//
	flex->set_width({ 0, SizeType::MATCH });
	flex->set_height({ 180, SizeType::PIXEL });
	
	auto fill = New<FillImage>(Path::resources("bench/img/21.jpeg"));
	
	fill->set_position_x({0, FillPositionType::CENTER});
	fill->set_position_y({0, FillPositionType::CENTER});
	fill->set_size_x({200, FillSizeType::PIXEL});
	//fill->set_size_y({100, FillSizeType::PIXEL});
	//fill->set_repeat(Repeat::REPEAT_Y);

	flex->set_fill(New<FillColor>(Color(255,0,0,255))->set_next(fill->set_next(
								 New<FillImageTest>(Path::resources("bench/img/99.jpeg"))
	)));
	flex->set_margin_left(10);
	flex->set_margin_top(20);
	flex->set_margin_right(10);
	flex->set_margin_bottom(20);
	flex->set_padding_left(20);
	flex->set_padding_top(20);
	flex->set_padding_right(20);
	flex->set_padding_bottom(20);
	//
	flow->set_width({ 50, SizeType::PIXEL });
	flow->set_height({ 50, SizeType::PIXEL });
	flow->set_fill(New<FillColor>(Color(255,0,0,255)));
	flow->set_layout_align(Align::LEFT_BOTTOM);
	flow->set_margin_left(10);
	flow->set_margin_top(10);
	flow->set_margin_right(10);
	flow->set_margin_bottom(10);
	// flow->set_padding_left(50);
	//
	img->set_height({ 50, SizeType::PIXEL });
	img->set_layout_align(Align::RIGHT_BOTTOM);
	img->set_src(Path::resources("bench/img2/21.jpeg"));
	img->set_fill(New<FillColor>(Color(255,0,0,255)));
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

void onload_handle(Event<>& evt, Application* app) {
	
	layout(app);

	app->render()->post_message(Cb([app](CbData&data){
		draw_skia(app->render()->canvas());
		app->render()->submit();
	}));

}

void test_skia(int argc, char **argv) {
	Application app;
	app.F_On(Load, onload_handle, &app);
	app.run(true);
}
