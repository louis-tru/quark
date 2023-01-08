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
 * ***** END LICENSE BLOCK ***** */

#include <math.h>
#include "./video.h"
#include "../app.h"
#include "../media/media_codec.h"
#include "../util/loop.h"
#include "../util/fs.h"
#include "../errno.h"

namespace quark {

	typedef MultimediaSource::TrackInfo TrackInfo;
	typedef PreRender::Task::ID TaskID;
	typedef MediaCodec::OutputBuffer OutputBuffer;

	Video::Video()
		: _source(NULL)
		, _audio(NULL)
		, _video(NULL)
		, _pcm(NULL)
		, _keep(nullptr)
		, _status(PLAYER_STATUS_STOP)
		, _audio_buffer(), _video_buffer()
		, _time(0), _duration(0)
		, _uninterrupted_play_start_time(0)
		, _uninterrupted_play_start_systime(0)
		, _prev_presentation_time(0)
		, _prev_run_task_systime(0)
		, _video_width(0), _video_height(0)
		, _task_id(0)
		, _color_format(VIDEO_COLOR_FORMAT_INVALID)
		, _volume(100)
		, _auto_play(true)
		, _mute(false)
		, _disable_wait_buffer(false)
		, _waiting_buffer(false)
	{
	}

	Qk_DEFINE_INLINE_MEMBERS(Video, Inl) {
	public:

		bool load_yuv_texture(OutputBuffer& buffer) { // set yuv texture ..
			Array<WeakBuffer> body(3);
			body[0] = WeakBuffer((char*)buffer.data[0], buffer.linesize[0]);  // y
			body[1] = WeakBuffer((char*)buffer.data[1], buffer.linesize[1]);  // u
			body[2] = WeakBuffer((char*)buffer.data[2], buffer.linesize[2]);  // v
			
			// PixelData pixel(body, _video_width, _video_height, (PixelData::Format)_color_format );
			
			// bool r = static_cast<TextureYUV*>(texture())->load_yuv(pixel); // load texture
			// _video->release(buffer);
			// return r;
		}
		
		bool advance_video(uint64_t sys_time) {
			Qk_ASSERT(m_status != PLAYER_STATUS_STOP, "#Video#Inl#advance_video 0");
			
			bool draw = false;
			
			if ( ! _video_buffer.total ) {
				if ( _status == PLAYER_STATUS_PLAYING || _status == PLAYER_STATUS_START ) {
					_video_buffer = _video->output();
					//t_debug("output, %llu", sys_time_monotonic() - sys_time);

					if ( _video_buffer.total ) {
						if ( _waiting_buffer ) {
							_waiting_buffer = false;
							_keep->post(Cb([this](Cb::Data& e){ trigger(UIEvent_WaitBuffer, Float(1.0F)); /* trigger source WAIT event */ }));
						}
					} else { // 没有取到数据
						MultimediaSourceStatus status = _source->status();
						if ( status == MULTIMEDIA_SOURCE_STATUS_WAIT ) { // 源..等待数据
							if ( _waiting_buffer == false ) {
								_waiting_buffer = true;
								_keep->post(Cb([this](Cb::Data& e) { trigger(UIEvent_WaitBuffer, Float(0.0F)); /* trigger source WAIT event */ }));
							}
						} else if ( status == MULTIMEDIA_SOURCE_STATUS_EOF ) {
							stop();
						}
					}
					
				}  else if ( _status == PLAYER_STATUS_PAUSED ) {
					// 大于1000ms可更新画面
					if ( _duration && sys_time - _prev_presentation_time > 1000000 ) {
						OutputBuffer buffer = _video->output();
						if ( buffer.total ) {
							load_yuv_texture(buffer);
							draw = true;
						}
					}
				}
			}
			
			if ( _video_buffer.total ) {
				
				uint64_t pts = _video_buffer.time;
				
				if (_uninterrupted_play_start_systime &&        // 0表示还没开始
						pts &&                                          // 演示时间为0表示开始或(即时渲染如视频电话)
						sys_time - _prev_presentation_time < 300000    // 距离上一帧超过300ms重新记时(如应用程序从休眠中恢复或数据缓冲)
				) {
					int64_t st = (sys_time - _uninterrupted_play_start_systime) -       // sys
										(pts - _uninterrupted_play_start_time);   // frame
					if ( st >= 0 ) { // 是否达到渲染帧时间
						uint64_t st_s = time_monotonic();
						load_yuv_texture(_video_buffer);
						//t_debug("+++++++ input_video_yuv, use_time: %llu, pts: %llu, delay: %lld",
						//        sys_time_monotonic() - st_s, pts, st);
						draw = true;
					}
				} else { // start reander one frame
					Qk_DEBUG("Reset timing : prs: %lld, %lld, %lld",
									pts,
									sys_time - _prev_presentation_time, _uninterrupted_play_start_systime);

					if ( _status == PLAYER_STATUS_START ) {
						ScopeLock scope(_mutex);
						_status = PLAYER_STATUS_PLAYING;
						_keep->post(Cb([this](Cb::Data& e){ trigger(UIEvent_StartPlay); /* trigger start_play event */ }));
					}
					{
						ScopeLock scope(_mutex);
						_uninterrupted_play_start_systime = sys_time;
						_uninterrupted_play_start_time = _video_buffer.time;
					}
					load_yuv_texture(_video_buffer);
					draw = true;
				}
			}
			
			// uint64 st = sys::time_monotonic();
			_video->advance();
			// t_debug("advance, %llu", sys_time_monotonic() - st);
			
			return draw;
		}

		// set pcm ..
		bool write_audio_pcm() {
			bool r = _pcm->write(WeakBuffer((char*)_audio_buffer.data[0], _audio_buffer.linesize[0]));
			if ( !r ) {
				Qk_LOG("Discard, audio PCM frame, %lld", _audio_buffer.time);
			}
			_audio->release(_audio_buffer);
			return r;
		}
		
		void run_play_audio_loop() {
			float compensate = _pcm->compensate();
			// m_audio->set_frame_size( m_pcm->buffer_size() );
		loop:

			uint64_t sys_time = time_monotonic();
			
			{ //
				ScopeLock scope(_mutex);
				
				if ( _status == PLAYER_STATUS_STOP ) { // stop
					return; // stop audio
				}
				
				if ( !_audio_buffer.total ) {
					if ( _status == PLAYER_STATUS_PLAYING || _status == PLAYER_STATUS_START ) {
						_audio_buffer = _audio->output();
					}
				}

				if (_audio_buffer.total) {
					if (_uninterrupted_play_start_systime) {
						if (_audio_buffer.time) {
							int64_t st = (sys_time - _uninterrupted_play_start_systime) -     // sys
												(_audio_buffer.time - _uninterrupted_play_start_time); // frame
							int delay = _audio->frame_interval() * compensate;

							if (st >= delay) { // 是否达到播放声音时间。输入pcm到能听到声音会有一些延时,这里设置补偿
								write_audio_pcm();
							}
						} else { // 演示时间为0表示开始或即时渲染(如视频电话)
							write_audio_pcm();
						}
					}
				}
				_audio->advance();
			}
			
			int frame_interval = 1000.0 / 120.0 * 1000; // 120fsp
			int64_t sleep_st = frame_interval - time_monotonic() + sys_time;
			if ( sleep_st > 0 ) {
				Thread::sleep(sleep_st);
			}

			goto loop;
		}
		
		void trigger(const UIEventName& name, const Object& data = Object()) {
			Handle<UIEvent> evt = New<UIEvent>(this, data);
			View::trigger(name, **evt);
		}
		
		bool stop_from(Lock& lock, bool is_event) {
			
			if ( _status != PLAYER_STATUS_STOP ) {
				
				_status = PLAYER_STATUS_STOP;
				_uninterrupted_play_start_systime = 0;
				_uninterrupted_play_start_time = 0;
				_prev_presentation_time = 0;
				_time = 0;
				
				if ( _audio ) {
					_audio->release(_audio_buffer);
					_audio->extractor()->set_disable(true);
					_audio->close();
				}
				if ( _video ) {
					_video->release(_video_buffer);
					_video->extractor()->set_disable(true);
					_video->close();
					SourceHold::set_source(nullptr);
				}

				if (_pcm) {
					_pcm->flush();
				}
				
				unregister_task();
				_source->stop();
				
				auto loop_id = _run_loop_id;
				lock.unlock();
				Thread::wait(loop_id); // wait audio thread end

				if ( is_event ) {
					_keep->post(Cb([this](Cb::Data& e){ trigger(UIEvent_Stop); /* trigger stop event */ }));
				}
				lock.lock();
				
				return true;
			}
			return false;
		}
		
		void stop_and_release(Lock& lock, bool is_event) {
			
			if ( _task_id ) {
				_keep->host()->cancel_work(_task_id);
				_task_id = 0;
			}
			
			stop_from(lock, is_event);
			
			Release(_audio); _audio = nullptr;
			Release(_video); _video = nullptr;
			Release(_source); _source = nullptr;
			Release(_keep); _keep = nullptr;
			PCMPlayer::Traits::Release(_pcm); _pcm = nullptr;
			
			_time = 0;
			_duration = 0;
			_video_width = 0;
			_video_height = 0;
		}
		
		void start_run() {
			Lock lock(_mutex);
			
			Qk_ASSERT( _source && _video, "#Video#Inl#start_run 0");
			Qk_ASSERT( _source->is_active(), "#Video#Inl#start_run 1");
			Qk_ASSERT( _status == PLAYER_STATUS_START, "#Video#Inl#start_run 2");

			_waiting_buffer = false;

			if ( _video->open() ) { // clear video
				_source->seek(0);
				_video->release( _video_buffer );
				_video->flush();
				_video->extractor()->set_disable(false);
			} else {
				stop_from(lock, true);
				Qk_ERR("Unable to open video decoder");
				return;
			}

			if ( _audio && _pcm && _audio->open() ) {  // clear audio
				_audio->release( _audio_buffer );
				_audio->flush();
				_audio->extractor()->set_disable(false);
				_pcm->flush();
				_pcm->set_volume(_volume);
				_pcm->set_mute(_mute);

				_run_loop_id = Thread::create([](Thread& t, void* self) { // new thread
					((Inl*)self)->run_play_audio_loop(); }, this, "audio_loop");
			}

			register_task();
		}

		bool is_active() {
			return _status == PLAYER_STATUS_PAUSED || _status == PLAYER_STATUS_PLAYING;
		}
	};

