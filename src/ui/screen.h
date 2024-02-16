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

#ifndef __quark__screen__
#define __quark__screen__

#include "../util/util.h"
#include "../render/math.h"
#include "../util/event.h"

namespace qk {
	class Application;

	/**
	 * @class Screen some common method properties and events for display and screen
	*/
	class Qk_EXPORT Screen: public Object {
		Qk_HIDDEN_ALL_COPY(Screen);
	public:
		enum Orientation {
			ORIENTATION_INVALID = -1,
			ORIENTATION_PORTRAIT = 0,
			ORIENTATION_LANDSCAPE,
			ORIENTATION_REVERSE_PORTRAIT,
			ORIENTATION_REVERSE_LANDSCAPE,
			ORIENTATION_USER,
			ORIENTATION_USER_PORTRAIT,
			ORIENTATION_USER_LANDSCAPE,
			ORIENTATION_USER_LOCKED,
		};

		enum StatusBarStyle {
			STATUS_BAR_STYLE_WHITE = 0,
			STATUS_BAR_STYLE_BLACK,
		};

		/**
		 * @event onOrientation Triggered when the screen orientation changes
		*/
		Qk_Event(Orientation);

		Qk_DEFINE_PROP_GET(Application*, host); // host app
		Qk_DEFINE_PROP_ACC(Orientation, orientation, Const); // orientation
		Qk_DEFINE_PROP_ACC_GET(float, status_bar_height, Const); // status_bar_height

		/**
		 * @constructor
		*/
		Screen(Application* host);

		/**
		* @method set_visible_status_bar(visible)
		*/
		void set_visible_status_bar(bool visible);

		/**
		* @method set_status_bar_style(style)
		*/
		void set_status_bar_style(StatusBarStyle style);

		/**
		* @method set_keep_screen(keep)
		*/
		void set_keep_screen(bool keep);

		/**
		 * @method set_fullscreen(fullscreen)
		*/
		void set_fullscreen(bool fullscreen);

		/**
		 * @method default_atom_pixel
		*/
		static float default_atom_pixel();

		/**
		 * @method default_status_bar_height
		*/
		static float default_status_bar_height();
	};

}
#endif
