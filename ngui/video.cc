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

#include <math.h>
#include "app.h"
#include "video.h"
#include "texture.h"
#include "draw.h"
#include "pcm-player.h"
#include "media-codec.h"
#include "nxutils/loop.h"
#include "ngui/errno.h"

XX_NS(ngui)

typedef MultimediaSource::TrackInfo TrackInfo;
typedef PreRender::Task::ID TaskID;
typedef MediaCodec::OutputBuffer OutputBuffer;

/**
 * @constructor
 */
Video::Video()
: m_source(NULL)
, m_audio(NULL)
, m_video(NULL)
, m_pcm(NULL)
, m_keep(nullptr)
, m_status(PLAYER_STATUS_STOP)
, m_audio_buffer(), m_video_buffer()
, m_time(0), m_duration(0)
, m_uninterrupted_play_start_time(0)
, m_uninterrupted_play_start_systime(0)
, m_prev_presentation_time(0)
, m_prev_run_task_systime(0)
, m_video_width(0), m_video_height(0)
, m_task_id(0)
, m_color_format(VIDEO_COLOR_FORMAT_INVALID)
, m_volume(100)
, m_auto_play(true)
, m_mute(false)
, m_disable_wait_buffer(false)
, m_waiting_buffer(false) {
	Image::set_texture( new TextureYUV() );
}

/**
 * @class Video::Inl
 */
