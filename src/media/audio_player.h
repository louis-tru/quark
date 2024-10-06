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

#ifndef __quark__audio_player__
#define __quark__audio_player__

#include "../ui/event.h"
#include "./media.h"
#include "./pcm_player.h"

namespace qk {

	class Player {
	public:
		typedef MediaCodec::Frame Frame;
		typedef MediaSource::Stream Stream;
		typedef MediaSource::Extractor Extractor;
		Player();
		~Player();
		// define props
		Qk_DEFINE_PGET(int64_t, pts, Const);
		Qk_DEFINE_PROP(float, volume, Const);
		Qk_DEFINE_PROP(bool, mute, Const);
		Qk_DEFINE_AGET(bool, is_pause, Const);
		Qk_DEFINE_AGET(uint64_t, duration, Const);
		Qk_DEFINE_AGET(MediaSourceStatus, status, Const);
		Qk_DEFINE_ACCE(String, src, Const);
		Qk_DEFINE_AGET(MediaSource*, media_source);
		Qk_DEFINE_AGET(const Stream*, video, Const);
		Qk_DEFINE_AGET(const Stream*, audio, Const); // current audio stream
		Qk_DEFINE_AGET(uint32_t, audio_streams, Const); // audio stream count
	protected:
		Sp<MediaSource> _msrc;
		Sp<MediaCodec> _audio, _video;
		Sp<PCMPlayer>  _pcm;
		Sp<Frame> _fa, _fv;
		int64_t _start, _seeking, _seek;
	};

	class Qk_Export AudioPlayer: public Notification<Event<>, UIEventName>,
															public MediaSource::Delegate {
		Qk_HIDDEN_ALL_COPY(AudioPlayer);
	public:
		typedef MediaCodec::Frame Frame;
		typedef MediaSource::Stream Stream;
		typedef MediaSource::Extractor Extractor;
		// define props
		Qk_DEFINE_PGET(int64_t, pts, Const);
		Qk_DEFINE_PROP(float, volume, Const);
		Qk_DEFINE_PROP(bool, mute, Const);
		Qk_DEFINE_AGET(bool, is_pause, Const);
		Qk_DEFINE_AGET(uint64_t, duration, Const);
		Qk_DEFINE_AGET(MediaSourceStatus, status, Const);
		Qk_DEFINE_ACCE(String, src, Const);
		Qk_DEFINE_AGET(MediaSource*, media_source);
		Qk_DEFINE_AGET(const Stream*, audio, Const); // current audio stream
		Qk_DEFINE_AGET(uint32_t, audio_streams, Const); // audio stream count
		AudioPlayer();
		~AudioPlayer() override;
		void media_source_open(MediaSource* src) override;
		void media_source_eof(MediaSource* src) override;
		void media_source_error(MediaSource* src, cError& err) override;
		void media_source_switch(MediaSource* src, Extractor *ex) override;
		void media_source_advance(MediaSource* src) override;
		void play();
		void pause();
		void stop();
		void seek(uint64_t timeUs);
		void switch_audio(uint32_t index);
	private:
		void skip_af();
		void end();
		void trigger(const UIEventName& name, const Object& data);
		Sp<MediaSource> _msrc;
		Sp<MediaCodec> _audio;
		Sp<PCMPlayer>  _pcm;
		Sp<Frame> _fa;
		int64_t _start, _seeking, _seek;
	};
}
#endif
