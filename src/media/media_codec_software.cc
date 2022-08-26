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

#include "./media_codec_inl.h"

namespace noug {

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
			, _codec_ctx(ctx)
			, _frame(NULL)
			, _audio_buffer_size(0)
			, _audio_swr_ctx(NULL)
			, _audio_frame_size(0)
			, _presentation_time(0)
			, _threads(1)
			, _background_run(false)
			, _is_open(false)
			, _output_occupy(false)
		{
			_frame = av_frame_alloc(); N_Asset(_frame);
			
			if (type() == MEDIA_TYPE_VIDEO) {
				_color_format = VIDEO_COLOR_FORMAT_YUV420P;
			} else {
				_channel_layout = CH_FRONT_LEFT | CH_FRONT_RIGHT;
				_channel_count  = 2;
				_audio_buffer = Buffer::alloc(1024 * 64); // 64k
			}
		}
		
		/**
		* @destructor
		*/
		virtual ~SoftwareMediaCodec() {
			
			close();
			
			avcodec_free_context(&_codec_ctx); _codec_ctx = nullptr;
			av_frame_free(&_frame); _frame = nullptr;
			
			if ( _audio_swr_ctx ) {
				swr_free(&_audio_swr_ctx); _audio_swr_ctx = nullptr;
			}
		}
		
		void init_audio_swr() {
			AVCodecContext* ctx = _codec_ctx;
			_audio_swr_ctx  =
			swr_alloc_set_opts(_audio_swr_ctx,
												_channel_layout, AV_SAMPLE_FMT_S16, ctx->sample_rate,
												ctx->channel_layout, ctx->sample_fmt, ctx->sample_rate,
												0, nullptr);
			swr_init(_audio_swr_ctx);
		}
		
		/**
		* @overwrite
		*/
		virtual bool open() {
			ScopeLock lock(_mutex);
			
			if ( !_is_open ) {
				
				AVStream* stream = _extractor->host()->get_stream(_extractor->track());
				if ( !stream ) {
					stream = _extractor->host()->get_stream(_extractor->track());
					N_Asset( stream );
				}
				
				const AVCodec* codec = get_avcodec(); N_Asset(codec);
				
				if ( _threads > 1 ) { // set threads
					if ((codec->capabilities & AV_CODEC_CAP_FRAME_THREADS)
							&& !(_codec_ctx->flags & AV_CODEC_FLAG_TRUNCATED)
							&& !(_codec_ctx->flags & AV_CODEC_FLAG_LOW_DELAY)
							&& !(_codec_ctx->flags2 & AV_CODEC_FLAG2_CHUNKS)) {
						_codec_ctx->thread_count = _threads;
						_codec_ctx->active_thread_type = FF_THREAD_FRAME;
					}
				}
				/* Copy codec parameters from input stream to output codec context */
				if (avcodec_parameters_to_context(_codec_ctx, stream->codecpar) >= 0) {
					if (avcodec_open2(_codec_ctx, codec, NULL) >= 0) {
						_is_open = true;
						
						if ( type() == MEDIA_TYPE_AUDIO ) {
							init_audio_swr();
						}
						
						if ( _background_run ) { // background_run
							_background_run_id = Thread::create([](Thread& t, void* arg) {
								auto self = (SoftwareMediaCodec*)arg;
								self->background_run(t);
							}, this, "x_decoder_background_run_thread");
						}
					}
				}
				
				flush2();
			}
			return _is_open;
		}
		
		/**
		* @overwrite
		*/
		virtual bool close() {
			Lock lock(_mutex);
			if ( _is_open ) {
				flush2();
				
				if ( _background_run ) {
					lock.unlock();
					Thread::abort(_background_run_id);
					Thread::wait(_background_run_id);
					lock.lock();
				}
				if ( avcodec_close(_codec_ctx) >= 0 ) {
					_is_open = false;
				}
			}
			return !_is_open;
		}
		
		void flush2() {
			if ( _is_open ) {
				_presentation_time = 0;
				_audio_buffer_size = 0;
				// avcodec_flush_buffers(_codec_ctx);
			}
		}
		
		/**
		* @overwrite
		* */
		virtual bool flush() {
			ScopeLock scope(_mutex);
			flush2();
			return false;
		}
		
		/**
		* @func background_run
		*/
		void background_run(Thread& t) {
			while ( !t.is_abort() ) {
				if ( !advance2() ) {
					Thread::sleep(10000); // sleep 10ms
				}
			}
		}
		
