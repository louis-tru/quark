
#include <quark/app.h>
#include <quark/render/paint.h>
#include <quark/display.h>
#include <quark/util/codec.h>
#include <quark/pre_render.h>
#include <quark/render/render.h>
#include <quark/layout/box.h>
#include <quark/layout/root.h>

using namespace qk;

class TestCanvas: public Box {
public:
	TestCanvas(App *host): Box(host) {}

	virtual void accept(ViewVisitor *visitor) override {
		// if (pre_render()->host()->render() != visitor) return visitor->visitBox(this);

		auto _canvas = shared_app()->render()->getCanvas();

		_canvas->clearColor(Color4f(1,1,1));
		//_canvas->drawColor(Color4f(1,0,0));
		Paint paint0, paint;

		auto size = shared_app()->display()->size();
		
		Array<Color4f> colors{Color4f(1,0,1), Color4f(0,1,0), Color4f(0,0,1)};
		Array<float>   pos{0,0.5,1};
		Rect           rect{ size*0.2*0.5, size*0.8 };
		Gradient       g{&colors, &pos, rect.origin + rect.size*0.5, rect.size*0.5};
		//paint0.setLinearGradient(&g, rect.origin, rect.origin+rect.size);
		paint0.setGradient(Paint::kRadial_GradientType, &g);
		
		_canvas->save();
		_canvas->setMatrix(_canvas->getMatrix() * Mat(Vec2(100,-50), Vec2(0.8, 0.8), -0.2, Vec2(0.3,0)));
		_canvas->drawRect(rect, paint0);
		_canvas->restore();

		// -------- clip ------
		_canvas->save();
		//_canvas->clipRect({ size*0.3*0.5, size*0.7 }, Canvas::kIntersect_ClipOp, 0);

		paint.color = Color4f(1, 0, 1, 0.5);

		paint.color = Color4f(0, 0, 1, 0.5);
		_canvas->drawPath(Path::MakeCircle(Vec2(300), 100), paint);

		paint.color = Color4f(1, 0, 0, 0.8);
		_canvas->drawPath(Path::MakeOval({Vec2(200, 100), Vec2(100, 200)}), paint);

		// -------- clip ------
		/*_canvas->save();
		_canvas->clipPath(Path::MakeCircle(size*0.5, 100), Canvas::kDifference_ClipOp, 0);

		paint.color = Color4f(1, 1, 0, 0.5);

		Path path(   Vec2(0, size.y() - 10) );
		path.lineTo( size );
		path.lineTo( Vec2(size.x()*0.5, 0) );

		path.moveTo( Vec2(100, 100) );
		path.lineTo( Vec2(100, 200) );
		path.lineTo( Vec2(200, 200) );
		path.close();
		_canvas->drawPath(path, paint);

		paint.color = Color4f(0, 1, 0, 0.8);
		_canvas->drawPath(Path::MakeArc({Vec2(400, 100), Vec2(200, 100)}, 0, 4.5, 1), paint);

		paint.color = Color4f(1, 0, 1, 0.8);
		_canvas->drawPath(Path::MakeArc({Vec2(450, 250), Vec2(200, 100)}, 4.5, 4, 0), paint);

		paint.color = Color4f(0, 0, 0, 0.8);
		_canvas->drawPath(Path::MakeArc({Vec2(450, 300), Vec2(100, 200)}, Qk_PI2, Qk_PI2+Qk_PI, 1), paint);

		_canvas->restore(2);*/

		paint.color = Color4f(0,0,0);

		auto stype = FontStyle(TextWeight::kBold, TextWidth::kDefault, TextSlant::kNormal);
		auto pool = shared_app()->font_pool();
		auto unicode = codec_decode_to_uint32(kUTF8_Encoding, "A 好 HgKr葵花pjAH");
		auto fgs = pool->getFFID()->makeFontGlyphs(unicode, stype, 64);

		Vec2 origin(10,60);

		for (auto &fg: fgs) {
			origin[0] += ceilf(_canvas->drawGlyphs(fg, origin, NULL, paint)) + 10;
		}

		paint.color = Color4f(0, 0, 0);
		_canvas->drawPath(Path::MakeRRect({ {180,150}, 200 }, {50, 80, 50, 80}), paint);

		paint.color = Color4f(0, 1, 1);
		_canvas->drawPath(Path::MakeRRectOutline({ {400,100}, 200 }, { {440,140}, 120 }, {50, 80, 50, 80}), paint);

		// Qk_DEBUG("%d", sizeof(signed long));

		// paint.color = Color4f(0, 0, 0);
		// paint.style = Paint::kStroke_Style;
		// paint.width = 4;
		// _canvas->drawPath(Path::MakeCircle(Vec2(500,400), 100), paint);
	}
};

void test_canvas(int argc, char **argv) {
	App app;
	// layout
	auto t = (new TestCanvas(&app))->append_to<Box>(app.root());
	t->set_width({ 0, SizeKind::kMatch });
	t->set_height({ 0, SizeKind::kMatch });
	// layout end
	app.run();
}
