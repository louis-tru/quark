
#include <stdio.h>
#include <quark/app.h>
#include <quark/layout/root.h>
#include <quark/render/skia/skia_canvas.h>
#include <quark/render/skia/skia_render.h>
#include <quark/render/render.h>
#include <quark/layout/flex.h>
#include <quark/layout/image.h>
#include <quark/effect.h>
#include <quark/display.h>
#include <quark/util/fs.h>
#include <vector>
#include <skia/core/SkImage.h>
#include <skia/core/SkFont.h>
#include <skia/core/SkMaskFilter.h>
#include <skia/effects/SkDashPathEffect.h>
#include <skia/core/SkBitmap.h>
#include <skia/core/SkPath.h>
#include <quark/render/scaner.h>

using namespace quark;

void testRenderScaner(SkCanvas* canvas) {
	
	SkBitmap bitmapCircle;
	bitmapCircle.allocPixels(SkImageInfo::MakeN32(640, 640, kPremul_SkAlphaType));
	//bitmapCircle.eraseColor(SK_ColorBLUE);
	SkCanvas offcanvas(bitmapCircle);
	//offcanvas.clear(0xff00ff00);

	SkPath oval = SkPath::Oval(SkRect::MakeXYWH(.5, .9, 320, 320), SkPathDirection::kCCW);
	Array<uint8_t> verbs(oval.countVerbs());
	oval.getVerbs(&verbs[0], verbs.length());
	
	auto pixmap = bitmapCircle.pixmap();
	
	PathLine oval2 = PathLine::Oval({ Vec2(.5,.9), Vec2(320, 320) });
	XLineScaner scaner(oval2, {}, 1, true);
	
	scaner.scan([](int left, int right, int y, void* ctx) {
		SkPixmap* pixmap = (SkPixmap*)ctx;
		int x1 = (left + (left & 0x8000)) >> 16;
		int x2 = (right + (right & 0x8000)) >> 16;
		Qk_DEBUG("%f, %f, %d, %d, %d", left / 65536.0, right / 65536.0, x1, x2, y);

		uint32_t red = 0xff0000ff;

		uint32_t *addr = (uint32_t*)pixmap->addr32() + (y * 640 + x1);
		
		memset_pattern4(addr, &red, (x2 - x1) << 2);
	}, &pixmap);
	
	canvas->drawImage(bitmapCircle.asImage(), 600, 10);

	// offcanvas
	SkPaint paint;
	//paint.setAntiAlias(true);
	paint.setStyle(SkPaint::kFill_Style);
	paint.setColor(0x8800ff00);
	paint.setColor(0xffff0000);
	canvas->save();
	canvas->translate(100, 10);
	canvas->drawPath(oval, paint);
	canvas->restore();
}

void testScaner(Application* app) {
	app->render()->post_message(Cb([app](CbData&data) {
		auto render = static_cast<SkiaRender*>(app->render()->visitor());
		auto canvas = render->getCanvas();
		canvas->clear(SK_ColorWHITE);
		testRenderScaner(canvas);
		app->render()->submit();
	}));
}

void test_scaner(int argc, char **argv) {
	Application app;
	app.Qk_On(Load, [&](Event<>& evt) { testScaner(&app); });
	app.display()->Qk_On(Change, [&app](Event<>& evt){ testScaner(&app); });
	app.run(true);
}
