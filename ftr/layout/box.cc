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

#include "./box.h"

namespace ftr {

	/**
		* @constructors
		*/
	Box::Box()
		: _layout_weight(0)
		, _fill(nullptr)
	{
	}

	/**
		* @destructor
		*/
	Box::~Box() {
	}
	
	/**
		*
		* Accepting visitors
		* 
		* @func accept(visitor)
		*/
	void Box::accept(Visitor *visitor) {
		visitor->visitBox(this);
	}

	bool Box::layout_forward(uint32_t mark) {
		// ...
		return true;
	}

	bool Box::layout_reverse(uint32_t mark) {
		// ...
		return true;
	}

	void Box::layout_recursive(uint32_t mark) {
		View::layout_recursive(mark);
	}

	Vec2 Box::layout_offset() {
		return _layout_offset;
	}

	Vec2 Box::layout_size() {
		return _layout_size;
	}

	Vec2 Box::layout_content_size(bool& is_explicit_out) {
		// is_explicit_out = false;
		return Vec2();
	}

	float Box::layout_weight() {
		return _layout_weight;
	}

	void Box::set_layout_weight(float val) {
		if (_layout_weight != val) {
			_layout_weight = val;
			if (parent()) {
				parent()->layout_weight_change_notice_from_child(this);
			}
		}
	}

	Vec2 Box::layout_transform_origin(Transform& t) {
		// TODO compute transform origin ...
		return Vec2();
	}

	void Box::lock_layout_size(bool is_lock, Vec2 layout_size) {
		// ...
	}

	void Box::set_layout_offset(Vec2 val) {
		if (val != _layout_offset) {
			_layout_offset = val;
			mark_recursive(M_TRANSFORM); // mark recursive transform
		}
	}

	void Box::set_layout_offset_lazy() {
		// ...
	}

	void Box::layout_content_change_notice(Layout* child) {
		// ... 
	}

	void Box::layout_weight_change_notice_from_child(Layout* child) {
		// noop
	}

	void View::layout_size_change_notice_from_parent(Layout* parent) {
		// ...
		// mark(M_LAYOUT_SIZE);
	}

}