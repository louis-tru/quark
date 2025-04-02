
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

#include "./media_inl.h"
#include "../util/loop.h"
#include "../errno.h"
#include "../util/fs.h"

namespace qk {
	// 10 seconds
	#define Qk_BUFFER_DURATION 1e7

	class DefaultMediaSourceDelegate: public MediaSource::Delegate {
	public:
		void media_source_open(MediaSource* source) override {}
		void media_source_eof(MediaSource* source) override {}
		void media_source_error(MediaSource* source, cError& err) override {}
		void media_source_advance(MediaSource* source) override {}
		void media_source_switch(MediaSource* source, Extractor *ex) override {}
	};

	static DefaultMediaSourceDelegate default_media_source_delegate;

	static Program get_program(AVFormatContext* fmt_ctx, int start, int count) {
		Program info          = { 0, 0, 0, };
		AVDictionaryEntry* entry = nullptr;

		for ( ; start < count; start++) {
			AVStream* stream = fmt_ctx->streams[start];
			AVCodecParameters* codecpar = stream->codecpar;
			AVMediaType type = codecpar->codec_type;

			if ( type == AVMEDIA_TYPE_VIDEO || type == AVMEDIA_TYPE_AUDIO ) {
				MediaSource::Stream str;
				str.index = start;
				str.type = (type == AVMEDIA_TYPE_VIDEO) ? kVideo_MediaType : kAudio_MediaType;
				str.mime = String::format("%s/%s", av_get_media_type_string(type),
																avcodec_get_name(codecpar->codec_id));
				str.codec_id = codecpar->codec_id;
				str.codec_tag = codecpar->codec_tag;
				str.format = codecpar->format;
				str.profile = codecpar->profile;
				str.level = codecpar->level;
				str.width = codecpar->width;
				str.height = codecpar->height;
				str.sample_rate = codecpar->sample_rate;
				//str.sample_frame = codecpar->frame_size; // audio sample rate for a frame
				str.channels = codecpar->channels;
				str.channel_layout = codecpar->channel_layout;
				str.extra.set_codecpar(codecpar);

				entry = av_dict_get(stream->metadata, "variant_language", nullptr, 0);
				if ( entry ) {
					str.language = entry->value;
				}
				entry = av_dict_get(stream->metadata, "variant_bitrate", nullptr, 0);
				if ( entry ) {
					str.bitrate = String(entry->value).toNumber<int>();
				}

				str.duration = stream->duration;
				str.avg_framerate[0] = stream->avg_frame_rate.num;
				str.avg_framerate[1] = stream->avg_frame_rate.den;
				str.time_base[0] = stream->time_base.num;
				str.time_base[1] = stream->time_base.den;

				Hash5381 hash;
				hash.updateu32(str.codec_tag);
				hash.updateu32(str.format);
				hash.updateu32(str.width);
				hash.updateu32(str.height);
				hash.updateu64(str.duration);
				hash.updateu32(str.sample_rate);
				hash.updateu32(str.channels);
				hash.updateu64(str.channel_layout);
				hash.update(codecpar->extradata, codecpar->extradata_size);

				str.hash_code = hash.hashCode();

				if ( type == AVMEDIA_TYPE_VIDEO ) {
					info.width = codecpar->width;
					info.height = codecpar->height;
				}

				info.codecs += String::format(info.codecs.isEmpty() ? "%s (%s)": ", %s (%s)",
											avcodec_get_name    (codecpar->codec_id),
											avcodec_profile_name(codecpar->codec_id, codecpar->profile)
										);
				info.streams.push(str);
			}
		}

		Qk_ReturnLocal(info);
	}

	static Array<Program> get_programs(AVFormatContext* fmt_ctx) {
		Array<Program> arr_prog;
		if (fmt_ctx->nb_programs) {
			for (int i = 0; i < fmt_ctx->nb_programs; i++) {
				auto avp = fmt_ctx->programs[i];
				auto prog = get_program(fmt_ctx,
																*avp->stream_index,
																*avp->stream_index + avp->nb_stream_indexes);
				auto entry = av_dict_get(avp->metadata, "variant_bitrate", NULL, 0);
				if ( entry ) {
					prog.bitrate = String(entry->value).toNumber<uint32_t>();
				}
				arr_prog.push(std::move(prog));
			}
		} else {
			arr_prog.push(get_program(fmt_ctx, 0, fmt_ctx->nb_streams));
		}
		Qk_ReturnLocal(arr_prog);
	}

