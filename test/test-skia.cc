
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
#include <vector>
#include <skia/core/SkFont.h>
#include <skia/core/SkMaskFilter.h>
#include <skia/effects/SkDashPathEffect.h>

using namespace flare;

namespace flare {
	SkImage* CastSkImage(ImageSource* img);
	SkRect MakeSkRectFrom(Box *host);
}

void draw_skia(SkCanvas* canvas) {
	canvas->clear(SK_ColorWHITE);
	
	std::vector<int32_t> test(100);
	
	auto a = SkColorGetA(0x00);
	
	SkBitmap bitmap;
	bitmap.allocN32Pixels(100, 100);
	SkCanvas offscreen(bitmap);
	
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

void testBlur(SkCanvas* canvas) {
	SkAutoCanvasRestore res(canvas, true);
	canvas->translate(300, 700);
	//canvas->scale(2, 2);
	SkPaint paint;
	paint.setColor(SK_ColorBLUE);
	paint.setMaskFilter(SkMaskFilter::MakeBlur(kSolid_SkBlurStyle, 20));
	canvas->drawRect(SkRect::MakeXYWH(40, 40, 175, 175), paint);
}

void testBorder(SkCanvas* canvas) {
	SkAutoCanvasRestore res(canvas, true);
	canvas->translate(500, 700);
	canvas->scale(2, 2);
	SkPaint paint;
	paint.setAntiAlias(true);
	paint.setStyle(SkPaint::kStroke_Style);
	paint.setStrokeWidth(4);
	paint.setColor(SK_ColorRED);
	SkRect oval = { 4, 4, 60, 60};
	float intervals[] = { 5, 5 };
	paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 2.5f));
	for (auto degrees : { 270, 360, 540, 720 } ) {
		canvas->drawArc(oval, 0, degrees, false, paint);
		canvas->translate(64, 0);
	}
}

void testNotifyPixelsChanged(SkCanvas* canvas) {
	SkAutoCanvasRestore res(canvas, true);

	canvas->translate(300, 550);
	
//	SkRSXform xforms;

	SkBitmap bitmap;
	bitmap.setInfo(SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kOpaque_SkAlphaType));
	bitmap.allocPixels();
	bitmap.eraseColor(SK_ColorRED);

	canvas->scale(64, 64);
	canvas->drawImage(bitmap.asImage(), 0, 0);

	*(SkPMColor*) bitmap.getPixels() = SkPreMultiplyColor(SK_ColorBLUE);
	canvas->drawImage(bitmap.asImage(), 2, 0);

	bitmap.notifyPixelsChanged(); // 好像并没有什么用处,既然都使用了引用内存,只要修改像素内容所有引用些对像都会受影响
	
	//bitmap.setIsVolatile(true);

	*(SkPMColor*) bitmap.getPixels() = (SK_ColorGREEN);

	canvas->drawImage(bitmap.asImage(), 4, 0);
}

void testExtractAlphaBlur(SkCanvas* canvas) {
	auto radiusToSigma = [](SkScalar radius) -> SkScalar {
		static const SkScalar kBLUR_SIGMA_SCALE = 0.57735f;
		return radius > 0 ? kBLUR_SIGMA_SCALE * radius + 0.5f : 0.0f;
	};

	SkBitmap alpha, bitmap;
	bitmap.allocN32Pixels(100, 100);
	SkCanvas offscreen(bitmap);
	//offscreen.clear(0);

	SkPaint paint;
	paint.setAntiAlias(true);
	paint.setColor(SK_ColorBLUE);
	paint.setStyle(SkPaint::kStroke_Style);
	paint.setStrokeWidth(20);
	offscreen.drawCircle(50, 50, 39, paint);
	//offscreen.flush();

	paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, radiusToSigma(10))); // software blur filter
	SkIPoint offset;

	bitmap.extractAlpha(&alpha, &paint, &offset);
	paint.setColor(SK_ColorRED);

	paint.setMaskFilter(nullptr);

	SkAutoCanvasRestore res(canvas, true);

	canvas->translate(300, 300);
	canvas->scale(2, 2);

	canvas->drawImage(bitmap.asImage(), 50, 0, SkSamplingOptions(), &paint);
	canvas->drawImage(alpha.asImage(), 100 + offset.fX, 0, SkSamplingOptions(), &paint);
}

void test(SkCanvas* canvas) {
	SkBitmap bitmap;
	bitmap.setInfo(SkImageInfo::MakeN32(16, 16, kPremul_SkAlphaType));
	SkDebugf("pixel address = %p\n", bitmap.getPixels());
	SkBitmap::HeapAllocator stdalloc;
	if (!stdalloc.allocPixelRef(&bitmap)) {
			SkDebugf("pixel allocation failed\n");
	} else {
			SkDebugf("pixel address = %p\n", bitmap.getPixels());
	}
	
	//kNoPremul_SkAlphaType
	
	SkPaint paint;
	paint.setAntiAlias(true);
	paint.setColor(SK_ColorGREEN);
	SkFont font(nullptr, 128);
	for (SkScalar sx : { -1, 1 } ) {
			for (SkScalar sy : { -1, 1 } ) {
					SkAutoCanvasRestore autoRestore(canvas, true);
					SkMatrix m = SkMatrix::MakeAll(sx, 1, 300,    0, sy, 300,   0, 0, 1);
					canvas->concat(m);
					canvas->drawString("R", 0, 0, font, paint);
			}
	}
	
	uint8_t set1[5] = { 0xCA, 0xDA, 0xCA, 0xC9, 0xA3 };
	uint8_t set2[5] = { 0xAC, 0xA8, 0x89, 0x47, 0x87 };
	SkBitmap bitmap2;
	// not hold data for installPixels
	bitmap2.installPixels(SkImageInfo::Make(5, 1, kGray_8_SkColorType, kOpaque_SkAlphaType), set1, 5);

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
			
			test(canvas);
			testExtractAlphaBlur(canvas);
			testNotifyPixelsChanged(canvas);
			testBorder(canvas);
			testBlur(canvas);

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
