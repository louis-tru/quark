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

#ifndef __noug__layout__input__
#define __noug__layout__input__

#include "./box.h"
#include "../text_blob.h"
#include "../text_opts.h"
#include "../text_lines.h"
#include "../text_input.h"

namespace noug {

	class N_EXPORT Input: public Box, public TextOptions, public TextInput {
		N_Define_View(Input);
	public:
		N_DEFINE_PROP(bool, is_multiline);
		N_DEFINE_PROP(TextAlign, text_align);
		N_DEFINE_PROP(String, text_value);
		// @override
		virtual bool layout_reverse(uint32_t mark) override;
		virtual bool solve_visible_region() override;
		virtual void set_visible(bool val) override;
		virtual void set_parent(View *val) override;
		virtual bool is_allow_append_child() override;
		virtual bool can_become_focus() override;
		virtual TextInput* as_text_input() override;
		virtual void input_delete(int count) override;
		virtual void input_insert(cString& text) override;
		virtual void input_marked(cString& text) override;
		virtual void input_unmark(cString& text) override;
		virtual void input_control(KeyboardKeyName name) override;
		virtual bool input_can_delete() override;
		virtual bool input_can_backspace() override;
		virtual Vec2 input_spot_location() override;
		virtual KeyboardType input_keyboard_type() override;
		virtual KeyboardReturnType input_keyboard_return_type() override;
	protected:
		virtual void onTextChange(uint32_t mark) override;
	private:
		Array<TextBlob> _blob;
		Array<uint32_t> _blob_visible;
		Sp<TextLines> _lines;
	};

}
#endif
