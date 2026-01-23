/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, Louis.chu
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

#include "./input.h"
// #include "../app.h"
#include "../window.h"

namespace qk {

	template<class T, typename... Args>
	inline static Handle<T> NewEvent(Args... args) { return new T(args...); }

	template<class T, typename... Args>
	inline static void triggerEvent(View *self, cUIEventName &name, Args... args) {
		self->pre_render().post(Cb([&name, args...](auto &e) {
			auto self = static_cast<View*>(e.data);
			self->trigger(name, **NewEvent<T>(self, args...));
		}), self);
	}

	static void setImeKeyboardAndOpen(InputSink *self) {
		if ( self->is_focus() ) {
			self->window()->dispatch()->
				setImeKeyboardAndOpen({ self->keyboard_type(), self->return_type(), self->input_spot_rect(), false,
					self->input_can_backspace(), self->input_can_delete() });
		}
	}

	InputSink::InputSink(): _cursor_index(0), _spot_rect(Rect()),
		_can_delete(true), _can_backspace(true), _is_marked_text(false), _readonly(false),
		_keyboard_type(KeyboardType::Normal), _return_type(KeyboardReturnType::Normal) {
	}

	uint32_t InputSink::marked_text_length() const {
		return _marked_text.length();
	}

	void InputSink::set_can_delete(bool val) {
		_can_delete = val;
		window()->dispatch()->setImeKeyboardCanBackspace(_can_backspace, _can_delete);
	}

	void InputSink::set_can_backspace(bool val) {
		_can_backspace = val;
		window()->dispatch()->setImeKeyboardCanBackspace(_can_backspace, _can_delete);
	}

	void InputSink::set_readonly(bool val, bool isRt) {
		if (_readonly != val) {
			_readonly = val;
			if (val && is_focus()) {
				if (isRt)
					pre_render().post(Cb([](auto e) {
						auto self = static_cast<InputSink*>(e.data);
						if (self->_readonly)
							self->blur();
					}), this);
				else
					blur();
			}
		}
	}

	void InputSink::set_spot_rect(Rect val) {
		if (val.begin != _spot_rect.begin || val.size != _spot_rect.size) {
			_spot_rect = val;
			setImeKeyboardAndOpen(this);
		}
	}

	void InputSink::set_keyboard_type(KeyboardType value, bool isRt) {
		if (value != _keyboard_type) {
			_keyboard_type = value;
			setImeKeyboardAndOpen(this);
		}
	}

	void InputSink::set_return_type(KeyboardReturnType value, bool isRt) {
		if (value != _return_type) {
			_return_type = value;
			setImeKeyboardAndOpen(this);
		}
	}

	void InputSink::cancel_marked_text() {
		if (is_focus()) {
			window()->dispatch()->cancelImeMarked();
			if (_is_marked_text) {
				window()->loop()->post(Cb([this](auto) { // trigger unmark event in next loop
					input_unmark(String());
				}, this), true); // always async post to main loop
			}
		}
	}

	bool InputSink::can_become_focus() {
		return !_readonly;
	}

	ViewType InputSink::view_type() const {
		return kInputSink_ViewType;
	}

	TextInput* InputSink::asTextInput() {
		return this;
	}

	void InputSink::input_delete(int count) {
		triggerEvent<InputEvent>(this, UIEvent_InputDelete, String(), 0, KEYCODE_NONE);
	}

	void InputSink::input_insert(cString& text) {
		triggerEvent<InputEvent>(this, UIEvent_InputInsert, text, 0, KEYCODE_NONE);
	}

	void InputSink::input_marked(cString& text, int caret_pos) {
		int safe_caret = caret_pos % (text.length() + 1);
		// Android 兼容：负数表示从末尾算,裁剪到合法区间
		if (safe_caret < 0) {
			safe_caret += (text.length() + 1);
		}
		_cursor_index = safe_caret;
		_marked_text = text;
		_is_marked_text = true;
		triggerEvent<InputEvent>(this, UIEvent_InputMarked, text, _cursor_index, KEYCODE_NONE);
	}

	void InputSink::input_unmark(cString& text) {
		if (_is_marked_text) {
			_is_marked_text = false;
			_cursor_index = 0;
			_marked_text = String();
			triggerEvent<InputEvent>(this, UIEvent_InputUnmark, text, 0, KEYCODE_NONE);
		}
	}

	void InputSink::input_control(KeyboardCode code) {
		triggerEvent<InputEvent>(this, UIEvent_InputControl, String(), 0, code);
	}

	bool InputSink::input_can_delete() {
		return _can_delete;
	}

	bool InputSink::input_can_backspace() {
		return _can_backspace;
	}

	Rect InputSink::input_spot_rect() {
		return {position() + _spot_rect.begin, _spot_rect.size};
	}

	void InputSink::input_close() {
		input_unmark(_marked_text);
	}

	KeyboardType InputSink::input_keyboard_type() {
		return _keyboard_type;
	}

	KeyboardReturnType InputSink::input_keyboard_return_type() {
		return _return_type;
	}

	Object* InputSink::asObject() {
		return this;
	}
}
