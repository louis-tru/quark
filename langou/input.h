/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
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

#ifndef __langou__input__
#define __langou__input__

#include "langou/text.h"
#include "langou/pre-render.h"

XX_NS(langou)

/**
 * @class Input
 */
class XX_EXPORT Input: public Text, public PreRender::Task, public ITextInput {
 public:
	XX_DEFINE_GUI_VIEW(INPUT, Input, input);
	
	typedef ReferenceTraits Traits;
	
	Input();
	
	/**
	 * @overwrite
	 */
	// @ITextInput
	virtual void input_delete(int count);
	virtual void input_insert(cString& text);
	virtual void input_marked(cString& text);
	virtual void input_unmark(cString& text);
	virtual void input_control(KeyboardKeyName name);
	virtual bool input_can_delete();
	virtual bool input_can_backspace();
	virtual Vec2 input_spot_location();
	virtual KeyboardType input_keyboard_type();
	virtual KeyboardReturnType input_keyboard_return_type();
	// @end
	virtual void set_value(cUcs2String& str);
	virtual View* append_text(cUcs2String& str) throw(Error);
	virtual bool run_task(int64 sys_time);
	virtual bool can_become_focus();
	virtual Object* to_object() { return this; }
	virtual ITextInput* as_itext_input() { return this; }

	/**
	 * @func type
	 */
	inline KeyboardType type() const { return type_; }
	
	/**
	 * @func return_type
	 */
	inline KeyboardReturnType return_type() const { return return_type_; }
	
	/**
	 * @func placeholder
	 */
	inline Ucs2String placeholder() const { return placeholder_; }
	
	/**
	 * @func placeholder_color
	 */
	inline Color placeholder_color() const { return placeholder_color_; }
	
	/**
	 * @func security
	 */
	inline bool security() const { return security_; }
	
	/**
	 * @func set_type
	 */
	void set_type(KeyboardType value);
	
	/**
	 * @func set_return_type
	 */
	void set_return_type(KeyboardReturnType value);
	
	/**
	 * @func set_placeholder
	 */
	void set_placeholder(cUcs2String& value);
	
	/**
	 * @func set_placeholder_color
	 */
	void set_placeholder_color(Color value);
	
	/**
	 * @func set_security
	 */
	void set_security(bool value);
	
	/**
	 * @func text_margin
	 */
	inline float text_margin() const { return text_margin_; }
	
	/**
	 * @func set_text_margin
	 */
	void set_text_margin(float value);

 protected:
	
	/**
	 * @overwrite
	 */
	virtual void draw(Draw* draw);
	virtual void set_layout_content_offset();
	
	/**
	 * @func is_multi_line_input
	 */
	virtual bool is_multi_line_input();
	
	/**
	 * @func input_text_offset
	 */
	virtual Vec2 input_text_offset();
	
	/**
	 * @func set_input_text_offset
	 */
	virtual void set_input_text_offset(Vec2 value);
	
	/**
	 * @func refresh_cursor_screen_position
	 */
	void refresh_cursor_screen_position();
	
 protected:
	Ucs2String  placeholder_, marked_text_;
	Color placeholder_color_, marked_color_;
	uint  marked_text_idx_, cursor_, cursor_linenum_;
	uint  marked_cell_begin_, marked_cell_end_;
	float text_margin_, cursor_x_, input_text_offset_x_;
	bool  editing_, cursor_twinkle_status_, security_;
	char  flag_;
	Vec2  point_;
	KeyboardType  type_;
	KeyboardReturnType  return_type_;
	
	XX_DEFINE_INLINE_CLASS(Inl);
};

XX_END

#endif
