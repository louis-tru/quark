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

#include "media-codec-1.h"
#include "utils/loop.h"
#include "ngui/app.h"

XX_NS(ngui)

#define CACHE_DATA_TIME_SECOND 10

class DefaultMultimediaSourceDelegate: public MultimediaSource::Delegate {
 public:
	virtual void multimedia_source_ready(MultimediaSource* source) {}
	virtual void multimedia_source_wait_buffer(MultimediaSource* source, float process) {}
	virtual void multimedia_source_eof(MultimediaSource* source) {}
	virtual void multimedia_source_error(MultimediaSource* source, cError& err) {}
};

static DefaultMultimediaSourceDelegate default_multimedia_source_delegate;

// ------------------- MultimediaSource::Inl ------------------

Inl::Inl(MultimediaSource* host, cString& uri, RunLoop* loop)
: ParallelWorking(loop)
, m_host(host)
, m_status(MULTIMEDIA_SOURCE_STATUS_UNINITIALIZED)
, m_delegate(&default_multimedia_source_delegate)
, m_bit_rate_index(0)
, m_duration(0)
, m_fmt_ctx(nullptr)
, m_read_eof(false)
, m_disable_wait_buffer(false)
{
	m_uri = URI( f_reader()->format(uri) );
	/* register all formats and codecs */
	av_register_all();
	avformat_network_init();
}

/**
 * @destructor
 */
Inl::~Inl() {
	ScopeLock scope(mutex());
	for (auto& i : m_extractors ) {
		Release(i.value());
	}
	m_extractors.clear();
	reset();
}

// @func reset
void Inl::reset() {
	
	abort_child(); //
	m_status = MULTIMEDIA_SOURCE_STATUS_UNINITIALIZED;
	m_duration = 0;
	m_bit_rate_index = 0;
	m_bit_rate.clear();
	m_read_eof = 0;
	m_fmt_ctx = nullptr;
	
	for ( auto& i : m_extractors ) {
		extractor_flush(i.value());
	}
}

/**
 * @func set_delegate
 */
void Inl::set_delegate(Delegate* delegate) {
	ScopeLock scope(mutex());
	m_delegate = delegate;
}

/**
 * @func bit_rate_index
 */
uint Inl::bit_rate_index() {
	ScopeLock scope(mutex());
	return m_bit_rate_index;
}

/**
 * @func bit_rates
 */
const Array<BitRateInfo>& Inl::bit_rate() {
	ScopeLock scope(mutex());
	return m_bit_rate;
}

/**
 * @func select_bit_rate
 */
bool Inl::select_bit_rate(uint index) {
	ScopeLock scope(mutex());
	bool rt = true;
	
	if ( index != m_bit_rate_index && index < m_bit_rate.length() ) {
		BitRateInfo& info = m_bit_rate[index];

		for ( auto& i: m_extractors ) {
			Extractor* ex = i.value();
			Array<TrackInfo> tracks;

			for ( uint j = 0; j < info.tracks.length(); j++ ) {
				if (info.tracks[j].type == ex->type()) {
					tracks.push(info.tracks[j]);
				}
			}
			if (tracks.length()) {
				extractor_flush(ex);
				ex->m_tracks = move(tracks);
				// notice decoder change
			} else {
				rt = false; break;
			}
		}
	} else {
		rt = false;
	}
	
	if (rt) {
		m_bit_rate_index = index;
	}
	return rt;
}

/**
 * @func extractor_flush
 * */
void Inl::extractor_flush(Extractor* ex) {
	ex->m_sample_index_cache = 0;
	ex->m_sample_count_cache = 0;
	ex->m_sample_data_cache = Array<SampleData>();
	ex->m_sample_data.size = 0;
	ex->m_sample_data.time = 0;
	ex->m_sample_data.flags = 0;
	ex->m_track_index = 0;
	ex->m_eof_flags = 0;
}

