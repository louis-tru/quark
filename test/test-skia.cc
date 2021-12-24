
#include <stdio.h>
#include <skia/core/SkICC.h>
#include <flare/app.h>
#include <flare/layout/root.h>
#include <flare/render/render.h>
#include <flare/layout/flex.h>
#include <flare/fill.h>
#include <flare/display.h>
#include <flare/util/fs.h>

using namespace flare;

void draw_skia(SkCanvas* canvas) {
	canvas->clear(SK_ColorWHITE);

	SkPaint paint;
	paint.setStyle(SkPaint::kFill_Style);
	paint.setAntiAlias(true);
	paint.setStrokeWidth(4);
	paint.setColor(0xff4285F4);

	SkRect rect = SkRect::MakeXYWH(10, 80, 100, 160);
	//canvas->drawRect(rect, paint);

	SkRRect rrect;// = SKRRect::MakeRect(rect);
	SkVector radii[4] = {{10,20},{10,20},{10,20},{10,20}};
	rrect.setRectRadii(rect, radii);
	paint.setColor(0xffDB4437);
	//paint.setStyle(SkPaint::kStroke_Style);
	//paint.setStrokeWidth(5);
	canvas->drawRRect(rrect, paint);

	SkRRect rrect0, rrect1;
	SkVector radii0[4] = {{10,20},{10,20},{10,20},{10,20}};
	SkVector radii1[4] = {{10,20},{10,20},{10,20},{10,20}};
	rrect0.setRectRadii(SkRect::MakeXYWH(50, 60, 100, 160), radii0);
	rrect1.setRectRadii(SkRect::MakeXYWH(60, 70, 80, 140), radii1);
	paint.setColor(0xff0000ff);
	//paint.setStyle(SkPaint::kStroke_Style);
	//paint.setStrokeWidth(1);
	canvas->drawDRRect(rrect0, rrect1, paint);

	// drawCircle
	paint.setColor(0xff00ff00);
	canvas->drawCircle(180, 40, 30, paint);

	// drawArc
	paint.setColor(0xffffff00);
	paint.setStyle(SkPaint::kStroke_Style);
	canvas->drawArc(SkRect::MakeXYWH(80, 60, 100, 160), 0, 180, 0, paint);
}

void onload_handle(Event<>& evt, Application* app) {
	//draw_skia(app->render()->canvas());
	//app->render()->commit();
	
	app->display()->set_status_bar_style(Display::STATUS_BAR_STYLE_BLACK);
	
	auto r = Root::create();
	auto flex = (FlexLayout*)New<FlexLayout>()->append_to(r);
	auto flow = (FlowLayout*)New<FlowLayout>()->append_to(r);

	New<Box>()->append_to(flex);
	New<Box>()->append_to(flex);
	New<Box>()->append_to(flow);
	New<Box>()->append_to(flow);
	
	//
	flex->set_width({ 0, SizeType::MATCH });
	flex->set_height({ 200, SizeType::PIXEL });
	flex->set_fill(New<FillColor>(Color(255,0,0,255))->set_next(
								 New<FillImage>(Path::resources("bench/img2/21.jpeg"))));
	flex->set_margin_left(10);
	flex->set_margin_top(20);
	flex->set_margin_right(10);
	flex->set_margin_bottom(20);
	flex->set_padding_left(20);
	flex->set_padding_top(20);
	flex->set_padding_right(20);
	flex->set_padding_bottom(20);
	//
	flow->set_width({ 50, SizeType::PIXEL });
	flow->set_height({ 50, SizeType::PIXEL });
	flow->set_fill(New<FillImage>(Path::resources("bench/img2/99.jpeg")));
	flow->set_layout_align(Align::CENTER_BOTTOM);
	flow->set_margin_left(10);
	flow->set_margin_top(10);
	flow->set_margin_right(10);
	flow->set_margin_bottom(10);
	flow->set_padding_left(50);
	
	F_DEBUG("%s, %p\n", "ok skia", app);
	
	F_DEBUG("Object size %d", sizeof(Object));
	F_DEBUG("Reference size %d", sizeof(Reference));
	F_DEBUG("Layout size %d", sizeof(Layout));
	F_DEBUG("Notification<UIEvent, UIEventName, Layout> size %d", sizeof(Notification<UIEvent, UIEventName, Layout>));
	F_DEBUG("View size %d", sizeof(View));
	F_DEBUG("Box size %d", sizeof(Box));
	F_DEBUG("FlowLayout size %d", sizeof(FlowLayout));
	F_DEBUG("FlexLayout size %d", sizeof(FlexLayout));
	F_DEBUG("Root size %d", sizeof(Root));
}

void test_skia(int argc, char **argv) {
	Application app;
	app.F_On(Load, onload_handle, &app);
	app.run(true);
}
