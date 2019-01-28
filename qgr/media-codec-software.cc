/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, xuewen.chu
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

XX_NS(qgr)

/**
 * @class SoftwareMediaCodec
 */
class SoftwareMediaCodec: public MediaCodec {
 public:
	
	/**
	 * @constructor
	 */
	SoftwareMediaCodec(Extractor* extractor, AVCodecContext* ctx)
		: MediaCodec(extractor)
		, m_codec_ctx(ctx)
		, m_frame(NULL)
		, m_audio_buffer_size(0)
		, m_audio_swr_ctx(NULL)
		, m_audio_frame_size(0)
		, m_presentation_time(0)
		, m_threads(1)
		, m_background_run(false)
		, m_is_open(false)
		, m_output_occupy(false)
	{
		m_frame = av_frame_alloc(); XX_ASSERT(m_frame);
		
		if (type() == MEDIA_TYPE_VIDEO) {
			m_color_format = VIDEO_COLOR_FORMAT_YUV420P;
		} else {
			m_channel_layout = CH_FRONT_LEFT | CH_FRONT_RIGHT;
			m_channel_count  = 2;
			m_audio_buffer = Buffer(1024 * 64); // 64k
		}
	}
	
	/**
	 * @destructor
	 */
	virtual ~SoftwareMediaCodec() {
		
		close();
		
		avcodec_free_context(&m_codec_ctx); m_codec_ctx = nullptr;
		av_frame_free(&m_frame); m_frame = nullptr;
		
		if ( m_audio_swr_ctx ) {
			swr_free(&m_audio_swr_ctx); m_audio_swr_ctx = nullptr;
		}
	}
	
	void init_audio_swr() {
		AVCodecContext* ctx = m_codec_ctx;
		m_audio_swr_ctx  =
		swr_alloc_set_opts(m_audio_swr_ctx,
											 m_channel_layout, AV_SAMPLE_FMT_S16, ctx->sample_rate,
											 ctx->channel_layout, ctx->sample_fmt, ctx->sample_rate,
											 0, nullptr);
		swr_init(m_audio_swr_ctx);
	}
	
	/**
	 * @overwrite
	 */
	virtual bool open() {
		ScopeLock lock(m_mutex);
		
		if ( !m_is_open ) {
			
			AVStream* stream = m_extractor->host()->get_stream(m_extractor->track());
			if ( !stream ) {
				stream = m_extractor->host()->get_stream(m_extractor->track());
				XX_ASSERT( stream );
			}
			
			const AVCodec* codec = get_avcodec(); XX_ASSERT(codec);
			
			if ( m_threads > 1 ) { // set threads
				if ((codec->capabilities & CODEC_CAP_FRAME_THREADS)
						&& !(m_codec_ctx->flags & CODEC_FLAG_TRUNCATED)
						&& !(m_codec_ctx->flags & CODEC_FLAG_LOW_DELAY)
						&& !(m_codec_ctx->flags2 & CODEC_FLAG2_CHUNKS)) {
					m_codec_ctx->thread_count = m_threads;
					m_codec_ctx->active_thread_type = FF_THREAD_FRAME;
				}
			}
			/* Copy codec parameters from input stream to output codec context */
			if (avcodec_parameters_to_context(m_codec_ctx, stream->codecpar) >= 0) {
				if (avcodec_open2(m_codec_ctx, codec, NULL) >= 0) {
					m_is_open = true;
					
					if ( type() == MEDIA_TYPE_AUDIO ) {
						init_audio_swr();
					}
					
					if ( m_background_run ) { // background_run
						m_background_run_id = SimpleThread::detach([this](SimpleThread& t) {
							background_run(t);
						}, "x_decoder_background_run_thread");
					}
				}
			}
			
			flush2();
		}
		return m_is_open;
	}
	
	/**
	 * @overwrite
	 */
	virtual bool close() {
		Lock lock(m_mutex);
		if ( m_is_open ) {
			flush2();
			
			if ( m_background_run ) {
				lock.unlock();
				SimpleThread::abort(m_background_run_id, true);
				lock.lock();
			}
			if ( avcodec_close(m_codec_ctx) >= 0 ) {
				m_is_open = false;
			}
		}
		return !m_is_open;
	}
	