		/**
		* @func advance2
		*/
		bool advance2() {
			if ( _extractor->advance() ) {
				WeakBuffer data = _extractor->sample_data();
				
				AVPacket pkt;
				av_init_packet(&pkt);
				av_packet_from_data(&pkt, (uint8_t*)*data, data.length());
				pkt.flags = _extractor->sample_flags();
				pkt.pts = _extractor->sample_time();
				pkt.dts = 0;
				
				int ret = avcodec_send_packet(_codec_ctx, &pkt);
				if (ret == 0) {
					_extractor->deplete_sample(pkt.size);
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
			if ( !_background_run ) {
				return advance2();
			}
			return false;
		}
		
		/**
		* @overwrite
		* */
		virtual OutputBuffer output() {
			if ( _output_occupy ) {
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
			
			if ( _audio_buffer_size ) {
				if (_audio_buffer_size >= _audio_frame_size) {
					OutputBuffer out;
					out.data[0] = (uint8_t *) *_audio_buffer;
					out.linesize[0] = _audio_frame_size;
					out.total = _audio_frame_size;
					out.time = _presentation_time + _frame_interval;
					_presentation_time = out.time;
					_output_occupy = true;
					return out;
				}
			}
			
			int ret = avcodec_receive_frame(_codec_ctx, _frame);
			if ( ret == 0 ) {
				int sample_bytes = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
				uint32_t size = _frame->nb_samples * sample_bytes * _channel_count;
				
				if (_audio_frame_size == 0) {
					set_frame_size(size);
				}
				
				if ( size ) {
					OutputBuffer out;
					uint8_t* buffer = (uint8_t*)*_audio_buffer + _audio_buffer_size;
					swr_convert(_audio_swr_ctx,
											&buffer, _frame->nb_samples,
											(const uint8_t  **) _frame->extended_data, _frame->nb_samples);
					_audio_buffer_size += size;
					
					if (_audio_buffer_size >= _audio_frame_size) {
						out.data[0] = (uint8_t*)*_audio_buffer;
						out.linesize[0] = _audio_frame_size;
						out.total = _audio_frame_size;
						// time
						float front = (_audio_buffer_size - size) / float(_audio_frame_size);
						out.time = _frame->pts - front * _frame_interval;
						_presentation_time = out.time;
						_output_occupy = true;
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
			int ret = avcodec_receive_frame(_codec_ctx, _frame);
			if ( ret == 0 ) {
				OutputBuffer out;
				// yuv420p
				out.linesize[0] = _frame->width * _frame->height;
				out.linesize[1] = out.linesize[0] / 4;
				out.linesize[2] = out.linesize[1];
				out.data[0] = _frame->data[0]; // y
				out.data[1] = _frame->data[1]; // u
				out.data[2] = _frame->data[2]; // v
				out.time = _frame->pts;
				out.total = out.linesize[0] + out.linesize[1] + out.linesize[2];
				//
				if ( out.time == Uint64::limit_max ) { // Unknown time frame
					out.time = _presentation_time + _frame_interval; // correct time
				}
				_presentation_time = out.time;
				_output_occupy = true;
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
					if ( _audio_buffer_size > buffer.total ) {
						_audio_buffer_size -= buffer.total;
						_audio_buffer.write(*_audio_buffer + buffer.total, 0, _audio_buffer_size);
					} else {
						_audio_buffer_size = 0;
					}
				}
				memset(&buffer, 0, sizeof(OutputBuffer));
				_output_occupy = false;
			}
		}
		
		/**
		* @overwrite
		* */
		virtual void set_frame_size(uint32_t size) {
			if ( type() == MEDIA_TYPE_AUDIO ) {
				_audio_frame_size = N_MAX(512, size);
				if (_audio_frame_size * 2 > _audio_buffer.length()) {
					_audio_buffer = Buffer::alloc(_audio_frame_size * 2);
					_audio_buffer_size = 0;
					_presentation_time = 0;
				}
				// compute audio frame interval
				const TrackInfo &track = extractor()->track();
				uint64_t second_size = track.sample_rate * _channel_count * 2;
				_frame_interval = uint32_t(uint64_t(_audio_frame_size) * 1000LL * 1000LL / second_size);
			}
		}
		
		/**
		* @overwrite
		* */
		virtual void set_threads(uint32_t value) {
			ScopeLock scope(_mutex);
			if ( !_is_open ) {
				_threads = N_MAX(1, N_MIN(8, value));
			}
		}
		
		/**
		* @overwrite
		* */
		virtual void set_background_run(bool value) {
			ScopeLock scope(_mutex);
			if ( !_is_open ) {
				_background_run = value;
			}
		}
		
		const AVCodec* get_avcodec() {
			const AVCodec* rv = _codec_ctx->codec;
			if ( rv ) return rv;
			
			const TrackInfo& track = _extractor->track();
			
			/* find decoder for the stream */
			AVCodec* codec = avcodec_find_decoder((AVCodecID)track.codec_id);
			if (codec) {
				/* Allocate a codec context for the decoder */
				avcodec_open2(_codec_ctx, codec, nullptr);
				rv = _codec_ctx->codec;
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
		AVCodecContext* _codec_ctx;
		AVFrame*        _frame;
		Buffer          _audio_buffer;
		uint32_t        _audio_buffer_size;
		SwrContext*     _audio_swr_ctx;
		uint32_t        _audio_frame_size;
		uint64_t        _presentation_time;
		uint32_t        _threads;
		bool            _background_run;
		bool            _is_open;
		bool            _output_occupy;
		Mutex           _mutex;
		ThreadID        _background_run_id;
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

}
