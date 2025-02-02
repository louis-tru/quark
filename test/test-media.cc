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
#include <quark/ui/view/video.h>
#include <quark/ui/view/free.h>
#include <quark/render/render.h>
#include <quark/render/canvas.h>

using namespace qk;

typedef MediaSource::Extractor Extractor;
typedef MediaCodec::Frame Frame;

class AudioPlayerTest: public Object, public MediaSource::Delegate {
public:
	Qk_DEFINE_PROP_GET(int64_t, pts);

	~AudioPlayerTest() override {
		Qk_DLog("~AudioPlayerTest()");
		_src = nullptr;
	}
	void media_source_open(MediaSource* src) override {
		Qk_DLog("media_source_open");
		_audio = MediaCodec::create(kAudio_MediaType, src);
		_pcm = PCMPlayer::create(_audio->stream());
		if (!_audio || !_pcm || !_audio->open()) {
			Qk_Warn("open audio codecer or pcm player fail");
			stop();
		}
	}
	void media_source_eof(MediaSource* src) override {
		Qk_DLog("media_source_eof");
		do {
			media_source_advance(src);
			thread_sleep(1e4); // 10 milliseconds
		} while (_audio && !_audio->finished());
		stop();
	}
	void media_source_error(MediaSource* src, cError& err) override {
		Qk_DLog("media_source_error");
		stop();
	}
	void media_source_switch(MediaSource* src, Extractor *ex) override {
		Qk_DLog("media_source_switch");
	}
	void media_source_advance(MediaSource* src) override {
		if (_seek) {
			if (src->seek(_seek)) {
				if (_audio) {
					_audio->flush();
					_pcm->flush();
				}
				_fa = nullptr;
				_start = 0; // reset start point
				_seeking = _seek;
			}
			_seek = 0;
		}
		auto now = time_monotonic();

		if (!_audio) {
			return;
		}
		auto& stream = src->audio()->stream();
		if (_audio->stream() != stream) {  // switch stream
			_audio->close();
			if (!_audio->open(&stream) || !(_pcm = PCMPlayer::create(stream))) {
				_audio = nullptr;
				_pcm = nullptr;
				return;
			}
		}
		_audio->send_packet(src->audio());

		if (!_fa) {
			_fa = _audio->receive_frame();
		}
		if (!_fa) {
			return;
		}
		if (!_start) {
			_start = now - (_seeking ? _seeking: _fa->pts);
		}
		if (_fa->pts) {
			auto play = now - _start;
			auto pts = _fa->pts;
			if (pts > play) return;
			int64_t du = play - pts;
			if (du > _fa->pkt_duration * 2 * _pcm->delay()) { // timeout, reset start point
				if (_seeking)
					return skip_af();
				Qk_DLog("pkt_duration, timeout %d", du);
				_start = now - pts; // correct play ts
			}
		}
		_seeking = 0;
		_pts = _fa->pts; // set current the presentation timestamp
		if (!_pcm->write(*_fa)) {
			Qk_DLog("PCM_write fail %ld, %ld", _fa->pts, now - _start);
		}
		_fa = nullptr;
	}
	void set_src(cString &uri) {
		stop();
		_src = new MediaSource(uri);
		_src->set_delegate(this);
	}
	void play() {
		if (_src)
			_src->play();
	}
	void stop() {
		if (_src)
			_src->stop();
		_audio = nullptr;
		_pcm = nullptr;
	}
	void seek(uint64_t timeUs) {
		_seek = Qk_Max(timeUs, 1);
	}

private:
	void skip_af() {
		do { // skip expired a frame
			_fa = nullptr;
			if (_audio->send_packet(_src->audio()) == 0)
				_fa = _audio->receive_frame();
		} while(_fa && _fa->pts < time_monotonic() - _start);
		_fa = nullptr; // delete v frame
	}
	Sp<MediaSource> _src;
	Sp<MediaCodec> _audio;
	Sp<PCMPlayer>  _pcm;
	Sp<Frame>      _fa;
	int64_t        _start = 0;
	int64_t        _seeking = 0;
	int64_t        _seek = 0;
};

