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

#include "./view.h"
#include "../../ui/view/scroll.h"

namespace qk { namespace js {

	void inheritScrollBase(JSClass* cls, Worker* worker) {
		Js_Set_Class_Accessor(scrollbar, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->scrollbar()) );
		}, {
			Js_Parse_Type(bool, val, "@prop ScrollBase.scrollbar = %s");
			Js_ScrollBase();
			self->set_scrollbar(out);
		});

		Js_Set_Class_Accessor(bounce, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->bounce()) );
		}, {
			Js_Parse_Type(bool, val, "@prop ScrollBase.bounce = %s");
			Js_ScrollBase();
			self->set_bounce(out);
		});

		Js_Set_Class_Accessor(bounceLock, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->bounce_lock()) );
		}, {
			Js_Parse_Type(bool, val, "@prop ScrollBase.bounceLock = %s");
			Js_ScrollBase();
			self->set_bounce_lock(out);
		});

		Js_Set_Class_Accessor(momentum, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->momentum()) );
		}, {
			Js_Parse_Type(bool, val, "@prop ScrollBase.momentum = %s");
			Js_ScrollBase();
			self->set_momentum(out);
		});

		Js_Set_Class_Accessor(lockDirection, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->lock_direction()) );
		}, {
			Js_Parse_Type(bool, val, "S@prop crollBase.lockDirection = %s");
			Js_ScrollBase();
			self->set_lock_direction(out);
		});

		Js_Set_Class_Accessor_Get(scrollbarH, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->scrollbar_h()) );
		});

		Js_Set_Class_Accessor_Get(scrollbarV, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->scrollbar_v()) );
		});

		Js_Set_Class_Accessor(scrollX, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->scroll_x()) );
		}, {
			Js_Parse_Type(float, val, "@prop ScrollBase.scrollX = %s");
			Js_ScrollBase();
			self->set_scroll_x(out);
		});

		Js_Set_Class_Accessor(scrollY, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->scroll_y()) );
		}, {
			Js_Parse_Type(float, val, "@prop ScrollBase.scrollY = %s");
			Js_ScrollBase();
			self->set_scroll_y(out);
		});

		Js_Set_Class_Accessor(scroll, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->scroll()) );
		}, {
			Js_Parse_Type(Vec2, val, "@prop ScrollBase.scroll = %s");
			Js_ScrollBase();
			self->set_scroll(out);
		});

		Js_Set_Class_Accessor_Get(scrollSize, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->scroll_size()) );
		});

		Js_Set_Class_Accessor(resistance, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->resistance()) );
		}, {
			Js_Parse_Type(float, val, "* @prop ScrollBase.resistance = %s");
			Js_ScrollBase();
			self->set_resistance(out);
		});

		Js_Set_Class_Accessor(catchPositionX, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->catch_position_x()) );
		}, {
			Js_Parse_Type(float, val, "* @prop ScrollBase.catchPositionX = %s");
			Js_ScrollBase();
			self->set_catch_position_x(out);
		});

		Js_Set_Class_Accessor(catchPositionY, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->catch_position_y()) );
		}, {
			Js_Parse_Type(float, val, "* @prop ScrollBase.catchPositionY = %s");
			Js_ScrollBase();
			self->set_catch_position_y(out);
		});

		Js_Set_Class_Accessor(scrollbarColor, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->scrollbar_color()) );
		}, {
			Js_Parse_Type(Color, val, "@prop ScrollBase.scrollbarColor = %s");
			Js_ScrollBase();
			self->set_scrollbar_color(out);
		});

		Js_Set_Class_Accessor(scrollbarWidth, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->scrollbar_width()) );
		}, {
			Js_Parse_Type(float, val, "@prop ScrollBase.scrollbarWidth = %s");
			Js_ScrollBase();
			self->set_scrollbar_width(out);
		});

		Js_Set_Class_Accessor(scrollbarMargin, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->scrollbar_margin()) );
		}, {
			Js_Parse_Type(float, val, "@prop ScrollBase.scrollbarMargin = %s");
			Js_ScrollBase();
			self->set_scrollbar_margin(out);
		});

		Js_Set_Class_Accessor(scroll_duration, {
			Js_ScrollBase();
			Js_Return( worker->newInstance(self->scroll_duration()) );
		}, {
			Js_Parse_Type(uint32_t, val, "@prop ScrollBase.scrollDuration = %s");
			Js_ScrollBase();
			self->set_scroll_duration(out);
		});

		Js_Set_Class_Accessor(default_curve, {
			Js_ScrollBase();
			Js_Return( worker->types()->newInstance(self->default_curve()) );
		}, {
			Js_Parse_Type(Curve, val, "@prop ScrollBase.defaultCurve = %s");
			Js_ScrollBase();
			self->set_default_curve(out);
		});

		Js_Set_Class_Method(scrollTo, {
			if (args.length() < 1) {
				Js_Throw(
					"* @method ScrollBase.scrollTo(vec2[,int[,curve]])\n"
					"* @param val {Vec2}\n"
					"* @param duration {int}\n"
					"* @param [curve] {Curve}\n"
				);
			}
			Vec2 value;
			uint64_t duration = 0;
			{
				Js_Parse_Type(Vec2, args[0], "@method ScrollBase.scrollTo(value = %s)");
				value = out;
			}
			if (args.length() > 1) {
				Js_Parse_Type(uint32_t, args[1], "@method ScrollBase.scrollTo(value, duration = %s)");
				duration = out;
			}
			Js_ScrollBase();
			if (args.length() > 2) {
				Js_Parse_Type(Curve, args[2], "@method ScrollBase.scrollTo(value, duration, curve = %s)");
				self->scrollTo(value, duration, out);
			} else {
				self->scrollTo(value, duration);
			}
		});

		Js_Set_Class_Method(terminate, {
			Js_ScrollBase();
			self->terminate();
		});
	}

	class WrapScroll: public WrapViewObject {
	public:
		virtual ScrollBase* asScrollBase() {
			return self<Scroll>();
		}
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Scroll, Float, {
				Js_NewView(Scroll);
			});
			inheritScrollBase(cls, worker);
			cls->exports("Scroll", exports);
		}
	};

	void binding_scroll(JSObject* exports, Worker* worker) {
		WrapScroll::binding(exports, worker);
	}
} }
