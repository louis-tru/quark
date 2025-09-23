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
 * ***** END LICENSE BLOCK *****/

#include "./video.h"
#include "../app.h"
#include "../window.h"
#include "../../errno.h"

namespace qk {

	Video::Video(): Player(kVideo_MediaType) {
	}

	String Video::src() const {
		return Player::src();
	}

	void Video::set_src(String value, bool isRt) {
		if (isRt) {
			preRender().post(Cb([this, value](auto e) {
				Player::set_src(value);
			}), this);
		} else {
			Player::set_src(value);
		}
	}

	void Video::lock() {
		_mutex.lock();
	}

	void Video::unlock() {
		_mutex.unlock();
	}

	void Video::onActivate() {
		if (level() == 0) { // remove
			stop();
		}
	}

	ViewType Video::viewType() const {
		return kVideo_ViewType;
	}

	void Video::onEvent(const UIEventName& name, Object* data) {
		if (name == UIEvent_Load) {
			preRender().addtask(this);
		} else if (name == UIEvent_Stop) {
			auto imgsrc = source();
			if (imgsrc)
				imgsrc->unload(); // unload, resource
			preRender().untask(this);
		}

		// trigger event in main thread
		if (!tryRetain_Rt()) {
			window()->loop()->post(Cb([this,name,data](auto e) {
				Sp<UIEvent> evt(new UIEvent(this, data));
				trigger(name, **evt);
				release(); // It must be release here @ if (tryRetain_Rt()) 
			}), true);
		} else {
			Release(data);
		}
	}

	bool Video::run_task(int64_t now, int64_t deltaTime) {
		ScopeLock lock(_mutex);
		if (!_video) {
			return false;
		}
		_video->send_packet(_msrc->video());

		if (!_fv) {
			_fv = _video->receive_frame();
		}
		if (!_fv) {
			return false;
		}
		if (!_start) {
			_start = now - (_seeking ? _seeking: _fv->pts);
		}
		if (_fv->pts) {
			auto play = now - _start;
			auto pts = _fv->pts;
			if (pts > play) return false;
			int64_t du = play - pts;
			if (du > _fv->pkt_duration << 1) {
				// decoding timeout, discard frame or reset start point
				if (_seeking)
					return skip_frame_unsafe(true), false;
				Qk_DLog("Video:pkt_duration, timeout %d", du);
				_start = now - pts; // correct play ts
			}
		}
		auto src = source();
		if (!src || !(src->state() & ImageSource::kSTATE_LOAD_COMPLETE)) {
			mark_layout(kLayout_Inner_Width | kLayout_Inner_Height, true);
		}
		_seeking = 0;
		_pts = _fv->pts; // set current the presentation timestamp
		set_source(ImageSource::Make(
			MediaCodec::frameToPixel(*_fv), window()->render()
		));
		_fv.collapse();
		return true;
	}

	void Video::onSourceState(ImageSource::State state) {
		// Noop
	}

}