	Video::~Video() {
		Lock lock(_mutex);
		Inl_Video(this)->stop_and_release(lock, false);
	}

	void Video::multimedia_source_wait_buffer(MultimediaSource* so, float process) {
		
		if ( _waiting_buffer ) { /* 开始等待数据缓存不触发事件,因为在解码器队列可能还存在数据,
															* 所以等待解码器也无法输出数据时再触发事件
															*/
			if ( process < 1 ) { // trigger event wait_buffer
				Inl_Video(this)->trigger(UIEvent_WaitBuffer, Float(process));
			}
		}
	}

	void Video::multimedia_source_eof(MultimediaSource* so) {
		Inl_Video(this)->trigger(UIEvent_SourceEnd); // trigger event eof
	}

	void Video::multimedia_source_error(MultimediaSource* so, cError& err) {
		Inl_Video(this)->trigger(UIEvent_Error, err); // trigger event error
		stop();
	}

	String Video::src() {
		if ( _source ) {
			return _source->uri().href();
		} else {
			return String();
		}
	}

	void Video::multimedia_source_ready(MultimediaSource* src) {
		Qk_ASSERT( _source == src, "#Video#multimedia_source_ready 0");
		
		if ( _video ) {
			Inl_Video(this)->trigger(UIEvent_Ready); // trigger event ready
			if ( _status == PLAYER_STATUS_START ) {
				Inl_Video(this)->start_run();
			}
			return;
		}

		Qk_ASSERT(!_video, "#Video#multimedia_source_ready 1");
		Qk_ASSERT(!_audio, "#Video#multimedia_source_ready 1");

		// 创建解码器很耗时这会导致gui线程延时,所以这里不在主线程创建
		_task_id = _keep->host()->work(Cb([=](Cb::Data& d) {
			if (_source != src) return; // 源已被更改,所以取消
			
			MediaCodec* audio = MediaCodec::create(MEDIA_TYPE_AUDIO, _source);
			MediaCodec* video = MediaCodec::create(MEDIA_TYPE_VIDEO, _source);
			PCMPlayer* pcm = _pcm;
			
			if ( audio && !_pcm ) {
				pcm = PCMPlayer::create(audio->channel_count(),
																audio->extractor()->track(0).sample_rate );
			}
			ScopeLock scope(_mutex);
			_pcm = pcm;
			
			if ( _source != src ) {
				Release(audio);
				Release(video); return;
			} else {
				_audio = audio;
				_video = video;
			}
		}, this/*保持Video*/), Cb([=](Cb::Data& d) {
			_task_id = 0;
			if ( _source != src ) return;
			if ( !_audio) Qk_ERR("Unable to create audio decoder");
			if (_video) {
				{ //
					ScopeLock scope(_mutex);
					const TrackInfo& info = _video->extractor()->track(0);
					_video_width   = info.width;
					_video_height  = info.height;
					_duration      = _source->duration();
					_color_format  = _video->color_format();
					_video->set_threads(2);
					_video->set_background_run(true);
				}
				Inl_Video(this)->trigger(UIEvent_Ready); // trigger event ready
				
				if ( _status == PLAYER_STATUS_START ) {
					Inl_Video(this)->start_run();
				} else {
					if ( _auto_play ) {
						start();
					}
				}
			} else {
				Error e(ERR_VIDEO_NEW_CODEC_FAIL, "Unable to create video decoder");
				Qk_ERR("%s", *e.message());
				Inl_Video(this)->trigger(UIEvent_Error, e); // trigger event error
				stop();
			} 
		}));
	}