// @func select_multi_bit_rate2
void Inl::select_multi_bit_rate2(uint index) {
	AVFormatContext* fmt_ctx = m_fmt_ctx;
	if ( fmt_ctx->nb_programs ) {
		for (uint i = 0; i < fmt_ctx->nb_programs; i++) {
			AVProgram* program = fmt_ctx->programs[i];
			for (uint j = 0; j < program->nb_stream_indexes; j++) {
				fmt_ctx->streams[*program->stream_index + j]->discard = AVDISCARD_ALL;
			}
		}
		AVProgram* program = fmt_ctx->programs[ XX_MIN(index, fmt_ctx->nb_programs - 1) ];
		for (uint j = 0; j < program->nb_stream_indexes; j++) {
			fmt_ctx->streams[*program->stream_index + j]->discard = AVDISCARD_NONE;
		}
	} else {
		for ( uint i = 0; i < fmt_ctx->nb_streams; i++ ) {
			fmt_ctx->streams[i]->discard = AVDISCARD_NONE;
		}
	}
}

/**
 * @func extractor
 */
Extractor* Inl::extractor(MediaType type) {
	ScopeLock scope(mutex());
	
	if (m_status == MULTIMEDIA_SOURCE_STATUS_READY ||
			m_status == MULTIMEDIA_SOURCE_STATUS_WAIT ) {
		auto i = m_extractors.find(type);
		
		if ( !i.is_null() ) {
			return i.value();
		} else {
			
			BitRateInfo& info = m_bit_rate[m_bit_rate_index];
			Array<TrackInfo> tr;
			
			for (uint i = 0; i < info.tracks.length(); i++) {
				if (info.tracks[i].type == type) {
					tr.push(info.tracks[i]);
				}
			}
			if ( tr.length() ) {
				Extractor* ex = new Extractor(type, m_host, move(tr));
				m_extractors.set(type, ex);
				return ex;
			}
		}
	}
	return NULL;
}

/**
 * @func seek
 * */
bool Inl::seek(uint64 timeUs) {
	ScopeLock scope(mutex());
	
	if ( (m_status == MULTIMEDIA_SOURCE_STATUS_READY ||
				m_status == MULTIMEDIA_SOURCE_STATUS_WAIT) && timeUs < m_duration ) {
		
		if ( m_read_eof == false ) { // not eof
			
			int stream = av_find_default_stream_index(m_fmt_ctx);
			
			auto time_base = m_fmt_ctx->streams[stream]->time_base;
			auto time = m_fmt_ctx->streams[stream]->start_time +
									av_rescale(timeUs / 1000000.0, time_base.den, time_base.num);
			
			int ok = av_seek_frame(m_fmt_ctx, stream, time, AVSEEK_FLAG_BACKWARD);
			
			if ( ok >= 0 ) {
				// clear Extractor cache sample data
				
				for ( auto& i : m_extractors ) {
					Extractor* ex = i.value();
					ex->m_sample_index_cache = 0;
					ex->m_sample_count_cache = 0;
					ex->m_sample_data.size = 0;
				}
				return true;
			}
		}
	}
	return false;
}

// @func read_bit_rate_info
BitRateInfo Inl::read_bit_rate_info(AVFormatContext* fmt_ctx, int start, int size) {
	BitRateInfo info          = { 0, 0, 0 };
	AVDictionaryEntry* entry  = nullptr;
	
	for ( ; start < size; start++) {
		AVStream* stream = fmt_ctx->streams[start];
		AVCodecParameters* codecpar = stream->codecpar;
		AVMediaType type = codecpar->codec_type;
		
		if ( type == AVMEDIA_TYPE_VIDEO || type == AVMEDIA_TYPE_AUDIO ) {
			TrackInfo tr;
			tr.track = uint(start);
			tr.type = (type == AVMEDIA_TYPE_VIDEO) ? MEDIA_TYPE_VIDEO : MEDIA_TYPE_AUDIO;
			tr.mime = String::format("%s/%s", av_get_media_type_string(type),
															 avcodec_get_name(codecpar->codec_id));
			tr.codec_id = codecpar->codec_id;
			tr.codec_tag = codecpar->codec_tag;
			tr.format = codecpar->format;
			tr.profile = codecpar->profile;
			tr.level = codecpar->level;
			tr.width = codecpar->width;
			tr.height = codecpar->height;
			tr.sample_rate = codecpar->sample_rate;
			tr.channel_count = codecpar->channels;
			tr.channel_layout = codecpar->channel_layout;
			tr.frame_interval = stream->avg_frame_rate.den *
							(double(1000000.0) / double(stream->avg_frame_rate.num));
			tr.extradata = WeakBuffer((char *) codecpar->extradata, codecpar->extradata_size).copy();

			entry = av_dict_get(stream->metadata, "variant_language", nullptr, 0);
			if ( entry ) {
				tr.language = entry->value;
			}
			entry = av_dict_get(stream->metadata, "variant_bitrate", nullptr, 0);
			if ( entry ) {
				tr.bitrate = String(entry->value).to_uint();
			}
			
			if ( type == AVMEDIA_TYPE_VIDEO ) {
				info.width = codecpar->width;
				info.height = codecpar->height;
			}
			
			info.codecs = (info.codecs.is_empty() ? info.codecs : info.codecs + ", ") +
			String::format( "%s (%s)",
										 avcodec_get_name    (codecpar->codec_id),
										 avcodec_profile_name(codecpar->codec_id, codecpar->profile)
										 );
			info.tracks.push(tr);
		}
	}
	
	return info;
}

