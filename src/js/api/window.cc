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
#include "../../ui/view/root.h"

namespace qk { namespace js {

	struct MixWindow: MixObject {
		typedef Window Type;

		virtual bool addEventListener(cString& name, cString& func, int id) {
			if ( name == "Change" ) {
				self<Type>()->Js_Native_On(Change, func, id);
			} else if ( name == "Background" ) {
				self<Type>()->Js_Native_On(Background, func, id);
			} else if ( name == "Foreground" ) {
				self<Type>()->Js_Native_On(Foreground, func, id);
			} else if ( name == "Close" ) {
				self<Type>()->Js_Native_On(Close, func, id);
			} else
				return false;
			return true;
		}

		virtual bool removeEventListener(cString& name, int id) {
			if ( name == "Change" ) {
				self<Type>()->Qk_Off(Change, id);
			} else if ( name == "Background" ) {
				self<Type>()->Qk_Off(Background, id);
			} else if ( name == "Foreground" ) {
				self<Type>()->Qk_Off(Foreground, id);
			} else if ( name == "Close" ) {
				self<Type>()->Qk_Off(Close, id);
			} else
				return false;
			return true;
		}

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Window, 0, {
				checkApp(worker);
				if (args.length()) {
					Js_Parse_Type(WindowOptions, args[0], "new Window(opts) %s");
					New<MixWindow>(args, Window::Make(out));
				} else {
					New<MixWindow>(args, Window::Make({}));
				}
			});

			Js_MixObject_Accessor(Window, Vec2, size, size);

			Js_Class_Accessor_Get(scale, {
				Js_Return( self->scale() );
			});

			Js_Class_Accessor_Get(defaultScale, {
				Js_Return( self->defaultScale() );
			});

			// Qk_DEFINE_PROP_GET(RegionSize, surfaceDisplayRange, Const); //!< Select the area on the drawing surface

			Js_Class_Accessor_Get(fsp, {
				Js_Return( self->fsp() );
			});

			Js_Class_Accessor_Get(atomPixel, {
				Js_Return( self->atomPixel() );
			});

			Js_Class_Accessor_Get(navigationRect, {
				Js_Return( worker->types()->jsvalue(self->navigationRect()) );
			});

			Js_Class_Accessor_Get(root, {
				Js_Return( worker->newValue(self->root()) );
			});

			// Qk_DEFINE_PROP_GET(Application*, host); //! application host
			// Qk_DEFINE_PROP_GET(Render*, render); //! render object
			// Qk_DEFINE_PROP_GET(EventDispatch*, dispatch); //! event dispatch
			// Qk_DEFINE_PROP_GET(RootStyleSheets*, styleSheets); //! root style sheets

			Js_MixObject_Accessor(Window, Color, backgroundColor, backgroundColor);

			// Qk_DEFINE_PROP_GET(WindowImpl*, impl); //! window platform impl
			// Qk_DEFINE_PROP_GET(ActionCenter*, actionCenter); //! Action scheduling
			// Qk_DEFINE_ACCE_GET(FontPool*, fontPool); //! Font pool
			// Qk_DEFINE_ACCE_GET(RunLoop*, loop); //! host main loop

			Js_Class_Accessor_Get(focusView, {
				Js_Return( self->focusView() );
			});

			Js_Class_Accessor_Get(surfaceSize, {
				Js_Return( worker->types()->jsvalue(self->surfaceSize()) );
			});

			Js_MixObject_Accessor(Window, bool, debugMode, debugMode);

			Js_Class_Method(nextFrame, {
				if (!args.length() || !args[0]->isFunction()) {
					Js_Throw(
						"@method Window.nextFrame(cb)\n"
						"@param cb {Function}\n"
					);
				}
				self->nextFrame(get_callback_for_none(worker, args[0]));
				Js_Return(args.thisObj());
			});

			Js_Class_Method(activate, {
				self->activate();
				Js_Return(args.thisObj());
			});

			Js_Class_Method(close, {
				self->close();
			});

			Js_Class_Method(pending, {
				self->pending();
			});

			Js_Class_Method(setFullscreen, {
				if (!args.length()) {
					Js_Throw(
						"@method Window.setFullscreen(fullscreen)\n"
						"@param fullscreen {bool}\n"
					);
				}
				self->setFullscreen(args[0]->toBoolean(worker));
			});

			Js_Class_Method(setCursorStyle, {
				if (!args.length()) {
					Js_Throw(
						"@method Window.setCursorStyle(cursor)\n"
						"@param cursor {CursorStyle}\n"
					);
				}
				Js_Parse_Type(CursorStyle, args[0], "@method Window.setCursorStyle(cursor = %s)");
				self->setCursorStyle(out);
			});

			cls->exports("Window", exports);
		}
	};

	void binding_window(JSObject* exports, Worker* worker) {
		MixWindow::binding(exports, worker);
	}
} }
