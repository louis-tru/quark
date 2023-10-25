
#include <quark/app.h>
#include <quark/render/render.h>
#include <quark/layout/root.h>
#include <quark/display.h>

using namespace qk;

constexpr unsigned int u32 = 1;

class TestBlur: public Box {
public:
	TestBlur(App *host): Box(host) {}

	void accept(ViewVisitor *visitor) override {
		auto app = pre_render()->host();
		auto canvas = app->render()->getCanvas();
		auto size = canvas->size();

		Paint paint;
		paint.color = Color4f(0, 0, 0);
		PaintFilter filter{PaintFilter::kBlur_Type,50};
		paint.filter = &filter;
		paint.antiAlias = false;
		auto path = Path::MakeArc({size/2-150,300}, Qk_PI_2_1 * 0.5f, Qk_PI + Qk_PI_2_1, true);
		// auto path = Path::MakeRect({size/2-100,200}); path.close();

		canvas->drawPath(path, paint);

		// paint.color = Color4f(1, 0, 0, 0.1);
		// paint.filter = nullptr;
		// canvas->drawPath(path, paint);

		mark_none(kLayout_None);
	}
};

void test_blur(int argc, char **argv) {
	App app({.fps=0x0, .windowFrame={{0,0}, {500,500}}});
	// layout
	auto r = app.root();
	auto t = app.root()->append_new<TestBlur>();
	r->set_background_color({255,255,255,0});
	t->set_width({ 0, SizeKind::kMatch });
	t->set_height({ 0, SizeKind::kMatch });
	// layout end
	app.run();
}
