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
 * ***** END LICENSE BLOCK *****/

#include <math.h>
#include "./video.h"
#include "../app.h"
#include "../../media/media.h"
#include "../../util/loop.h"
#include "../../util/fs.h"
#include "../../errno.h"

namespace qk {
	typedef MediaSource::Stream AvStream;
	typedef RenderTask::ID TaskID;
	typedef MediaCodec::Frame Frame;

	Qk_DEFINE_INLINE_MEMBERS(Video, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<Video::Inl*>(self)

		bool load_yuv_texture(Frame *frame) { // set yuv texture ..
			Array<WeakBuffer> body(3);
			//body[0] = WeakBuffer((char*)buffer.data[0], buffer.linesize[0]);  // y
			//body[1] = WeakBuffer((char*)buffer.data[1], buffer.linesize[1]);  // u
			//body[2] = WeakBuffer((char*)buffer.data[2], buffer.linesize[2]);  // v

			// Pixel(cPixelInfo& info, Buffer body); // move body

			auto width = _video->stream().width;
			auto height = _video->stream().height;

			PixelInfo info0(width, height, kYUV420P_Y_8_ColorType);

			Pixel pixel0(info0, Buffer());

			// bool r = static_cast<TextureYUV*>(texture())->load_yuv(pixel); // load texture
			// _video->release(buffer);
			// return r;
		}
		
		bool advance_video(uint64_t sys_time) {
			Qk_Assert(_status != kStop_PlayerStatus, "Video::Inl::advance_video()");

			bool draw = false;

			// if ( ! _video_buffer.total ) {
			// 	if ( _status == kPlaying_PlayerStatus || _status == kStart_PlayerStatus ) {
			// 		_video_buffer = _video->receive_frame();
			// 		//t_debug("output, %llu", sys_time_monotonic() - sys_time);

			// 		if ( _video_buffer.total ) {
			// 			if ( _waiting_buffer ) {
			// 				_waiting_buffer = false;
			// 				_loop->post(Cb([this](auto e){
			// 					trigger(UIEvent_WaitBuffer, Float32(1.0F)); /* trigger source WAIT event */
			// 				}));
			// 			}
			// 		} else { // 没有取到数据
			// 			MediaSourceStatus status = _source->status();
			// 			/*if ( status == kLoading_MediaSourceStatus ) { // 源..等待数据
			// 				if ( _waiting_buffer == false ) {
			// 					_waiting_buffer = true;
			// 					_loop->post(Cb([this](auto e) {
			// 						trigger(UIEvent_WaitBuffer, Float32(0.0F)); / trigger source WAIT event
			// 					}));
			// 				}
			// 			} else*/
			// 			if ( status == kEOF_MediaSourceStatus ) {
			// 				stop();
			// 			}
			// 		}
					
			// 	}  else if ( _status == kPaused_PlayerStatus ) {
			// 		// 大于1000ms可更新画面
			// 		if ( _duration && sys_time - _prev_presentation_time > 1000000 ) {
			// 			Frame buffer = _video->receive_frame();
			// 			if ( buffer.total ) {
			// 				load_yuv_texture(buffer);
			// 				draw = true;
			// 			}
			// 		}
			// 	}
			// }

			// if ( _video_buffer.total ) {

			// 	uint64_t pts = _video_buffer.time;

			// 	if (_uninterrupted_play_start_systime &&        // 0表示还没开始
			// 			pts &&                                          // 演示时间为0表示开始或(即时渲染如视频电话)
			// 			sys_time - _prev_presentation_time < 300000    // 距离上一帧超过300ms重新记时(如应用程序从休眠中恢复或数据缓冲)
			// 	) {
			// 		int64_t st = (sys_time - _uninterrupted_play_start_systime) -       // sys
			// 							(pts - _uninterrupted_play_start_time);   // frame
			// 		if ( st >= 0 ) { // 是否达到渲染帧时间
			// 			uint64_t st_s = time_monotonic();
			// 			load_yuv_texture(_video_buffer);
			// 			//t_debug("+++++++ input_video_yuv, use_time: %llu, pts: %llu, delay: %lld",
			// 			//        sys_time_monotonic() - st_s, pts, st);
			// 			draw = true;
			// 		}
			// 	} else { // start reander one frame
			// 		Qk_DEBUG("Reset timing : prs: %lld, %lld, %lld",
			// 						pts,
			// 						sys_time - _prev_presentation_time, _uninterrupted_play_start_systime);

			// 		if ( _status == kStart_PlayerStatus ) {
			// 			ScopeLock scope(_mutex);
			// 			_status = kPlaying_PlayerStatus;
			// 			_loop->post(Cb([this](Cb::Data& e){
			// 				trigger(UIEvent_StartPlay); /* trigger start_play event */
			// 			}));
			// 		}
			// 		{
			// 			ScopeLock scope(_mutex);
			// 			_uninterrupted_play_start_systime = sys_time;
			// 			_uninterrupted_play_start_time = _video_buffer.time;
			// 		}
			// 		load_yuv_texture(_video_buffer);
			// 		draw = true;
			// 	}
			// }

			// _video->send_packet_for(_source->video_extractor());

			return draw;
		}

		// set pcm ..
		bool write_audio_pcm() {
			// WeakBuffer buff((char*)_audio_buffer.data[0], _audio_buffer.linesize[0]);
			// bool r = _pcm->write(buff.buffer());
			// if ( !r ) {
			// 	Qk_LOG("Discard, audio PCM frame, %lld", _audio_buffer.time);
			// }
			// _audio->release(_audio_buffer);
			// return r;
		}

		void run_play_audio_loop() {
		// 	float compensate = _pcm->compensate();
		// 	// _audio->set_frame_size( _pcm->buffer_size() );
		// loop:

		// 	uint64_t sys_time = time_monotonic();

		// 	{ //
		// 		ScopeLock scope(_mutex);

		// 		if ( _status == kStop_PlayerStatus ) { // stop
		// 			return; // stop audio
		// 		}

		// 		if ( !_audio_buffer.total ) {
		// 			if ( _status == kPlaying_PlayerStatus || _status == kStart_PlayerStatus ) {
		// 				_audio_buffer = _audio->receive_frame();
		// 			}
		// 		}

		// 		if (_audio_buffer.total) {
		// 			if (_uninterrupted_play_start_systime) {
		// 				if (_audio_buffer.time) {
		// 					int64_t st = (sys_time - _uninterrupted_play_start_systime) -     // sys
		// 										(_audio_buffer.time - _uninterrupted_play_start_time); // frame
		// 					int delay = _audio->stream().average_framerate * compensate;

		// 					if (st >= delay) { // 是否达到播放声音时间。输入pcm到能听到声音会有一些延时,这里设置补偿
		// 						write_audio_pcm();
		// 					}
		// 				} else { // 演示时间为0表示开始或即时渲染(如视频电话)
		// 					write_audio_pcm();
		// 				}
		// 			}
		// 		}
		// 		_audio->send_packet_for(_source->audio_extractor());
		// 	}

		// 	int frame_interval = 1000.0 / 120.0 * 1000; // 120fsp
		// 	int64_t sleep_st = frame_interval - time_monotonic() + sys_time;
		// 	if ( sleep_st > 0 ) {
		// 		thread_sleep(sleep_st);
		// 	}

		// 	goto loop;
		}

		void trigger(const UIEventName& name, const Object& data = Object()) {
			Sp<UIEvent> evt = new UIEvent(this, data);
			View::trigger(name, **evt);
		}

		bool stop_from(Lock& lock, bool is_event) {
			if ( _status == kStop_PlayerStatus )
				return false;

			_status = kStop_PlayerStatus;
			_uninterrupted_play_start_systime = 0;
			_uninterrupted_play_start_time = 0;
			_prev_presentation_time = 0;
			_time = 0;

			if ( _audio ) {
				//_audio->release(_audio_buffer);
				_audio->close();
			}
			if ( _video ) {
				//_video->release(_video_buffer);
				_video->close();
				// SourceHold::set_source(nullptr);
				ImageSourceHold::set_source(nullptr); // TODO ...
			}

			if (_pcm) {
				_pcm->flush();
			}

			preRender().untask(this);
			_source->stop();

			//auto loop_id = _run_loop_id;
			lock.unlock();
			//thread_join_for(loop_id); // wait audio thread end

			if ( is_event ) {
				_loop->post(Cb([this](auto e){
					trigger(UIEvent_Stop); /* trigger stop event */
				}));
			}
			//lock.lock();

			return true;
		}

		void stop_and_release(Lock& lock, bool is_event) {
			stop_from(lock, is_event);

			Release(_audio); _audio = nullptr;
			Release(_video); _video = nullptr;
			Release(_source); _source = nullptr;
			object_traits<PCMPlayer>::
			release(_pcm); _pcm = nullptr;

			_time = 0;
			_duration = 0;
		}

		void start_run() {
			Lock lock(_mutex);

			Qk_Assert( _source && _video);
			Qk_Assert( _source->is_open());
			Qk_Assert_Eq( _status, kStart_PlayerStatus);

			_waiting_buffer = false;

			if ( _video->open() ) { // clear video
				_source->seek(0);
				//_video->release( _video_buffer );
				_video->flush();
			} else {
				stop_from(lock, true);
				Qk_ERR("Unable to open video decoder");
				return;
			}

			if ( _pcm && _audio && _audio->open() ) {  // clear audio
				//_audio->release( _audio_buffer );
				_audio->flush();
				_pcm->flush();
				_pcm->set_volume(_volume);
				_pcm->set_mute(_mute);

				//_run_loop_id = thread_new([](auto t, void* self) { // new thread
				//	static_cast<Inl*>(self)->run_play_audio_loop();
				//}, this, "Video::audio_loop");
			}

			preRender().addtask(this);
		}

		bool is_active() {
			return _status == kPaused_PlayerStatus || _status == kPlaying_PlayerStatus;
		}
	};

