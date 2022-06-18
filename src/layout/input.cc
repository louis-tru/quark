/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, xuewen.chu
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

#include "./input.h"

namespace noug {

	bool Input::can_become_focus() {
		return true;
	}

	TextInput* Input::as_text_input() {
		return this;
	}

	void Input::input_delete(int count) {
		// TODO ..
	}

	void Input::input_insert(cString& text) {
		// TODO ..
	}

	void Input::input_marked(cString& text) {
		// TODO ..
	}

	void Input::input_unmark(cString& text) {
		// TODO ..
	}

	void Input::input_control(KeyboardKeyName name) {
		// TODO ..
	}

	bool Input::input_can_delete() {
		// TODO ..
	}

	bool Input::input_can_backspace() {
		// TODO ..
	}

	Vec2 Input::input_spot_location() {
		// TODO ..
	}

	KeyboardType Input::input_keyboard_type() {
		// TODO ..
	}

	KeyboardReturnType Input::input_keyboard_return_type() {
		// TODO ..
	}

	void Input::onTextChange(uint32_t value) {
		value ? mark(value): mark_none();
	}
}