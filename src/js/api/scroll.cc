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

#include "./ui.h"
#include "../../ui/view/scroll.h"

namespace qk { namespace js {

	void inheritScrollView(JSClass* cls, Worker* worker) {
		typedef Object Type;
		Js_UIObject_Accessor(ScrollView, bool, scrollbar, scrollbar);
		Js_UIObject_Accessor(ScrollView, bool, bounce, bounce);
		Js_UIObject_Accessor(ScrollView, bool, bounce_lock, bounceLock);
		Js_UIObject_Accessor(ScrollView, bool, momentum, momentum);
		Js_UIObject_Accessor(ScrollView, bool, lock_direction, lockDirection);
		Js_UIObject_Accessor(ScrollView, float, scroll_x, scrollX);
		Js_UIObject_Accessor(ScrollView, float, scroll_y, scrollY);
		Js_UIObject_Accessor(ScrollView, Vec2, scroll, scroll);
		Js_UIObject_Accessor(ScrollView, float, resistance, resistance);
		Js_UIObject_Accessor(ScrollView, float, catch_position_x, catchPositionX);
		Js_UIObject_Accessor(ScrollView, float, catch_position_y, catchPositionY);
		Js_UIObject_Accessor(ScrollView, Color, scrollbar_color, scrollbarColor);
		Js_UIObject_Accessor(ScrollView, float, scrollbar_width, scrollbarWidth);
		Js_UIObject_Accessor(ScrollView, float, scrollbar_margin, scrollbarMargin);
		Js_UIObject_Accessor(ScrollView, uint32_t, scroll_duration, scrollDuration);
		Js_UIObject_Accessor(ScrollView, Curve, default_curve, defaultCurve);
		Js_UIObject_Acce_Get(ScrollView, bool, scrollbar_h, scrollbarH);
		Js_UIObject_Acce_Get(ScrollView, bool, scrollbar_v, scrollbarV);
		Js_UIObject_Acce_Get(ScrollView, Vec2, scroll_size, scrollSize);

		Js_Class_Method(scrollTo, {
			if (args.length() < 1) {
				Js_Throw(
					"@method ScrollView.scrollTo(vec2[,uint64_t[,curve]])\n"
					"@param val {Vec2}\n"
					"@param [duration] {uint64_t}\n"
					"@param [curve] {Curve}\n"
				);
			}
			Vec2 value;
			uint64_t duration = 0;
			{
				Js_Parse_Type(Vec2, args[0], "@method ScrollView.scrollTo(value = %s)");
				value = out;
			}
			if (args.length() > 1) {
				Js_Parse_Type(uint32_t, args[1], "@method ScrollView.scrollTo(value, duration = %s)");
				duration = out;
			}
			Js_UISelf(ScrollView);
			if (args.length() > 2) {
				Js_Parse_Type(Curve, args[2], "@method ScrollView.scrollTo(value, duration, curve = %s)");
				self->scrollTo(value, duration, out);
			} else {
				self->scrollTo(value, duration);
			}
		});

		Js_Class_Method(terminate, {
			Js_UISelf(ScrollView);
			self->terminate();
		});
	}

	class MixScroll: public MixViewObject {
	public:
		virtual ScrollView* asScrollView() { return self<Scroll>(); }
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Scroll, Box, { Js_NewView(Scroll); });
			inheritScrollView(cls, worker);
			cls->exports("Scroll", exports);
		}
	};

	void binding_scroll(JSObject* exports, Worker* worker) {
		MixScroll::binding(exports, worker);
	}
} }
