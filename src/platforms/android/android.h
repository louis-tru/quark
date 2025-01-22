// @private head
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

#ifndef __quark_platforms_android_android__
#define __quark_platforms_android_android__

#include ".././util/macros.h"

#if Qk_ANDROID
#include "../../util/jni.h"

namespace qk {

	class Android {
	public:
		static void ime_keyboard_open(bool clear, int type, int return_type);
		static void ime_keyboard_can_backspace(bool can_backspace, bool can_delete);
		static void ime_keyboard_close();
		static void prevent_screen_sleep(bool prevent);
		static int  get_status_bar_height();
		static void set_visible_status_bar(bool visible);
		static void set_status_bar_style(int style);
		static void set_fullscreen(bool fullscreen);
		static int  get_orientation();
		static void set_orientation(int orientation);
		static float get_display_scale();
		static bool is_screen_on();
		static void set_volume_up();
		static void set_volume_down();
		static void open_url(cString& url);
		static void send_email(cString& recipient, cString& subject, cString& cc, cString& bcc, cString& body);
		static String start_cmd();
		static String version();
		static String brand();
		static String model();
		static int    network_status();
		static bool   is_ac_power();
		static bool   is_battery();
		static float  battery_level();
		static String language();
		static uint64 available_memory();
		static uint64 memory();
		static uint64 used_memory();
	};
}

extern "C" {
	Qk_Export void Java_org_quark_Android_initPaths(JNIEnv* env, jclass clazz, jstring package, jstring files_dir, jstring cache_dir);
}

#endif
#endif