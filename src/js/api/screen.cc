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
#include "../../ui/screen.h"
#include "../../ui/app.h"

namespace qk { namespace js {

	struct MixScreen: MixObject {
		typedef Screen Type;

		virtual bool addEventListener(cString& name, cString& func, int id) {
			if ( name == "Orientation" ) {
				self<Type>()->Js_Native_On(Orientation, func, id);
				return true;
			}
			return false;
		}

		virtual bool removeEventListener(cString& name, int id) {
			if ( name == "Orientation" ) {
				self<Type>()->Qk_Off(Orientation, id);
				return true;
			}
			return false;
		}

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Screen, 0, {
				Js_Throw("Access forbidden.");
			});

			// Qk_DEFINE_P_GET(Application*, host); // host app

			Js_Class_Accessor(orientation, {
				Js_Self(Type);
				Js_Return( self->orientation() );
			}, {
				Js_Parse_Type(uint32_t, val, "@prop Screen.orientation = %s");
				Js_Self(Type);
				self->set_orientation(Screen::Orientation(out));
			});

			Js_Class_Accessor_Get(statusBarHeight, {
				Js_Self(Type);
				Js_Return( self->status_bar_height() );
			});

			Js_Class_Method(setVisibleStatusBar, {
				if (!args.length()) {
					Js_Throw("@method Screen.setVisibleStatusBar(visible), Parameter cannot be empty");
				}
				Js_Parse_Type(bool, args[0], "@method Screen.setVisibleStatusBar(visible = %s)");
				Js_Self(Type);
				self->set_visible_status_bar(out);
			});

			Js_Class_Method(setStatusBarStyle, {
				if (!args.length()) {
					Js_Throw("@method Screen.setStatusBarStyle(style), Parameter cannot be empty");
				}
				Js_Parse_Type(uint32_t, args[0], "@method Screen.setStatusBarStyle(style = %s)");
				Js_Self(Type);
				self->set_status_bar_style(Screen::StatusBarStyle(out));
			});

			Js_Class_Method(preventScreenSleep, {
				if (!args.length()) {
					Js_Throw("@method Screen.preventScreenSleep(prevent), Parameter cannot be empty");
				}
				Js_Parse_Type(bool, args[0], "@method Screen.preventScreenSleep(prevent = %s)");
				Js_Self(Type);
				self->prevent_screen_sleep(out);
			});

			Js_Method(mainScreenScale, {
				Js_Return( Type::main_screen_scale() );
			});

			cls->exports("Screen", exports);
		}
	};

	void binding_screen(JSObject* exports, Worker* worker) {
		MixScreen::binding(exports, worker);
	}
} }
