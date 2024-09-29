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

extern "C" {
	#include <libavutil/imgutils.h>
	#include <libavutil/samplefmt.h>
	#include <libavutil/timestamp.h>
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	//#include <libavcodec/fft.h>
}
#include <quark/util/util.h>
#include <quark/util/fs.h>
#include <quark/media/media.h>
#include <quark/media/pcm_player.h>
#include <quark/ui/app.h>
#include <quark/ui/view/view.h>
#include <quark/ui/window.h>
#include <quark/ui/screen.h>
#include <quark/ui/view/root.h>
#include <quark/ui/view/image.h>
#include <quark/ui/view/free.h>
#include <quark/render/render.h>
#include <quark/render/canvas.h>

using namespace qk;

typedef MediaSource::Extractor Extractor;
typedef MediaCodec::Frame Frame;

class TestMedia: public Image, public MediaSource::Delegate, public PreRender::Task {
public:
	~TestMedia() {
		delete _frame_v;
		delete _frame_a;
	}
	void media_source_open(MediaSource* source) override {
		UILock lock(window());
		Qk_DEBUG("media_source_open");
		_video = MediaCodec::create(kVideo_MediaType, source);
		_video->set_threads(2);
		Qk_Assert(_video->open());

		_audio = MediaCodec::create(kAudio_MediaType, source);
		if (_audio) {
			_pcm = PCMPlayer::create(_audio->stream());
			if (_pcm) {
				Qk_Assert_Eq(true, _audio->open());
			} else {
				_audio.release();
			}
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

		_audio->send_packet_for(_src->audio_extractor());

		if (!_frame_a) {
			_audio->receive_frame(&_frame_a);
		}
		if (!_frame_a) {
			return;
		}
		if (!_play_ts) {
			_play_ts = time_monotonic();
		}

		if (_frame_a->pts) {
			auto pts = _frame_a->pts - (_pcm->delay() * _frame_a->pkt_duration);
			if (time_monotonic() - _play_ts < pts)
				return;
		}
		if (!_pcm->write(_frame_a)) {
			Qk_DEBUG("PCM_write fail %d, %d", _frame_a->pts, time_monotonic() - _play_ts);
		}
		delete _frame_a; _frame_a = nullptr;
	}
	bool run_task(int64_t now) override {
		if (!_video) {
			return false;
		}
		_video->send_packet_for(_src->video_extractor());

		if (!_frame_v) {
			_video->receive_frame(&_frame_v);
		}
		if (!_frame_v) {
			return false;
		}
		if (!_play_ts) {
			_play_ts = now;
		}
		if (_frame_v->pts) {
			int64_t pts = now - _play_ts;
			if (pts < _frame_v->pts)
				return false;
			int64_t dts = pts - _frame_v->pts;
			if (dts > _frame_v->pkt_duration * 2) { // decoding timeout
				Qk_DEBUG("pkt_duration, timeout %d", dts - _frame_v->pkt_duration);
				_play_ts += (dts - _frame_v->pkt_duration); // correct play ts
			}
		}
		auto src = source();
		if (!src || !(src->state() & ImageSource::kSTATE_LOAD_COMPLETE)) {
			mark_layout(kLayout_Size_Width | kLayout_Size_Height, true);
		}
		set_source(ImageSource::Make(
			MediaCodec::frameToPixel(_frame_v), window()->render(), window()->loop()
		));
		return true;
	}
	void open(cString &uri) {
		UILock lock(window());
		_src = new MediaSource(uri);
		_src->set_delegate(this);
		_src->open();
	}
private:
	Sp<MediaSource> _src;
	Sp<MediaCodec> _video, _audio;
	Sp<PCMPlayer>  _pcm;
	Frame         *_frame_a = 0;
	Frame         *_frame_v = 0;
	int64_t        _play_ts = 0;
};

int test_media(int argc, char **argv) {
	App app;
	auto win = Window::Make({.fps=0x0, .frame={{0,0}, {400,400}}});
	win->activate();
	win->root()->set_background_color({0,0,0,1});
	auto f = win->root()->append_new<Free>();
	f->set_width({ 0, BoxSizeKind::Match });
	f->set_height({ 0, BoxSizeKind::Match });
	auto t = f->append_new<TestMedia>();
	t->set_width({ 0, BoxSizeKind::Match });
	t->set_align(Align::CenterMiddle);

	//t->open("/Users/louis/Movies/flame-piper.2016.1080p.bluray.x264.mkv");
	//t->open("/Users/louis/Movies/e7bb722c-3f66-11ee-ab2c-aad3d399777e-v8_f2_t1_maSNnEvY.mp4");
	//t->open("/Users/louis/Movies/申冤人/The.Equalizer.3.2023.2160p.WEB.H265-HUZZAH[TGx]/the.equalizer.3.2023.2160p.web.h265-huzzah.mkv");
	t->open("/Users/louis/Movies/[电影天堂www.dytt89.com]记忆-2022_HD中英双字.mp4/[电影天堂www.dytt89.com]记忆-2022_HD中英双字.mp4");
	//t->open("/Users/louis/Movies/[电影天堂www.dytt89.com]多哥BD中英双字.mp4/[电影天堂www.dytt89.com]多哥BD中英双字.mp4");
	//t->open("/Users/louis/Movies/[www.domp4.cc]神迹.2004.HD1080p.中文字幕.mp4/[www.domp4.cc]神迹.2004.HD1080p.中文字幕.mp4");

	app.run();
}
