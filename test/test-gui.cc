/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include <src/util/util.h>
#include <src/util/fs.h>
#include <src/ui/app.h>
#include <src/ui/window.h>
#include <src/ui/view/image.h>
#include <src/ui/view/root.h>
#include "./test.h"

using namespace qk;

Qk_TEST_Func(gui) {
	App app;
	auto win0 = Window::Make({.fps=0x0, .frame={{0,0}, {700,700}}, .title="Test 0", .backgroundColor={255,255,255}});
	win0->activate();
	auto win = Window::Make({.fps=0x0, .frame={{0,0}, {700,700}}, .title="Test GUI"});
	win->activate();
	auto r = win->root();
	//t->set_width({ 0, BoxSizeKind::Match });
	//t->set_height({ 0, BoxSizeKind::Match });
	r->set_background_color(Color(180, 80, 0));

	auto sp0 = r->append_new<Image>();
	// sp0->set_src(fs_resources("jsapi/res/0.bmp"));
	// sp0->set_src(fs_resources("jsapi/res/0.jpeg"));
	// sp0->set_src(fs_resources("jsapi/res/0.jpg"));
	sp0->set_src(fs_resources("jsapi/res/0.gif"));
	// sp0->set_src(fs_resources("jsapi/res/0.png"));
	// sp0->set_src(fs_resources("jsapi/res/0.tiff"));
	// sp0->set_src(fs_resources("jsapi/res/0.webp"));
	// sp0->set_src(fs_resources("jsapi/res/aa.tiff")); // res://res/bb
	// sp0->set_src(fs_resources("jsapi/res/aa.webp"));
	// sp0->set_src(fs_resources("jsapi/res/aa.ico"));
	// sp0->set_src(fs_resources("jsapi/res/test.png"));
	//sp0->set_width({256});
	sp0->set_height({256});
	sp0->set_align(Align::Start);

	// box sprite
	auto div0 = r->append_new<Box>();
	//div0->set_margin({10});
	div0->set_margin_left(10);
	div0->set_width({ 296, BoxSizeKind::Minus });
	div0->set_height({256});
	div0->set_align(Align::Start);
	div0->set_background_color({255,255,0,255});

	auto sp = div0->append_new<Image>();
	sp->set_src(fs_resources("jsapi/res/cc.pvr"));
	//sp->set_width({307});
	sp->set_height({1, BoxSizeKind::Ratio });

	// div sprite
	auto div = r->append_new<Box>();
	div->set_background_color(Color(255, 0, 0, 180));
	div->set_border_radius({30});
	div->set_margin({5});
	div->set_margin_top(30);
	div->set_width({256});
	div->set_height({280});
	div->set_border_width({5});
	div->set_border_color_left(Color(255, 0, 0));
	div->set_border_color_top(Color(0, 255, 0));
	div->set_border_color_right(Color(0, 0, 255));
	div->set_border_color_bottom(Color(255, 0, 255));
	div->set_align(Align::Start);

	auto sp1 = div->append_new<Image>();
	sp1->set_src(fs_resources("jsapi/res/aa.webp"));
	sp1->set_width({256});
	sp1->set_height({256});

	// Image
	auto img = r->append_new<Image>();
	img->set_src(fs_resources("jsapi/res/cc.tga"));
	img->set_width({320});
  // img->set_height({250});
	img->set_color({255,255,255,uint8_t(255*0.9)});
	// img->set_border_radius({60});
	img->set_border_radius_right_top(40);
	img->set_border_radius_left_bottom(40);
	img->set_margin({ 5 });
	img->set_margin_top(20);
	img->set_margin_bottom(0);
	img->set_border_width({5});
	img->set_border_width_top(15);
	img->set_border_width_right(15);
	img->set_border_color({Color(255, 0, 255)});
	// img->border_top_color(Color(0, 255, 255));
	// img->border_bottom_color(Color(0, 0, 255));
	// img->border_left_color(Color(255, 0, 0));
	// img->border_bottom_width(0);
	img->set_align(Align::Start);

	app.run();
	Qk_DLog("Exit");
}
