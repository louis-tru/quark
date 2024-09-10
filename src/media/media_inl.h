// @private head
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

// @private head

#ifndef __quark__media_codec_inl__
#define __quark__media_codec_inl__

extern "C" {
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}
#include "./media_codec.h"
#include "../util/working.h"

namespace qk {
	typedef MediaSource::BitRateInfo BitRateInfo;
	typedef MediaSource::Extractor   Extractor;
	typedef MediaSource::Inl         Inl;
	typedef MediaSource::TrackInfo   TrackInfo;

	class SoftwareMediaCodec;

	class MediaSource::Inl: public ParallelWorking {
	public:

		Inl(MediaSource*, cString& uri, RunLoop* loop);
		~Inl() override;

		/**
		* @method set_delegate
		*/
		void set_delegate(Delegate* delegate);

		/**
		* @method bit_rate_index
		*/
		uint32_t bit_rate_index();

		/**
		* @method bit_rates
		*/
		const Array<BitRateInfo>& bit_rate();

		/**
		* @method bit_rate
		*/
		bool select_bit_rate(uint32_t index);

		/**
		* @method extractor
		*/
		Extractor* extractor(MediaType type);

		/**
		* @method seek
		* */
		bool seek(uint64_t timeUs);

		/**
		* @method start
		* */
		void start();

		/**
		* @method start
		* */
		void stop();

		/**
		* @method is_active
		*/
		inline bool is_active() {
			return  _status == MULTIMEDIA_SOURCE_STATUS_READY ||
							_status == MULTIMEDIA_SOURCE_STATUS_WAIT;
		}

		/**
		* @method disable_wait_buffer
		*/
		void disable_wait_buffer(bool value);

		/**
		* @method get_stream
		*/
		AVStream* get_stream(const TrackInfo& track);

	private:
		typedef Extractor::SampleData SampleData;

		void reset();
		bool has_empty_extractor();
		void extractor_flush(Extractor* ex);
		BitRateInfo read_bit_rate_info(AVFormatContext* fmt_ctx, int i, int size);
		void select_multi_bit_rate2(uint32_t index);
		void read_stream(Thread& t, AVFormatContext* fmt_ctx, cString& uri, uint32_t bit_rate_index);
		bool extractor_push(Extractor* ex, AVPacket& pkt, AVStream* stream, double tbn);
		bool extractor_advance(Extractor* ex);
		bool extractor_advance_no_wait(Extractor* ex);
		Extractor* valid_extractor(AVMediaType type);
		bool has_valid_extractor();
		void trigger_error(cError& err);
		void trigger_wait_buffer();
		void trigger_ready_buffer();
		void trigger_eof();
		inline Mutex& mutex() { return _mutex; }

		friend class MediaSource;
		friend class Extractor;
		friend class MediaCodec;
		friend class SoftwareMediaCodec;

		MediaSource*           _host;
		URI                         _uri;
		MediaSourceStatus      _status;
		Delegate*                   _delegate;
		uint32_t                    _bit_rate_index;
		Array<BitRateInfo>          _bit_rate;
		Dict<int, Extractor*>       _extractors;
		uint64_t                    _duration;
		AVFormatContext*            _fmt_ctx;
		Mutex                       _mutex;
		bool                        _read_eof;
		bool                        _disable_wait_buffer;
	};

}
#endif
