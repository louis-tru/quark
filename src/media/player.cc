/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK *****/

#include "./player.h"
#include "../errno.h"

namespace qk {

	struct PlayerLock {
		PlayerLock(Player* host): h(host) {
			h->lock();
		}
		~PlayerLock() {
			h->unlock();
		}
		Player* h;
		bool is_lock;
	};

	Player::Player(MediaType type)
		: _pts(0), _volume(1), _mute(false), _type(type), _start(0), _seeking(0), _seek(0) {
		Qk_ASSERT_NE(0, type);
	}

	Player::~Player() {
		PlayerLock lock(this);
		_msrc = nullptr;
	}

	void Player::set_volume(float val) {
		PlayerLock lock(this);
		_volume = Float32::clamp(val, 0, 1);
		if (_pcm) {
			_pcm->set_volume(_volume);
		}
	}

	void Player::set_mute(bool val) {
		PlayerLock lock(this);
		_mute = val;
		if (_pcm) {
			_pcm->set_mute(_mute);
		}
	}

	bool Player::is_pause() const { // please call in work thread
		return _msrc ? _msrc->is_pause(): false;
	}

	uint64_t Player::duration() const { // please call in work thread
		return _msrc ? _msrc->duration(): 0;
	}

	MediaSourceStatus Player::status() const { // please call in work thread
		return _msrc ? _msrc->status(): kNormal_MediaSourceStatus;
	}

	MediaSource* Player::media_source() { // please call in work thread
		return *_msrc;
	}

	const Player::Stream* Player::video() const { // please call in work thread
		return _msrc && const_cast<MediaSource*>(*_msrc)->video() ?
			&const_cast<MediaSource*>(*_msrc)->video()->stream(): nullptr;
	}

	const Player::Stream* Player::audio() const { // please call in work thread
		return _msrc && const_cast<MediaSource*>(*_msrc)->audio() ?
			&const_cast<MediaSource*>(*_msrc)->audio()->stream(): nullptr;
	}

	uint32_t Player::audio_streams() const { // please call in work thread
		return _msrc && const_cast<MediaSource*>(*_msrc)->audio() ?
			const_cast<MediaSource*>(*_msrc)->audio()->streams().length(): 0;
	}

	String Player::src() const { // please call in work thread
		return _msrc ? _msrc->uri().href() : String();
	}

	void Player::set_src(String value) { // please call in work thread
		stop();
		PlayerLock lock(this);
		_msrc = new MediaSource(value);
		_msrc->set_delegate(this);
	}

	void Player::play() { // please call in work thread
		if (_msrc)
			_msrc->play();
	}

	void Player::pause() { // please call in work thread
		if (_msrc)
			_msrc->pause();
	}

	void Player::stop() { // please call in work thread
		PlayerLock lock(this);
		if (_video || _audio) {
			if (_msrc)
				_msrc->stop();
			_video = nullptr;
			_audio = nullptr;
			_pcm = nullptr;
			onEvent(UIEvent_Stop, nullptr);
		}
	}

	void Player::seek(uint64_t timeUs) {
		_seek = Qk_Max(timeUs, 1);
	}

	void Player::switch_audio(uint32_t index) { // please call in work thread
		if (_msrc->audio())
			_msrc->audio()->switch_stream(index);
	}

	void Player::media_source_open(MediaSource* src) {
		Qk_DLog("media_source_open");
		PlayerLock lock(this);
		if (_type == kVideo_MediaType) {
			_video = MediaCodec::create(kVideo_MediaType, src);
			if (!_video || (_video->set_threads(2), !_video->open())) {
				Qk_Warn("open video codecer fail");
				_video = nullptr;
				src->stop();
				return;
			}
		}
		_audio = MediaCodec::create(kAudio_MediaType, src);
		if (_audio && _audio->open()) {
			_pcm = PCMPlayer::create(_audio->stream());
		}
		if (_pcm) {
			_pcm->set_volume(_volume);
			_pcm->set_mute(_mute);
		} else {
			Qk_Warn("open audio codecer or pcm player fail");
			src->remove_extractor(kAudio_MediaType);
			_audio = nullptr;
			if (_type == kAudio_MediaType) {
				src->stop();
				return;
			}
		}
		onEvent(UIEvent_Load, nullptr);
	}

