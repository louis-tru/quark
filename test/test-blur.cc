
#include <quark/app.h>
#include <quark/render/render.h>
#include <quark/layout/root.h>
#include <quark/display.h>

using namespace qk;

constexpr unsigned int u32 = 1;

class TestBlur: public Box {
public:
	TestBlur(App *host): Box(host) {}
	float i = 0;

	void accept(Visitor *vv) override {
		if (vv->flags()) return;
		auto canvas = pre_render()->host()->render()->getCanvas();
		auto size = canvas->size();

		i+=Qk_PI_RATIO_180*0.2;

		float c = abs(sinf(i));

		float width = 300;

		Paint paint;
		paint.color = Color4f(0, 0, 1);
		PaintFilter filter{PaintFilter::kBlur_Type,c*200};
		paint.filter = &filter;
		paint.antiAlias = false;
		Rect rect{size/2-width*0.5,width};
		// auto path = Path::MakeArc(rect, Qk_PI_2_1 * 0.5f, Qk_PI + Qk_PI_2_1, true);
		auto path = Path::MakeArc(rect, Qk_PI_2_1 * 0.5f, Qk_PI + Qk_PI_2_1, true);

		canvas->drawPath(path, paint);

		auto img = canvas->readImage(rect, {width}, kColor_Type_RGBA_8888, false);

		paint.color = Color4f(1, 1, 0, 1);
		paint.filter = nullptr;
		ImagePaint ipaint;
		ipaint.tileModeX = ImagePaint::kMirror_TileMode;
		ipaint.tileModeY = ImagePaint::kRepeat_TileMode;
		ipaint.mipmapMode = ImagePaint::kLinear_MipmapMode;
		ipaint.filterMode = ImagePaint::kLinear_FilterMode;
		ipaint.setImage(*img, {{0},{width*0.5f}});
		paint.image = &ipaint;
		paint.type = Paint::kBitmapMask_Type;
		canvas->drawRect({{0},{width}}, paint);

		mark_render();
	}
};

void test_blur(int argc, char **argv) {
	App app({.fps=0x0, .windowFrame={{0,0}, {1000,1000}}});
	// layout
	auto r = app.root();
	auto t = app.root()->append_new<TestBlur>();
	r->set_background_color({255,255,255,0});
	t->set_width({ 0, SizeKind::kMatch });
	t->set_height({ 0, SizeKind::kMatch });
	// layout end
	app.run();
}
