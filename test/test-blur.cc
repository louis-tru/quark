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

	void draw(Painter *r) override {
		auto canvas = window()->render()->getCanvas();
		auto size = canvas->size();

		i+=Qk_PI_RATIO_180*0.1;

		float c = abs(sinf(i));
		float width = 300;

		Paint paint;
		paint.fill.color = Color4f(int(256*c)%255/255.0, 0, 1, 1);
		PaintFilter filter{PaintFilter::kBlur_Type,c*50};
		paint.filter = &filter;
		// paint.antiAlias = false;
		Rect rect{size/2-width*0.5,width};
		//auto path = Path::MakeRect(rect);
		auto path = Path::MakeArc(rect, Qk_PI_2_1 * 0.5f +c*50, Qk_PI + Qk_PI_2_1, true);

		canvas->drawPath(path, paint);

		auto img = canvas->readImage(rect, {width}, kRGBA_8888_ColorType, kSrcOver_BlendMode);
		// paint.blendMode = kSrcOver_BlendMode;
		paint.fill.color = Color4f(1, 1, 1, 1);
		paint.filter = nullptr;
		PaintImage pimg;
		pimg.tileModeX = PaintImage::kMirror_TileMode;
		pimg.tileModeY = PaintImage::kRepeat_TileMode;
		pimg.mipmapMode = PaintImage::kLinear_MipmapMode;
		pimg.filterMode = PaintImage::kLinear_FilterMode;
		pimg.setImage(*img, {{0},{width*0.25f}});
		// paint.mask = &pimg;
		paint.fill.image = &pimg;
		canvas->drawRect({{0},{width}}, paint);

		mark_render();
	}
};

Qk_TEST_Func(blur) {
	App app;
	// auto win2 = Window::Make({.frame={{0,0}, {200,200}}, .title="win2"});
	auto win = Window::Make({.frame={{0,0}, {500,500}}, .title="Test Blur", .backgroundColor={0,255,0,0}});
	auto r = win->root();
	auto t = r->append_new<TestBlur>();
	r->set_origin({BoxOrigin{0,BoxOriginKind::Value}});
	r->set_background_color({255,255,255,0});
	t->set_width({ 0, BoxSizeKind::Match });
	t->set_height({ 0, BoxSizeKind::Match });
	app.run();
}
