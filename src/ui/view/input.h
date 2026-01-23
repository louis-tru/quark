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

#ifndef __quark__view__input__
#define __quark__view__input__

#include "./box.h"
#include "../text/text_blob.h"
#include "../text/text_opts.h"
#include "../text/text_lines.h"
#include "../text/text_input.h"

namespace qk {

	/**
	 * Input sink view, implement TextInput protocol only
	*/
	class Qk_EXPORT InputSink: public View, public TextInput {
	public:
		InputSink();
		Qk_DEFINE_PROP_GET(String, marked_text, Const);
		Qk_DEFINE_ACCE_GET(uint32_t, marked_text_length, Const);
		Qk_DEFINE_PROP_GET(uint32_t, cursor_index, Const); // marked text cursor position
		// spotRect is an IME anchor only.
		// It does not affect rendering or layout.
		Qk_DEFINE_PROPERTY(Rect, spot_rect, Const);
		Qk_DEFINE_PROPERTY(bool, can_delete, Const);
		Qk_DEFINE_PROPERTY(bool, can_backspace, Const);
		Qk_DEFINE_PROP_GET(bool, is_marked_text, Const);
		Qk_DEFINE_VIEW_PROPERTY(bool, readonly, Const);
		Qk_DEFINE_VIEW_PROPERTY(KeyboardType, keyboard_type, Const);
		Qk_DEFINE_VIEW_PROPERTY(KeyboardReturnType, return_type, Const);
		void cancel_marked_text();
		bool can_become_focus() override;
		ViewType view_type() const override;
		TextInput* asTextInput() override;
		void input_delete(int count) override;
		void input_insert(cString& text) override;
		void input_marked(cString& text, int caret) override;
		void input_unmark(cString& text) override;
		void input_control(KeyboardCode name) override;
		bool input_can_delete() override;
		bool input_can_backspace() override;
		Rect input_spot_rect() override;
		void input_close() override;
		KeyboardType input_keyboard_type() override;
		KeyboardReturnType input_keyboard_return_type() override;
		Object* asObject() override;
	};

	/**
	 * This is a text input view that cannot have subviews.
	*/
	class Qk_EXPORT Input: public Box
		, public TextOptions, public RenderTask, public TextInput {
		Qk_DEFINE_INLINE_CLASS(Inl);
	public:
		Qk_DEFINE_VIEW_PROPERTY(bool, security, Const);
		Qk_DEFINE_VIEW_PROPERTY(bool, readonly, Const);
		Qk_DEFINE_VIEW_PROP_GET(bool, is_marked_text, Const);
		Qk_DEFINE_VIEW_PROPERTY(KeyboardType, keyboard_type, Const);
		Qk_DEFINE_VIEW_PROPERTY(KeyboardReturnType, return_type, Const);
		Qk_DEFINE_VIEW_PROPERTY(String4, value_u4, Const);
		Qk_DEFINE_VIEW_PROPERTY(String4, placeholder_u4, Const);
		Qk_DEFINE_VIEW_PROPERTY(Color, placeholder_color, Const);
		Qk_DEFINE_VIEW_PROPERTY(Color, cursor_color, Const);
		Qk_DEFINE_VIEW_PROPERTY(uint32_t, max_length, Const);
		Qk_DEFINE_VIEW_ACCESSOR(String, value, Const);
		Qk_DEFINE_VIEW_ACCESSOR(String, placeholder, Const);
		Qk_DEFINE_VIEW_ACCE_GET(uint32_t, text_length, Const);
		Qk_DEFINE_VIEW_PROP_GET(uint32_t, cursor_index, Const); // cursor index in text
		Qk_DEFINE_VIEW_PROP_GET(uint32_t, cursor_line, Const); // cursor line number in text
		Qk_DEFINE_VIEW_ACCE_GET(String, marked_text, Const);
		Qk_DEFINE_VIEW_PROP_GET(uint32_t, marked_text_index, Const); // marked text start index in text
		Qk_DEFINE_VIEW_ACCE_GET(uint32_t, marked_text_length, Const);

		Input();
		void cancel_marked_text();
		virtual bool is_multiline();
		// @override
		virtual bool can_become_focus() override;
		virtual ViewType view_type() const override;
		virtual void layout_forward(uint32_t mark) override;
		virtual void layout_reverse(uint32_t mark) override;
		virtual void text_config(TextOptions* inherit) override;
		virtual Vec2 layout_offset_inside() override;
		virtual void solve_marks(const Mat &mat, View *parent, uint32_t mark) override;
		virtual void solve_visible_area(const Mat &mat) override;
		virtual void onActivate() override;
		virtual TextInput* asTextInput() override;
		virtual TextOptions* asTextOptions() override;
		virtual bool run_task(int64_t time, int64_t deltaTime) override;
		// impl text input
		virtual void input_delete(int count) override;
		virtual void input_insert(cString& text) override;
		virtual void input_marked(cString& text, int caret) override;
		virtual void input_unmark(cString& text) override;
		virtual void input_control(KeyboardCode name) override;
		virtual bool input_can_delete() override;
		virtual bool input_can_backspace() override;
		virtual Rect input_spot_rect() override;
		virtual void input_close() override;
		virtual KeyboardType input_keyboard_type() override;
		virtual KeyboardReturnType input_keyboard_return_type() override;
		virtual Object* asObject() override;
		virtual void draw(Painter *render) override;
	protected:
		Vec2 layout_typesetting_input_text();
		void solve_cursor_offset();
		virtual View* getViewForTextOptions() override;
		virtual Vec2 input_text_offset();
		virtual void set_input_text_offset(Vec2 val);
		virtual View* init(Window *win) override;
	private:
		Sp<TextLinesCore> _lines;
		Array<TextBlob> _blob;
		Array<uint32_t> _blob_visible;
		String4 _marked_text;
		Color   _marked_color;
		uint32_t  _marked_blob_begin, _marked_blob_end;
		float _cursor_x, _input_text_offset_x, _input_text_offset_y;
		float _cursor_ascent, _cursor_height;
		bool  _editing, _cursor_twinkle_status;
		char  _flag;
		Vec2  _point;
		Mat _mat; // position matrix

		friend class Textarea;
		friend class Painter;
	};

}
#endif
