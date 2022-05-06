
#include <noug/app.h>
#include <noug/display.h>
#include <noug/render/skia/skia_render.h>
#include <skia/core/SkCanvas.h>
#include <skia/core/SkFont.h>
#include <skia/core/SkFontMgr.h>

using namespace noug;

void testSkfont(Application* app, SkCanvas* canvas) {
	SkString str;
	auto mgr = SkFontMgr::RefDefault();
	int count = mgr->countFamilies();

	for (int i = 0; i < count; i++) {
		mgr->getFamilyName(i, &str);
		F_LOG("%d, %s", i, str.c_str());
	}
	
	sk_sp<SkFontStyleSet> set(mgr->matchFamily("Academy Engraved LET"));
	F_LOG(set->count());
	
	SkFontStyle style;
	sk_sp<SkTypeface> tf(mgr->matchFamilyStyle(nullptr, style));
	sk_sp<SkTypeface> tf2(mgr->matchFamilyStyleCharacter("", style, nullptr, 0, 65533));
	sk_sp<SkTypeface> tf3(mgr->matchFamilyStyle("PingFang HK", style));
	
	F_LOG("%d", tf->uniqueID());
	F_LOG("%d", tf2->uniqueID());
	F_LOG("%d", tf3->uniqueID());
	
	tf->getFamilyName(&str);
	F_LOG("%s", str.c_str());

	tf2->getFamilyName(&str);
	F_LOG("%s", str.c_str());
	
	tf3->getFamilyName(&str);
	F_LOG("%s", str.c_str());
	
	F_LOG("楚(26970):%d", tf->unicharToGlyph(26970)); // 楚
	F_LOG("学(23398):%d", tf->unicharToGlyph(23398)); // 学
	F_LOG("文(25991):%d", tf->unicharToGlyph(25991)); // 文
	F_LOG("�(65533):%d", tf->unicharToGlyph(65533)); // �

	F_LOG("楚(26970):%d", tf2->unicharToGlyph(26970)); // 楚
	F_LOG("学(23398):%d", tf2->unicharToGlyph(23398)); // 学
	F_LOG("文(25991):%d", tf2->unicharToGlyph(25991)); // 文
	F_LOG("�(65533):%d", tf2->unicharToGlyph(65533)); // �
	
	F_LOG("楚(26970):%d", tf3->unicharToGlyph(26970)); // 楚
	F_LOG("学(23398):%d", tf3->unicharToGlyph(23398)); // 学
	F_LOG("文(25991):%d", tf3->unicharToGlyph(25991)); // 文
	F_LOG("�(65533):%d", tf3->unicharToGlyph(65533)); // �
	
	
	F_LOG("countGlyphs:%d", tf->countGlyphs());
	F_LOG("countGlyphs:%d", tf2->countGlyphs());
	F_LOG("countGlyphs:%d", tf3->countGlyphs());
	
	SkFont font(tf, 16);
	SkFont font2(tf2, 16);
	SkFont font3(tf3, 16);
	
	//SkFontMetrics metrics;
	//font.getMetrics(&metrics);
	
	

}

void test_skfont(int argc, char **argv) {
	auto post = [](Application* app) {
		app->render()->post_message(Cb([app](CbData&data) {
			auto render = static_cast<SkiaRender*>(app->render()->visitor());
			testSkfont(app, render->getCanvas());
			app->render()->submit();
		}));
	};

	Application app;
	app.F_On(Load, [&](Event<>& evt) { post(&app); });
	app.display()->F_On(Change, [&](Event<>& evt){ post(&app); });
	app.run(true);
}
