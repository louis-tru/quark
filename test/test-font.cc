
#include <noug/app.h>
#include <noug/display.h>
#include <noug/render/skia/skia_render.h>
#include <noug/render/font/pool.h>
#include <skia/core/SkCanvas.h>
#include <skia/core/SkFont.h>
#include <skia/core/SkData.h>
#include <skia/core/SkFontMgr.h>
#include <native-font.h>

using namespace noug;

void testSkFont(Application* app, SkCanvas* canvas) {
	SkString str;
	auto mgr = SkFontMgr::RefDefault();
	int count = mgr->countFamilies();

	for (int i = 0; i < count; i++) {
		mgr->getFamilyName(i, &str);
		N_LOG("%d, %s", i, str.c_str());
	}
	
	sk_sp<SkFontStyleSet> set(mgr->matchFamily("Academy Engraved LET"));
	N_LOG(set->count());
	
	SkFontStyle style;
	sk_sp<SkTypeface> tf(mgr->matchFamilyStyle(nullptr, style));
	sk_sp<SkTypeface> tf2(mgr->matchFamilyStyleCharacter("", style, nullptr, 0, 65533));
	sk_sp<SkTypeface> tf3(mgr->matchFamilyStyle("PingFang HK", style));
	
	N_LOG("%d", tf->uniqueID());
	N_LOG("%d", tf2->uniqueID());
	N_LOG("%d", tf3->uniqueID());
	
	tf->getFamilyName(&str);
	N_LOG("%s", str.c_str());

	tf2->getFamilyName(&str);
	N_LOG("%s", str.c_str());
	
	tf3->getFamilyName(&str);
	N_LOG("%s", str.c_str());
	
	N_LOG("");
	N_LOG("楚(26970):%d", tf->unicharToGlyph(26970)); // 楚
	N_LOG("学(23398):%d", tf->unicharToGlyph(23398)); // 学
	N_LOG("文(25991):%d", tf->unicharToGlyph(25991)); // 文
	N_LOG("�(65533):%d", tf->unicharToGlyph(65533));  // �

	N_LOG("");
	N_LOG("楚(26970):%d", tf2->unicharToGlyph(26970)); // 楚
	N_LOG("学(23398):%d", tf2->unicharToGlyph(23398)); // 学
	N_LOG("文(25991):%d", tf2->unicharToGlyph(25991)); // 文
	N_LOG("�(65533):%d", tf2->unicharToGlyph(65533));  // �
	
	N_LOG("");
	N_LOG("楚(26970):%d", tf3->unicharToGlyph(26970)); // 楚
	N_LOG("学(23398):%d", tf3->unicharToGlyph(23398)); // 学
	N_LOG("文(25991):%d", tf3->unicharToGlyph(25991)); // 文
	N_LOG("�(65533):%d", tf3->unicharToGlyph(65533));  // �
	
	N_LOG("");
	N_LOG("countGlyphs:%d", tf->countGlyphs());
	N_LOG("countGlyphs:%d", tf2->countGlyphs());
	N_LOG("countGlyphs:%d", tf3->countGlyphs());
	
	SkFont font(tf, 16);
	SkFont font2(tf2, 16);
	SkFont font3(tf3, 16);
	
	//SkFontMetrics metrics;
	//font.getMetrics(&metrics);
}

void testFontPool(Application* app, SkCanvas* canvas) {
	auto pool = app->font_pool();
	
	N_LOG("family_names,%d", pool->familys().length());

	FontStyle style;
	auto tf1 = pool->match("", style, true);
	auto tf2 = pool->match("PingFang HK", style);
	
	N_LOG(tf1.getFamilyName());
	N_LOG(tf2.getFamilyName());
	
	auto DejaVuSerif_ttf = native_fonts_[0];
	WeakBuffer buf((char*)DejaVuSerif_ttf.data, DejaVuSerif_ttf.count);
	pool->register_from_data(buf);
	
	auto tf3 = pool->match("DejaVu Serif", style);
	
	N_LOG(tf3.getFamilyName());

	FFID ffid = pool->getFFID("Helvetica, DejaVu Serif, PingFang HK");

	N_LOG("%p", ffid);

	ffid->makeFontGlyphs({ 65, 66, 26970, 23398, 25991 }, style, 16);

}

void test_font(int argc, char **argv) {
	auto post = [](Application* app) {
		app->render()->post_message(Cb([app](CbData&data) {
			auto render = static_cast<SkiaRender*>(app->render()->visitor());
			auto canvas = render->getCanvas();
			//testSkFont(app, render->getCanvas());
			testFontPool(app, render->getCanvas());
			app->render()->submit();
		}));
	};

	Application app;
	app.N_On(Load, [&](Event<>& evt) { post(&app); });
	app.display()->N_On(Change, [&](Event<>& evt){ post(&app); });
	app.run(true);
}
