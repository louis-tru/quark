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

#ifndef __ftr__video__
#define __ftr__video__

#include "ftr/image.h"
#include "ftr/pre-render.h"
#include "ftr/media-codec.h"

/**
 * @ns ftr
 */

namespace ftr {

class TextureYUV;
class PCMPlayer;

/**
 * @class Video
 */
class FX_EXPORT Video: public Image,
											 public PreRender::Task,
											 public MultimediaSource::Delegate {
 public:
	FX_DEFINE_GUI_VIEW(VIDEO, Video, video);

	Video();

	/**
	 * @destructor
	 */
	virtual ~Video();

	typedef MediaCodec::OutputBuffer OutputBuffer;
	typedef MultimediaSource::TrackInfo TrackInfo;
	typedef PreRender::Task::ID TaskID;

	/**
	 * @overwrite
	 */
	virtual bool run_task(int64 time);
	virtual void multimedia_source_ready(MultimediaSource* src);
	virtual void multimedia_source_wait_buffer(MultimediaSource* src, float process);
	virtual void multimedia_source_eof(MultimediaSource* src);
	virtual void multimedia_source_error(MultimediaSource* src, cError& err);
	virtual void remove();

	/**
	 * @overwrite
	 */
	virtual void set_texture(Texture* value);

	/**
	 * @func auto_play
	 */
	inline bool auto_play() const { return _auto_play; }

	/**
	 * @func set_auto_play # setting auto play
	 */
	FX_MEDIA_DYNAMIC void set_auto_play(bool value);

	/**
	 * @func source_status
	 * */
	FX_MEDIA_DYNAMIC MultimediaSourceStatus source_status();

	/**
	 * @func video_width
	 */
	FX_MEDIA_DYNAMIC uint video_width();

	/**
	 * @func video_height
	 */
	FX_MEDIA_DYNAMIC uint video_height();

	/**
	 * @func status getting play status
	 */
	FX_MEDIA_DYNAMIC PlayerStatus status();

	/**
	 * @func mute getting mute status
	 * */
	inline bool mute() const { return _mute; }

	/**
	 * @func mute setting mute status
	 * */
	FX_MEDIA_DYNAMIC void set_mute(bool value);

	/**
	 * @func volume
	 */
	inline uint volume() { return _volume; }

	/**
	 * @func volume
	 */
	FX_MEDIA_DYNAMIC void set_volume(uint value);

	/**
	 * @func time
	 * */
	FX_MEDIA_DYNAMIC uint64 time();

	/**
	 * @func duration
	 * */
	FX_MEDIA_DYNAMIC uint64 duration();

	/**
	 * @func audio_track_count
	 */
	FX_MEDIA_DYNAMIC uint audio_track_count();

	/**
	 * @func audio_track
	 */
	FX_MEDIA_DYNAMIC uint audio_track_index();

	/**
	 * @func audio_track
	 */
	FX_MEDIA_DYNAMIC const TrackInfo* audio_track();

	/**
	 * @func audio_track
	 */
	FX_MEDIA_DYNAMIC const TrackInfo* audio_track(uint index);

	/**
	 * @func video_track
	 * */
	FX_MEDIA_DYNAMIC const TrackInfo* video_track();

	/**
	 * @func select_audio_track
	 * */
	FX_MEDIA_DYNAMIC void select_audio_track(uint index);

	/**
	 * @func start play
	 */
	FX_MEDIA_DYNAMIC void start();

	/**
	 * @func seek to target time
	 */
	FX_MEDIA_DYNAMIC bool seek(uint64 timeUs);

	/**
	 * @func pause play
	 * */
	FX_MEDIA_DYNAMIC void pause();

	/**
	 * @func resume play
	 * */
	FX_MEDIA_DYNAMIC void resume();

	/**
	 * @func stop play
	 * */
	FX_MEDIA_DYNAMIC void stop();

	/**
	 * @func disable_wait_buffer
	 */
	FX_MEDIA_DYNAMIC void disable_wait_buffer(bool value);

	/**
	 * @func disable_wait_buffer
	 */
	inline bool disable_wait_buffer() const { return _disable_wait_buffer; }

 protected:

	/**
	 * @overwrite
	 */
	virtual String source() const;
	virtual void set_source(cString& value);
	virtual void draw(Draw* draw);

 private:

	MultimediaSource* _source;
	MediaCodec*   _audio;
	MediaCodec*   _video;
	PCMPlayer*    _pcm;
	KeepLoop*     _keep;
	PlayerStatus  _status;
	OutputBuffer  _audio_buffer;
	OutputBuffer  _video_buffer;
	uint64  _time, _duration;
	uint64  _uninterrupted_play_start_time;
	uint64  _uninterrupted_play_start_systime;
	uint64  _prev_presentation_time;
	uint64  _prev_run_task_systime;
	uint    _video_width, _video_height;
	uint    _task_id;
	VideoColorFormat  _color_format;
	Mutex   _audio_loop_mutex, _mutex;
	uint    _volume;
	bool    _auto_play;
	bool    _mute;
	bool    _disable_wait_buffer;
	bool    _waiting_buffer;

	FX_DEFINE_INLINE_CLASS(Inl);
};

}
#endif

