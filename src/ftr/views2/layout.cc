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

#include "layout.h"
#include "app.h"
#include "pre-render.h"

FX_NS(ftr)

/**
 * @constructor
 */
Layout::Layout()
: _parent_layout(nullptr) {
	_need_draw = false;
}

/**
 * @overwrite
 */
void Layout::remove() {
	View* Parent = parent();
	if (Parent) {
		_parent_layout = nullptr;
		Layout* layout = Parent->as_layout();
		if ( layout && layout->level() ) {
			layout->mark_pre(M_CONTENT_OFFSET);
		}
	}
	View::remove();
}

/**
 * @overwrite
 */
void Layout::remove_all_child() {
	if (first())
		mark_pre(M_CONTENT_OFFSET);
	View::remove_all_child();
}

/**
 * @overwrite
 */
void Layout::set_parent(View* parent) throw(Error) {
	
	View* old_parent = View::parent();
	
	_parent_layout = nullptr;
	
	if (parent != old_parent) {
		if (old_parent) {
			Layout* layout = old_parent->as_layout();
			if (layout) { // 旧的父视图必然受影响
				layout->mark_pre(M_CONTENT_OFFSET);
			}
		}
		View::set_parent(parent);
		// 父视图的变化会影响继承类型的M_TEXT_FONT值
		mark_pre(M_LAYOUT | M_SIZE_HORIZONTAL | M_SIZE_VERTICAL | M_TEXT_FONT );
	} 
	else { // 相同父视图也可能受到影响
		Layout* layout = parent->as_layout();
		if (layout) {
			layout->mark_pre(M_CONTENT_OFFSET);
		}
	}
}

FX_END
