/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include <quark/util/util.h>
#include <quark/util/fs.h>
#include <quark/ui/app.h>
#include <quark/ui/window.h>
#include <quark/ui/view/image.h>
#include <quark/ui/view/root.h>

using namespace qk;

void test_gui(int argc, char **argv) {
	App app;
	auto win = Window::Make({.fps=0x0, .frame={{0,0}, {500,500}}});
	win->activate();
	auto r = win->root();
	//auto t = r->append_new<TestSubcanvas>(r->window());
	//r->set_background_color({255,255,255,0});
	//t->set_width({ 0, BoxSizeKind::Match });
	//t->set_height({ 0, BoxSizeKind::Match });
	
	r->set_background_color(Color(255, 255, 255));

	auto sp0 = r->append_new<Image>();
	sp0->set_src(fs_resources("testing/res/bb.pvr")); // res://res/bb
	sp0->set_width({256});
	sp0->set_height({256});
	// sp0->set_translate(Vec2(100, 500));
	// sp0->set_rotate_z(10);

	// div sprite
	auto div = r->append_new<Box>();
	div->set_background_color(Color(255, 0, 0, 180));
	div->set_border_radius({30});
	div->set_margin(5);
	div->set_margin_top(30);
	div->set_width({256});
	div->set_height({280});
	div->set_border_width({5});
	div->set_border_color_left(Color(255, 0, 0));
	div->set_border_color_top(Color(0, 255, 0));
	div->set_border_color_right(Color(0, 0, 255));
	div->set_border_color_bottom(Color(255, 0, 255));

	auto sp1 = div->append_new<Image>();
	sp0->set_src(fs_resources("testing/res/bb.pvr"));
	sp1->set_width({256});
	sp1->set_height({256});

	// box sprite
	auto div2 = r->append_new<Box>();
	div2->set_margin({10});
	div2->set_margin_top(30);
	div2->set_width({ 296, BoxSizeKind::Minus });
	div2->set_height({100});

	auto sp = div2->append_new<Image>();
	sp0->set_src(fs_resources("testing/res/cc.pvr"));
	sp->set_width({307});
	sp->set_height({307});
	// sp->translate(-450, -450);
	// sp->origin(500, 500);
	// sp->rotate(-45);

	// Image
	auto img = r->append_new<Image>();
	img->set_src(fs_resources("testing/res/cc.pvr"));
	img->set_width({320});
	// img->height(250);
	img->set_opacity(0.9);
	img->set_border_radius(60);
	img->set_border_radius_right_top(40);
	img->set_border_radius_left_bottom(40);
	//img->set_origin(Vec2(160, 160));
	//img->set_rotate_z(-10);
	// img->xy(50, 50);
	// img->x(-50);
	// img->y(-50);
	// img->set_margin({ 0 });
	img->set_margin_top(20);
	img->set_margin_bottom(0);
	img->set_border_width(5);
	img->set_border_width_top(15);
	img->set_border_width_right(15);
	img->set_border_color({Color(255, 0, 255)});
	// img->border_top_color(Color(0, 255, 255));
	// img->border_bottom_color(Color(0, 0, 255));
	// img->border_left_color(Color(255, 0, 0));
	// img->border_bottom_width(0);

	app.run();
}
