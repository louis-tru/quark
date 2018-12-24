/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include <ngui/utils/util.h>
#include <ngui/image.h>
#include <ngui/sprite.h>
#include <ngui/div.h>
#include <ngui/app.h>
#include <ngui/display-port.h>
#include <ngui/root.h>
#include <ngui/utils/fs.h>
#include <ngui/gl/gl.h>

using namespace ngui;
using ngui::value::ValueType;
using ngui::value::Value;

void onload_handle (Event<>& evt, void* user) {
	Root* r = New<Root>();
	
	r->set_background_color(Color(255, 255, 255));
	
	Sprite* sp0 = Sprite::create(Path::resources("res/bb.pvr"), {512, 512});
	sp0->set_translate(Vec2(100, 500));
	sp0->set_rotate_z(10);
	sp0->set_scale(Vec2(0.5, 0.5));
	sp0->append_to(r);
	
	// div sprite
	
	Div* div = New<Div>();
	div->set_background_color(Color(255, 0, 0, 180));
	div->set_border_radius(30);
	div->set_margin(5);
	div->set_margin_top(30);
	div->set_width(256);
	div->set_height(280);
	div->set_border_width(5);
	div->set_border_left_color(Color(255, 0, 0));
	div->set_border_top_color(Color(0, 255, 0));
	div->set_border_right_color(Color(0, 0, 255));
	div->set_border_bottom_color(Color(255, 0, 255));
	div->append_to(r);
	
	Sprite* sp1 = Sprite::create(Path::resources("res/bb.pvr"), {512, 512});
	sp1->set_scale(Vec2(0.5, 0.5));
	sp1->append_to(div);
	
	// box sprite
	
	Div* div2 = New<Div>();
	div2->set_margin(10);
	div2->set_margin_top(30);
	div2->set_width({ ValueType::MINUS, 296 });
	div2->set_height(100);
	div2->append_to(r);
	
	Sprite* sp = Sprite::create(Path::resources("res/cc.pvr"), {1024, 1024});
	//  sp->translate(-450, -450);
	sp->set_scale(0.3);
	//  sp->origin(500, 500);
	//  sp->rotate(-45);
	sp->append_to(div2);
	
	// Image
	
	Image* img = Image::create(Path::resources("res/cc.pvr"));
	img->set_width(320);
	//  img->height(250);
	img->set_opacity(0.9);
	img->set_border_radius(60);
	img->set_border_radius_right_top(40);
	img->set_border_radius_left_bottom(40);
	img->set_origin(Vec2(160, 160));
	img->set_rotate_z(-10);
	//  img->xy(50, 50);
	//  img->x(-50);
	//  img->y(-50);
	img->set_margin({ ValueType::AUTO, 0 });
	img->set_margin_top(20);
	img->set_margin_bottom(0);
	img->set_border_width(5);
	img->set_border_top_width(15);
	img->set_border_right_width(15);
	img->set_border_color(Color(255, 0, 255));
	//  img->border_top_color(Color(0, 255, 255));
	//  img->border_bottom_color(Color(0, 0, 255));
	//  img->border_left_color(Color(255, 0, 0));
	//  img->border_bottom_width(0);
	img->append_to(r);
	
}

void test_gui() {
	GUIApplication app;
	app.XX_ON(load, onload_handle);
	app.run();
}