#define ABORT() { XX_THREAD_LOCK(t, { trigger_error(e); }); return; }

/**
* @func start
* */
void Inl::start() {
	ScopeLock scope(mutex());

	if (m_status == MULTIMEDIA_SOURCE_STATUS_READYING ||
			m_status == MULTIMEDIA_SOURCE_STATUS_READY ||
			m_status == MULTIMEDIA_SOURCE_STATUS_WAIT ) {
		return;
	}
	
	reset();
	
	m_status = MULTIMEDIA_SOURCE_STATUS_READYING;
	
	if ( m_uri.type() == URI_ZIP ) { // the now not support zip path
		trigger_error(Error(ERR_MEDIA_INVALID_SOURCE, "invalid source file `%s`", *m_uri.href()));
		return;
	}
	
	String uri = Path::fallback_c(m_uri.href());

	detach_child([this, uri](SimpleThread& t) {
		
		AVFormatContext* fmt_ctx = nullptr;
		
		ScopeClear clear([&fmt_ctx, this]() {
			if ( fmt_ctx ) {
				avformat_close_input(&fmt_ctx);
			}
			XX_DEBUG("free ffmpeg AVFormatContext");
		});
		
		int r;
		
		/* open input file, and allocate format context */
		r = avformat_open_input(&fmt_ctx, *uri, nullptr, nullptr);
		if ( r < 0 ) {
			
			char msg[AV_ERROR_MAX_STRING_SIZE] = { 0 };
			av_make_error_string(msg, AV_ERROR_MAX_STRING_SIZE, r);
			Error e(ERR_MEDIA_INVALID_SOURCE,
							"Could not open source file: `%s`, msg: %s", *uri, msg);
			ABORT();
		}
		
		/* retrieve stream information */
		r = avformat_find_stream_info(fmt_ctx, nullptr);
		if ( r < 0 ) {
			char msg[AV_ERROR_MAX_STRING_SIZE] = { 0 };
			av_make_error_string(msg, AV_ERROR_MAX_STRING_SIZE, r);
			Error e(ERR_MEDIA_INVALID_SOURCE,
							"Could not find stream information `%s`, msg: %s", *uri, *msg);
			ABORT();
		}
		
#if DEBUG
		av_dump_format(fmt_ctx, 0, *uri, 0); // print info
#endif
		
		Array<BitRateInfo> bit_rate;
		
		if (fmt_ctx->nb_programs) {
			for (uint i = 0; i < fmt_ctx->nb_programs; i++) {
				AVProgram* program = fmt_ctx->programs[i];
				BitRateInfo info = read_bit_rate_info(fmt_ctx, *program->stream_index,
																						 *program->stream_index + program->nb_stream_indexes);
				AVDictionaryEntry* entry = av_dict_get(program->metadata, "variant_bitrate", NULL, 0);
				if ( entry ) {
					info.bandwidth = String(entry->value).to_uint();
				}
				bit_rate.push(info);
			}
		} else {
			bit_rate.push(read_bit_rate_info(fmt_ctx, 0, fmt_ctx->nb_streams));
		}
		
		int bit_rate_index;
		
		XX_THREAD_LOCK(t, {
			{
				ScopeLock scope(mutex());
				m_bit_rate = move(bit_rate);
				m_duration = fmt_ctx->duration > 0 ? fmt_ctx->duration : 0;
				m_fmt_ctx = fmt_ctx;
			}
			bit_rate_index = bit_rate.length() / 2; // default value
			select_bit_rate(bit_rate_index);
			select_multi_bit_rate2(bit_rate_index);

			post(Cb([this](Se& d) {
				{ ScopeLock scope(mutex());
					m_status = MULTIMEDIA_SOURCE_STATUS_READY;
				}
				m_delegate->multimedia_source_ready(m_host);
			}));
		}, {
			return;
		});
		
		read_stream(t, fmt_ctx, uri, bit_rate_index);
	}, "ffmpeg_read_source");
}