	void Player::media_source_eof(MediaSource* src) {
		Qk_DLog("media_source_eof");
		auto &codec = _video ? _video : _audio;
		do {
			media_source_advance(src);
			thread_sleep(1e4); // 10 milliseconds
		} while (codec && !codec->finished());
		stop();
	}

	void Player::media_source_error(MediaSource* src, cError& err) {
		Qk_DLog("media_source_error");
		onEvent(UIEvent_Error, new Error(err));
		stop();
	}

	void Player::media_source_switch(MediaSource* src, Extractor *ex) {
		Qk_DLog("media_source_switch");
		if (_video && ex->type() == kVideo_MediaType) {
				_video->close();
				if (!_video->open(&ex->stream()))
					stop();
		}
	}

	void Player::media_source_advance(MediaSource* src) {
		if (_seek) {
			if (src->seek(_seek)) {
				if (_video) {
					_video->flush();
				}
				if (_audio) {
					_audio->flush();
					_pcm->flush();
				}
				PlayerLock lock(this);
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
			PlayerLock lock(this);
			if (!_audio->open(&stream) || !(_pcm = PCMPlayer::create(stream))) {
				_audio = nullptr;
				_pcm = nullptr;
				return;
			}
			_pcm->set_volume(_volume);
			_pcm->set_mute(_mute);
		}
		_audio->send_packet(src->audio());

		if (!_fa) {
			_fa = _audio->receive_frame();
		}
		if (!_fa) {
			return;
		}

		if (_type == kVideo_MediaType) {
			if (_fa->pts) {
				if (!_start)
					return;
				auto play = now - _start;
				auto pts = _fa->pts - (_fa->pkt_duration * _pcm->delayed()); // after pcm delay pts
				if (pts > play)
					return;
				int64_t du = play - pts;
				if (du > _fa->pkt_duration << 1) { // decoding timeout, discard frame
					_fa = nullptr;
					return;
				}
			}
		} else { // only audio
			if (!_start) {
				_start = now - (_seeking ? _seeking: _fa->pts);
			}
			if (_fa->pts) {
				auto play = now - _start;
				auto pts = _fa->pts;
				if (pts > play)
					return;
				int64_t du = play - pts;
				if (du > _fa->pkt_duration * _pcm->delayed() * 2) { // timeout, reset start point
					if (_seeking) {
						PlayerLock lock(this);
						return skip_frame_unsafe(false);
					}
					Qk_DLog("pkt_duration, timeout %d", du);
					_start = now - pts; // correct play ts
				}
			}
			_seeking = 0;
			_pts =  _fa->pts; // set current the presentation timestamp
		}

		// Writing PCM audio data to audio device
		if (!_pcm->write(*_fa)) {
			Qk_DLog("PCM_write fail %ld, %ld", _fa->pts, now - _start);
		}
		_fa = nullptr;
	}

	void Player::skip_frame_unsafe(bool video) {
		auto &f = video ? _fv: _fa;
		auto codec = video ? *_video: *_audio;
		auto ex = video ? _msrc->video(): _msrc->audio();
		do { // skip expired frame
			f = nullptr;
			if (codec->send_packet(ex) == 0)
				f = codec->receive_frame();
		} while(f && f->pts < time_monotonic() - _start);
		f = nullptr; // delete frame
	}

	void Player::lock() {
		_mutex.lock();
	}

	void Player::unlock() {
		_mutex.unlock();
	}

	// ------------------------ AudioPlayer ------------------------
	
	Sp<AudioPlayer> AudioPlayer::Make() {
		return new AudioPlayer();
	}

	AudioPlayer::AudioPlayer(): Player(kAudio_MediaType), _loop(RunLoop::current()) {
	}

	void AudioPlayer::onEvent(const UIEventName& name, Object* data) {
		Sp<Object> sp(data);
		if (!_loop && ref_count() <= 0)
			return;
		auto evt = new Event<>(sp.collapse());
		_loop->post(Cb([this, evt, name](auto e) {
			Sp<Event<>> sp(evt);
			trigger(name, *evt);
		}, this));
	}
}