XX_DEFINE_INLINE_MEMBERS(Video, Inl) {
 public:

	bool load_yuv_texture(OutputBuffer& buffer) { // set yuv texture ..
		Array<WeakBuffer> body(3);
		body[0] = WeakBuffer((char*)buffer.data[0], buffer.linesize[0]);  // y
		body[1] = WeakBuffer((char*)buffer.data[1], buffer.linesize[1]);  // u
		body[2] = WeakBuffer((char*)buffer.data[2], buffer.linesize[2]);  // v
		
		PixelData pixel(body, m_video_width, m_video_height, (PixelData::Format)m_color_format );
		
		bool r = static_cast<TextureYUV*>(texture())->load_yuv(pixel); // load texture
		m_video->release(buffer);
		return r;
	}
	
	bool advance_video(uint64 sys_time) {
		XX_ASSERT(m_status != PLAYER_STATUS_STOP);
		
		bool draw = false;
		
		if ( ! m_video_buffer.total ) {
			if ( m_status == PLAYER_STATUS_PLAYING || m_status == PLAYER_STATUS_START ) {
				m_video_buffer = m_video->output();
				//t_debug("output, %llu", sys_time_monotonic() - sys_time);
				
				if ( m_video_buffer.total ) {
					if ( m_waiting_buffer ) {
						m_waiting_buffer = false;
						m_keep->post(Cb([this](Se& e){
							trigger(GUI_EVENT_WAIT_BUFFER, Float(1.0F)); // trigger source WAIT event
						}));
					}
				} else { // 没有取到数据
					MultimediaSourceStatus status = m_source->status();
					if ( status == MULTIMEDIA_SOURCE_STATUS_WAIT ) { // 源..等待数据
						if ( m_waiting_buffer == false ) {
							m_waiting_buffer = true;
							m_keep->post(Cb([this](Se& e){
								trigger(GUI_EVENT_WAIT_BUFFER, Float(0.0F)); // trigger source WAIT event
							}));
						}
					} else if ( status == MULTIMEDIA_SOURCE_STATUS_EOF ) {
						stop();
					}
				}
				
			}  else if ( m_status == PLAYER_STATUS_PAUSED ) {
				// 大于1000ms可更新画面
				if ( m_duration && sys_time - m_prev_presentation_time > 1000000 ) {
					OutputBuffer buffer = m_video->output();
					if ( buffer.total ) {
						load_yuv_texture(buffer);
						draw = true;
					}
				}
			}
		}
		
		if ( m_video_buffer.total ) {
			
			uint64 pts = m_video_buffer.time;
			
			if (m_uninterrupted_play_start_systime &&        // 0表示还没开始
					pts &&                                          // 演示时间为0表示开始或(即时渲染如视频电话)
					sys_time - m_prev_presentation_time < 300000    // 距离上一帧超过300ms重新记时(如应用程序从休眠中恢复或数据缓冲)
			) {
				int64 st = (sys_time - m_uninterrupted_play_start_systime) -       // sys
									 (pts - m_uninterrupted_play_start_time);   // frame
				if ( st >= 0 ) { // 是否达到渲染帧时间
					uint64 st_s = sys::time_monotonic();
					load_yuv_texture(m_video_buffer);
					//t_debug("+++++++ input_video_yuv, use_time: %llu, pts: %llu, delay: %lld",
					//        sys_time_monotonic() - st_s, pts, st);
					draw = true;
				}
			} else { // start reander one frame
				XX_DEBUG("Reset timing : prs: %lld, %lld, %lld",
								pts,
								sys_time - m_prev_presentation_time, m_uninterrupted_play_start_systime);

				if ( m_status == PLAYER_STATUS_START ) {
					ScopeLock scope(m_mutex);
					m_status = PLAYER_STATUS_PLAYING;
					m_keep->post(Cb([this](Se& e){
						trigger(GUI_EVENT_START_PLAY); // trigger start_play event
					}));
				}
				{
					ScopeLock scope(m_mutex);
					m_uninterrupted_play_start_systime = sys_time;
					m_uninterrupted_play_start_time = m_video_buffer.time;
				}
				load_yuv_texture(m_video_buffer);
				draw = true;
			}
		}
		
		// uint64 st = sys::time_monotonic();
		m_video->advance();
		// t_debug("advance, %llu", sys_time_monotonic() - st);
		
		return draw;
	}

	// set pcm ..
	bool write_audio_pcm() {
		bool r = m_pcm->write(WeakBuffer((char*)m_audio_buffer.data[0], m_audio_buffer.linesize[0]));
		if ( !r ) {
			DLOG("Discard, audio PCM frame, %lld", m_audio_buffer.time);
		}
		m_audio->release(m_audio_buffer);
		return r;
	}
	
	void play_audio() {
		float compensate = m_pcm->compensate();
		// m_audio->set_frame_size( m_pcm->buffer_size() );
	 loop:

		uint64 sys_time = sys::time_monotonic();
		
		{ //
			ScopeLock scope(m_mutex);
			
			if ( m_status == PLAYER_STATUS_STOP ) { // stop
				return; // stop audio
			}
			
			if ( !m_audio_buffer.total ) {
				if ( m_status == PLAYER_STATUS_PLAYING || m_status == PLAYER_STATUS_START ) {
					m_audio_buffer = m_audio->output();
				}
			}

			if (m_audio_buffer.total) {
				if (m_uninterrupted_play_start_systime) {
					if (m_audio_buffer.time) {
						int64 st = (sys_time - m_uninterrupted_play_start_systime) -     // sys
											 (m_audio_buffer.time - m_uninterrupted_play_start_time); // frame
						int delay = m_audio->frame_interval() * compensate;

						if (st >= delay) { // 是否达到播放声音时间。输入pcm到能听到声音会有一些延时,这里设置补偿
							write_audio_pcm();
						}
					} else { // 演示时间为0表示开始或即时渲染(如视频电话)
						write_audio_pcm();
					}
				}
			}
			m_audio->advance();
		}
		
		int frame_interval = 1000.0 / 120.0 * 1000; // 120fsp
		int64 sleep_st = frame_interval - sys::time_monotonic() + sys_time;
		if ( sleep_st > 0 ) {
			Thread::sleep(sleep_st);
		}

		goto loop;
	}
	
	void trigger(const GUIEventName& name, const Object& data = Object()) {
		Handle<GUIEvent> evt = New<GUIEvent>(this, data);
		View::trigger(name, **evt);
	}
	
	bool stop_2(Lock& lock, bool is_event) {
		
		if ( m_status != PLAYER_STATUS_STOP ) {
			
			m_status = PLAYER_STATUS_STOP;
			m_uninterrupted_play_start_systime = 0;
			m_uninterrupted_play_start_time = 0;
			m_prev_presentation_time = 0;
			m_time = 0;
			
			if ( m_audio ) {
				m_audio->release(m_audio_buffer);
				m_audio->extractor()->set_disable(true);
				m_audio->close();
			}
			if ( m_video ) {
				m_video->release(m_video_buffer);
				m_video->extractor()->set_disable(true);
				m_video->close();
				texture()->unload();
			}

			if (m_pcm) {
				m_pcm->flush();
			}
			
			unregister_task();
			m_source->stop();
			
			lock.unlock();
			{ // wait audio thread end
				ScopeLock scope(m_audio_loop_mutex);
			}
			if ( is_event ) {
				m_keep->post(Cb([this](Se& e){
					trigger(GUI_EVENT_STOP); // trigger stop event
				}));
			}
			lock.lock();
			
			return true;
		}
		return false;
	}
	
	void stop_and_release(Lock& lock, bool is_event) {
		
		if ( m_task_id ) {
			m_keep->host()->cancel_work(m_task_id);
			m_task_id = 0;
		}
		
		stop_2(lock, is_event);
		
		Release(m_audio); m_audio = nullptr;
		Release(m_video); m_video = nullptr;
		Release(m_source); m_source = nullptr;
		Release(m_keep); m_keep = nullptr;
		PCMPlayer::Traits::Release(m_pcm); m_pcm = nullptr;
		
		m_time = 0;
		m_duration = 0;
		m_video_width = 0;
		m_video_height = 0;
	}
	
	inline void stop_and_release(bool is_event) {
		Lock lock(m_mutex);
		stop_and_release(lock, is_event);
	}
	
	void start_run() {
		Lock lock(m_mutex);
		
		XX_ASSERT( m_source && m_video );
		XX_ASSERT( m_source->is_active() );
		XX_ASSERT( m_status == PLAYER_STATUS_START );
		
		m_waiting_buffer = false;
		
		if ( m_video->open() ) { // clear video
			m_source->seek(0);
			m_video->release( m_video_buffer );
			m_video->flush();
			m_video->extractor()->set_disable(false);
		} else {
			stop_2(lock, true);
			XX_ERR("Unable to open video decoder"); return;
		}
		
		if ( m_audio && m_pcm && m_audio->open() ) {  // clear audio
			m_audio->release( m_audio_buffer );
			m_audio->flush();
			m_audio->extractor()->set_disable(false);
			m_pcm->flush();
			m_pcm->set_volume(m_volume);
			m_pcm->set_mute(m_mute);
			
			Thread::spawn([this](Thread& t){
				ScopeLock scope(m_audio_loop_mutex);
				Inl_Video(this)->play_audio();
				return 0;
			}, "audio");
		}
		
		register_task();
	}
	
	/**
	 * @func is_active
	 */
	inline bool is_active() {
		return m_status == PLAYER_STATUS_PAUSED || m_status == PLAYER_STATUS_PLAYING;
	}
};