	void flush2() {
		if ( m_is_open ) {
			m_presentation_time = 0;
			m_audio_buffer_size = 0;
			// avcodec_flush_buffers(m_codec_ctx);
		}
	}
	
	/**
	 * @overwrite
	 * */
	virtual bool flush() {
		ScopeLock scope(m_mutex);
		flush2();
		return false;
	}
	
	/**
	 * @func background_run
	 */
	void background_run(SimpleThread& t) {
	 loop:
		bool ok = 0;
		{ //
			ScopeLock scope(t.mutex());
			if (t.is_abort())
				return;
			ok = advance2();
		}
		
		if ( !ok ) {
			SimpleThread::sleep_for(10000); // sleep 10ms
		}
		goto loop;
	}
	
	/**
	 * @func advance2
	 */
	bool advance2() {
		if ( m_extractor->advance() ) {
			WeakBuffer data = m_extractor->sample_data();
			
			AVPacket pkt;
			av_init_packet(&pkt);
			av_packet_from_data(&pkt, (byte*)*data, data.length());
			pkt.flags = m_extractor->sample_flags();
			pkt.pts = m_extractor->sample_time();
			pkt.dts = 0;
			
			int ret = avcodec_send_packet(m_codec_ctx, &pkt);
			if (ret == 0) {
				m_extractor->deplete_sample(pkt.size);
				return true;
			} else if ( ret < 0 ) {
				// err..
			}
		}
		return false;
	}
	
	/**
	 * @overwrite
	 * */
	virtual bool advance() {
		if ( !m_background_run ) {
			return advance2();
		}
		return false;
	}
	
	/**
	 * @overwrite
	 * */
	virtual OutputBuffer output() {
		if ( m_output_occupy ) {
			return OutputBuffer();
		}
		if ( type() == MEDIA_TYPE_AUDIO ) {
			return output_audio();
		} else {
			return output_video();
		}
	}
	
	/**
	 * @func output_audio
	 */
	OutputBuffer output_audio() {
		
		if ( m_audio_buffer_size ) {
			if (m_audio_buffer_size >= m_audio_frame_size) {
				OutputBuffer out;
				out.data[0] = (byte *) *m_audio_buffer;
				out.linesize[0] = m_audio_frame_size;
				out.total = m_audio_frame_size;
				out.time = m_presentation_time + m_frame_interval;
				m_presentation_time = out.time;
				m_output_occupy = true;
				return out;
			}
		}
		
		int ret = avcodec_receive_frame(m_codec_ctx, m_frame);
		if ( ret == 0 ) {
			int sample_bytes = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
			uint size = m_frame->nb_samples * sample_bytes * m_channel_count;
			
			if (m_audio_frame_size == 0) {
				set_frame_size(size);
			}
			
			if ( size ) {
				OutputBuffer out;
				byte* buffer = (byte*)*m_audio_buffer + m_audio_buffer_size;
				swr_convert(m_audio_swr_ctx,
										&buffer, m_frame->nb_samples,
										(const uint8_t **) m_frame->extended_data, m_frame->nb_samples);
				m_audio_buffer_size += size;
				
				if (m_audio_buffer_size >= m_audio_frame_size) {
					out.data[0] = (byte*)*m_audio_buffer;
					out.linesize[0] = m_audio_frame_size;
					out.total = m_audio_frame_size;
					// time
					float front = (m_audio_buffer_size - size) / float(m_audio_frame_size);
					out.time = m_frame->pts - front * m_frame_interval;
					m_presentation_time = out.time;
					m_output_occupy = true;
					return out;
				}
			}
		}
		return OutputBuffer();
	}
	
	/**
	 * @func output_video
	 */
	OutputBuffer output_video() {
		int ret = avcodec_receive_frame(m_codec_ctx, m_frame);
		if ( ret == 0 ) {
			OutputBuffer out;
			// yuv420p
			out.linesize[0] = m_frame->width * m_frame->height;
			out.linesize[1] = out.linesize[0] / 4;
			out.linesize[2] = out.linesize[1];
			out.data[0] = m_frame->data[0]; // y
			out.data[1] = m_frame->data[1]; // u
			out.data[2] = m_frame->data[2]; // v
			out.time = m_frame->pts;
			out.total = out.linesize[0] + out.linesize[1] + out.linesize[2];
			//
			if ( out.time == Uint64::max ) { // Unknown time frame
				out.time = m_presentation_time + m_frame_interval; // correct time
			}
			m_presentation_time = out.time;
			m_output_occupy = true;
			return out;
		}
		return OutputBuffer();
	}
	
