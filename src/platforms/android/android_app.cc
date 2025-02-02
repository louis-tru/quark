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

#include "../../ui/event.h"
#include "../../ui/app.h"
#include "./android.h"

namespace qk
{
	// ***************** A p p l i c a t i o n *****************

	void Application::openURL(cString& url) {
		Android::open_url(url);
	}

	void Application::sendEmail(cString& recipient, cString& subject, cString& cc, cString& bcc, cString& body) {
		Android::send_email(recipient, subject, cc, bcc, body);
	}

	// ***************** E v e n t . D i s p a t c h *****************

	void EventDispatch::setVolumeUp() {
		Android::set_volume_up();
	}

	void EventDispatch::setVolumeDown() {
		Android::set_volume_down();
	}

	void EventDispatch::setImeKeyboardCanBackspace(bool can_backspace, bool can_delete) {
		Android::ime_keyboard_can_backspace(can_backspace, can_delete);
	}

	void EventDispatch::setImeKeyboardOpen(KeyboardOptions options) {
		Android::ime_keyboard_open(options.clear, int(options.type), int(options.return_type));
	}

	void EventDispatch::setImeKeyboardClose() {
		Android::ime_keyboard_close();
	}

	void EventDispatch::setImeKeyboardSpotRect(Rect rect) {
	}
} // namespace qk


extern "C" {

	Qk_EXPORT void Java_org_quark_IMEHelper_dispatchIMEDelete(JNIEnv* env, jclass clazz, jint count) {
		shared_app()->dispatch()->dispatch_ime_delete(count);
	}

	Qk_EXPORT void Java_org_quark_IMEHelper_dispatchIMEInsert(JNIEnv* env, jclass clazz, jstring text) {
		shared_app()->dispatch()->dispatch_ime_insert(JNI::jstring_to_string(text));
	}

	Qk_EXPORT void Java_org_quark_IMEHelper_dispatchIMEMarked(JNIEnv* env, jclass clazz, jstring text) {
		shared_app()->dispatch()->dispatch_ime_marked(JNI::jstring_to_string(text));
	}

	Qk_EXPORT void Java_org_quark_IMEHelper_dispatchIMEUnmark(JNIEnv* env, jclass clazz, jstring text) {
		shared_app()->dispatch()->dispatch_ime_unmark(JNI::jstring_to_string(text));
	}
	
	Qk_EXPORT void Java_org_quark_IMEHelper_dispatchKeyboardInput(JNIEnv* env, jclass clazz,
		jint keycode, jboolean ascii, jboolean down, jint repeat, jint device, jint source) {
		shared_app()->dispatch()->keyboard_adapter()->
			dispatch(keycode, ascii, down, repeat, device, source);
	}

}