/**
 * @destructor
 */
Video::~Video() {
	Inl_Video(this)->stop_and_release(false);
}

void Video::multimedia_source_wait_buffer(MultimediaSource* so, float process) {
	
	if ( m_waiting_buffer ) { /* 开始等待数据缓存不触发事件,因为在解码器队列可能还存在数据,
														 * 所以等待解码器也无法输出数据时再触发事件
														 */
		if ( process < 1 ) { // trigger event wait_buffer
			Inl_Video(this)->trigger(GUI_EVENT_WAIT_BUFFER, Float(process));
		}
	}
}

void Video::multimedia_source_eof(MultimediaSource* so) {
	Inl_Video(this)->trigger(GUI_EVENT_SOURCE_EOF); // trigger event eof
}

void Video::multimedia_source_error(MultimediaSource* so, cError& err) {
	Inl_Video(this)->trigger(GUI_EVENT_ERROR, err); // trigger event error
	stop();
}

String Video::source() const {
	if ( m_source ) {
		return m_source->uri().href();
	} else {
		return String();
	}
}

void Video::multimedia_source_ready(MultimediaSource* src) {
	XX_ASSERT( m_source == src );
	
	if ( m_video ) {
		Inl_Video(this)->trigger(GUI_EVENT_READY); // trigger event ready
		if ( m_status == PLAYER_STATUS_START ) {
			Inl_Video(this)->start_run();
		}
		return;
	}

	XX_ASSERT(!m_video);
	XX_ASSERT(!m_audio);
	
	// 创建解码器很耗时这会导致gui线程延时,所以这里不在主线程创建
	
	m_task_id = m_keep->host()->work(Cb([=](Se& d) {
		if (m_source != src) return; // 源已被更改,所以取消
		
		MediaCodec* audio = MediaCodec::create(MEDIA_TYPE_AUDIO, m_source);
		MediaCodec* video = MediaCodec::create(MEDIA_TYPE_VIDEO, m_source);
		PCMPlayer* pcm = m_pcm;
		
		if ( audio && !m_pcm ) {
			pcm = PCMPlayer::create(audio->channel_count(),
															audio->extractor()->track(0).sample_rate );
		}
		ScopeLock scope(m_mutex);
		m_pcm = pcm;
		
		if ( m_source != src ) {
			Release(audio);
			Release(video); return;
		} else {
			m_audio = audio;
			m_video = video;
		}
	}, this/*保持Video*/), Cb([=](Se& d) {
		m_task_id = 0;
		if ( m_source != src ) return;
		if ( !m_audio) XX_ERR("Unable to create audio decoder");
		if (m_video) {
			{ //
				ScopeLock scope(m_mutex);
				const TrackInfo& info = m_video->extractor()->track(0);
				m_video_width   = info.width;
				m_video_height  = info.height;
				m_duration      = m_source->duration();
				m_color_format  = m_video->color_format();
				m_video->set_threads(2);
				m_video->set_background_run(true);
			}
			Inl_Video(this)->trigger(GUI_EVENT_READY); // trigger event ready
			
			if ( m_status == PLAYER_STATUS_START ) {
				Inl_Video(this)->start_run();
			} else {
				if ( m_auto_play ) {
					start();
				}
			}
		} else {
			Error e(ERR_VIDEO_NEW_CODEC_FAIL, "Unable to create video decoder");
			XX_ERR("%s", *e.message());
			Inl_Video(this)->trigger(GUI_EVENT_ERROR, e); // trigger event error
			stop();
		} 
	}));
}

