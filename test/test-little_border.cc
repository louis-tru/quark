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

#include <src/util/util.h>
#include <src/util/fs.h>
#include <src/ui/app.h>
#include <src/ui/window.h>
#include <src/ui/view/matrix.h>
#include <src/ui/view/spine.h>
#include <src/ui/view/root.h>
#include <src/ui/view/label.h>
#include <src/ui/action/keyframe.h>
#include "./test.h"

using namespace qk;

Qk_TEST_Func(little_border) {
	App app;
	auto w = Window::Make({.msaa=1, .fps=0x0, .frame={{0,0}, {600,600}}, .title="Test Little Border"});
	auto r = w->root();
	auto m0 = r->append_new<Matrix>();

	{
		m0->set_background_color({230,230,230});
		m0->set_width({302});
		m0->set_height({450});
		m0->set_align(Align::CenterMiddle);
		// m0->set_translate({200,200});
		//m0->set_rotate_z(1);
		//m0->set_scale(0.5);
		auto act = new KeyframeAction(w);
		act->set_loop(0xffffffff);
		auto k0 = act->addFrame(0);
		k0->set_rotate_z(0);
		k0->set_scale(1);
		auto k1 = act->addFrame(5e4, LINEAR);
		k1->set_rotate_z(360);
		//k1->set_scale(0.3);
		m0->set_action(act);
		act->play();
	}
	{ // border test
		auto b0 = m0->append_new<Box>();
		b0->set_background_color({255,0,0,128});
		b0->set_width({150});
		b0->set_height({150});
		// b0->set_margin_top(0.3);
		//b0->set_color({0,255,255,50});
		//b0->set_cascade_color(CascadeColor::Both);
		b0 = m0->append_new<Box>();
		b0->set_background_color({255,0,0,128});
		b0->set_width({150});
		b0->set_height({150});
		b0->set_border_radius_right_top(80);
		//b0->set_margin_top(0.3);
		b0 = m0->append_new<Box>();
		b0->set_background_color({255,0,0,128});
		b0->set_width({150});
		b0->set_height({150});
		// little border test
		b0 = m0->append_new<Box>();
		b0->set_margin({2});
		b0->set_width({140});
		b0->set_height({140});
		b0->set_border({{0.5, {0,0,0}}}); // little border
		b0->set_border_radius({200,200,200,0});
		b0->append_new<Label>()->set_value("Border: 0.5");
		// little border test
		b0 = m0->append_new<Box>();
		b0->set_margin({2});
		b0->set_width({140});
		b0->set_height({140});
		b0->set_border({{1, {0,0,0}}}); // little border
		b0->set_border_radius({200,200,200,0});
		b0->append_new<Label>()->set_value("Border: 1");
		// little border test
		b0 = m0->append_new<Box>();
		b0->set_margin({2});
		b0->set_width({140});
		b0->set_height({140});
		b0->set_border({{0.8, {0,0,0}}}); // little border
		b0->set_border_radius({200,200,200,0});
		b0->append_new<Label>()->set_value("Border: 0.8");
	}

	app.run();
}
