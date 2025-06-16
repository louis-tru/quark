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
	typedef Box::Container::Pre Pre;

	ViewType Image::viewType() const {
		return kImage_ViewType;
	}

	uint32_t Image::solve_layout_content_size_pre(uint32_t &mark, const Container &pContainer) {
		uint32_t change_mark = kLayout_None;
		
		if (mark & (kLayout_Inner_Width | kLayout_Inner_Height)) {
			auto src = source(); // Rt

			if (src && src->type()) {
				_container.set_pre_width(solve_layout_content_pre_width(pContainer));
				_container.set_pre_height(solve_layout_content_pre_height(pContainer));

				Vec2 content;
				if (_container.float_x()) { // wrap x
					if (_container.float_y()) { // wrap y
						content[0] = _container.clamp_width(src->width());
						content[1] = _container.clamp_height(src->height());
					} else { // no wrap y and rawp x
						content[1] = _container.pre_height[0];
						content[0] = _container.clamp_width(src->width() * content[1] / src->height());
					}
				} else if (_container.float_y()) { // x is wrap and y is no wrap
					content[0] = _container.pre_width[0];
					content[1] = _container.clamp_height(src->height() * content[0] / src->width());
				} else { // all of both are no wrap
					content[0] = _container.pre_width[0];
					content[1] = _container.pre_height[0];
				}

				if (_container.content[0] != content[0]) {
					mark       |= kLayout_Outside_Width;
					change_mark = kLayout_Inner_Width;
				}
				if (_container.content[1] != content[1]) {
					mark        |= kLayout_Outside_Height;
					change_mark |= kLayout_Inner_Height;
				}

				_container.state_x = _container.state_y = kFixed_FloatState;
				_container.content = content;
				_container.pre_width = content[0];
				_container.pre_height = content[1];
			} else {
				return Box::solve_layout_content_size_pre(mark, pContainer);
			}
		}
	
		return change_mark;
	}

	void Image::onSourceState(Event<ImageSource, ImageSource::State>& evt) {
		if (evt.data() & ImageSource::kSTATE_LOAD_COMPLETE) {
			mark_layout(kLayout_Inner_Width | kLayout_Inner_Height, false);
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