	// ------------------- MediaSource::Inl ------------------

	Inl::Inl(MediaSource* host, cString& uri)
		: _host(host)
		, _status(kNormal_MediaSourceStatus)
		, _delegate(&default_media_source_delegate)
		, _program_idx(0)
		, _duration(0), _seek(0), _buffer_pkt_duration(Qk_BUFFER_DURATION)
		, _fmt_ctx(nullptr)
		, _uri(fs_reader()->format(uri))
		, _video_ex(nullptr), _audio_ex(nullptr), _pause(false)
	{
		/* register all formats and codecs */
		av_register_all();
		avformat_network_init();
	}

	Inl::~Inl() {
		thread_abort(); // abort thread
		auto lock = _cm.scope_lock();
		for (auto& i : _extractors ) {
			i.value->release();
		}
		_video_ex = _audio_ex = nullptr;
	}

	void Inl::trigger_error(cError& e) {
#if DEBUG
		Qk_ELog(e);
#endif
		_status = kError_MediaSourceStatus;
		_delegate->media_source_error(_host, e);
	}

	void Inl::trigger_fferr(int err, cChar *f, ...) {
		int r;
		char msg[AV_ERROR_MAX_STRING_SIZE] = { 0 };
		av_make_error_string(msg, AV_ERROR_MAX_STRING_SIZE, r);
		va_list arg;
		va_start(arg, f);
		auto msg1 = _Str::printfv(f, arg);
		va_end(arg);
		trigger_error(Error(err, "%s, msg: %s", *msg1, msg));
	}

	void Inl::thread_abort() {
		thread_try_abort(_tid);
		thread_join_for(_tid);
	}

	Extractor* Inl::extractor(MediaType type) {
		auto lock = _cm.scope_lock();
		Extractor *ex = nullptr;

		if (is_open()) {
			if ( !_extractors.get(type, ex) ) {
				Array<Stream> streams;

				for (auto &i: _programs[_program_idx].streams) {
					if (i.type == type)
						streams.push(i);
				}
				if (streams.length()) {
					ex = _extractors.set(type, new Extractor(type, _host, std::move(streams)));
					if (type == kVideo_MediaType)
						_video_ex = ex;
					else if (type == kAudio_MediaType)
						_audio_ex = ex;
				}
			}
		}
		return ex;
	}

	void Inl::remove_extractor(MediaType type) {
		auto lock = _cm.scope_lock();
		auto it = _extractors.find(type);
		if (it != _extractors.end()) {
			auto ex = it->value;
			if (ex == _video_ex) {
				_video_ex = nullptr;
			} else if (ex == _audio_ex) {
				_audio_ex = nullptr;
			}
			_extractors.erase(it);
			ex->release();
		}
	}

	void Inl::stop() {
		resume();
		thread_abort(); // abort thread
		auto lock = _cm.scope_lock();
		_status = kNormal_MediaSourceStatus;

		for (auto i : _extractors) {
			i.value->flush();
		}
	}

	bool Inl::advance_eof() {
		for (auto i : _extractors) {
			if (i.value->_pkt != i.value->_packets.end())
				return false;
		}
		return true;
	}

	bool Inl::is_open() {
		return _status == kPlaying_MediaSourceStatus || _status == kPaused_MediaSourceStatus;
	}

	void Inl::pause() {
		_pause = true;
	}

	void Inl::resume() {
		if (_pause) {
			_pause = false;
			_cm.lock_and_notify_one();
		}
	}

