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

#ifndef __quark__view__video__
#define __quark__view__video__

#include "./image.h"
#include "../../media/media.h"
#include "../../media/pcm_player.h"

namespace qk {

	class Qk_Export Video: public Image,
												public RenderTask, public MediaSource::Delegate {
	public:
		typedef MediaCodec::Frame Frame;
		typedef MediaCodec::Extractor Extractor;
		typedef MediaSource::Stream Stream;
		typedef RenderTask::ID TaskID;

		// define props
		Qk_DEFINE_ACCE(bool, auto_play);
		Qk_DEFINE_ACCE(bool, mute);
		Qk_DEFINE_ACCE(uint32_t, volume);
		Qk_DEFINE_VIEW_ACCE(String, src);
		Qk_DEFINE_AGET(MediaSourceStatus, source_status);
		Qk_DEFINE_AGET(PlayerStatus, status);
		Qk_DEFINE_AGET(uint64_t, time);
		Qk_DEFINE_AGET(uint64_t, duration);
		Qk_DEFINE_AGET(uint32_t, audio_stream_count);
		Qk_DEFINE_AGET(const Stream*, audio_stream);
		Qk_DEFINE_AGET(const Stream*, video_stream);
		// Qk_DEFINE_AGET(uint32_t, video_width);
		// Qk_DEFINE_AGET(uint32_t, video_height);

		Video();
		~Video() override;
		// @overwrite
		void media_source_open(MediaSource* src) override;
		void media_source_eof(MediaSource* src) override;
		void media_source_error(MediaSource* src, cError& err) override;
		// methods
		void start();
		bool seek(uint64_t timeUs);
		void pause();
		void resume();
		void stop();
		void switch_audio_stream(uint32_t index);
		bool run_task(int64_t time) override;
		void onActivate() override;
		ViewType viewType() const override;

	private:
		MediaSource  *_source;
		MediaCodec   *_audio, *_video;
		PCMPlayer    *_pcm;
		RunLoop      *_loop;
		bool          _auto_play, _mute, _waiting_buffer;
		PlayerStatus  _status;
		Frame  _audio_buffer, _video_buffer;
		uint64_t  _time, _duration;
		uint64_t  _uninterrupted_play_start_time;
		uint64_t  _uninterrupted_play_start_systime;
		uint64_t  _prev_presentation_time;
		uint64_t  _prev_run_task_systime;
		uint32_t  _volume;
		Mutex     _mutex;

		Qk_DEFINE_INLINE_CLASS(Inl);
	};

}
#endif