	void Video::set_src(String value) {
		if ( value.is_empty() ) {
			return;
		}
		String src = fs_reader()->format(value);
		Lock lock(_mutex);

		if ( _source ) {
			if ( _source->uri().href() == src ) {
				return;
			}
			Inl_Video(this)->stop_and_release(lock, true);
		}
		auto loop = pre_render()->host()->loop();
		Qk_ASSERT(loop, "Cannot find main run loop");
		_source = new MultimediaSource(src, loop);
		_keep = loop->keep_alive("Video::set_source");
		_source->set_delegate(this);
		_source->disable_wait_buffer(_disable_wait_buffer);
		_source->start();
	}

	void Video::start() {
		Lock scope(_mutex);
		
		if ( _status == PLAYER_STATUS_STOP && _source ) {
			_status = PLAYER_STATUS_START;
			_uninterrupted_play_start_systime = 0;
			_uninterrupted_play_start_time = 0;
			_prev_presentation_time = 0;
			_time = 0;
			_source->start();
			
			if ( _video ) {
				if ( _source->is_active() ) {
					scope.unlock();
					Inl_Video(this)->start_run();
				}
			}
		}
	}

	void Video::stop() {
		Lock lock(_mutex);
		if ( Inl_Video(this)->stop_from(lock, true) ) {
			mark_none();
		}
	}

