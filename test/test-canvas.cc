
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
#include "./test.h"

using namespace qk;

class MyCanvas: public Box {
public:

	void draw(Painter *render) override {
		// mark_none(kLayout_None); return;
		auto canvas = window()->render()->getCanvas();
		auto size = window()->size();

		// clear color
		//canvas->clearColor(Color4f(1,1,1));
		//canvas->drawColor(Color4f(1,0,0));
		Paint paint;

		// -------- clip ------
		canvas->save();
		{ // clip
			canvas->clipRect({ size*-0.35, size*0.7 }, Canvas::kIntersect_ClipOp, 1);
		}

		{ // gradient
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
			canvas->setMatrix(canvas->getMatrix() * Mat({0,0}, {0.8, 0.8}, -0.2, {0.3,0}));
			canvas->drawRect(rect, paint);
			canvas->restore();
		}

		{ // Circle
			paint.fill.gradient = nullptr;
			paint.fill.color = Color4f(0, 0, 1, 0.5);
			canvas->drawPath(Path::MakeCircle(0, 100), paint);
			paint.fill.color = Color4f(1, 0, 0, 0.8);
			canvas->drawPath(Path::MakeOval({0, {100, 200}}), paint);
		}

		{ // -------- clip ------
			auto clip = Path::MakeCircle(0, 100);
			auto aa = 1;
			canvas->clipPath(clip, Canvas::kDifference_ClipOp, aa);
		}

		canvas->translate(size*-0.5);

		{ // polygon
			paint.fill.color = Color4f(0, 0, 0, 0.8);
			Path path(   Vec2(0, size.y() - 10) );
			path.lineTo( size );
			path.lineTo( Vec2(size.x()*0.5, 0) );
			path.close();
			path.moveTo( Vec2(100, 100) );
			path.lineTo( Vec2(100, 200) );
			path.lineTo( Vec2(200, 200) );
			path.close();
			canvas->drawPath(path, paint);
		}

		{ // Arc
			paint.fill.color = Color4f(0, 1, 0, 0.8);
			canvas->drawPath(Path::MakeArc({Vec2(400, 100), Vec2(200, 100)}, 0, 4.5, 1), paint);
			paint.fill.color = Color4f(1, 0, 1, 0.8);
			canvas->drawPath(Path::MakeArc({Vec2(450, 250), Vec2(200, 100)}, 4.5, 4, 0), paint);
			paint.fill.color = Color4f(0, 0, 0, 0.8);
			canvas->drawPath(Path::MakeArc({Vec2(450, 300), Vec2(100, 200)}, Qk_PI_2, Qk_PI_2+Qk_PI, 1), paint);
		}

		{ // font text
			paint.fill.color = Color4f(255,0,255);
			auto stype = FontStyle(FontWeight::Bold, FontWidth::Default, FontSlant::Normal);
			auto pool = shared_app()->fontPool();
			auto unicode = codec_decode_to_unicode(kUTF8_Encoding, "A 好 HgKr葵花pjAH");
			auto fgs = pool->getFontFamilies()->makeFontGlyphs(unicode, stype, 64);
			Vec2 origin(10,60);
			for (auto &fg: fgs) {
				origin[0] += ceilf(canvas->drawGlyphs(fg, origin, NULL, paint)) + 10;
			}
		}

		{ // outline
			paint.fill.color = Color4f(0, 0, 0);
			canvas->drawPath(Path::MakeRRect({ {180,150}, 200 }, {50, 80, 50, 80}), paint);
			paint.fill.color = Color4f(0, 1, 1);
			canvas->drawPath(Path::MakeRRectOutline({ {400,100}, 200 }, { {440,140}, 120 }, {50, 80, 50, 80}), paint);
			//Qk_DLog("%d", sizeof(signed long));
			paint.stroke.color = Color4f(0, 0, 0);
			paint.style = Paint::kStroke_Style;
			paint.strokeWidth = 4;
			canvas->drawPath(Path::MakeCircle(Vec2(500,400), 100), paint);
		}

		canvas->restore();

		mark(kLayout_None,true);
	}

};

Qk_TEST_Func(canvas) {
	App app;
	auto win = Window::Make({.fps=0x0});
	win->activate();
	// layout
	auto t = win->root()->append_new<MyCanvas>();
	t->set_width({ 0, BoxSizeKind::Match });
	t->set_height({ 0, BoxSizeKind::Match });
	// layout end
	app.run();
}