	void Inl::switch_program_sure(uint32_t index, bool event) {
		Lock lock(_cm.mutex);

		if (_extractors.length() == 0) {
			return;
		}

		for ( auto i: _extractors ) {
			auto ex = i.value;
			Array<Stream> streams;

			for (auto &s: _programs[index].streams) {
				if (s.type == ex->type())
					streams.push(s); // copy
			}
			if (streams.length()) {
				ex->_streams = std::move(streams); // update streams on extractor
				ex->_stream_index = Uint32::min(ex->_stream_index, ex->_streams.length() - 1);
			} else {
				Qk_Log("No streams object the program of switch_program to %d, keep last", index);
			}
		}
		_program_idx = index;

		if (_fmt_ctx) {
			for (int i = 0; i < _fmt_ctx->nb_programs; i++) {
				_fmt_ctx->programs[i]->discard = index == i ? AVDISCARD_NONE: AVDISCARD_ALL;
			}
		}

		if (event) {
			auto ex = _extractors.values();
			lock.unlock();
			for (auto i: ex) {
				_delegate->media_source_switch(_host, i);
			}
		}
	}

	bool Inl::switch_stream(Extractor *ex, uint32_t index) {
		Lock lock(_cm.mutex);
		index = Uint32::min(index, ex->_streams.length() - 1);
		if ( ex->_stream_index != index ) {
			ex->_stream_index = index;
			lock.unlock();
			_delegate->media_source_switch(_host, ex);
			return true;
		} else {
			return false;
		}
	}

	#define Abort_fferr(err, msg, ...) trigger_fferr(err, msg, ##__VA_ARGS__); return

	void Inl::play() {
		if (_status == kOpening_MediaSourceStatus || is_open()) {
			resume();
			return;
		}
		stop();
		_status = kOpening_MediaSourceStatus;

		// running new thread
		_tid = thread_new([this](auto t) {
			String url(fs_fallback_c(_uri.href()));
			AVFormatContext* fmt_ctx = nullptr;
			int r;

			/* open input file, and allocate format context */
			r = avformat_open_input(&fmt_ctx, *url, nullptr, nullptr);
			if ( r < 0 ) {
				Abort_fferr(ERR_MEDIA_INVALID_SOURCE, "cannot open source file: `%s`", *url);
			}

			CPointerHold<AVFormatContext> clear(fmt_ctx, [](AVFormatContext *fmt_ctx) {
				avformat_close_input(&fmt_ctx);
				Qk_DLog("Free ffmpeg AVFormatContext");
			});

			/* retrieve stream information */
			r = avformat_find_stream_info(fmt_ctx, nullptr);
			if ( r < 0 ) {
				Abort_fferr(ERR_MEDIA_INVALID_SOURCE, "cannot find stream information: `%s`", *url);
			}

#if DEBUG
			av_dump_format(fmt_ctx, 0, *url, 0); // print info
#endif
			_cm.lock();
			_programs = get_programs(fmt_ctx);
			_duration = fmt_ctx->duration > 0 ? fmt_ctx->duration : 0;
			_status = kPlaying_MediaSourceStatus;
			_fmt_ctx = fmt_ctx;
			_cm.unlock();
			switch_program_sure(Uint32::min(_programs.length() - 1, _program_idx), false);
			_delegate->media_source_open(_host);
			// Qk_ASSERT_NE(_extractors.length(), 0, "No Extractors on MediaSource");
			if (_extractors.length())
				read_stream(t, url);
			_cm.lock();
			_fmt_ctx = nullptr;
			_cm.unlock();
			_seek = 0;
			_pause = false;
		}, "MediaSource::open,read_stream");
	}

	// -------------------------------------------------------------------------------