/**
 * @func stop
 * */
void Inl::stop() {
	
	ScopeLock scope(mutex());
	abort_child();
	m_read_eof = 0;
	m_fmt_ctx = nullptr;
	
	for ( auto& i : m_extractors ) {
		extractor_flush(i.value());
	}
	
	post(Cb([this](Se& d) {
		ScopeLock scope(mutex());
		m_status = MULTIMEDIA_SOURCE_STATUS_UNINITIALIZED;
	}));
}

/**
 * @func disable_wait_buffer
 */
void Inl::disable_wait_buffer(bool value) {
	m_disable_wait_buffer = value;
}

/*
 * @func extractor_push
 * */
bool Inl::extractor_push(Extractor* ex, AVPacket& pkt, AVStream* stream, double tbn) {

	if ( ex->type() == MEDIA_TYPE_VIDEO ) {
		if ( ex->m_sample_data_cache.length() == 0 ) { // allocation
			AVRational& rat = stream->avg_frame_rate.den ?
												stream->avg_frame_rate : stream->r_frame_rate;
			int len = rat.num * CACHE_DATA_TIME_SECOND / rat.den;
			ex->m_sample_data_cache = Array<SampleData>(XX_MAX(len, 32));
		}
	} else { // AUDIO
		int len = ex->m_sample_data_cache.length();
		
		if ( len == 0 ) {
			if ( pkt.pts < stream->start_time ) { // Discard
				return true;
			}
			ex->m_sample_data_cache = Array<SampleData>(1);
		}
		else if (len == 1 && ex->m_sample_count_cache == 1 /* && pkt.pts */ ) {
			SampleData data = move(ex->m_sample_data_cache[0]);
			if ( pkt.pts ) {
				len = 1000000 * CACHE_DATA_TIME_SECOND / (pkt.pts * tbn - data.time);
			} else {
				len = 256;
			}
			ex->m_sample_data_cache = Array<SampleData>(XX_MIN(XX_MAX(len, 32), 1024));
			ex->m_sample_data_cache[0] = move(data);
		}
	}
	
	uint len = ex->m_sample_data_cache.length();
	
	if ( ex->m_sample_count_cache >= len ) {
		return false;
	} else {
		ex->m_sample_count_cache++;
	}
	int i = ex->m_sample_index_cache + ex->m_sample_count_cache - 1;
	
	SampleData& data = ex->m_sample_data_cache[i % len];
	
	data._buf.write(WeakBuffer((char*)pkt.data, pkt.size), 0);
	data.data   = *data._buf;
	data.size   = pkt.size;
	data.time   = pkt.pts * tbn;
	data.d_time = pkt.dts * tbn;
	data.flags  = pkt.flags;
	
	if ( pkt.pts < 0 ) { // Correction time
	
		if ( ex->m_sample_count_cache >= 2 ) {
			int i0 = ex->m_sample_index_cache + ex->m_sample_count_cache - 3;
			int i1 = ex->m_sample_index_cache + ex->m_sample_count_cache - 2;
			SampleData& d0 = ex->m_sample_data_cache[(i0 + len) % len];
			SampleData& d1 = ex->m_sample_data_cache[(i1 + len) % len];
			data.time = d1.time + d1.time - d0.time;
			data.d_time =  d1.d_time + d1.d_time - d0.d_time;
			XX_DEBUG("extractor_push(), time == 0, Correction time: %llu", data.time);
		}
		
		if ( ex->type() == MEDIA_TYPE_VIDEO ) { // VIDEO
			data.time = Uint64::max; // Unknown time
		}
	}
	
	return true;
}

/**
 * @func valid_extractor
 */