	Video::Video()
		: Image()
		, _source(nullptr)
		, _audio(nullptr), _video(nullptr)
		, _pcm(nullptr)
		, _loop(current_loop())
		, _status(kStop_PlayerStatus)
		, _audio_buffer(), _video_buffer()
		, _time(0), _duration(0)
		, _uninterrupted_play_start_time(0)
		, _uninterrupted_play_start_systime(0)
		, _prev_presentation_time(0)
		, _prev_run_task_systime(0)
		, _volume(100)
		, _auto_play(true)
		, _mute(false)
		, _waiting_buffer(false)
	{
		Qk_Assert(_loop);
	}

	Video::~Video() {
		Lock lock(_mutex);
		_this->stop_and_release(lock, false);
	}

	void Video::media_source_eof(MediaSource* so) {
		_this->trigger(UIEvent_SourceEnd); // trigger event eof
	}

	void Video::media_source_error(MediaSource* so, cError& err) {
		_this->trigger(UIEvent_Error, err); // trigger event error
		stop();
	}

	String Video::src() {
		if ( _source ) {
			return _source->uri().href();
		} else {
			return String();
		}
	}

	void Video::media_source_open(MediaSource* src) {
		Qk_Assert_Eq(_source, src);

		if ( _video ) {
			// _this->trigger(UIEvent_Ready); // trigger event open
			// if ( _status == kStart_PlayerStatus ) {
			// 	_this->start_run();
			// }
			return;
		}

		auto video = MediaCodec::create(kVideo_MediaType, _source);
		auto audio = MediaCodec::create(kAudio_MediaType, _source);
		if (!audio)
			Qk_ERR("Unable to create audio decoder, %s", *src->uri().href());
		auto pcm = audio && !_pcm ?PCMPlayer::create(audio->stream()): _pcm;
		_mutex.lock();
		Qk_Assert_Eq(_video, nullptr);
		Qk_Assert_Eq(_audio, nullptr);
		_video = video;
		_audio = audio;
		_pcm = pcm;
		_mutex.unlock();

		if (_video) {
			{ //
				ScopeLock scope(_mutex);
				const AvStream& info = _video->stream();
				_duration      = _source->duration();
				_video->set_threads(2);
			}
			_this->trigger(UIEvent_Ready); // trigger event open

			if ( _status == kStart_PlayerStatus ) {
				_this->start_run();
			} else {
				if ( _auto_play ) {
					start();
				}
			}
		} else {
			Error e(ERR_VIDEO_NEW_CODEC_FAIL, "Unable to create video decoder");
			Qk_ERR("%s", *e.message());
			// _this->trigger(UIEvent_Error, e); // trigger event error
			// stop();
		}
	}