	bool Video::seek(uint64_t timeUs) {
		ScopeLock scope(_mutex);
		
		if ( Inl_Video(this)->is_active() && timeUs < _duration ) {
			Qk_ASSERT( m_source );
			
			if ( _source->seek(timeUs) ) {
				_uninterrupted_play_start_systime = 0;
				_time = timeUs;
				{ // clear video
					_video->release( _video_buffer );
					_video->flush();
				}
				if ( _audio ) { // clear audio
					_audio->release( _audio_buffer );
					_audio->flush();
				}
				if ( _pcm ) {
					_pcm->flush();
				}
				_keep->post(Cb([this](Cb::Data& e) {
					Inl_Video(this)->trigger(UIEvent_Seek, Uint64(_time)); /* trigger seek event */
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
		ScopeLock scope(_mutex);
		if ( _status == PLAYER_STATUS_PLAYING && _duration /* 没有长度信息不能暂停*/ ) {
			_status = PLAYER_STATUS_PAUSED;
			_uninterrupted_play_start_systime = 0;
			_keep->post(Cb([this](Cb::Data& e) {
				Inl_Video(this)->trigger(UIEvent_Pause); // trigger pause event
			}));
		}
	}

	/**
	 * @func resume play
	 * */
	void Video::resume() {
		ScopeLock scope(_mutex);
		if ( _status == PLAYER_STATUS_PAUSED ) {
			_status = PLAYER_STATUS_PLAYING;
			_uninterrupted_play_start_systime = 0;
			_keep->post(Cb([this](Cb::Data& e) {
				Inl_Video(this)->trigger(UIEvent_Resume); // trigger resume event
			}));
		}
	}

	void Video::set_mute(bool value) {
		ScopeLock scope(_mutex);
		if ( value != _mute ) {
			_mute = value;
			if ( _pcm ) { // action
				_pcm->set_mute(_mute);
			}
		}
	}

	void Video::set_volume(uint32_t value) {
		ScopeLock scope(_mutex);
		value = Qk_MIN(value, 100);
		_volume = value;
		if ( _pcm ) {
			_pcm->set_volume(value);
		}
	}

	uint64_t Video::time() {
		ScopeLock scope(_mutex);
		return _time;
	}

	uint64_t Video::duration() {
		ScopeLock scope(_mutex);
		return _duration;
	}

	uint32_t Video::audio_track_count() {
		ScopeLock lock(_mutex);
		if ( _audio ) {
			return _audio->extractor()->track_count();
		}
		return 0;
	}

	uint32_t Video::audio_track_index() {
		ScopeLock lock(_mutex);
		if ( _audio ) {
			return _audio->extractor()->track_index();
		}
		return 0;
	}

	const TrackInfo* Video::audio_track() {
		ScopeLock lock(_mutex);
		if ( _audio ) {
			return &_audio->extractor()->track();
		}
		return nullptr;
	}

	const TrackInfo* Video::audio_track_at(uint32_t index) {
		ScopeLock lock(_mutex);
		if ( _audio && index < _audio->extractor()->track_count() ) {
			return &_audio->extractor()->track(index);
		}
		return nullptr;
	}

	const TrackInfo* Video::video_track() {
		ScopeLock lock(_mutex);
		if ( _video ) {
			return &_video->extractor()->track();
		}
		return nullptr;
	}

	void Video::select_audio_track(uint32_t index) {
		ScopeLock scope(_mutex);
		if ( _audio && index < _audio->extractor()->track_count() ) {
			_audio->extractor()->select_track(index);
		}
	}

	MultimediaSourceStatus Video::source_status() {
		ScopeLock lock(_mutex);
		if ( _source ) {
			return _source->status();
		}
		return MULTIMEDIA_SOURCE_STATUS_UNINITIALIZED;
	}

	uint32_t Video::video_width() {
		ScopeLock lock(_mutex);
		return _video_width;
	}

	uint32_t Video::video_height() {
		ScopeLock lock(_mutex);
		return _video_height;
	}

	PlayerStatus Video::status() {
		ScopeLock lock(_mutex);
		return _status;
	}

	bool Video::run_task(int64_t sys_time) {
		// video
		bool draw = Inl_Video(this)->advance_video(sys_time);
		// FX_DEBUG("------------------------ frame: %llu", sys_time_monotonic() - sys_time);
		{
			ScopeLock scope(_mutex);
			if (_uninterrupted_play_start_systime) {
				_time = sys_time - _uninterrupted_play_start_systime + _uninterrupted_play_start_time;
			}
			_prev_run_task_systime = sys_time;
			
			if (draw) {
				_prev_presentation_time = sys_time;
			}
		}
		
		return draw && layout_depth();
	}

	void Video::set_disable_wait_buffer(bool value) {
		ScopeLock scope(_mutex);
		_disable_wait_buffer = value;
		if (_source) {
			_source->disable_wait_buffer(value);
		}
	}

	void Video::set_auto_play(bool value) {
		ScopeLock scope(_mutex);
		_auto_play = value;
	}

	void Video::remove() {
		{
			Lock lock(_mutex);
			Inl_Video(this)->stop_and_release(lock, true);
		}
		Image::remove();
	}

}