Extractor* Inl::valid_extractor(AVMediaType type) {
	if (type == AVMEDIA_TYPE_VIDEO || type == AVMEDIA_TYPE_AUDIO) {
		auto it = m_extractors.find(type == AVMEDIA_TYPE_VIDEO ? MEDIA_TYPE_VIDEO : MEDIA_TYPE_AUDIO);
		
		if (m_extractors.end() != it) {
			Extractor* ex = it.value();
			if ( ex->m_disable ) {
				return nullptr;
			} else {
				return ex;
			}
		}
	}
	return nullptr;
}

/**
 * @func has_valid_extractor
 * */
bool Inl::has_valid_extractor() {
	for (auto i = m_extractors.begin(),
						e = m_extractors.end(); i != e; i++) {
		if ( !i.value()->m_disable ) {
			return true;
		}
	}
	return false;
}

/**
 * @func extractor_advance_no_wait
 */
bool Inl::extractor_advance_no_wait(Extractor* ex) {
	
	if ( ex->m_sample_count_cache ) {
		// swap data
		SampleData data = move(ex->m_sample_data);
		ex->m_sample_data = move(ex->m_sample_data_cache[ex->m_sample_index_cache]);
		ex->m_sample_data_cache[ex->m_sample_index_cache] = move(data);
		ex->m_sample_index_cache = (ex->m_sample_index_cache + 1) % ex->m_sample_data_cache.length();
		ex->m_sample_count_cache--;
		if (ex->m_sample_count_cache == 0 && m_read_eof) {
			ex->m_eof_flags = 1;
		}
	} else { // no data
		if ( m_read_eof ) { // eos
			trigger_eof();
		}
	}
	
	return ex->m_sample_data.size != 0;
}

/**
 * @func extractor_advance
 */
bool Inl::extractor_advance(Extractor* ex) {
	ScopeLock scope(mutex());

	if (ex->m_sample_data.size) {
		return true;
	}
	if ( m_status != MULTIMEDIA_SOURCE_STATUS_READY &&
			m_status != MULTIMEDIA_SOURCE_STATUS_WAIT ) {
		return false;
	}
	
	if ( m_disable_wait_buffer ) {
		if ( m_status == MULTIMEDIA_SOURCE_STATUS_WAIT ) {
			trigger_ready_buffer();
		}
		return extractor_advance_no_wait(ex);
	}
	
	if ( ex->m_sample_count_cache ) {
		if ( m_status == MULTIMEDIA_SOURCE_STATUS_WAIT ) {
			if ( ex->m_sample_count_cache == ex->m_sample_data_cache.length() || m_read_eof ) { //
				trigger_ready_buffer();
			}
		} else {
			// swap data
			SampleData data = move(ex->m_sample_data);
			ex->m_sample_data = move(ex->m_sample_data_cache[ex->m_sample_index_cache]);
			ex->m_sample_data_cache[ex->m_sample_index_cache] = move(data);
			ex->m_sample_index_cache = (ex->m_sample_index_cache + 1) % ex->m_sample_data_cache.length();
			ex->m_sample_count_cache--;
			// XX_DEBUG("extractor_advance(), m_sample_count_cache:%d", ex->m_sample_count_cache);
			if (ex->m_sample_count_cache == 0 && m_read_eof) {
				ex->m_eof_flags = 1;
			}
		}

	} else { // no data
		// XX_DEBUG("extractor_advance(), no data ");
		
		if ( m_read_eof ) { // eos
			trigger_eof();
		} else {
			if ( m_status == MULTIMEDIA_SOURCE_STATUS_READY ) {
				trigger_wait_buffer();
			}
		}
	}

	return ex->m_sample_data.size != 0;
}

/**
 * @func read_stream
 * */