void Video::set_source(cString& value) {
	
	if ( value.is_empty() ) {
		return;
	}
	
	String src = f_reader()->format(value);

	Lock lock(m_mutex);
	
	if ( m_source ) {
		if ( m_source->uri().href() == src ) {
			return;
		}
		Inl_Video(this)->stop_and_release(lock, true);
	}
	auto loop = main_loop(); XX_CHECK(loop, "Cannot find main run loop");
	m_source = new MultimediaSource(src, loop);
	m_keep = loop->keep_alive("Video::set_source");
	m_source->set_delegate(this);
	m_source->disable_wait_buffer(m_disable_wait_buffer);
	m_source->start();
}

/**
 * @func start play
 */
void Video::start() {
	Lock scope(m_mutex);
	
	if ( m_status == PLAYER_STATUS_STOP && m_source ) {
		m_status = PLAYER_STATUS_START;
		m_uninterrupted_play_start_systime = 0;
		m_uninterrupted_play_start_time = 0;
		m_prev_presentation_time = 0;
		m_time = 0;
		m_source->start();
		
		if ( m_video ) {
			if ( m_source->is_active() ) {
				scope.unlock();
				Inl_Video(this)->start_run();
			}
		}
	}
}

/**
 * @func stop play
 * */
void Video::stop() {
	Lock lock(m_mutex);
	if ( Inl_Video(this)->stop_2(lock, true) ) {
		mark(M_TEXTURE);
	}
}

/**
 * @func seek to target time
 */
bool Video::seek(uint64 timeUs) {
	ScopeLock scope(m_mutex);
	
	if ( Inl_Video(this)->is_active() && timeUs < m_duration ) {
		XX_ASSERT( m_source );
		
		if ( m_source->seek(timeUs) ) {
			m_uninterrupted_play_start_systime = 0;
			m_time = timeUs;
			{ // clear video
				m_video->release( m_video_buffer );
				m_video->flush();
			}
			if ( m_audio ) { // clear audio
				m_audio->release( m_audio_buffer );
				m_audio->flush();
			}
			if ( m_pcm ) {
				m_pcm->flush();
			}
			m_keep->post(Cb([this](SimpleEvent& e){
				Inl_Video(this)->trigger(GUI_EVENT_SEEK, Uint64(m_time)); // trigger seek event
			}));
			return true;
		}
	}
	return false;
}

/**
 * @func pause play
 * */
void Video::pause() {
	ScopeLock scope(m_mutex);
	if ( m_status == PLAYER_STATUS_PLAYING && m_duration /* 没有长度信息不能暂停*/ ) {
		m_status = PLAYER_STATUS_PAUSED;
		m_uninterrupted_play_start_systime = 0;
		m_keep->post(Cb([this](SimpleEvent& e){
			Inl_Video(this)->trigger(GUI_EVENT_PAUSE); // trigger pause event
		}));
	}
}

/**
 * @func resume play
 * */
void Video::resume() {
	ScopeLock scope(m_mutex);
	if ( m_status == PLAYER_STATUS_PAUSED ) {
		m_status = PLAYER_STATUS_PLAYING;
		m_uninterrupted_play_start_systime = 0;
		m_keep->post(Cb([this](SimpleEvent& e){
			Inl_Video(this)->trigger(GUI_EVENT_RESUME); // trigger resume event
		}));
	}
}

