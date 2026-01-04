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

#ifndef __quark__view__label__
#define __quark__view__label__

#include "./view.h"
#include "../text/text_blob.h"
#include "../text/text_lines.h"
#include "../text/text_opts.h"

namespace qk {

	/**
	 * This is a text-type view that needs to be used within a Text container.
	*/
	class Qk_EXPORT Label: public View, public TextOptions {
	public:
		Label();
		Qk_DEFINE_VIEW_PROPERTY(String, value);
		Qk_DEFINE_VIEW_PROPERTY(Align, align, Const); //!< view align in parent box
		virtual ViewType viewType() const override;
		virtual TextOptions* asTextOptions() override;
		virtual void layout_forward(uint32_t mark) override;
		virtual void layout_reverse(uint32_t mark) override;
		virtual void layout_text(TextLines *lines, TextOptions* opts) override;
		virtual void text_config(TextOptions* inherit) override;
		virtual void set_layout_offset(Vec2 val) override;
		virtual void set_layout_offset_free(Vec2 size) override;
		virtual void solve_visible_area(const Mat &mat) override;
		virtual void onActivate() override;
		virtual void draw(Painter *render) override;
		virtual View* getViewForTextOptions() override;
		virtual Vec2 layout_size() override;
		virtual float layout_lock_width(float size) override;
		virtual float layout_lock_height(float size) override;
		virtual Align layout_align() override;
	private:
		Sp<TextLinesCore> _lines;
		Array<TextBlob> _blob;
		Array<uint32_t> _blob_visible;
		friend class Painter;
	};

}
#endif
