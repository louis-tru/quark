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

#ifndef __quark__layout__flex__
#define __quark__layout__flex__

#include "./box.h"

namespace qk {

	class Qk_EXPORT FlexLayout: public BoxLayout {
	public:
		FlexLayout(Window *win);
		// define props
		Qk_DEFINE_PROP(Direction, direction); // typesetting direction
		Qk_DEFINE_PROP(ItemsAlign, items_align); // alignment mode of the main axis
		Qk_DEFINE_PROP(CrossAlign, cross_align); // alignment mode of the cross axis
		// @overwrite
		virtual bool layout_forward(uint32_t mark) override;
		virtual bool layout_reverse(uint32_t mark) override;
		virtual Vec2 layout_lock(Vec2 layout_size) override;
		virtual bool is_lock_child_layout_size() override;
		virtual void onChildLayoutChange(Layout* child, uint32_t mark) override;
	private:
		template<bool is_horizontal> void layout_typesetting_auto(bool is_reverse);
		template<bool is_horizontal> void layout_typesetting_flex(bool is_reverse);
		void layout_typesetting_auto_impl(bool is_horizontal, bool is_reverse);
		bool update_IsLockChild();
		bool _is_lock_child;
		friend class FlowLayout;
		Qk_DEFINE_INLINE_CLASS(Inl);
	};

	class Qk_EXPORT Flex: public Box {
	public:
		Qk_Define_View(Flex, Box);
	};

}
#endif