/**
 * @func set_mute setting mute status
 * */
void Video::set_mute(bool value) {
	ScopeLock scope(m_mutex);
	if ( value != m_mute ) {
		m_mute = value;
		if ( m_pcm ) { // action
			m_pcm->set_mute(m_mute);
		}
	}
}

/**
 * @func set_volume
 */
void Video::set_volume(uint value) {
	ScopeLock scope(m_mutex);
	value = XX_MIN(value, 100);
	m_volume = value;
	if ( m_pcm ) {
		m_pcm->set_volume(value);
	}
}


/**
 * @func time
 * */
uint64 Video::time() { ScopeLock scope(m_mutex); return m_time; }

/**
 * @func duration
 * */
uint64 Video::duration() { ScopeLock scope(m_mutex); return m_duration; }

/**
 * @func audio_track_count
 */
uint Video::audio_track_count() {
	ScopeLock lock(m_mutex);
	if ( m_audio ) {
		return m_audio->extractor()->track_count();
	}
	return 0;
}

/**
 * @func audio_track_index
 */
uint Video::audio_track_index() {
	ScopeLock lock(m_mutex);
	if ( m_audio ) {
		return m_audio->extractor()->track_index();
	}
	return 0;
}

/**
 * @func audio_track
 */
const TrackInfo* Video::audio_track() {
	ScopeLock lock(m_mutex);
	if ( m_audio ) {
		return &m_audio->extractor()->track();
	}
	return nullptr;
}

/**
 * @func audio_track
 */
const TrackInfo* Video::audio_track(uint index) {
	ScopeLock lock(m_mutex);
	if ( m_audio && index < m_audio->extractor()->track_count() ) {
		return &m_audio->extractor()->track(index);
	}
	return nullptr;
}

/**
 * @func video_track
 * */
const TrackInfo* Video::video_track() {
	ScopeLock lock(m_mutex);
	if ( m_video ) {
		return &m_video->extractor()->track();
	}
	return nullptr;
}

/**
 * @func select_audio_track
 * */
void Video::select_audio_track(uint index) {
	ScopeLock scope(m_mutex);
	if ( m_audio && index < m_audio->extractor()->track_count() ) {
		m_audio->extractor()->select_track(index);
	}
}

/**
 * @func source_status
 * */
MultimediaSourceStatus Video::source_status() {
	ScopeLock lock(m_mutex);
	if ( m_source ) {
		return m_source->status();
	}
	return MULTIMEDIA_SOURCE_STATUS_UNINITIALIZED;
}

/**
 * @func video_width
 */
uint Video::video_width() {
	ScopeLock lock(m_mutex);
	return m_video_width;
}

/**
 * @func video_height
 */
uint Video::video_height() {
	ScopeLock lock(m_mutex);
	return m_video_height;
}

PlayerStatus Video::status() {
	ScopeLock lock(m_mutex);
	return m_status;
}

bool Video::run_task(int64 sys_time) {
	// video
	bool draw = Inl_Video(this)->advance_video(sys_time);
	
	// XX_DEBUG("------------------------ frame: %llu", sys_time_monotonic() - sys_time);
	
	{
		ScopeLock scope(m_mutex);
		if (m_uninterrupted_play_start_systime) {
			m_time = sys_time - m_uninterrupted_play_start_systime + m_uninterrupted_play_start_time;
		}
		m_prev_run_task_systime = sys_time;
		
		if (draw) {
			m_prev_presentation_time = sys_time;
		}
	}
	
	return draw && m_final_visible;
}

void Video::disable_wait_buffer(bool value) {
	ScopeLock scope(m_mutex);
	m_disable_wait_buffer = value;
	if (m_source) {
		m_source->disable_wait_buffer(value);
	}
}

void Video::set_texture(Texture* value) {
	XX_WARN("Video cannot set this property");
}

void Video::set_auto_play(bool value) {
	ScopeLock scope(m_mutex); m_auto_play = value;
}

/**
 * @overwrite
 */
void Video::draw(Draw* draw) {
	if ( m_visible ) {
		if ( mark_value ) {
			solve();
		}
		draw->draw(this);
		mark_value = M_NONE;
	}
}

/**
 * @overwrite
 */
void Video::remove() {
	Inl_Video(this)->stop_and_release(true);
	Image::remove();
}

XX_END