	void Video::set_src(String value, bool isRt) {
		if ( value.isEmpty() ) return;

		String src = fs_reader()->format(value);
		Lock lock(_mutex);

		if ( _source ) {
			if ( _source->uri().href() == src ) {
				return;
			}
			_this->stop_and_release(lock, true);
		}
		_source = new MediaSource(src);
		_source->set_delegate(this);
		_source->open();
	}

	void Video::start() {
		Lock scope(_mutex);

		if ( _status == kStop_PlayerStatus && _source ) {
			_status = kStart_PlayerStatus;
			_uninterrupted_play_start_systime = 0;
			_uninterrupted_play_start_time = 0;
			_prev_presentation_time = 0;
			_time = 0;
			_source->open();

			if ( _video ) {
				if ( _source->is_open() ) {
				//	scope.unlock();
				//	_this->start_run();
				}
			}
		}
	}

	void Video::stop() {
		Lock lock(_mutex);
		if ( _this->stop_from(lock, true) ) {
			mark(kLayout_None, false);
		}
	}

	bool Video::seek(uint64_t timeUs) {
		ScopeLock scope(_mutex);

		if ( _this->is_active() && timeUs < _duration ) {
			Qk_Assert( _source );

			if ( _source->seek(timeUs) ) {
				_uninterrupted_play_start_systime = 0;
				_time = timeUs;
				{ // clear video
					//_video->release( _video_buffer );
					_video->flush();
				}
				if ( _audio ) { // clear audio
					//_audio->release( _audio_buffer );
					_audio->flush();
				}
				if ( _pcm ) {
					_pcm->flush();
				}
				_loop->post(Cb([this](Cb::Data& e) {
					_this->trigger(UIEvent_Seek, Uint64(_time)); /* trigger seek event */
				}));
				return true;
			}
		}
		return false;
	}