class VideoPlayer: public Image, public MediaSource::Delegate, public PreRender::Task {
public:
	Qk_DEFINE_PROP_GET(int64_t, pts);

	~VideoPlayer() override {
		Qk_DLog("~VideoPlayer()");
		_src = nullptr;
	}
	void media_source_open(MediaSource* src) override {
		Qk_DLog("media_source_open");
		UILock lock(window());
		_video = MediaCodec::create(kVideo_MediaType, src);
		if (!_video && (_video->set_threads(2), !_video->open())) {
			Qk_Warn("open video codecer fail");
			_video = nullptr;
			src->stop();
			return;
		}
		_audio = MediaCodec::create(kAudio_MediaType, src);
		if (_audio && _audio->open()) {
			_pcm = PCMPlayer::create(_audio->stream());
		}
		if (!_pcm) {
			Qk_Warn("open audio codecer or pcm player fail");
			_audio = nullptr;
			src->remove_extractor(kAudio_MediaType);
		}
		preRender().addtask(this);
	}
	void media_source_eof(MediaSource* src) override {
		Qk_DLog("media_source_eof");
		do {
			media_source_advance(src);
			thread_sleep(1e4); // 10 milliseconds
		} while (_video && !_video->finished());
		stop();
	}
	void media_source_error(MediaSource* src, cError& err) override {
		Qk_DLog("media_source_error");
		stop();
	}
	void media_source_switch(MediaSource* src, Extractor *ex) override {
		Qk_DLog("media_source_switch");
		if (_video && ex->type() == kVideo_MediaType) {
				_video->close();
				if (!_video->open(&ex->stream()))
					stop();
		}
	}
	void media_source_advance(MediaSource* src) override {
		if (_seek) {
			if (src->seek(_seek)) {
				if (_video) {
					_video->flush();
				}
				if (_audio) {
					_audio->flush();
					_pcm->flush();
				}
				UILock lock(window());
				_fv = _fa = nullptr;
				_start = 0; // reset start point
				_seeking = _seek;
			}
			_seek = 0;
		}
		auto now = time_monotonic();

		if (!_audio) {
			return;
		}
		auto& stream = src->audio()->stream();
		if (_audio->stream() != stream) {  // switch stream
			_audio->close();
			UILock lock(window());
			if (!_audio->open(&stream) || !(_pcm = PCMPlayer::create(stream))) {
				_audio = nullptr;
				_pcm = nullptr;
				return;
			}
		}
		_audio->send_packet(src->audio());

		if (!_fa) {
			_fa = _audio->receive_frame();
		}
		if (!_fa) {
			return;
		}
		if (_fa->pts) {
			if (!_start) return;
			auto play = now - _start;
			auto pts = _fa->pts - (_fa->pkt_duration * _pcm->delay()); // after pcm delay pts
			if (pts > play) return;
			int64_t du = play - pts;
			if (du > _fa->pkt_duration << 1) { // decoding timeout, discard frame
				_fa = nullptr;
				return;
			}
		}
		if (!_pcm->write(*_fa)) {
			Qk_DLog("PCM_write fail %ld, %ld", _fa->pts, now - _start);
		}
		_fa = nullptr;
	}
	bool run_task(int64_t now) override {
		if (!_video) {
			return false;
		}
		_video->send_packet(_src->video());

		if (!_fv) {
			_fv = _video->receive_frame();
		}
		if (!_fv) {
			return false;
		}
		if (!_start) {
			_start = now - (_seeking ? _seeking: _fv->pts);
		}
		if (_fv->pts) {
			auto play = now - _start;
			auto pts = _fv->pts;
			if (pts > play) return false;
			int64_t du = play - pts;
			if (du > _fv->pkt_duration << 1) {
				// decoding timeout, discard frame or reset start point
				if (_seeking)
					return skip_vf(), false;
				Qk_DLog("pkt_duration, timeout %d", du);
				_start = now - pts; // correct play ts
			}
		}
		auto src = source();
		if (!src || !(src->state() & ImageSource::kSTATE_LOAD_COMPLETE)) {
			mark_layout(kLayout_Size_Width | kLayout_Size_Height, true);
		}
		_seeking = 0;
		_pts = _fv->pts; // set current the presentation timestamp
		set_source(ImageSource::Make(
			MediaCodec::frameToPixel(*_fv), window()->render(), window()->loop()
		));
		_fv.collapse();
		return true;
	}
	void set_src(cString &uri) {
		stop();
		_src = new MediaSource(uri);
		_src->set_delegate(this);
	}
	void play() {
		if (_src)
			_src->play();
	}
	void stop() {
		UILock lock(window());
		if (_src)
			_src->stop();
		_video = nullptr;
		_audio = nullptr;
		_pcm = nullptr;
		auto imgsrc = source();
		if (imgsrc)
			imgsrc->unload(); // unload, resource
		preRender().untask(this);
	}
	void seek(uint64_t timeUs) {
		_seek = Qk_Max(timeUs, 1);
	}

private:
	void skip_vf() {
		do { // skip expired v frame
			_fv = nullptr;
			if (_video->send_packet(_src->video()) == 0)
				_fv = _video->receive_frame();
		} while(_fv && _fv->pts < time_monotonic() - _start);
		_fv = nullptr; // delete v frame
	}

