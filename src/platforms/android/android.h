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

#ifndef __quark__platforms__android__android__
#define __quark__platforms__android__android__

#include "../../util/macros.h"
#if Qk_ANDROID
#include "../../util/util.h"
#include "../../util/jni.h"

namespace qk {
	void Android_ime_keyboard_open(bool clear, int type, int return_type);
	void Android_ime_keyboard_can_backspace(bool can_backspace, bool can_delete);
	void Android_ime_keyboard_close();
	void Android_prevent_screen_sleep(bool prevent);
	int  Android_get_status_bar_height();
	void Android_set_visible_status_bar(bool visible);
	void Android_set_status_bar_style(int style);
	void Android_set_fullscreen(bool fullscreen);
	int  Android_get_orientation();
	void Android_set_orientation(int orientation);
	float Android_get_display_scale();
	bool Android_is_screen_on();
	void Android_set_volume_up();
	void Android_set_volume_down();
	void Android_open_url(cString& url);
	void Android_send_email(cString& recipient,
		cString& subject, cString& cc, cString& bcc, cString& body);
	String Android_startup_argv();
	String Android_version();
	String Android_brand();
	String Android_model();
	int    Android_network_status();
	bool   Android_is_ac_power();
	bool   Android_is_battery();
	float  Android_battery_level();
	String Android_language();
	uint64_t Android_available_memory();
	uint64_t Android_memory();
	uint64_t Android_used_memory();
	void Android_resolve_msg_onmain();
}

#endif
#endif