	void Inl::read_stream(cThread* t, cString& url) {
		auto ctx = _fmt_ctx;
		AVPacket pkt;
		av_init_packet(&pkt);
		int64_t st = 0;
		Extractor *ex = nullptr;
		int rc;
		bool can_pause = false;

		ctx->flags |= AVFMT_FLAG_NONBLOCK; // nonblock

		do {
			if (t->abort) {
				Qk_DLog("read_frame() abort break;"); break;
			}
			if (_pause && can_pause) {
				Qk_ASSERT_EQ(0, av_read_pause(ctx));
				_status = kPaused_MediaSourceStatus;
				_cm.lock_and_wait_for();
				_status = kPlaying_MediaSourceStatus;
				Qk_ASSERT_EQ(0, av_read_play(ctx));

				if (_pause && _seek) {
					can_pause = false;
				}
			}

			// SEEK check
			if (_seek) {
				if (_seek < _duration) {
					_cm.lock();
					for (auto i : _extractors)
						i.value->flush();
					_cm.unlock();

					auto idx = _video_ex ? _video_ex->stream().index:
										_audio_ex ? _audio_ex->stream().index:
										av_find_default_stream_index(ctx);
					auto stream = ctx->streams[idx];
					auto time_base = stream->time_base;
					// auto time = stream->start_time + av_rescale(_seek / 1000000.0, time_base.den, time_base.num);
					auto time = stream->start_time + _seek * time_base.den / (time_base.num * 1000000);

					if ( av_seek_frame(ctx, idx, time, AVSEEK_FLAG_BACKWARD) >= 0 ) {
						rc = AVERROR(EAGAIN); // modify AVERROR_EOF
						av_packet_unref(&pkt);
					}
				}
				_seek = 0;
			}

			if (rc != AVERROR_EOF && pkt.buf == nullptr) {
				rc = av_read_frame(ctx, &pkt); // read next frame
			}

			if (rc == AVERROR_EOF) {
				if (advance_eof()) {
					Qk_DLog("read_frame() eof break;");
					_status = kEOF_MediaSourceStatus;
					_delegate->media_source_eof(_host);
					break;
				}
			} else if (rc == 0) {
				if (packet_push(pkt)) {
					_delegate->media_source_advance(_host);
					continue;
				} else {
					can_pause = true; // buffer is full the enable pause
				}
			} else if (rc != AVERROR(EAGAIN)) {
				Qk_DLog("read_frame() error break;");
				trigger_fferr(ERR_MEDIA_SOURCE_READ_ERROR, "Read source error `%s`", *url);
				break;
			}
			_delegate->media_source_advance(_host); // notification the call medhod extractor::advance

			auto st0 = st; st = time_monotonic();
			auto sleep = 1e4 - (st - st0);
			if (sleep > 0) {
				thread_sleep(sleep); // Sleep for up to 10 millisecond
			}
		} while (true);

		av_packet_unref(&pkt);
	}

	void Inl::flush() {
		auto lock = _cm.scope_lock();
		for (auto i: _extractors) {
			auto ex = i.value;
			if (ex->_pkt != ex->_packets.end()) {
				auto pts = (*ex->_pkt)->pts;
				if (pts) {
					_seek = pts; // seek core
				}
				break;
			}
		}
		for (auto i: _extractors) {
			i.value->flush();
		}
	}

	bool Inl::seek(uint64_t timeUs) {
		auto lock = _cm.scope_lock();
		if (is_open()) { // _status == kPlayer_MediaSourceStatus
			if (timeUs >= _duration) {
				return false;
			}
			for (auto i: _extractors) {
				if (!seek_ex(timeUs, i.value)) {
					for (auto i: _extractors) {
						i.value->flush();
					}
					_seek = Qk_Max(1, timeUs); // seek core
					_cm.cond.notify_one(); // thread resume
					break;
				}
			}
		} else {
			_seek = Qk_Max(1, timeUs); // seek core
			_cm.cond.notify_one(); // thread resume
		}
		return true;
	}

	bool Inl::seek_ex(uint64_t timeUs, Extractor *ex) {
		if (ex->_packets.length() == 0)
			return false;

		if (timeUs < ex->_packets.front()->pts)
			return false;

		auto it = ex->_packets.begin(), end = ex->_packets.end();
		auto key = end;
		do {
			auto pkt = *it;
			if (pkt->flags & AV_PKT_FLAG_KEY)
				key = it;
			if (pkt->pts >= timeUs) {
				if (key != end) {
					ex->_pkt = key;
					return true;
				}
				break;
			}
		} while (++it != end);

		return false;
	}

