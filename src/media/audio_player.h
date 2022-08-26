/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __noug__audio_player__
#define __noug__audio_player__

#include "../app.h"
#include "../event.h"
#include "./media/media.h"
#include "./media/pcm.h"
#include "./media/media_codec.h"

namespace noug {

	class N_EXPORT AudioPlayer: public Notification<Event<>, UIEventName>,
															public MultimediaSource::Delegate {
		N_HIDDEN_ALL_COPY(AudioPlayer);
	public:
		typedef MultimediaSource::TrackInfo TrackInfo;
		typedef MediaCodec::OutputBuffer    OutputBuffer;

		AudioPlayer(cString& uri = String());
		virtual ~AudioPlayer();
		// define props
		N_DEFINE_PROP(bool, auto_play);
		N_DEFINE_PROP(bool, mute);
		N_DEFINE_PROP(bool, disable_wait_buffer);
		N_DEFINE_PROP(uint32_t, volume);
		N_DEFINE_ACCESSOR(String, src);
		N_DEFINE_ACCESSOR_READ(MultimediaSourceStatus, source_status);
		N_DEFINE_ACCESSOR_READ(PlayerStatus, status);
		N_DEFINE_ACCESSOR_READ(uint64_t, time);
		N_DEFINE_ACCESSOR_READ(uint64_t, duration);
		N_DEFINE_ACCESSOR_READ(uint32_t, audio_track_count);
		N_DEFINE_ACCESSOR_READ(uint32_t, audio_track_index);
		N_DEFINE_ACCESSOR_READ(const TrackInfo*, audio_track);
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
		PlayerStatus  _status;
		OutputBuffer  _audio_buffer;
		uint64_t  _duration, _time;
		uint64_t  _uninterrupted_play_start_time;
		uint64_t  _uninterrupted_play_start_systime;
		uint64_t  _prev_presentation_time;
		Mutex     _audio_loop_mutex, _mutex;
		uint32_t  _task_id;
		uint32_t  _volume;
		bool      _waiting_buffer;

		N_DEFINE_INLINE_CLASS(Inl);
	};
}
#endif