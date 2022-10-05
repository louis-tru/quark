
#include <quark/app.h>
#include <quark/display.h>
#include <quark/render/skia/skia_render.h>
#include <quark/render/font/pool.h>
#include <skia/core/SkCanvas.h>
#include <skia/core/SkFont.h>
#include <skia/core/SkData.h>
#include <skia/core/SkFontMgr.h>
#include <native-font.h>

using namespace quark;

void testSkFont(Application* app, SkCanvas* canvas) {
	SkString str;
	auto mgr = SkFontMgr::RefDefault();
	int count = mgr->countFamilies();

	for (int i = 0; i < count; i++) {
		mgr->getFamilyName(i, &str);
		Qk_LOG("%d, %s", i, str.c_str());
	}
	
	sk_sp<SkFontStyleSet> set(mgr->matchFamily("Academy Engraved LET"));
	Qk_LOG(set->count());

	SkFontStyle style;
	sk_sp<SkTypeface> tf(mgr->matchFamilyStyle(nullptr, style));
	sk_sp<SkTypeface> tf2(mgr->matchFamilyStyleCharacter("", style, nullptr, 0, 65533));
	sk_sp<SkTypeface> tf3(mgr->matchFamilyStyle("PingFang HK", style));

	Qk_LOG("%d", tf->uniqueID());
	Qk_LOG("%d", tf2->uniqueID());
	Qk_LOG("%d", tf3->uniqueID());

	tf->getFamilyName(&str);
	Qk_LOG("%s", str.c_str());

	tf2->getFamilyName(&str);
	Qk_LOG("%s", str.c_str());

	tf3->getFamilyName(&str);
	Qk_LOG("%s", str.c_str());

	Qk_LOG("");
	Qk_LOG("楚(26970):%d", tf->unicharToGlyph(26970)); // 楚
	Qk_LOG("学(23398):%d", tf->unicharToGlyph(23398)); // 学
	Qk_LOG("文(25991):%d", tf->unicharToGlyph(25991)); // 文
	Qk_LOG("�(65533):%d", tf->unicharToGlyph(65533));  // �

	Qk_LOG("");
	Qk_LOG("楚(26970):%d", tf2->unicharToGlyph(26970)); // 楚
	Qk_LOG("学(23398):%d", tf2->unicharToGlyph(23398)); // 学
	Qk_LOG("文(25991):%d", tf2->unicharToGlyph(25991)); // 文
	Qk_LOG("�(65533):%d", tf2->unicharToGlyph(65533));  // �

	Qk_LOG("");
	Qk_LOG("楚(26970):%d", tf3->unicharToGlyph(26970)); // 楚
	Qk_LOG("学(23398):%d", tf3->unicharToGlyph(23398)); // 学
	Qk_LOG("文(25991):%d", tf3->unicharToGlyph(25991)); // 文
	Qk_LOG("�(65533):%d", tf3->unicharToGlyph(65533));  // �

	Qk_LOG("");
	Qk_LOG("countGlyphs:%d", tf->countGlyphs());
	Qk_LOG("countGlyphs:%d", tf2->countGlyphs());
	Qk_LOG("countGlyphs:%d", tf3->countGlyphs());

	SkFont font(tf, 16);
	SkFont font2(tf2, 16);
	SkFont font3(tf3, 16);

	//SkFontMetrics metrics;
	//font.getMetrics(&metrics);
}

void testFontPool(Application* app, SkCanvas* canvas) {
	auto pool = app->font_pool();

	Qk_LOG("family_names,%d", pool->familys().length());

	FontStyle style;
	auto tf1 = pool->match("", style, true);
	auto tf2 = pool->match("PingFang HK", style);

	Qk_LOG(tf1.getFamilyName());
	Qk_LOG(tf2.getFamilyName());

	auto DejaVuSerif_ttf = native_fonts_[0];
	WeakBuffer buf((char*)DejaVuSerif_ttf.data, DejaVuSerif_ttf.count);
	pool->register_from_data(buf);

	auto tf3 = pool->match("DejaVu Serif", style);

	Qk_LOG(tf3.getFamilyName());

	//FFID ffid = pool->getFFID("Helvetica, PingFang HK");
	// FFID ffid = pool->getFFID("PingFang HK");
	FFID ffid = pool->getFFID("DejaVu Serifa");

	Qk_LOG("%p", ffid);

	auto fontGlyphs = ffid->makeFontGlyphs({ /*32,*/ 65, 66, 26970, 23398, 25991 }, style, 16);

	for (auto& fg: fontGlyphs) {
		Qk_LOG("Family: %s, fontSize: %f", fg.typeface().getFamilyName().c_str(), fg.fontSize());

		int i = 0;
		auto gs = fg.glyphs();

		for (auto& glyph: fg.get_offset()) {
			Qk_LOG("OffsetX: %f, GlyphId: %d", glyph, i < gs.length() ? gs[i++]: 0);
		}
	}

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
