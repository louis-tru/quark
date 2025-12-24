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
#include <src/ui/view/morph.h>
#include <src/ui/view/spine.h>
#include <src/ui/view/root.h>
#include <src/ui/view/label.h>
#include <src/ui/view/text.h>
#include <src/ui/action/keyframe.h>
#include "./test.h"

using namespace qk;

Qk_TEST_Func(spine) {
	App app;
	auto w = Window::Make({.msaa=1, .fps=0x0, .frame={{0,0}, {800,800}}, .title="Test Spine"});
	auto r = w->root();
	r->set_background_color(Color(180, 80, 0));

	auto m0 = r->append_new<Morph>();

	{ // matrix
		m0->set_background_color({230,0,0});
		m0->set_width({301});
		m0->set_height({301});
		m0->set_translate({200,200});
		//m0->set_clip(true);
		m0->set_border_radius({20});
		//m0->set_rotate_z(1);
		//m0->set_scale(0.5);
		auto act = new KeyframeAction(w);
		act->set_loop(0xffffffff);
		auto k0 = act->addFrame(0);
		k0->set_rotate_z(0);
		auto k1 = act->addFrame(5e4, LINEAR);
		k1->set_rotate_z(360);
		m0->set_action(act);
		//act->play();
	}
	{ // box / spine
		auto b0 = m0->append_new<Box>();
		b0->set_background_color({255,255,0});
		b0->set_width({150});
		b0->set_height({150});
		b0->set_margin_top(80);
		b0->set_margin_left(80);
		b0->set_border_radius({20});
		//b0->set_color({0,255,255,128});
		b0->set_cascade_color(CascadeColor::Both);
		//b0->set_clip(true);

		auto sp1 = b0->append_new<Spine>();
		//auto data = SkeletonData::Make(fs_resources("jsapi/res/skel/alien-ess.skel"), "", 0.5f);
		//auto data = SkeletonData::Make(fs_resources("jsapi/res/coin/coin-pro.skel"), "", 0.5f);
		auto data = SkeletonData::Make(fs_resources("jsapi/res/loading/loading.skel"), "", 0.5f);
		sp1->set_skel(data.get());
		sp1->set_skin("default");
		//sp1->set_animation(0, "death", true);
		//sp1->set_animation(0, "hit", true);
		sp1->set_animation(0, "jump", true);
		//sp1->set_animation(0, "run", true);
		sp1->set_animation(1, "animation", true);
		//sp1->set_origin({{0.5,BoxOriginKind::Ratio},{0.5,BoxOriginKind::Ratio}});
		// sp1->set_origin({{80},{80}});
		//sp1->set_animation(2, "open", true);
		sp1->set_animation(2, "loop", true);

		auto act = new KeyframeAction(w);
		act->set_loop(0xffffffff);
		auto k0 = act->addFrame(0);
		k0->set_rotate_z(0);
		auto k1 = act->addFrame(5e3, LINEAR);
		k1->set_rotate_z(360);
		sp1->set_action(act);
		// act->play();

		b0 = sp1->append_new<Box>();
		b0->set_background_color({255,0,255,128});
		b0->set_width({50});
		b0->set_height({50});
		b0->set_align(Align::LeftTop);
		b0 = sp1->append_new<Box>();
		b0->set_background_color({255,0,255,128});
		b0->set_width({50});
		b0->set_height({50});
		b0->set_align(Align::RightTop);
		b0 = sp1->append_new<Box>();
		b0->set_background_color({255,0,255,128});
		b0->set_width({50});
		b0->set_height({50});
		b0->set_align(Align::LeftBottom);
		b0 = sp1->append_new<Box>();
		b0->set_background_color({255,0,255,128});
		b0->set_width({50});
		b0->set_height({50});
		b0->set_align(Align::RightBottom);
	}
	if (1) { // sprite
		auto m1 = m0->append_new<Morph>();
		auto sp0 = m1->append_new<Sprite>();
		m1->set_background_color({0,0,255});
		m1->set_width({100});
		m1->set_height({100});
		//m1->set_color({0,0,255,128});
		m1->set_cascade_color(CascadeColor::Both);
		// m1->set_rotate_z(30);
		//m1->set_translate({0,-10});
		auto act = new KeyframeAction(w);
		act->set_loop(0xffffffff);
		auto k0 = act->addFrame(0);
		k0->set_rotate_z(0);
		auto k1 = act->addFrame(3e3, LINEAR);
		k1->set_rotate_z(360);
		m1->set_action(act);
		// act->play();

		sp0->set_src(fs_resources("jsapi/res/sprite.png"));
		sp0->set_width(114 * w->atomPixel());
		sp0->set_height(114 * w->atomPixel());
		sp0->set_frames(9);
		sp0->set_sets(6);
		sp0->set_set(1);
		sp0->set_spacing(1);
		sp0->set_frequency(8);
		sp0->set_playing(true);
	}
	if (1) {
		auto t0 = m0->append_new<Text>();
		t0->set_value("CTFont"); // DrawGlyphs
		t0->set_text_size({24});
		t0->set_text_stroke({1, {0,0,255}});
		//t0->set_text_shadow({2,2,3,{0,0,0,255}});
		//t0->set_text_background_color({{0,255,255,128}});
	}

	app.run();
}
