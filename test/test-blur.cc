// Ref:
// https://raphlinus.github.io/graphics/2020/04/21/blurred-rounded-rects.html
// https://madebyevan.com/shaders/fast-rounded-rectangle-shadows/
// https://www.shadertoy.com/view/DsdfDN

#include <src/ui/app.h>
#include <src/ui/window.h>
#include <src/ui/screen.h>
#include <src/ui/view/root.h>
#include <src/render/render.h>
#include <src/render/canvas.h>
#include "./test.h"

using namespace qk;

constexpr unsigned int u32 = 1;

class TestBlur: public Box {
public:
	float i = 0;

	void draw(UIDraw *r) override {
		auto canvas = window()->render()->getCanvas();
		auto size = canvas->size();

		i+=Qk_PI_RATIO_180*0.1;

		float c = abs(sinf(i));
		float width = 300;

		Paint paint;
		paint.color = Color4f(int(256*c)%255/255.0, 0, 1, 1);
		PaintFilter filter{PaintFilter::kBlur_Type,c*50};
		paint.filter = &filter;
		// paint.antiAlias = false;
		Rect rect{size/2-width*0.5,width};
		//auto path = Path::MakeRect(rect);
		auto path = Path::MakeArc(rect, Qk_PI_2_1 * 0.5f +c*50, Qk_PI + Qk_PI_2_1, true);

		canvas->drawPath(path, paint);

		auto img = canvas->readImage(rect, {width}, kRGBA_8888_ColorType, false);
		paint.color = Color4f(1, 0, 0, 1);
		paint.filter = nullptr;
		ImagePaint ipaint;
		ipaint.tileModeX = ImagePaint::kMirror_TileMode;
		ipaint.tileModeY = ImagePaint::kRepeat_TileMode;
		ipaint.mipmapMode = ImagePaint::kLinear_MipmapMode;
		ipaint.filterMode = ImagePaint::kLinear_FilterMode;
		ipaint.setImage(*img, {{0},{width*0.5f}});
		paint.image = &ipaint;
		paint.type = Paint::kBitmapMask_Type;
		//paint.type = Paint::kBitmap_Type;
		canvas->drawRect({{0},{width}}, paint);

		mark(kLayout_None,true);
	}
};

Qk_TEST_Func(blur) {
	App app;
	// auto win2 = Window::Make({.frame={{0,0}, {200,200}}, .title="win2"});
	auto win = Window::Make({.frame={{0,0}, {500,500}}, .title="Test Blur"});
	auto r = win->root();
	auto t = r->append_new<TestBlur>();
	r->set_box_origin({BoxOrigin{0,BoxOriginKind::Value}});
	r->set_background_color({255,255,255,0});
	t->set_width({ 0, BoxSizeKind::Match });
	t->set_height({ 0, BoxSizeKind::Match });
	app.run();
}
