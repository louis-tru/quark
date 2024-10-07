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

	void Image::layout_forward(uint32_t mark) {
		if (mark & (kLayout_Size_Width | kLayout_Size_Height)) {
			mark |= (kLayout_Size_Width | kLayout_Size_Height);
		}
		Box::layout_forward(mark);
	}

	float Image::solve_layout_content_width(Size &pSize) {
		auto result = Box::solve_layout_content_width(pSize);
		auto src = source(); // Rt

		if (src && src->type()) {
			if (pSize.wrap_x) { // wrap x
				auto v = Box::solve_layout_content_height(pSize);
				result = pSize.wrap_y ? src->width():
					v / src->height() * src->width();
			}
		}
		if (pSize.wrap_x) {
			result = solve_layout_content_wrap_limit_width(result);
			pSize.wrap_x = false;
		}
		return result;
	}

	float Image::solve_layout_content_height(Size &pSize) {
		auto result = Box::solve_layout_content_height(pSize);
		auto src = source(); // Rt

		if (src && src->type()) {
			if (pSize.wrap_y) { // wrap y
				auto v = Box::solve_layout_content_width(pSize);
				result = pSize.wrap_x ? src->height():
					v / src->width() * src->height();
			}
		}
		if (pSize.wrap_y) {
			result = solve_layout_content_wrap_limit_height(result);
			pSize.wrap_y = false;
		}
		return result;
	}

	void Image::onSourceState(Event<ImageSource, ImageSource::State>& evt) {
		if (evt.data() & ImageSource::kSTATE_LOAD_COMPLETE) {
			mark_layout(kLayout_Size_Width | kLayout_Size_Height, false);
			Sp<UIEvent> evt = new UIEvent(this);
			trigger(UIEvent_Load, **evt);
		} else if (evt.data() & (ImageSource::kSTATE_LOAD_ERROR | ImageSource::kSTATE_DECODE_ERROR)) {
			Sp<UIEvent> evt = new UIEvent(this, new Error(ERR_IMAGE_LOAD_ERROR, "ERR_IMAGE_LOAD_ERROR"));
			trigger(UIEvent_Error, **evt);
		}
	}

	ImagePool* Image::imgPool() {
		return window()->host()->imgPool();
	}

	String Image::src() const {
		return ImageSourceHold::src();
	}

	void Image::set_src(String val, bool isRt) {
		ImageSourceHold::set_src(val);
	}
}
