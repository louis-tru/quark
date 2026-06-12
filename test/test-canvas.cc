
#include <src/ui/app.h>
#include <src/ui/window.h>
#include <src/ui/screen.h>
#include <src/ui/view/box.h>
#include <src/ui/view/root.h>
#include <src/render/paint.h>
#include <src/util/codec.h>
#include <src/render/render.h>
#include <src/render/canvas.h>
#include <src/render/font/pool.h>
#include <src/ui/text/text_blob.h>
#include "./test.h"

using namespace qk;

#define Qk_TEST_CANVAS(Fn) Fn(2)

void test_gui_new(Box *r);

class TestCanvas0: public Box {
	float i = 0;
	Array<TextBlob> textBlobs;
	View* init(Window* win) override {
		View::init(win);
		test_gui_new(win->root());
		return this;
	}

	void draw(Painter *painter) override {
		auto canvas = window()->render()->getCanvas();
		auto size = window()->size();

		i+=Qk_PI_RATIO_180*0.01;
		float c = abs(sinf(i)) * 5;

		Paint paint;

		// -------- clip ------
		canvas->save();

		if (1) { // clip
			canvas->clipRect({ size*-0.35, size*0.7 }, Canvas::kIntersect_ClipOp, 1);
		}

		if (1) { // gradient
			Paint paint;
			Color4f colors[] = {Color4f(1,0,1), Color4f(0,1,0), Color4f(0,0,1)};
			float   pos[]    = {0,0.5,1};
			Rect           rect{ size*(-0.5*0.8), size*0.8 };
			PaintGradient  gPaint{
				PaintGradient::kRadial_Type,rect.begin+rect.size*0.5, rect.size*0.5, 3, colors, pos
			};
			// paint.antiAlias = false;
			paint.fill.gradient = &gPaint;

			canvas->save();
			canvas->setMatrix(canvas->getMatrix() * Mat({0,0}, {0.8, 0.8}, c, {0.3,0}));
			canvas->drawRect(rect, paint);
			canvas->restore();
		}

		if (1) { // Circle
			paint.fill.gradient = nullptr;
			paint.fill.color = Color4f(0, 0, 1, 0.5);
			canvas->drawPath(Path::MakeCircle(0, 100), paint);
			paint.fill.color = Color4f(1, 0, 0, 0.8);
			canvas->drawPath(Path::MakeOval({0, {100, 200}}), paint);
		}

		if (1) { // -------- clip ------
			auto clip = Path::MakeCircle(0, 105);
			auto aa = 1;
			canvas->clipPath(clip, Canvas::kDifference_ClipOp, aa);
		}

		canvas->translate(size*-0.5);

		if (1) { // polygon
			paint.fill.color = Color4f(0, 0, 0, 0.5);
			Path path(   Vec2(110, size.y() - 150) );
			path.lineTo( Vec2(size.x()*0.5, 0) );
			path.lineTo( size );
			path.close();
			path.moveTo( Vec2(100, 100) );
			path.lineTo( Vec2(200, 180) );
			path.lineTo( Vec2(100, 200) );
			path.close();
			canvas->drawPath(path, paint);
		}

		if (1) { // Arc
			paint.fill.color = Color4f(0, 1, 0, 0.8);
			canvas->drawPath(Path::MakeArc({Vec2(500, 420), Vec2(200, 100)}, 0, 4.5, 1), paint);
			paint.fill.color = Color4f(1, 0, 1, 0.8);
			canvas->drawPath(Path::MakeArc({Vec2(200, 400), Vec2(200, 100)}, 4.5, 4, 0), paint);
			paint.fill.color = Color4f(0, 1, 0, 0.8);
			canvas->drawPath(Path::MakeArc({Vec2(450, 300), Vec2(100, 200)}, Qk_PI_2, Qk_PI_2+Qk_PI*0.5, 1), paint);
		}

		if (1) { // font blob
			paint.fill.color = Color4f(255,0,255);
			paint.stroke.color = Color4f(0,0,0);
			paint.strokeWidth = 5;
			paint.style = Paint::kStroke_Style;
			if (textBlobs.isNull()) {
				TextOptions opts(shared_app()->defaultTextOptions());
				opts.set_font_size({64});
				opts.set_font_weight(FontWeight::Bold);
				textBlobs = TextBlobBuilder::makeTextBlob("A 好 HgKr葵花pjAH", &opts);
			}
			Vec2 origin(10,160);
			for (auto &blob: textBlobs) {
				canvas->drawTextBlob(&blob.blob, origin + Vec2{blob.origin,0}, 64, paint);
				origin[0] += 10;
			}
		}

		if (0) { // font glyphs
			paint.fill.color = Color4f(0,120,120);
			paint.style = Paint::kFill_Style;
			auto stype = FontStyle(FontWeight::Bold, FontWidth::Default, FontSlant::Normal);
			auto pool = shared_app()->fontPool();
			auto unicode = codec_decode_to_unicode(kUTF8_Encoding, "Hello World! 你好，世界！👋🌍");
			auto fgs = pool->getFontFamilies()->makeFontGlyphs(unicode, stype, 64);
			Vec2 origin(10,500);
			for (auto &fg: fgs) {
				origin[0] += ceilf(canvas->drawGlyphs(fg, origin, NULL, paint)) + 10;
			}
		}

		if (1) { // outline
			paint.style = Paint::kStrokeAndFill_Style;
			paint.stroke.color = Color4f(0,0,0,0.3);
			paint.strokeWidth = 8;
			paint.fill.color = Color4f(0.5,0,0.3);
			canvas->drawPath(Path::MakeRRect({ {180,150}, 200 }, {50, 80, 50, 80}), paint);
			paint.style = Paint::kStroke_Style;
			paint.fill.color = Color4f(0, 1, 1);
			paint.stroke.color = Color4f(0,0,0);
			paint.strokeWidth = 0.5;
			canvas->drawPath(Path::MakeRRectOutline({ {400,100}, 200 }, { {440,140}, 120 }, {50, 80, 50, 80}), paint);
			paint.stroke.color = Color4f(1, 1, 0);
			paint.style = Paint::kStroke_Style;
			paint.strokeWidth = 10;
			canvas->drawPath(Path::MakeCircle(Vec2(500,400), 100), paint);
		}

		canvas->restore();

		mark_rerender();
	}
};

