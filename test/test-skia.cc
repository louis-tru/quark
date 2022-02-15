
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
#include <skia/core/SkBitmap.h>
#include <skia/core/SkPath.h>

using namespace flare;

namespace flare {
	SkImage* CastSkImage(ImageSource* img);
	SkRect MakeSkRectFrom(Box *host);
}

void draw_skia(SkCanvas* canvas) {
	canvas->clear(SK_ColorWHITE);
	
	SkBitmap bitmap;
	bitmap.allocN32Pixels(100, 100);
	SkCanvas offscreen(bitmap);
	
	SkPaint paint;
	paint.setStyle(SkPaint::kFill_Style);
	//paint.setAntiAlias(true);
	paint.setStrokeWidth(4);
	paint.setColor(0xFFFF0000);

	
	// ------------------------- drawCircle -------------------------
	SkBitmap bitmapCircle;
	bitmapCircle.allocPixels(SkImageInfo::MakeN32(320, 320, kPremul_SkAlphaType));
	//bitmapCircle.eraseColor(SK_ColorBLUE);
	SkCanvas offcanvas(bitmapCircle);
	//offcanvas.clear(0xff00ff00);
	paint.setColor(0xffff0000);
	SkPath oval = SkPath::Oval(SkRect::MakeWH(320, 320), SkPathDirection::kCCW);
	
	Array<uint8_t> verbs(oval.countVerbs());
	oval.getVerbs(&verbs[0], verbs.length());
	
	F_DEBUG("");
	for (int i = 0; i < oval.countPoints(); i++) {
		F_DEBUG("Point: %f, %f", oval.getPoint(i).fX, oval.getPoint(i).fY);
	}
	for (int i = 0; i < oval.countVerbs(); i++) {
		F_DEBUG("Verb: %d", verbs[i]);
	}
	F_DEBUG("");
	
	offcanvas.drawPath(oval, paint);
	canvas->drawImage(bitmapCircle.asImage(), 600, 30);
	
	return;
	
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
	//canvas->drawRRect(rrect, paint);

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

void testBitmap(SkCanvas* canvas) {
	SkBitmap bitmap;
	bitmap.setInfo(SkImageInfo::MakeN32(16, 16, kPremul_SkAlphaType));
	SkDebugf("pixel address = %p\n", bitmap.getPixels());
	SkBitmap::HeapAllocator stdalloc;
	if (!stdalloc.allocPixelRef(&bitmap)) {
			SkDebugf("pixel allocation failed\n");
	} else {
			SkDebugf("pixel address = %p\n", bitmap.getPixels());
	}
	
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

void testSkia(Application* app) {
	app->render()->post_message(Cb([app](CbData&data){
		auto canvas = app->render()->canvas();
		draw_skia(canvas);
		//testBitmap(canvas);
		//testExtractAlphaBlur(canvas);
		//testNotifyPixelsChanged(canvas);
		//testBorder(canvas);
		//testBlur(canvas);
		app->render()->submit();
	}));
}

void test_skia(int argc, char **argv) {
	Application app;
	
	app.F_On(Load, [&](Event<>& evt) { testSkia(&app); });
	//app.display()->F_On(Orientation, [&app](Event<>& evt){ testSkia(&app); });
	app.display()->F_On(Change, [&app](Event<>& evt){ testSkia(&app); });
	
	//layout(&app);
	
	app.run(true);
}
