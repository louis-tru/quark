
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
	#define CACHE_DATA_TIME_SECOND 10

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
				str.index = uint32_t(start);
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
					str.bitrate = String(entry->value).toNumber<uint32_t>();
				}

				str.avg_framerate[0] = stream->avg_frame_rate.num;
				str.avg_framerate[1] = stream->avg_frame_rate.den;
				str.time_base[0] = stream->time_base.num;
				str.time_base[1] = stream->time_base.den;

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
		, _status(kUninitialized_MediaSourceStatus)
		, _delegate(&default_media_source_delegate)
		, _program_idx(0)
		, _duration(0), _seek(0), _packets(0)
		, _fmt_ctx(nullptr)
		, _uri(fs_reader()->format(uri))
		, _video_ex(nullptr), _audio_ex(nullptr)
	{
		/* register all formats and codecs */
		av_register_all();
		avformat_network_init();
	}

	Inl::~Inl() {
		ScopeLock scope(_mutex);
		thread_abort(); // abort thread
		for (auto& i : _extractors ) {
			i.value->release();
		}
		_video_ex = _audio_ex = nullptr;
	}

	void Inl::set_delegate(Delegate* delegate) {
		ScopeLock scope(_mutex);
		_delegate = delegate ? delegate : &default_media_source_delegate;
	}

	void Inl::trigger_error(cError& e) {
#if DEBUG
		Qk_ERR(e);
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
		ScopeLock scope(_mutex);
		Extractor *ex = nullptr;

		if ( _status == kOpen_MediaSourceStatus ) {
			if ( !_extractors.get(type, ex) ) {
				Array<Stream> streams;

				for (auto &i: _programs[_program_idx].streams) {
					if (i.type == type)
						streams.push(i);
				}
				if ( streams.length() ) {
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

	bool Inl::seek(uint64_t timeUs) {
		if ( _status == kOpen_MediaSourceStatus && timeUs < _duration ) {
			_seek = Qk_MAX(1, timeUs);
			return true;
		}
		return false;
	}

	bool Inl::switch_program(uint32_t index) {
		ScopeLock scope(_mutex);
		return Uint32::min(_programs.length() - 1, index) == _program_idx ?
			false: (switch_program_for(index), true);
	}

	void Inl::stop() {
		ScopeLock scope(_mutex);
		thread_abort(); // abort thread
		_status = kUninitialized_MediaSourceStatus;

		for ( auto& i : _extractors ) {
			i.value->flush();
		}
	}

	#define Abort_fferr(err, msg, ...) trigger_fferr(err, msg, ##__VA_ARGS__); return

	void Inl::open() {
		if (_status == kOpening_MediaSourceStatus || 
				_status == kOpen_MediaSourceStatus ) {
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
				Qk_DEBUG("Free ffmpeg AVFormatContext");
			});

			/* retrieve stream information */
			r = avformat_find_stream_info(fmt_ctx, nullptr);
			if ( r < 0 ) {
				Abort_fferr(ERR_MEDIA_INVALID_SOURCE, "cannot find stream information: `%s`", *url);
			}

#if DEBUG
			av_dump_format(fmt_ctx, 0, *url, 0); // print info
#endif
			_mutex.lock();
			_programs = get_programs(fmt_ctx);
			_duration = fmt_ctx->duration > 0 ? fmt_ctx->duration : 0;
			_status = kOpen_MediaSourceStatus;
			_fmt_ctx = fmt_ctx;
			switch_program_for(Uint32::min(_programs.length() - 1, _program_idx));
			_mutex.unlock();
			_delegate->media_source_open(_host);
			Qk_Assert_Ne(_extractors.length(), 0, "No Extractors on MediaSource");
			if (_extractors.length())
				read_stream(t, fmt_ctx, url);
			_mutex.lock();
			_fmt_ctx = nullptr;
			_mutex.unlock();
		}, "MediaSource::open,read_stream");
	}

	// -------------------------------------------------------------------------------

	void Inl::read_stream(cThread* t, AVFormatContext* fmt_ctx, cString& url) {
		AVPacket pkt;
		av_init_packet(&pkt);
		int64_t st = 0;
		Extractor *ex = nullptr;
		int rc;

		fmt_ctx->flags |= AVFMT_FLAG_NONBLOCK; // nonblock

		do {
			if (t->abort) {
				Qk_DEBUG("read_frame() abort break;"); break;
			}
			// SEEK check
			if (_seek) {
				int stream = av_find_default_stream_index(fmt_ctx);

				auto time_base = fmt_ctx->streams[stream]->time_base;
				auto time = fmt_ctx->streams[stream]->start_time +
										av_rescale(_seek / 1000000.0, time_base.den, time_base.num);

				if ( av_seek_frame(fmt_ctx, stream, time, AVSEEK_FLAG_BACKWARD) >= 0 ) {
					// TODO ... clear Extractor cache sample data
					rc = EAGAIN; // modify AVERROR_EOF
					av_packet_unref(&pkt);
				}
				_seek = 0;
			}

			if (rc != AVERROR_EOF && pkt.buf == nullptr) {
				rc = av_read_frame(fmt_ctx, &pkt); // read next frame
			}

			if (rc == AVERROR_EOF) {
				if (_packets == 0) {
					Qk_DEBUG("read_frame() eof break;");
					_status = kEOF_MediaSourceStatus;
					_delegate->media_source_eof(_host);
					break;
				}
			} else if (rc == 0) {
				if (packet_push(pkt)) {
					_delegate->media_source_advance(_host);
					continue;
				}
			} else if (rc != EAGAIN) {
				Qk_DEBUG("read_frame() error break;");
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

	void Inl::switch_program_for(uint32_t index) {
		for ( auto& i: _extractors ) {
			auto ex = i.value;
			Array<Stream> streams;

			for (auto &s: _programs[index].streams) {
				if (s.type == ex->type())
					streams.push(s); // copy
			}
			if (streams.length()) {
				ex->flush();
				ex->_streams = std::move(streams); // update streams on extractor
				ex->_stream_index = Uint32::min(ex->_stream_index, ex->_streams.length() - 1);
				// notice decoder change..
			} else {
				Qk_LOG("No streams object the program of switch_program to %d, keep last", index);
			}
		}
		_program_idx = index;

		if (_fmt_ctx) {
			for (int i = 0; i < _fmt_ctx->nb_programs; i++) {
				_fmt_ctx->programs[i]->discard = index == i ? AVDISCARD_NONE: AVDISCARD_ALL;
			}
		}
	}

	bool Inl::switch_stream(Extractor *ex, uint32_t index) {
		ScopeLock lock(_mutex);
		index = Uint32::min(index, ex->_streams.length() - 1);
		if ( ex->_stream_index != index ) {
			ex->flush();
			ex->_stream_index = index;
			// notice decoder change..
			return true;
		} else {
			return false;
		}
	}

	bool Inl::packet_push(AVPacket& avpkt) {
		Qk_Assert_Ne(avpkt.size, 0, "Inl::packet_push(), avpkt.size == 0");
		ScopeLock lock(_mutex);
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
		if (ex->_packets.length() > 1024) {
			return false;
		}
		if (ex->_packets_duration > 1e7/*10 second*/) { // cache packet data 10 second
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
		pkt->avpkt = reinterpret_cast<AVPacket*>(pkt + 1);
		*pkt->avpkt = avpkt; // copy
		av_init_packet(&avpkt);
		_packets++;
		ex->_packets_duration += pkt->duration;
		ex->_packets.pushBack(pkt);
		return true;
	}

	MediaSource::Packet* Inl::advance(Extractor* ex) {
		ScopeLock scope(_mutex);
		if ( ex->_packets.length() ) {
			auto pkt = ex->_packets.front();
			ex->_packets.popFront();
			ex->_packets_duration -= pkt->duration;
			_packets--;
			return pkt;
		} else {
			return nullptr;
		}
	}

	// ------------------- MediaSource ------------------

	MediaSource::MediaSource(cString& uri): _inl(nullptr) {
		_inl = new Inl(this, uri);
	}

	MediaSource::~MediaSource() {
		delete _inl; _inl = nullptr;
	}

	void MediaSource::set_delegate(Delegate* delegate) { _inl->set_delegate(delegate); }
	const URI& MediaSource::uri() const { return _inl->_uri; }
	MediaSourceStatus MediaSource::status() const { return _inl->_status; }
	uint64_t MediaSource::duration() const { return _inl->_duration; }
	uint32_t MediaSource::programs()const{ return _inl->_programs.length();}
	bool MediaSource::is_open() const { return _inl->_status == kOpen_MediaSourceStatus; }
	Extractor* MediaSource::video_extractor() { return _inl->_video_ex; }
	Extractor* MediaSource::audio_extractor() { return _inl->_audio_ex; }
	bool MediaSource::switch_program(uint32_t index) { return _inl->switch_program(index); }
	Extractor* MediaSource::extractor(MediaType type) { return _inl->extractor(type); }
	bool MediaSource::seek(uint64_t timeUs) { return _inl->seek(timeUs); }
	void MediaSource::open() { _inl->open(); }
	void MediaSource::stop() { _inl->stop(); }
}
