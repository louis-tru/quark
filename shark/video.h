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

#ifndef __shark__video__
#define __shark__video__

#include "player.h"
#include "image.h"
#include "pre-render.h"
#include "media-codec.h"

/**
 * @ns shark
 */

XX_NS(shark)

class TextureYUV;
class PCMPlayer;

/**
 * @class Video
 */
class XX_EXPORT Video: public Image,
											 public PreRender::Task,
											 public MultimediaSource::Delegate {
 public:
	XX_DEFINE_GUI_VIEW(VIDEO, Video, video);
	
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
	inline bool auto_play() const { return m_auto_play; }
	
	/**
	 * @func set_auto_play # setting auto play
	 */
	void set_auto_play(bool value);
	
	/**
	 * @func source_status
	 * */
	MultimediaSourceStatus source_status();
	
	/**
	 * @func video_width
	 */
	uint video_width();
	
	/**
	 * @func video_height
	 */
	uint video_height();
	
	/**
	 * @func status getting play status
	 */
	PlayerStatus status();
	
	/**
	 * @func mute getting mute status
	 * */
	inline bool mute() const { return m_mute; }
	
	/**
	 * @func mute setting mute status
	 * */
	void set_mute(bool value);
	
	/**
	 * @func volume
	 */
	inline uint volume() { return m_volume; }
	
	/**
	 * @func volume
	 */
	void set_volume(uint value);
	
	/**
	 * @func time
	 * */
	uint64 time();
	
	/**
	 * @func duration
	 * */
	uint64 duration();
	
	/**
	 * @func audio_track_count
	 */
	uint audio_track_count();
	
	/**
	 * @func audio_track
	 */
	uint audio_track_index();
	
	/**
	 * @func audio_track
	 */
	const TrackInfo* audio_track();
	
	/**
	 * @func audio_track
	 */
	const TrackInfo* audio_track(uint index);
	
	/**
	 * @func video_track
	 * */
	const TrackInfo* video_track();
	
	/**
	 * @func select_audio_track
	 * */
	void select_audio_track(uint index);
	
	/**
	 * @func start play
	 */
	void start();
	
	/**
	 * @func seek to target time
	 */
	bool seek(uint64 timeUs);
	
	/**
	 * @func pause play
	 * */
	void pause();
	
	/**
	 * @func resume play
	 * */
	void resume();
	
	/**
	 * @func stop play
	 * */
	void stop();
	
	/**
	 * @func disable_wait_buffer
	 */
	void disable_wait_buffer(bool value);
	
	/**
	 * @func disable_wait_buffer
	 */
	inline bool disable_wait_buffer() const { return m_disable_wait_buffer; }
	
 protected:
	
	/**
	 * @overwrite
	 */
	virtual String source() const;
	virtual void set_source(cString& value);
	virtual void draw(Draw* draw);
	
 private:
	
	MultimediaSource* m_source;
	MediaCodec*   m_audio;
	MediaCodec*   m_video;
	PCMPlayer*    m_pcm;
	KeepLoop*     m_keep;
	PlayerStatus  m_status;
	OutputBuffer  m_audio_buffer;
	OutputBuffer  m_video_buffer;
	uint64  m_time, m_duration;
	uint64  m_uninterrupted_play_start_time;
	uint64  m_uninterrupted_play_start_systime;
	uint64  m_prev_presentation_time;
	uint64  m_prev_run_task_systime;
	uint    m_video_width, m_video_height;
	uint    m_task_id;
	VideoColorFormat  m_color_format;
	Mutex   m_audio_loop_mutex, m_mutex;
	uint    m_volume;
	bool    m_auto_play;
	bool    m_mute;
	bool    m_disable_wait_buffer;
	bool    m_waiting_buffer;
	
	XX_DEFINE_INLINE_CLASS(Inl);
};

XX_END
#endif