	Sp<MediaSource> _src;
	Sp<MediaCodec> _video, _audio;
	Sp<PCMPlayer>  _pcm;
	Sp<Frame> _fv, _fa;
	int64_t  _start = 0;
	int64_t  _seeking = 0;
	int64_t  _seek = 0;
};

int test_media(int argc, char **argv) {
	App app;
	auto win = Window::Make({.fps=0x0, .frame={{0,0}, {400,400}}});
	win->activate();
	win->root()->set_background_color({0,0,0,1});
	auto f = win->root()->append_new<Free>();
	f->set_width({ 0, BoxSizeKind::Match });
	f->set_height({ 0, BoxSizeKind::Match });
	auto v = f->append_new<Video>();
	v->set_width({ 0, BoxSizeKind::Match });
	v->set_align(Align::CenterMiddle);

	//v->set_src("/Users/louis/Movies/flame-piper.2016.1080p.bluray.x264.mkv");
	//v->set_src("/Users/louis/Movies/e7bb722c-3f66-11ee-ab2c-aad3d399777e-v8_f2_t1_maSNnEvY.mp4");
	//v->set_src("/Users/louis/Movies/申冤人/The.Equalizer.3.2023.2160p.WEB.H265-HUZZAH[TGx]/the.equalizer.3.2023.2160p.web.h265-huzzah.mkv");
	v->set_src("/Users/louis/Movies/[电影天堂www.dytt89.com]记忆-2022_HD中英双字.mp4/[电影天堂www.dytt89.com]记忆-2022_HD中英双字.mp4");
	//v->set_src("/Users/louis/Movies/[电影天堂www.dytt89.com]多哥BD中英双字.mp4/[电影天堂www.dytt89.com]多哥BD中英双字.mp4");
	//v->set_src("/Users/louis/Movies/[www.domp4.cc]神迹.2004.HD1080p.中文字幕.mp4/[www.domp4.cc]神迹.2004.HD1080p.中文字幕.mp4");
	//v->set_src("/Users/louis/Movies/巡回检察组/巡回检察组.2020.EP01-43.HD1080P.X264.AAC.Mandarin.CHS.BDE4/巡回检察组.2020.EP03.HD1080P.X264.AAC.Mandarin.CHS.BDE4.mp4");

	//auto a = AudioPlayer::Make();
	//a.set_src("/Users/louis/Movies/[电影天堂www.dytt89.com]多哥BD中英双字.mp4/[电影天堂www.dytt89.com]多哥BD中英双字.mp4");
	//a->set_src("/Users/louis/Movies/flame-piper.2016.1080p.bluray.x264.mkv");
	//a.seek(1e6*250);
	//a->play();

	v->seek(1e6*200);
	v->play();

	app.loop()->timer(Cb([v](auto e){
		//v->seek(1e6*9);
	}), 2e6);

	app.run();
	
	return 0;
}
