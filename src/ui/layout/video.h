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

#ifndef __quark__layout__video__
#define __quark__layout__video__

#include "./image.h"
#include "../task.h"
#include "../media/media_codec.h"
#include "../media/pcm.h"

namespace qk {

	class Qk_EXPORT VideoLayout: public Image,
															public RenderTask, public MultimediaSource::Delegate {
	public:
		typedef Mediacodec_OutputBuffer OutputBuffer;
		typedef MultimediaSource::TrackInfo TrackInfo;
		typedef RenderTask::ID TaskID;

		VideoLayout();
		virtual ~VideoLayout();
		// define props
		Qk_DEFINE_PROP_ACC(bool, auto_play);
		Qk_DEFINE_PROP_ACC(bool, mute);
		Qk_DEFINE_PROP_ACC(bool, disable_wait_buffer);
		Qk_DEFINE_PROP_ACC(uint32_t, volume);
		Qk_DEFINE_PROP_ACC(String, src);
		Qk_DEFINE_PROP_ACC_GET(MultimediaSourceStatus, source_status);
		Qk_DEFINE_PROP_ACC_GET(PlayerStatus, status);
		Qk_DEFINE_PROP_ACC_GET(uint64_t, time);
		Qk_DEFINE_PROP_ACC_GET(uint64_t, duration);
		Qk_DEFINE_PROP_ACC_GET(uint32_t, audio_track_count);
		Qk_DEFINE_PROP_ACC_GET(uint32_t, audio_track_index);
		Qk_DEFINE_PROP_ACC_GET(const TrackInfo*, audio_track);
		Qk_DEFINE_PROP_ACC_GET(const TrackInfo*, video_track);
		Qk_DEFINE_PROP_ACC_GET(uint32_t, video_width);
		Qk_DEFINE_PROP_ACC_GET(uint32_t, video_height);
		// define methods
		const TrackInfo* audio_track_at(uint32_t index);
		void select_audio_track(uint32_t index);
		void start();
		bool seek(uint64_t timeUs);
		void pause();
		void resume();
		void stop();
		// @overwrite
		virtual bool run_task(int64_t time);
		virtual void multimedia_source_ready(MultimediaSource* src);
		virtual void multimedia_source_wait_buffer(MultimediaSource* src, float process);
		virtual void multimedia_source_eof(MultimediaSource* src);
		virtual void multimedia_source_error(MultimediaSource* src, cError& err);
		virtual void onActivate() override;
		virtual ViewType viewType() const override;
	private:
		MultimediaSource* _source;
		MediaCodec   *_audio, *_video;
		PCMPlayer*    _pcm;
		KeepLoop*     _keep;
		bool          _auto_play, _mute, _disable_wait_buffer, _waiting_buffer;
		PlayerStatus  _status;
		VideoColorFormat _color_format;
		OutputBuffer  _audio_buffer, _video_buffer;
		uint64_t  _time, _duration;
		uint64_t  _uninterrupted_play_start_time;
		uint64_t  _uninterrupted_play_start_systime;
		uint64_t  _prev_presentation_time;
		uint64_t  _prev_run_task_systime;
		uint32_t  _video_width, _video_height;
		uint32_t  _task_id;
		uint32_t  _volume;
		Mutex     _mutex;
		ThreadID  _run_loop_id;

		Qk_DEFINE_INLINE_CLASS(Inl);
	};

}
#endif
