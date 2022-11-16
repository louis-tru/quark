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

#include "../app.h"
#include "./source_hold.h"
#include "./source_pool.h"
#include "../pre_render.h"

namespace quark {

	SourceHold::~SourceHold() {
		if (_imageSource) {
			_imageSource->Qk_Off(State, &SourceHold::handleSourceState, this);
		}
	}

	String SourceHold::src() const {
		return _imageSource ? _imageSource->uri(): String();
	}

	ImageSource* SourceHold::source() {
		return _imageSource.value();
	}

	void SourceHold::set_src(String value) {
		set_source(app() ? app()->img_pool()->get(value): new ImageSource(value));
	}

	void SourceHold::set_source(ImageSource* source) {
		if (_imageSource.value() != source) {
			if (_imageSource) {
				_imageSource->Qk_Off(State, &SourceHold::handleSourceState, this);
			}
			if (source) {
				source->Qk_On(State, &SourceHold::handleSourceState, this);
			}
			_imageSource = Handle<ImageSource>(source);
		}
	}

	void SourceHold::handleSourceState(Event<ImageSource, ImageSource::State>& evt) { // 收到图像变化通知
		onSourceState(evt);
	}

	void SourceHold::onSourceState(Event<ImageSource, ImageSource::State>& evt) {
		if (*evt.data() & ImageSource::STATE_DECODE_COMPLETE) {
			auto _ = app();
			// Qk_ASSERT(_, "Application needs to be initialized first");
			if (_) {
				_->pre_render()->mark_none();
			}
		}
	}


}
