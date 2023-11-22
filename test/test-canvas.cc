
#include <quark/ui/app.h>
#include <quark/ui/window.h>
#include <quark/ui/screen.h>
#include <quark/ui/layout/box.h>
#include <quark/ui/layout/root.h>
#include <quark/render/paint.h>
#include <quark/util/codec.h>
#include <quark/render/render.h>
#include <quark/render/canvas.h>

using namespace qk;

class MyCanvas: public Box {
public:

	void draw() {
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
			canvas->clipRect({ size*0.3*0.5, size*0.7 }, Canvas::kIntersect_ClipOp, 1);
		}

		{ // gradient
			Paint paint;
			Color4f colors[] = {Color4f(1,0,1), Color4f(0,1,0), Color4f(0,0,1)};
			float   pos[]    = {0,0.5,1};
			Rect           rect{ size*0.2*0.5, size*0.8 };
			GradientPaint  gPaint{
				GradientPaint::kRadial_Type,rect.origin+rect.size*0.5, rect.size*0.5, 3, colors, pos
			};
			// paint.antiAlias = false;
			paint.gradient = &gPaint;
			paint.type = Paint::kGradient_Type;

			canvas->save();
			canvas->setMatrix(canvas->getMatrix() * Mat(Vec2(100,-50), Vec2(0.8, 0.8), -0.2, Vec2(0.3,0)));
			canvas->drawRect(rect, paint);
			canvas->restore();
		}

		{ // Circle
			paint.color = Color4f(0, 0, 1, 0.5);
			canvas->drawPath(Path::MakeCircle(Vec2(300), 100), paint);
			paint.color = Color4f(1, 0, 0, 0.8);
			canvas->drawPath(Path::MakeOval({Vec2(200, 100), Vec2(100, 200)}), paint);
		}

		{ // -------- clip ------
			auto clip = Path::MakeCircle(size*0.5, 100);
			auto aa = 1;
			canvas->clipPath(clip, Canvas::kDifference_ClipOp, aa);
		}

		{ // polygon
			paint.color = Color4f(0, 0, 0, 0.8);
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
			paint.color = Color4f(0, 1, 0, 0.8);
			canvas->drawPath(Path::MakeArc({Vec2(400, 100), Vec2(200, 100)}, 0, 4.5, 1), paint);
			paint.color = Color4f(1, 0, 1, 0.8);
			canvas->drawPath(Path::MakeArc({Vec2(450, 250), Vec2(200, 100)}, 4.5, 4, 0), paint);
			paint.color = Color4f(0, 0, 0, 0.8);
			canvas->drawPath(Path::MakeArc({Vec2(450, 300), Vec2(100, 200)}, Qk_PI_2, Qk_PI_2+Qk_PI, 1), paint);
		}

		canvas->restore();

		{ // font text
			// paint.color = Color4f(0,0,0);
			// auto stype = FontStyle(TextWeight::kBold, TextWidth::kDefault, TextSlant::kNormal);
			// auto pool = shared_app()->font_pool();
			// auto unicode = codec_decode_to_uint32(kUTF8_Encoding, "A 好 HgKr葵花pjAH");
			// auto fgs = pool->getFFID()->makeFontGlyphs(unicode, stype, 64);
			// Vec2 origin(10,60);
			// for (auto &fg: fgs) {
			// 	origin[0] += ceilf(canvas->drawGlyphs(fg, origin, NULL, paint)) + 10;
			// }
		}

		{ // outline
			// paint.color = Color4f(0, 0, 0);
			// canvas->drawPath(Path::MakeRRect({ {180,150}, 200 }, {50, 80, 50, 80}), paint);
			// paint.color = Color4f(0, 1, 1);
			// canvas->drawPath(Path::MakeRRectOutline({ {400,100}, 200 }, { {440,140}, 120 }, {50, 80, 50, 80}), paint);
			// Qk_DEBUG("%d", sizeof(signed long));
			// paint.color = Color4f(0, 0, 0);
			// paint.style = Paint::kStroke_Style;
			// paint.width = 4;
			// canvas->drawPath(Path::MakeCircle(Vec2(500,400), 100), paint);
		}

		mark_render();
	}

	void accept(Visitor *vv) {
		if (vv->flags() == 0)
			draw();
	}
};

void test_canvas(int argc, char **argv) {
	App app;
	auto win = Window::Make({.fps=0x0});
	win->activate();
	// layout
	auto t = New<MyCanvas>()->append_to<Box>(win->root());
	t->set_width({ 0, SizeKind::kMatch });
	t->set_height({ 0, SizeKind::kMatch });
	// layout end
	app.run();
}
