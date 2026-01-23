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

#include "../../ui/event.h"
#include "../../ui/app.h"
#include "../../ui/window.h"
#include "./android.h"

namespace qk
{
	// ***************** A p p l i c a t i o n *****************

	void Application::openURL(cString& url) {
		Android_open_url(url);
	}

	void Application::sendEmail(cString& recipient, cString& subject, cString& cc, cString& bcc, cString& body) {
		Android_send_email(recipient, subject, cc, bcc, body);
	}

	// ***************** E v e n t . D i s p a t c h *****************

	void EventDispatch::setVolumeUp() {
		Android_set_volume_up();
	}

	void EventDispatch::setVolumeDown() {
		Android_set_volume_down();
	}

	void EventDispatch::setImeKeyboardCanBackspace(bool can_backspace, bool can_delete) {
		Android_ime_keyboard_can_backspace(can_backspace, can_delete);
	}

	void EventDispatch::setImeKeyboardAndOpen(KeyboardOptions options) {
		Android_ime_keyboard_open(options.cancel_marked, int(options.keyboard_type), int(options.return_type));
		Android_ime_keyboard_can_backspace(options.can_backspace, options.can_delete);
	}

	void EventDispatch::setImeKeyboardClose() {
		Android_ime_keyboard_close();
	}

	void EventDispatch::setImeKeyboardSpotRect(Rect rect) {
	}

	void EventDispatch::cancelImeMarked() {
		Android_ime_keyboard_cancel_marked();
	}
} // namespace qk


extern "C" {
	using namespace qk;

	Qk_EXPORT void Java_org_quark_IMEHelper_dispatchIMEDelete(JNIEnv* env, jclass clazz, jint count) {
		shared_app()->activeWindow()->dispatch()->onImeDelete(count);
	}

	Qk_EXPORT void Java_org_quark_IMEHelper_dispatchIMEInsert(JNIEnv* env, jclass clazz, jstring text) {
		shared_app()->activeWindow()->dispatch()->onImeInsert(JNI::jstring_to_string(text));
	}

	Qk_EXPORT void Java_org_quark_IMEHelper_dispatchIMEMarked(JNIEnv* env, jclass clazz, jstring text, jint newCursor) {
		shared_app()->activeWindow()->dispatch()->onImeMarked(JNI::jstring_to_string(text), newCursor);
	}

	Qk_EXPORT void Java_org_quark_IMEHelper_dispatchIMEUnmark(JNIEnv* env, jclass clazz, jstring text) {
		shared_app()->activeWindow()->dispatch()->onImeUnmark(JNI::jstring_to_string(text));
	}

	Qk_EXPORT void Java_org_quark_IMEHelper_dispatchKeyboardInput(JNIEnv* env, jclass clazz,
		jint keycode, jboolean ascii, jboolean down, jint repeat, jint device, jint source) {
		shared_app()->activeWindow()->dispatch()->keyboard()->
			dispatch(keycode, ascii, down, false, repeat, device, source);
	}

	Qk_EXPORT int main(int argc, char* argv[]) {
		// TODO: Maybe it will be useful
		Application::runMain(argc, argv, true);
		return 0;
	}
}
