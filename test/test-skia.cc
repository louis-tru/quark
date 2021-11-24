
#include <stdio.h>
#include <skia/core/SkICC.h>
#include <flare/app.h>
#include <flare/layout/root.h>
#include <flare/render/render.h>

#if F_IOS
# include <OpenGLES/ES3/gl.h>
# include <OpenGLES/ES3/glext.h>
#elif F_ANDROID
# define GL_GLEXT_PROTOTYPES
# include <GLES3/gl3.h>
# include <GLES3/gl3ext.h>
#elif F_OSX
# include <OpenGL/gl3.h>
# include <OpenGL/gl3ext.h>
#elif F_LINUX
# define GL_GLEXT_PROTOTYPES
# include <GLES3/gl3.h>
# include <GLES3/gl3ext.h>
#else
# error "The operating system does not support"
#endif


using namespace flare;

void draw(SkCanvas* canvas) {
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
	// auto root = Root::create();
	// printf("%s, %p\n", "ok skia", app);
	auto render = app->render();
	auto canvas = render->canvas();
	
	//glClearColor(1, 1, 1, 1);
	//glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	draw(canvas);
	
	render->commit();
}

void test_skia(int argc, char **argv) {
	Application app;
	app.initialize();
	app.F_On(Load, onload_handle, &app);
	app.run_loop();
}
