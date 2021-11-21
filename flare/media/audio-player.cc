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

#include "./audio-player.h"
#include "../app.h"
#include "../errno.h"
#include "../util/fs.h"
#include "../util/os.h"

namespace flare {

	/**
	* @constructor
	*/
	AudioPlayer::AudioPlayer(cString& uri)
		: _source(NULL)
		, _pcm(NULL)
		, _audio(NULL)
		, _keep(NULL)
		, _status(PLAYER_STATUS_STOP)
		, _audio_buffer()
		, _duration(0), _time(0)
		, _uninterrupted_play_start_time(0)
		, _uninterrupted_play_start_systime(0)
		, _prev_presentation_time(0)
		, _task_id(0)
		, _volume(100)
		, _mute(false)
		, _auto_play(true)
		, _disable_wait_buffer(false)
		, _waiting_buffer(false)
	{
		set_src(uri);
	}

	typedef MultimediaSource::TrackInfo TrackInfo;

	/**
	* @class Video::Inl
	*/
	FX_DEFINE_INLINE_MEMBERS(AudioPlayer, Inl) {
		public:
		
		// set pcm ..
		bool write_audio_pcm(uint64_t st) {
			bool r = _pcm->write(WeakBuffer((Char*)_audio_buffer.data[0], _audio_buffer.linesize[0]));
			if ( !r ) {
				FX_DEBUG("Discard, audio PCM frame, %lld", _audio_buffer.time);
			} else {
				_prev_presentation_time = st;
			}
			_audio->release(_audio_buffer);
			return r;
		}
		
		void play_audio() {
			float compensate = _pcm->compensate();
			loop:
			
			uint64_t sys_time = os::time_monotonic();
			
			{ //
				ScopeLock scope(_mutex);
				
				if ( _status == PLAYER_STATUS_STOP ) { // stop
					return; // stop audio
				}
				
				if ( !_audio_buffer.total ) {
					if ( _status == PLAYER_STATUS_PLAYING || _status == PLAYER_STATUS_START ) {
						_audio_buffer = _audio->output();
						
						if ( _audio_buffer.total ) {
							if ( _waiting_buffer ) {
								_keep->post(Cb([this](CbData& evt) {
									trigger(UIEvent_WaitBuffer, Float(1.0F)); // trigger source WAIT event
								}));
								_waiting_buffer = false;
							}
						} else {
							MultimediaSourceStatus status = _source->status();
							if ( status == MULTIMEDIA_SOURCE_STATUS_WAIT ) { // 源..等待数据
								if ( _waiting_buffer == false ) {
									_keep->post(Cb([this](CbData& evt) {
										trigger(UIEvent_WaitBuffer, Float(0.0F)); // trigger source WAIT event
									}));
									_waiting_buffer = true;
								}
							} else if ( status == MULTIMEDIA_SOURCE_STATUS_EOF ) {
								_keep->post(Cb([this](CbData& evt) {
									stop();
								}));
								return;
							}
						}
						//
					}
				}
				
				if ( _audio_buffer.total ) {
					uint64_t pts = _audio_buffer.time;
					
					if (_uninterrupted_play_start_systime &&         // 0表示还没开始
							pts &&                                        // 演示时间为0表示开始或(即时渲染如视频电话)
							sys_time - _prev_presentation_time < 300000  // 距离上一帧超过300ms重新记时(如应用程序从休眠中恢复或数据缓冲)
					) {
						int64_t st =  (sys_time - _uninterrupted_play_start_systime) -     // sys
												(pts - _uninterrupted_play_start_time); // frame
						int delay = _audio->frame_interval() * compensate;
						
						if (st >= delay) { // 是否达到播放声音时间。输入pcm到能听到声音会有一些延时,这里设置补偿
							write_audio_pcm(sys_time);
						}
					} else {
						if ( _status == PLAYER_STATUS_START ) {
							_status = PLAYER_STATUS_PLAYING;
							_keep->post(Cb([this](CbData& evt) {
								trigger(UIEvent_StartPlay); // trigger start_play event
							}));
						}
						_uninterrupted_play_start_systime = sys_time;
						_uninterrupted_play_start_time = _audio_buffer.time;
						write_audio_pcm(sys_time);
					}
				}
				
				if (_uninterrupted_play_start_systime) {
					_time = sys_time - _uninterrupted_play_start_systime + _uninterrupted_play_start_time;
				}
				
				_audio->advance();
			}
			
			int frame_interval = 1000.0 / 120.0 * 1000; // 120fsp
			int64_t sleep_st = frame_interval - os::time_monotonic() + sys_time;
			if ( sleep_st > 0 ) {
				Thread::sleep(sleep_st);
			}

			goto loop;
		}
		
		void trigger(const UIEventName& type, const Object& data = Object()) {
			Handle<Event<>> evt = New<Event<>>(data);
			Notification::trigger(type, **evt);
		}
		
		bool stop_2(Lock& lock, bool is_event) {
			if ( _status != PLAYER_STATUS_STOP ) {
				_status = PLAYER_STATUS_STOP;
				_uninterrupted_play_start_time = 0;
				_uninterrupted_play_start_systime = 0;
				_prev_presentation_time = 0;
				_time = 0;
				
				if ( _audio ) {
					_audio->release(_audio_buffer);
					_audio->extractor()->set_disable(true);
					_audio->close();
					Release(_audio); _audio = nullptr;
				}

				PCMPlayer::Traits::Release(_pcm); _pcm = nullptr;
				
				_source->stop();
				
				lock.unlock();
				{ // wait audio thread end
					ScopeLock scope(_audio_loop_mutex);
				}
				if ( is_event ) {
					_keep->post(Cb([this](CbData& e){
						Inl_AudioPlayer(this)->trigger(UIEvent_Stop); // trigger stop event
					}));
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
			
			stop_2(lock, is_event);
			
			Release(_audio); _audio = nullptr;
			Release(_source); _source = nullptr;
			Release(_keep); _keep = nullptr;
			PCMPlayer::Traits::Release(_pcm); _pcm = nullptr;
			
			_time = 0;
			_duration = 0;
		}
		
		inline void stop_and_release(bool is_event) {
			Lock lock(_mutex);
			stop_and_release(lock, is_event);
		}
		
		/**
		* @func is_active
		*/
		inline bool is_active() {
			return _status == PLAYER_STATUS_PAUSED || _status == PLAYER_STATUS_PLAYING;
		}
		
		void start_run() {
			Lock lock(_mutex);
			
			ASSERT( _source && _audio && _pcm );
			ASSERT( _source->is_active() );
			ASSERT( _status == PLAYER_STATUS_START );
			
			_waiting_buffer = false;
			
			if ( _audio->open() ) {
				_source->seek(0);
				_audio->release( _audio_buffer );
				_audio->flush();
				_audio->extractor()->set_disable(false);
				_pcm->flush();
				_pcm->set_volume(_volume);
				_pcm->set_mute(_mute);
				
				Thread::spawn([this](Thread& t) {
					ScopeLock scope(_audio_loop_mutex);
					Inl_AudioPlayer(this)->play_audio();
					return 0;
				}, "audio");
			} else {
				stop_2(lock, true);
			}
		}
		
	};

	void AudioPlayer::multimedia_source_ready(MultimediaSource* src) {
		ASSERT(_source == src);
		
		if (_audio) {
			Inl_AudioPlayer(this)->trigger(UIEvent_Ready); // trigger event ready
			if ( _status == PLAYER_STATUS_START ) {
				Inl_AudioPlayer(this)->start_run();
			}
			return;
		}
		
		// 创建解码器很耗时这会导致主线程延时,所以任务方式发送到工作线程
		_task_id = _keep->host()->work(Cb([=](CbData& d) {
			if (_source != src) return; // 源已被更改,所以取消
			
			MediaCodec* audio = MediaCodec::create(MEDIA_TYPE_AUDIO, _source);
			PCMPlayer* pcm = _pcm;
			
			if ( audio && !_pcm ) {
				pcm = PCMPlayer::create(audio->channel_count(),
																audio->extractor()->track(0).sample_rate );
			}
			
			ScopeLock scope(_mutex);
			_pcm = pcm;
			
			if ( _source != src ) {
				Release(audio); return;
			} else {
				_audio = audio;
			}
		}, this/*保持AutoPlayer不释放直到任务结束*/), Cb([=](CbData& d) { //任务完成后回调
			_task_id = 0; //
			if ( _source != src ) return;
			
			if (_audio && _pcm) {
				{ //
					ScopeLock scope(_mutex);
					_duration = _source->duration();
				}
				Inl_AudioPlayer(this)->trigger(UIEvent_Ready); // trigger event ready
				
				if ( _status == PLAYER_STATUS_START ) {
					Inl_AudioPlayer(this)->start_run();
				} else {
					if (_auto_play) {
						start();
					}
				}
			} else {
				Error e(ERR_AUDIO_NEW_CODEC_FAIL, "Unable to create video decoder");
				FX_ERR("%s", *e.message());
				Inl_AudioPlayer(this)->trigger(UIEvent_Error, e); // trigger event error
				stop();
			}
		}));
	}

	void AudioPlayer::multimedia_source_wait_buffer(MultimediaSource* so, float process) {
		if ( _waiting_buffer ) {/* 开始等待数据缓存不触发事件,因为在解码器队列可能还存在数据,
															* 所以等待解码器也无法输出数据时再触发事件
															*/
			if ( process < 1.0 ) {
				// trigger event wait_buffer
				Inl_AudioPlayer(this)->trigger(UIEvent_WaitBuffer, Float(process));
			}
		}
	}

	void AudioPlayer::multimedia_source_eof(MultimediaSource* so) {
		Inl_AudioPlayer(this)->trigger(UIEvent_SourceEnd); // trigger event eof
	}

	void AudioPlayer::multimedia_source_error(MultimediaSource* so, cError& err) {
		Inl_AudioPlayer(this)->trigger(UIEvent_Error, err); // trigger event error
		stop();
	}

	/**
	* @destructor
	*/
	AudioPlayer::~AudioPlayer() {
		Inl_AudioPlayer(this)->stop_and_release(false);
	}

	/**
	* @func src # get src
	*/
	String AudioPlayer::src() {
		ScopeLock lock(_mutex);
		if ( _source ) {
			return _source->uri().href();
		} else {
			return String(); //::empty;
		}
	}

	/**
	* @func src # set src
	*/
	void AudioPlayer::set_src(cString& value) {

		if ( value.is_empty() ) {
			return;
		}
		String src = fs_reader()->format(value);
		
		Lock lock(_mutex);
		
		if ( _source ) {
			if ( _source->uri().href() == src ) {
				return;
			}
			Inl_AudioPlayer(this)->stop_and_release(lock, true);
		}
		auto loop = RunLoop::main_loop();
		ASSERT(loop, "Cannot find main run loop");
		_source = new MultimediaSource(src, loop);
		_keep = loop->keep_alive("AudioPlayer::set_src");
		_source->set_delegate(this);
		_source->disable_wait_buffer(_disable_wait_buffer);
		_source->start();
	}

	/**
	* @func start play
	*/
	void AudioPlayer::start() {
		Lock scope(_mutex);
		if ( _status == PLAYER_STATUS_STOP && _source ) {
			_status = PLAYER_STATUS_START;
			_uninterrupted_play_start_time = 0;
			_uninterrupted_play_start_systime = 0;
			_prev_presentation_time = 0;
			_time = 0;
			_source->start();
			if ( _audio && _pcm ) {
				if ( _source->is_active() ) {
					scope.unlock();
					Inl_AudioPlayer(this)->start_run();
				}
			}
		}
	}

	/**
	* @func stop play
	* */
	void AudioPlayer::stop() {
		Lock lock(_mutex);
		Inl_AudioPlayer(this)->stop_2(lock, true);
	}

	/**
	* @func set_auto_play setting auto play
	*/
	void AudioPlayer::set_auto_play(bool value) {
		ScopeLock scope(_mutex); 
		_auto_play = value; 
	}

	/**
	* @func source_status
	* */
	MultimediaSourceStatus AudioPlayer::source_status() {
		ScopeLock scope(_mutex);
		if ( _source ) {
			return _source->status();
		}
		return MULTIMEDIA_SOURCE_STATUS_UNINITIALIZED;
	}

	/**
	* @func status getting play status
	*/
	PlayerStatus AudioPlayer::status() {
		ScopeLock scope(_mutex); 
		return _status; 
	}

	/**
	* @func seek to target time
	*/
	bool AudioPlayer::seek(uint64_t timeUs) {
		ScopeLock scope(_mutex);
		if ( Inl_AudioPlayer(this)->is_active() && timeUs < _duration ) {
			ASSERT(_source);
			if ( _source->seek(timeUs) ) {
				_uninterrupted_play_start_systime = 0;
				_time = timeUs;
				_audio->release( _audio_buffer );
				_audio->flush();
				_pcm->flush();
				_keep->post(Cb([this](CbData& e){
					Inl_AudioPlayer(this)->trigger(UIEvent_Seek, Uint64(_time)); // trigger seek event
				}));
				return true;
			}
		}
		return false;
	}

	/**
	* @func pause play
	* */
	void AudioPlayer::pause() {
		ScopeLock scope(_mutex);
		if ( _status == PLAYER_STATUS_PLAYING && _duration /*没有长度信息不能暂停*/ ) {
			_status = PLAYER_STATUS_PAUSED;
			_uninterrupted_play_start_systime = 0;
			_keep->post(Cb([this](CbData& e){
				Inl_AudioPlayer(this)->trigger(UIEvent_Pause); // trigger pause event
			}));
		}
	}

	/**
	* @func resume play
	* */
	void AudioPlayer::resume() {
		ScopeLock scope(_mutex);
		if ( _status == PLAYER_STATUS_PAUSED ) {
			_status = PLAYER_STATUS_PLAYING;
			_uninterrupted_play_start_systime = 0;
			_keep->post(Cb([this](CbData& e){
				Inl_AudioPlayer(this)->trigger(UIEvent_Resume); // trigger resume event
			}));
		}
	}

	/**
	* @func set_mute setting mute status
	* */
	void AudioPlayer::set_mute(bool value) {
		ScopeLock scope(_mutex);
		if ( value != _mute ) {
			_mute = value;
			if ( _pcm ) { // action
				_pcm->set_mute(_mute);
			}
		}
	}

	/**
	* @func set_volume
	*/
	void AudioPlayer::set_volume(uint32_t value) {
		ScopeLock scope(_mutex);
		value = FX_MIN(value, 100);
		_volume = value;
		if ( _pcm ) {
			_pcm->set_volume(value);
		}
	}

	/**
	* @func time
	* */
	uint64_t AudioPlayer::time() {
		ScopeLock scope(_mutex); return _time;
	}

	/**
	* @func duration
	* */
	uint64_t AudioPlayer::duration() {
		ScopeLock scope(_mutex); return _duration;
	}

	/**
	* @func disable_wait_buffer
	*/
	void AudioPlayer::disable_wait_buffer(bool value) {
		ScopeLock scope(_mutex);
		_disable_wait_buffer = value;
		if (_source) {
			_source->disable_wait_buffer(value);
		}
	}

	/**
	* @func audio_track_count
	*/
	uint32_t AudioPlayer::track_count() {
		ScopeLock lock(_mutex);
		if ( _audio ) {
			return _audio->extractor()->track_count();
		}
		return 0;
	}

	/**
	* @func audio_track
	*/
	uint32_t AudioPlayer::track_index() {
		ScopeLock lock(_mutex);
		if ( _audio ) {
			return _audio->extractor()->track_index();
		}
		return 0;
	}

	/**
	* @func audio_track
	*/
	const TrackInfo* AudioPlayer::track() {
		ScopeLock lock(_mutex);
		if ( _audio ) {
			return &_audio->extractor()->track();
		}
		return nullptr;
	}

	/**
	* @func audio_track
	*/
	const TrackInfo* AudioPlayer::track(uint32_t index) {
		ScopeLock lock(_mutex);
		if ( _audio && index < _audio->extractor()->track_count() ) {
			return &_audio->extractor()->track(index);
		}
		return nullptr;
	}

	/**
	* @func select_audio_track
	* */
	void AudioPlayer::select_track(uint32_t index) {
		ScopeLock scope(_mutex);
		if ( _audio && index < _audio->extractor()->track_count() ) {
			_audio->extractor()->select_track(index);
		}
	}

}