	/**
	 * @method pause play
	 */
	void Video::pause() {
		ScopeLock scope(_mutex);
		if ( _status == kPlaying_PlayerStatus && _duration /* 没有长度信息不能暂停*/ ) {
			_status = kPaused_PlayerStatus;
			_uninterrupted_play_start_systime = 0;
			_loop->post(Cb([this](Cb::Data& e) {
				_this->trigger(UIEvent_Pause); // trigger pause event
			}));
		}
	}

	/**
	 * @method resume play
	 */
	void Video::resume() {
		ScopeLock scope(_mutex);
		if ( _status == kPaused_PlayerStatus ) {
			_status = kPlaying_PlayerStatus;
			_uninterrupted_play_start_systime = 0;
			_loop->post(Cb([this](auto e) {
				_this->trigger(UIEvent_Resume); // trigger resume event
			}));
		}
	}

	void Video::set_mute(bool value) {
		//ScopeLock scope(_mutex);
		if ( value != _mute ) {
			_mute = value;
			if ( _pcm ) { // action
				_pcm->set_mute(_mute);
			}
		}
	}

	void Video::set_volume(uint32_t value) {
		//ScopeLock scope(_mutex);
		value = Qk_MIN(value, 100);
		_volume = value;
		if ( _pcm ) {
			_pcm->set_volume(value);
		}
	}

	uint64_t Video::time() {
		//ScopeLock scope(_mutex);
		return _time;
	}

	uint64_t Video::duration() {
		//ScopeLock scope(_mutex);
		return _duration;
	}

	uint32_t Video::audio_stream_count() {
		//ScopeLock lock(_mutex);
		if ( _audio ) {
			//return _audio->extractor()->streams().length();
		}
		return 0;
	}

	const AvStream* Video::audio_stream() {
		//ScopeLock lock(_mutex);
		if ( _audio ) {
			//return &_audio->extractor()->stream();
		}
		return nullptr;
	}

	const AvStream* Video::video_stream() {
		//ScopeLock lock(_mutex);
		if ( _video ) {
			//return &_video->extractor()->stream();
		}
		return nullptr;
	}

	void Video::switch_audio_stream(uint32_t index) {
		//ScopeLock scope(_mutex);
		//if ( _audio && index < _audio->extractor()->streams().length() ) {
		//	_audio->extractor()->switch_stream(index);
		//}
	}

	MediaSourceStatus Video::source_status() {
		//ScopeLock lock(_mutex);
		if ( _source ) {
			return _source->status();
		}
		return kUninitialized_MediaSourceStatus;
	}

	PlayerStatus Video::status() {
		//ScopeLock lock(_mutex);
		return _status;
	}

	bool Video::run_task(int64_t sys_time) {
		// video
		bool draw = _this->advance_video(sys_time);
		// Qk_DEBUG("------------------------ frame: %llu", sys_time_monotonic() - sys_time);
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

		//return draw && view_depth();
		return draw && level();
	}

	void Video::set_auto_play(bool value) {
		// ScopeLock scope(_mutex);
		_auto_play = value;
	}

	void Video::onActivate() {
		if (level() == 0) { // remove
			Lock lock(_mutex);
			if (_audio)
				_this->stop_and_release(lock, true);
		}
	}

	ViewType Video::viewType() const {
		return kVideo_ViewType;
	}

}