void Inl::read_stream(SimpleThread& t, AVFormatContext* fmt_ctx, cString& uri, uint bit_rate_index) {
	
	Array<double> tbns;
	
	for (uint i = 0; i < fmt_ctx->nb_streams; i++) {
		AVStream* stream  = fmt_ctx->streams[i];
		double n = double(1000000.0) / (double(stream->time_base.den) / double(stream->time_base.num));
		tbns.push(n);
	}
	
	AVPacket pkt;
	
	/* initialize packet, set data to NULL, let the demuxer fill it */
	av_init_packet(&pkt);
	pkt.data = nullptr;
	pkt.size = 0;
	
	int  ok;
	bool sleep;
	
	while (1) {
		/* read frames from the file */
		ok = av_read_frame(fmt_ctx, &pkt);
		
		tag1:
		sleep = 0;
		{ //
			ScopeLock scope(t.mutex());
			
			if ( t.is_abort() ) {
				XX_DEBUG("read_frame() abort break;"); break;
			}
			
			if ( ok < 0 ) { // err or end
				if ( AVERROR_EOF == ok ) {
					XX_DEBUG("read_frame() eof break;");
					
					post(Cb([this](Se& d) {
						ScopeLock scope(mutex());
						m_read_eof = 1;
						m_fmt_ctx = nullptr;
					}));
					
				} else {
					XX_DEBUG("read_frame() error break;");
					
					char err_desc[AV_ERROR_MAX_STRING_SIZE] = {0};
					av_make_error_string(err_desc, AV_ERROR_MAX_STRING_SIZE, ok);
					
					XX_ERR("%s", err_desc);
					
					Error err(ERR_MEDIA_NETWORK_ERROR,
										"Read source error `%s`, `%s`", err_desc, *uri);
					trigger_error(err);
				}
				break;
			}

			if ( pkt.size ) { //
				ScopeLock scope(mutex());

				if ( bit_rate_index != m_bit_rate_index ) { // bit_rate_index change
					bit_rate_index = m_bit_rate_index;
					select_multi_bit_rate2(m_bit_rate_index);
				}

				if ( has_valid_extractor() ) {
					uint stream = pkt.stream_index; // stream index
					
					Extractor *ex = valid_extractor(fmt_ctx->streams[stream]->codecpar->codec_type);
					if (ex) {
						if (ex->track().track == stream) { // 是否为选中的轨道,比如有多路声音轨道
							if ( !extractor_push(ex, pkt, fmt_ctx->streams[stream], tbns[stream]) ) {
								sleep = 1; // 添加不成功休眠
							}
						}
					}
				} else { // 没有有效的ex
					sleep = 1; // sleep
				}
			}
		}
		
		if ( pkt.size ) {
			if (sleep) {
				SimpleThread::sleep_for(200000); // sleep 200ms
				goto tag1;
			}
			av_packet_unref(&pkt);
		}
	}
	
	av_packet_unref(&pkt);
}

/**
 * @func trigger_error
 * */
void Inl::trigger_error(cError& e) {
	XX_DEBUG("Err, %s", *e.message());
	post(Cb([e, this](Se& d) {
		{ ScopeLock scope(mutex());
			m_status = MULTIMEDIA_SOURCE_STATUS_FAULT;
			m_fmt_ctx = nullptr;
		}
		m_delegate->multimedia_source_error(m_host, e);
	}));
}

/**
 * @func trigger_wait_buffer
 * */
void Inl::trigger_wait_buffer() {
	post(Cb([this](Se& d) {
		{ ScopeLock scope(mutex());
			if ( m_status != MULTIMEDIA_SOURCE_STATUS_READY ) {
				return;
			}
			m_status = MULTIMEDIA_SOURCE_STATUS_WAIT;
		}
		XX_DEBUG("extractor_advance(), WAIT, 0");
		m_delegate->multimedia_source_wait_buffer(m_host, 0);
	}));
}

/**
 * @func trigger_ready_buffer
 * */
void Inl::trigger_ready_buffer() {
	post(Cb([this](Se& d) {
		{ ScopeLock scope(mutex());
			if ( m_status != MULTIMEDIA_SOURCE_STATUS_WAIT ) return;
			m_status = MULTIMEDIA_SOURCE_STATUS_READY;
		}
		XX_DEBUG("extractor_advance(), WAIT, 1");
		m_delegate->multimedia_source_wait_buffer(m_host, 1);
	}));
}

/**
 * @func trigger_eof
 * */
void Inl::trigger_eof() {
	post(Cb([this](Se& d) {
		{ ScopeLock scope(mutex());
			if (m_status == MULTIMEDIA_SOURCE_STATUS_EOF) return;
			m_status = MULTIMEDIA_SOURCE_STATUS_EOF;
		}
		m_delegate->multimedia_source_eof(m_host);
	}));
}

/**
 * @func get_stream
 * */
AVStream* Inl::get_stream(const TrackInfo& track) {
	ScopeLock scope(mutex());
	if ( m_fmt_ctx ) {
		if ( track.track < m_fmt_ctx->nb_streams ) {
			return m_fmt_ctx->streams[track.track];
		}
	}
	return NULL;
}

XX_END