	bool Inl::packet_push(AVPacket& avpkt) {
		Qk_ASSERT_NE(avpkt.size, 0, "Inl::packet_push(), avpkt.size == 0");
		auto lock = _cm.scope_lock();
		AVStream *avstream = _fmt_ctx->streams[avpkt.stream_index];
		Extractor *ex;

		if (!_extractors.get(avstream->codecpar->codec_type + 1, ex)) { // MediaType => ex
			av_packet_unref(&avpkt); // discard packet
			return true;
		}
		auto &stream = ex->stream(); // current select stream
		if (stream.index != avstream->index) {
			av_packet_unref(&avpkt);
			return true; // discard packet
		}
		if (ex->_after_duration > _buffer_pkt_duration/*default 10 second*/) {
			return false;
		}
		auto unit = 1000000.0 * stream.time_base[0] / stream.time_base[1];

		auto pkt = new (::malloc(sizeof(Packet) + sizeof(AVPacket))) Packet{
			nullptr,
			avpkt.data,
			static_cast<uint32_t>(avpkt.size),
			static_cast<uint64_t>(avpkt.pts * unit),
			static_cast<uint64_t>(avpkt.dts * unit),
			static_cast<uint64_t>(avpkt.duration * unit),
			avpkt.flags,
		};
		Qk_ASSERT_NE(pkt->duration, 0);
		pkt->avpkt = reinterpret_cast<AVPacket*>(pkt + 1);
		*pkt->avpkt = avpkt; // copy
		av_init_packet(&avpkt);
		ex->_after_duration += pkt->duration;

		if (ex->_pkt == ex->_packets.end()) {
			ex->_pkt = ex->_packets.pushBack(pkt);
		} else {
			ex->_packets.pushBack(pkt);
		}
		return true;
	}

	MediaSource::Packet* Inl::advance(Extractor* ex) {
		auto lock = _cm.scope_lock();
		if (ex->_pkt == ex->_packets.end())
			return nullptr;
		auto pkt = *ex->_pkt;
		ex->_after_duration -= pkt->duration;
		ex->_before_duration += pkt->duration;

		while (ex->_before_duration > _buffer_pkt_duration) {
			if (ex->_pkt != ex->_packets.begin()) {
				auto pkt = ex->_packets.front();
				ex->_before_duration -= pkt->duration;
				ex->_packets.popFront();
				delete pkt;
			}
		}
		ex->_pkt++;
		return pkt->clone();
	}

	// ------------------- MediaSource ------------------

	MediaSource::MediaSource(cString& uri): _inl(nullptr) {
		_inl = new Inl(this, uri);
	}

	MediaSource::~MediaSource() {
		delete _inl; _inl = nullptr;
	}

	void MediaSource::set_delegate(Delegate* delegate) {
		auto lock = _inl->_cm.scope_lock();
		_inl->_delegate = delegate ? delegate : &default_media_source_delegate;
	}

	const URI& MediaSource::uri() const { return _inl->_uri; }
	MediaSourceStatus MediaSource::status() const { return _inl->_status; }
	uint64_t MediaSource::duration() const { return _inl->_duration; }
	uint32_t MediaSource::programs()const{ return _inl->_programs.length();}
	bool MediaSource::is_open() const { return _inl->is_open(); }
	bool MediaSource::is_pause() const { return _inl->_pause; }
	Extractor* MediaSource::video() { return _inl->_video_ex; }
	Extractor* MediaSource::audio() { return _inl->_audio_ex; }
	bool MediaSource::switch_program(uint32_t index) {
		return Uint32::min(_inl->_programs.length() - 1, index) == _inl->_program_idx ?
			false: (_inl->switch_program_sure(index, true), true);
	}
	Extractor* MediaSource::extractor(MediaType type) { return _inl->extractor(type); }
	void MediaSource::remove_extractor(MediaType type) { _inl->remove_extractor(type); }
	void MediaSource::flush() { _inl->flush(); }
	bool MediaSource::seek(uint64_t timeUs) { return _inl->seek(timeUs); }
	void MediaSource::play() { _inl->play(); }
	void MediaSource::stop() { _inl->stop(); }
	void MediaSource::pause() { _inl->pause(); }
	uint64_t MediaSource::buffer_pkt_duration() { return _inl->_buffer_pkt_duration; }
	void MediaSource::set_buffer_pkt_duration(uint64_t val) { _inl->_buffer_pkt_duration = val; }
}
