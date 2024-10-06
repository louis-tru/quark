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

	Video::Video(): _pts(0), _volume(1), _mute(false), _start(0), _seeking(0), _seek(0) {
	}

	Video::~Video() {
		_msrc = nullptr;
	}

	void Video::trigger(const UIEventName& name, const Object& data) {
		if (!window()) return;
		auto evt = new UIEvent(this, data);
		preRender().post(Cb([this,evt,name](auto e) {
			Sp<UIEvent> sp(evt);
			View::trigger(name, *evt);
		}, this));
	}

	void Video::media_source_open(MediaSource* src) {
		Qk_DLog("media_source_open");
		UILock lock(window());
		_video = MediaCodec::create(kVideo_MediaType, src);
		if (!_video && (_video->set_threads(2), !_video->open())) {
			Qk_Warn("open video codecer fail");
			_video = nullptr;
			return;
		}
		_audio = MediaCodec::create(kAudio_MediaType, src);
		if (_audio && _audio->open()) {
			_pcm = PCMPlayer::create(_audio->stream());
		}
		if (!_pcm) {
			Qk_Warn("open audio codecer or pcm player fail");
			_audio = nullptr;
			src->remove_extractor(kAudio_MediaType);
		}
		preRender().addtask(this);
		trigger(UIEvent_Load, Object());
	}

	void Video::media_source_eof(MediaSource* src) {
		Qk_DLog("media_source_eof");
		do {
			media_source_advance(src);
			thread_sleep(1e4); // 10 milliseconds
		} while (_video && !_video->finished());
		stop();
	}

	void Video::media_source_error(MediaSource* src, cError& err) {
		Qk_DLog("media_source_error");
		trigger(UIEvent_Error, err);
		stop();
	}

	void Video::media_source_switch(MediaSource* src, Extractor *ex) {
		Qk_DLog("media_source_switch");
		if (_video && ex->type() == kVideo_MediaType) {
				_video->close();
				if (!_video->open(&ex->stream()))
					stop();
		}
	}

	void Video::media_source_advance(MediaSource* src) {
		if (_seek) {
			if (src->seek(_seek)) {
				if (_video) {
					_video->flush();
				}
				if (_audio) {
					_audio->flush();
					_pcm->flush();
				}
				UILock lock(window());
				_fv = _fa = nullptr;
				_start = 0; // reset start point
				_seeking = _seek;
			}
			_seek = 0;
		}
		auto now = time_monotonic();

		if (!_audio) {
			return;
		}
		auto& stream = src->audio()->stream();
		if (_audio->stream() != stream) {  // switch stream
			_audio->close();
			UILock lock(window());
			if (!_audio->open(&stream) || !(_pcm = PCMPlayer::create(stream))) {
				_audio = nullptr;
				_pcm = nullptr;
				return;
			}
		}
		_audio->send_packet(src->audio());

		if (!_fa) {
			_fa = _audio->receive_frame();
		}
		if (!_fa) {
			return;
		}
		if (_fa->pts) {
			if (!_start) return;
			auto play = now - _start;
			auto pts = _fa->pts - (_fa->pkt_duration * _pcm->delay()); // after pcm delay pts
			if (pts > play) return;
			int64_t du = play - pts;
			if (du > _fa->pkt_duration << 1) { // decoding timeout, discard frame
				_fa = nullptr;
				return;
			}
		}
		if (!_pcm->write(*_fa)) {
			Qk_DLog("PCM_write fail %ld, %ld", _fa->pts, now - _start);
		}
		_fa = nullptr;
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
					return skip_vf(), false;
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

	void Video::set_volume(float val) {
		UILock lock(window());
		_volume = Float32::clamp(val, 0, 1);
		if (_pcm) {
			_pcm->set_volume(_volume);
		}
	}

	void Video::set_mute(bool val) {
		UILock lock(window());
		_mute = val;
		if (_pcm) {
			_pcm->set_mute(_mute);
		}
	}
	
	bool Video::is_pause() const {
		return _msrc ? _msrc->is_pause(): false;
	}

	uint64_t Video::duration() const {
		return _msrc ? _msrc->duration(): 0;
	}

	MediaSourceStatus Video::status() const {
		return _msrc ? _msrc->status(): kNone_MediaSourceStatus;
	}

	MediaSource* Video::media_source() {
		return *_msrc;
	}

	const Video::Stream* Video::video() const {
		return _msrc && const_cast<MediaSource*>(*_msrc)->video() ?
			&const_cast<MediaSource*>(*_msrc)->video()->stream(): nullptr;
	}

	const Video::Stream* Video::audio() const {
		return _msrc && const_cast<MediaSource*>(*_msrc)->audio() ?
			&const_cast<MediaSource*>(*_msrc)->audio()->stream(): nullptr;
	}

	uint32_t Video::audio_streams() const {
		return _msrc && const_cast<MediaSource*>(*_msrc)->audio() ?
			const_cast<MediaSource*>(*_msrc)->audio()->streams().length(): 0;
	}

	String Video::src() const {
		return _msrc ? _msrc->uri().href() : String();
	}

	void Video::set_src(String value) {
		UILock lock(window());
		end();
		_msrc = new MediaSource(value);
		_msrc->set_delegate(this);
	}

	void Video::play() {
		if (_msrc)
			_msrc->play();
	}

	void Video::pause() {
		if (_msrc)
			_msrc->pause();
	}

	void Video::stop() {
		UILock lock(window());
		end();
	}

	void Video::end() {
		if (_video) {
			if (_msrc)
				_msrc->stop();
			_video = nullptr;
			_audio = nullptr;
			_pcm = nullptr;
			auto imgsrc = source();
			if (imgsrc)
				imgsrc->unload(); // unload, resource
			preRender().untask(this);
			trigger(UIEvent_Stop, Object());
		}
	}

	void Video::seek(uint64_t timeUs) {
		_seek = Qk_Max(timeUs, 1);
	}

	void Video::switch_audio(uint32_t index) {
		if (_msrc->audio())
			_msrc->audio()->switch_stream(index);
	}

	void Video::skip_vf() {
		do { // skip expired v frame
			_fv = nullptr;
			if (_video->send_packet(_msrc->video()) == 0)
				_fv = _video->receive_frame();
		} while(_fv && _fv->pts < time_monotonic() - _start);
		_fv = nullptr; // delete v frame
	}

	void Video::onActivate() {
		if (level() == 0) { // remove
			end();
		}
	}

	ViewType Video::viewType() const {
		return kVideo_ViewType;
	}
}