	/**
	 * @overwrite
	 * */
	virtual void release(OutputBuffer& buffer) {
		if (buffer.total) {
			if (type() == MEDIA_TYPE_AUDIO) {
				if ( m_audio_buffer_size > buffer.total ) {
					m_audio_buffer_size -= buffer.total;
					m_audio_buffer.write(*m_audio_buffer + buffer.total, 0, m_audio_buffer_size);
				} else {
					m_audio_buffer_size = 0;
				}
			}
			memset(&buffer, 0, sizeof(OutputBuffer));
			m_output_occupy = false;
		}
	}
	
	/**
	 * @overwrite
	 * */
	virtual void set_frame_size(uint size) {
		if ( type() == MEDIA_TYPE_AUDIO ) {
			m_audio_frame_size = XX_MAX(512, size);
			if (m_audio_frame_size * 2 > m_audio_buffer.length()) {
				m_audio_buffer = Buffer(m_audio_frame_size * 2);
				m_audio_buffer_size = 0;
				m_presentation_time = 0;
			}
			// compute audio frame interval
			const TrackInfo &track = extractor()->track();
			uint64 second_size = track.sample_rate * m_channel_count * 2;
			m_frame_interval = uint(uint64(m_audio_frame_size) * 1000LL * 1000LL / second_size);
		}
	}
	
	/**
	 * @overwrite
	 * */
	virtual void set_threads(uint value) {
		ScopeLock scope(m_mutex);
		if ( !m_is_open ) {
			m_threads = XX_MAX(1, XX_MIN(8, value));
		}
	}
	
	/**
	 * @overwrite
	 * */
	virtual void set_background_run(bool value) {
		ScopeLock scope(m_mutex);
		if ( !m_is_open ) {
			m_background_run = value;
		}
	}
	
	const AVCodec* get_avcodec() {
		const AVCodec* rv = m_codec_ctx->codec;
		if ( rv ) return rv;
		
		const TrackInfo& track = m_extractor->track();
		
		/* find decoder for the stream */
		AVCodec* codec = avcodec_find_decoder((AVCodecID)track.codec_id);
		if (codec) {
			/* Allocate a codec context for the decoder */
			avcodec_open2(m_codec_ctx, codec, nullptr);
			rv = m_codec_ctx->codec;
		}
		return rv;
	}
	
	static AVCodecContext* find_avcodec_ctx(Extractor* ex) {
		const TrackInfo& track = ex->track();
		
		/* find decoder for the stream */
		AVCodec* codec = avcodec_find_decoder((AVCodecID)track.codec_id);
		if (codec) {
			/* Allocate a codec context for the decoder */
			return avcodec_alloc_context3(codec);
		}
		return nullptr;
	}
	
 private:
	AVCodecContext* m_codec_ctx;
	AVFrame*        m_frame;
	Buffer          m_audio_buffer;
	uint            m_audio_buffer_size;
	SwrContext*     m_audio_swr_ctx;
	uint            m_audio_frame_size;
	uint64          m_presentation_time;
	uint            m_threads;
	bool            m_background_run;
	bool            m_is_open;
	bool            m_output_occupy;
	Mutex           m_mutex;
	ThreadID        m_background_run_id;
};

/**
 * @func software create software decoder
 * */
MediaCodec* MediaCodec::software(MediaType type, MultimediaSource* source) {
	SoftwareMediaCodec* rv = nullptr;
	Extractor* ex = source->extractor(type);
	
	if ( ex ) {
		AVCodecContext* codec_ctx = SoftwareMediaCodec::find_avcodec_ctx(ex);
		if (codec_ctx) {
			rv = new SoftwareMediaCodec(ex, codec_ctx);
		}
	}
	return rv;
}

XX_END