class TestCanvas1: public Box {
	void draw(Painter *render) override {
		auto canvas = window()->render()->getCanvas();
		auto size = window()->size();

		Paint paint;

		window()->root()->set_background_color_direct({0,0,0}, true);
		canvas->save();
		canvas->translate(size*-0.5);

		if (1) { // polygon
			paint.fill.color = Color4f(0, 0, 0, 0.5);
			Path path(   Vec2(110, size.y() - 150) ); // left/bottom
			path.lineTo( Vec2(size.x()*0.5, 0) ); // top, 放下面逆时针方向
			path.lineTo( size * 0.85 ); // right/bottom
			path.close();
			path.moveTo( Vec2(100, 100) ); // left/top
			path.lineTo( Vec2(200, 200) ); // right/bottom, 放下面逆时针方向
			path.lineTo( Vec2(100, 200) ); // left/bottom
			path.close();
			canvas->drawPath(path, paint);
			paint.antiAlias = false;
			paint.style = Paint::kStroke_Style;
			////
			// paint.stroke.color = Color4f(0, 0, 1, 0.3);
			// paint.strokeWidth = 50;
			// canvas->drawPath(path, paint);
			////
			// paint.strokeWidth = 0.5;
			// paint.stroke.color = Color4f(1, 0, 0, 1);
			// canvas->drawPath(path, paint);
		}

		if (0) { // line stroke
			Path path;//(Vec2(100, 100));
			// path.lineTo(Vec2(300, 300));
			path.moveTo(Vec2(300, 100));
			path.lineTo(Vec2(500, 300));
			path.lineTo(Vec2(600, 100));
			path.close();
			// paint.antiAlias = false;
			paint.style = Paint::kStroke_Style;
			paint.fill.color = Color4f(0, 0, 0, 0.5);
			paint.strokeWidth = 50;
			paint.stroke.color = Color4f(1, 0, 0, 0.5);
			canvas->drawPath(path, paint);
		}

		if (1) { // outline
			paint.antiAlias = true;
			paint.stroke.color = Color4f(0, 0, 1);
			paint.style = Paint::kStroke_Style;
			paint.strokeWidth = 20;
			canvas->drawPath(Path::MakeCircle(Vec2(300,160), 100), paint);
		}

		canvas->restore();

		mark_rerender();
	}

};

static Path make_compute_aa_path(float t) {
	Path path;
	Vec2 center(260, 260);
	float rot = t * 0.9f;
	for (int i = 0; i < 10; i++) {
		float r = (i & 1) ? 84.0f : 190.0f;
		float a = rot + float(i) * Qk_PI * 0.2f - Qk_PI_2_1;
		Vec2 p(center.x() + cosf(a) * r, center.y() + sinf(a) * r);
		if (i == 0) path.moveTo(p);
		else path.lineTo(p);
	}
	path.close();

	path.moveTo({110, 260});
	path.cubicTo({160, 35}, {360, 485}, {410, 260});
	path.cubicTo({360, 35}, {160, 485}, {110, 260});
	path.close();
	return path;
}

class TestCanvas2: public Box {
	uint32_t f = 0;
	void draw(Painter *render) override {
		auto canvas = window()->render()->getCanvas();
		auto size = window()->size();
		float t = float(f++) / 60.0f;
		Paint paint;

		canvas->translate(size*-0.5);

		paint.fill.color = Color4f(1, 0, 0);

		auto path = make_compute_aa_path(t);
		path.transform(Mat({-50,-50},{1.5},0,0));
		canvas->drawPath(path, paint);

		mark_rerender();
	}
};

#define _DefFn(id) typedef TestCanvas ##id TestCanvas
Qk_TEST_CANVAS(_DefFn);
Qk_TEST_Func(canvas) {
	App app;
	auto win = Window::Make({.frame={{0,0}, {700,700}}});
	win->activate();

	auto t = win->root()->append_new<TestCanvas>();

	t->set_width({ 0, BoxSizeKind::Match });
	t->set_height({ 0, BoxSizeKind::Match });

	app.run();
}
