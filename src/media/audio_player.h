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

#include "../app.h"
#include "../event.h"
#include "./media.h"
#include "./pcm.h"
#include "./media_codec.h"

namespace quark {

	class Qk_EXPORT AudioPlayer: public Notification<Event<>, UIEventName>,
															public MultimediaSource::Delegate {
		Qk_HIDDEN_ALL_COPY(AudioPlayer);
	public:
		typedef MultimediaSource::TrackInfo TrackInfo;
		typedef MediaCodec::OutputBuffer    OutputBuffer;

		static AudioPlayer* create(String src, Application* host = nullptr);

		AudioPlayer(Application* host = nullptr);
		virtual ~AudioPlayer();
		// define props
		Qk_Define_Prop_Acc(bool, auto_play, NoConst);
		Qk_Define_Prop_Acc(bool, mute, NoConst);
		Qk_Define_Prop_Acc(bool, disable_wait_buffer, NoConst);
		Qk_Define_Prop_Acc(uint32_t, volume, NoConst);
		Qk_Define_Prop_Acc(String, src, NoConst);
		Qk_Define_Prop_Acc_Get(MultimediaSourceStatus, source_status, NoConst);
		Qk_Define_Prop_Acc_Get(PlayerStatus, status, NoConst);
		Qk_Define_Prop_Acc_Get(uint64_t, time, NoConst);
		Qk_Define_Prop_Acc_Get(uint64_t, duration, NoConst);
		Qk_Define_Prop_Acc_Get(uint32_t, audio_track_count, NoConst);
		Qk_Define_Prop_Acc_Get(uint32_t, audio_track_index, NoConst);
		Qk_Define_Prop_Acc_Get(const TrackInfo*, audio_track, NoConst);
		// define methods
		const TrackInfo* audio_track_at(uint32_t index);
		void select_audio_track(uint32_t index);
		void start();
		bool seek(uint64_t timeUs);
		void pause();
		void resume();
		void stop();
		// @overwrite
		virtual void multimedia_source_ready(MultimediaSource* src);
		virtual void multimedia_source_wait_buffer(MultimediaSource* src, float process);
		virtual void multimedia_source_eof(MultimediaSource* src);
		virtual void multimedia_source_error(MultimediaSource* src, cError& err);
	private:
		Application *_host;
		MultimediaSource* _source;
		PCMPlayer*    _pcm;
		MediaCodec*   _audio;
		KeepLoop*     _keep;
		bool          _auto_play, _mute, _disable_wait_buffer, _waiting_buffer;
		PlayerStatus  _status;
		OutputBuffer  _audio_buffer;
		uint64_t  _duration, _time;
		uint64_t  _uninterrupted_play_start_time;
		uint64_t  _uninterrupted_play_start_systime;
		uint64_t  _prev_presentation_time;
		uint32_t  _task_id;
		uint32_t  _volume;
		Mutex     _mutex;
		ThreadID  _run_loop_id;

		Qk_DEFINE_INLINE_CLASS(Inl);
	};
}
#endif