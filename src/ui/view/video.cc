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

	struct VideoLock {
		Window *_win;
		bool _lock;
	};

	static_assert(sizeof(Lock) == sizeof(Lock), "assert sizeof(Lock) == sizeof(Lock)");

	void Video::lock() {
		VideoLock lock{window(),false};
		reinterpret_cast<UILock*>(&lock)->lock();
	}

	void Video::unlock() {
		VideoLock lock{window(),true};
		reinterpret_cast<UILock*>(&lock)->unlock();
	}

	Video::Video(): Player(kVideo_MediaType) {
	}

	String Video::src() const {
		return Player::src();
	}

	void Video::set_src(String value) {
		Player::set_src(value);
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

		struct Core: CallbackCore<Object> {
			Core(Video *v, Object* d, const UIEventName& n)
				: evt(new UIEvent(v, d)), name(n)
			{
				view.uncollapse(v);
			}
			void call(Data& e) {
				view->trigger(name, **evt);
			}
			Sp<Video> view;
			Sp<UIEvent> evt;
			UIEventName name;
		};

		if (tryRetain()) {
			window()->loop()->post(Cb(new Core(this, data, name)), true);
		} else {
			Release(data);
		}
	}

	bool Video::run_task(int64_t now) {
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
					return skip_frame(true), false;
				Qk_DLog("pkt_duration, timeout %d", du);
				_start = now - pts; // correct play ts
			}
		}
		auto src = source();
		if (!src || !(src->state() & ImageSource::kSTATE_LOAD_COMPLETE)) {
			mark_layout(kLayout_Size_Width | kLayout_Size_Height, true);
		}
		_seeking = 0;
		_pts = _fv->pts; // set current the presentation timestamp
		set_source(ImageSource::Make(
			MediaCodec::frameToPixel(*_fv), window()->render(), window()->loop()
		));
		_fv.collapse();
		return true;
	}

}
