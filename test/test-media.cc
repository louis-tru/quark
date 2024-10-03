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

#include <quark/util/util.h>
#include <quark/util/fs.h>
#include <quark/media/media.h>
#include <quark/media/pcm_player.h>
#include <quark/ui/app.h>
#include <quark/ui/window.h>
#include <quark/ui/view/root.h>
#include <quark/ui/view/image.h>
#include <quark/ui/view/free.h>
#include <quark/render/render.h>
#include <quark/render/canvas.h>

using namespace qk;

typedef MediaSource::Extractor Extractor;
typedef MediaCodec::Frame Frame;

class VideoPlayer: public Image, public MediaSource::Delegate, public PreRender::Task {
public:
	void media_source_open(MediaSource* source) override {
		UILock lock(window());
		Qk_DEBUG("media_source_open");
		_video = MediaCodec::create(kVideo_MediaType, source);
		_video->set_threads(2);
		Qk_Assert_Eq(true, _video->open());

		_audio = MediaCodec::create(kAudio_MediaType, source);
		if (_audio) {
			_pcm = PCMPlayer::create(_audio->stream());
			if (_pcm) {
				Qk_Assert_Eq(true, _audio->open());
			} else {
				_audio = nullptr;
			}
		}
		if (!_audio) {
			source->remove_extractor(kVideo_MediaType);
		}
		preRender().addtask(this);
	}
	void media_source_eof(MediaSource* msrc) override {
		UILock lock(window());
		Qk_DEBUG("media_source_eof");
		_video = nullptr;
		_audio = nullptr;
		_pcm = nullptr;
		auto imgsrc = source();
		if (imgsrc)
			imgsrc->unload(); // unload, resource
		preRender().untask(this);
	}
	void media_source_error(MediaSource* source, cError& err) override {
		Qk_DEBUG("media_source_error");
		media_source_eof(source);
	}
	void media_source_switch(MediaSource* source, Extractor *ex) override {
		Qk_DEBUG("media_source_switch");
	}
	void media_source_advance(MediaSource* source) override {
		if (!_audio)
			return;
		ScopeLock lock(_mutex);

		_audio->send_packet(_src->audio_extractor());

		if (!_fa) {
			_fa = _audio->receive_frame();
		}
		if (!_fa) {
			return;
		}
		auto now = time_monotonic();
		if (!_begen) {
			_begen = now - _fa->pts;
		}
		if (_fa->pts) {
			auto play = now - _begen;
			auto pts = _fa->pts - (_pcm->delay() * _fa->pkt_duration);
			if (play < pts)
				return;
		}
		if (!_pcm->write(*_fa)) {
			Qk_DEBUG("PCM_write fail %d, %d", _fa->pts, now - _begen);
		}
		_fa = nullptr;
	}
	bool run_task(int64_t now) override {
		if (!_video) {
			return false;
		}
		_video->send_packet(_src->video_extractor());

		if (!_fv) {
			_fv = _video->receive_frame();
		}
		if (!_fv) {
			return false;
		}
		if (!_begen) {
			_begen = now - (_seek ? _seek: _fv->pts);
		}
		if (_fv->pts) {
			auto play = now - _begen;
			if (play < _fv->pts) {
				return false;
			}
			int64_t dur = play - _fv->pts;
			if (dur > _fv->pkt_duration * 2) { // decoding timeout
				if (_seek) {
				 	int j = 3;
					do { // Accelerated decoding
						_fv = nullptr;
						_video->send_packet(_src->video_extractor());
						_fv = _video->receive_frame();
					} while(--j && (!_fv || _fv->pts <= play));
					return;
				}
				Qk_DEBUG("pkt_duration, timeout %d", dur - _fv->pkt_duration);
				_begen += (dur - _fv->pkt_duration); // correct play ts
			}
		}
		auto src = source();
		if (!src || !(src->state() & ImageSource::kSTATE_LOAD_COMPLETE)) {
			mark_layout(kLayout_Size_Width | kLayout_Size_Height, true);
		}
		_seek = 0;
		_pts = _fv->pts; // set current the presentation timestamp
		// Qk_DEBUG("_pts %d", _pts);
		set_source(ImageSource::Make(
			MediaCodec::frameToPixel(*_fv), window()->render(), window()->loop()
		));
		_fv.collapse();
		return true;
	}
	void open(cString &uri) {
		UILock lock(window());
		_src = new MediaSource(uri);
		_src->set_delegate(this);
		_src->open();
	}
	void seek(uint64_t timeUs) {
		if (_src->seek(timeUs)) {
			if (_video) {
				_video->flush();
			}
			if (_audio) {
				_audio->flush();
				_pcm->flush();
			}
			{
				UILock lock(window());
				ScopeLock lock2(_mutex);
				_fv = _fa = nullptr;
			}
			_begen = 0; // reset begen point
			_seek = timeUs;
		}
	}
private:
	Sp<MediaSource> _src;
	Sp<MediaCodec> _video, _audio;
	Sp<PCMPlayer>  _pcm;
	Sp<Frame>      _fv, _fa;
	int64_t        _begen = 0;
	int64_t        _seek = 0;
	int64_t        _pts = 0;
	Mutex          _mutex;
};

int test_media(int argc, char **argv) {
	App app;
	auto win = Window::Make({.fps=0x0, .frame={{0,0}, {400,400}}});
	win->activate();
	win->root()->set_background_color({0,0,0,1});
	auto f = win->root()->append_new<Free>();
	f->set_width({ 0, BoxSizeKind::Match });
	f->set_height({ 0, BoxSizeKind::Match });
	auto v = f->append_new<VideoPlayer>();
	v->set_width({ 0, BoxSizeKind::Match });
	v->set_align(Align::CenterMiddle);

	//v->open("/Users/louis/Movies/flame-piper.2016.1080p.bluray.x264.mkv");
	//v->open("/Users/louis/Movies/e7bb722c-3f66-11ee-ab2c-aad3d399777e-v8_f2_t1_maSNnEvY.mp4");
	//v->open("/Users/louis/Movies/申冤人/The.Equalizer.3.2023.2160p.WEB.H265-HUZZAH[TGx]/the.equalizer.3.2023.2160p.web.h265-huzzah.mkv");
	//v->open("/Users/louis/Movies/[电影天堂www.dytt89.com]记忆-2022_HD中英双字.mp4/[电影天堂www.dytt89.com]记忆-2022_HD中英双字.mp4");
	//v->open("/Users/louis/Movies/[电影天堂www.dytt89.com]多哥BD中英双字.mp4/[电影天堂www.dytt89.com]多哥BD中英双字.mp4");
	//v->open("/Users/louis/Movies/[www.domp4.cc]神迹.2004.HD1080p.中文字幕.mp4/[www.domp4.cc]神迹.2004.HD1080p.中文字幕.mp4");
	v->open("/Users/louis/Movies/巡回检察组/巡回检察组.2020.EP01-43.HD1080P.X264.AAC.Mandarin.CHS.BDE4/巡回检察组.2020.EP01.HD1080P.X264.AAC.Mandarin.CHS.BDE4.mp4");

	// v->seek(1e6*1000); // seek to 1000 seconds

	app.loop()->timer(Cb([v](auto e){
		v->seek(1e6*9); // seek to 8 seconds
	}), 1e6);

	app.run();
}
