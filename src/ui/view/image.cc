/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
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

#include "./image.h"
#include "../../render/render.h"
#include "../window.h"
#include "../app.h"
#include "../../errno.h"

namespace qk {

	ViewType Image::viewType() const {
		return kImage_ViewType;
	}

	bool Image::layout_forward(uint32_t mark) {
		if (mark & (kLayout_Size_Width | kLayout_Size_Height)) {
			mark |= (kLayout_Size_Width | kLayout_Size_Height);
		}
		return Box::layout_forward(mark);
	}

	float Image::solve_layout_content_width(Size &parent_layout_size) {
		auto result = Box::solve_layout_content_width(parent_layout_size);
		auto src = ImageSourceHolder::source(); // Rt

		if (parent_layout_size.wrap_x && src && src->type()) { // wrap x
			auto v = Box::solve_layout_content_height(parent_layout_size);
			if (parent_layout_size.wrap_y) { // wrap y
				result = src->width();
			} else {
				result = v / src->height() * src->width();
			}
		}
		parent_layout_size.wrap_x = false;
		return result;
	}

	float Image::solve_layout_content_height(Size &parent_layout_size) {
		auto result = Box::solve_layout_content_height(parent_layout_size);
		auto src = ImageSourceHolder::source(); // Rt

		if (parent_layout_size.wrap_y && src && src->type()) { // wrap y
			auto v = Box::solve_layout_content_width(parent_layout_size);
			if (parent_layout_size.wrap_x) { // wrap x
				result = src->height();
			} else {
				result = v / src->width() * src->height();
			}
		}
		parent_layout_size.wrap_y = false;

		return result;
	}

	void Image::onSourceState(Event<ImageSource, ImageSource::State>& evt) {
		if (*evt.data() & ImageSource::kSTATE_LOAD_COMPLETE) {
			mark_size(kLayout_Size_Width | kLayout_Size_Height, false);
			Sp<UIEvent> evt = New<UIEvent>(this);
			trigger(UIEvent_Load, **evt);
		} else if (*evt.data() & (ImageSource::kSTATE_LOAD_ERROR | ImageSource::kSTATE_DECODE_ERROR)) {
			Error err(ERR_IMAGE_LOAD_ERROR, "ERR_IMAGE_LOAD_ERROR");
			Sp<UIEvent> evt = New<UIEvent>(this, err);
			trigger(UIEvent_Error, **evt);
		}
	}

	ImagePool* Image::imgPool() {
		return window()->host()->imgPool();
	}

	String Image::src() const {
		return ImageSourceHolder::src();
	}

	ImageSource* Image::source() {
		return ImageSourceHolder::source();
	}

	void Image::set_src(String val, bool isRt) {
		set_source(window()->host()->imgPool()->get(val), isRt);
	}

	void Image::set_source(ImageSource* val, bool isRt) {
		if (isRt) {
			ImageSourceHolder::set_source(val);
		} else {
			Retain(val); // temp retain
			preRender().async_call([](auto self, auto arg) {
				self->ImageSourceHolder::set_source(arg.arg);
				Release(arg.arg); // temp release
			}, this, val);
		}
	}
}
