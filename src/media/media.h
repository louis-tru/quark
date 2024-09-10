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

#ifndef __quark__media__media__
#define __quark__media__media__

#include "../util/util.h"
#include "../util/string.h"
#include "../util/array.h"
#include "../util/http.h"

typedef struct AVStream AVStream;

namespace qk {

	enum PlayerStatus {
		kStop_PlayerStatus = 0,
		kStart_PlayerStatus,
		kPlaying_PlayerStatus,
		kPaused_PlayerStatus,
	};

	enum MediaSourceStatus {
		kUninitialized_MediaSourceStatus = 0,
		kReadying_MediaSourceStatus,
		kReady_MediaSourceStatus,
		kWait_MediaSourceStatus,
		kFault_MediaSourceStatus,
		kEOF_MediaSourceStatus,
	};

	enum MediaType {
		kAudio_MediaType,
		kVideo_MediaType,
	};

	enum AudioChannelMask {
		kInvalid_AudioChannelMask                = 0,
		kFront_Left_AudioChannelMask             = 0x00000001,
		kFront_Right_AudioChannelMask            = 0x00000002,
		kFront_Center_AudioChannelMask           = 0x00000004,
		kLow_Frequency_AudioChannelMask          = 0x00000008,
		kBack_Left_AudioChannelMask              = 0x00000010,
		kBack_Right_AudioChannelMask             = 0x00000020,
		kFront_Left_Of_Center_AudioChannelMask   = 0x00000040,
		kFront_Right_Of_Center_AudioChannelMask  = 0x00000080,
		kBack_Center_AudioChannelMask            = 0x00000100,
		kSide_Left_AudioChannelMask              = 0x00000200,
		kSide_Right_AudioChannelMask             = 0x00000400,
		kTop_Center_AudioChannelMask             = 0x00000800,
		kTop_Front_Left_AudioChannelMask         = 0x00001000,
		kTop_Front_Center_AudioChannelMask       = 0x00002000,
		kTop_Front_Right_AudioChannelMask        = 0x00004000,
		kTop_Back_Left_AudioChannelMask          = 0x00008000,
		kTop_Back_Center_AudioChannelMask        = 0x00010000,
		kTop_Back_Right_AudioChannelMask         = 0x00020000,
	};

	enum VideoColorFormat {
		kInvalid_VideoColorFormat = 0,
		kYUV420P_VideoColorFormat,
		kYUV420SP_VideoColorFormat,
		kYUV411P_VideoColorFormat,
		kYUV411SP_VideoColorFormat,
	};

	class Qk_EXPORT MediaSource: public Object {
		Qk_HIDDEN_ALL_COPY(MediaSource);
		Qk_DEFINE_INLINE_CLASS(Inl);
	public:

		struct TrackInfo {
			uint32_t    track;            /* 轨道在源中的索引 */
			MediaType   type;             /* type */
			String      mime;             /* mime类型 */
			int         codec_id;         /* codec id */
			uint32_t    codec_tag;        /* codec tag */
			int         format;           /* format */
			int         profile;          /* profile */
			int         level;            /* level */
			uint32_t    width;            /* 输出图像宽度 */
			uint32_t    height;           /* 输出图像高度 */
			String      language;         /* 语言 */
			uint32_t    bitrate;          /* 码率 */
			uint32_t    sample_rate;      /* 声音采样率 */
			uint32_t    channel_count;    /* 声音声道数量 */
			uint64_t    channel_layout;   /* channel_layout */
			uint32_t    frame_interval;   /* 图像帧时间间隔 */
			Array<char> extradata;        /* extradata */
		};

		struct BitRateInfo {
			int         bandwidth;
			uint32_t    width;
			uint32_t    height;
			String      codecs;
			Array<TrackInfo>  tracks;
		};

		class Delegate {
		public:
			virtual void media_source_ready(MediaSource* source) = 0;
			virtual void media_source_wait_buffer(MediaSource* source, float process) = 0;
			virtual void media_source_eof(MediaSource* source) = 0;
			virtual void media_source_error(MediaSource* source, cError& err) = 0;
		};

		class Qk_EXPORT Extractor: public Object {
			Qk_HIDDEN_ALL_COPY(Extractor);
		public:
			
			/**
			* @method track_count
			*/
			inline uint32_t track_count() const { return _tracks.length(); }
			
			/**
			* @method track_index current
			*/
			inline uint32_t track_index() const { return _track_index; }
			
			/**
			* @method track current
			* */
			inline const TrackInfo& track() const { return _tracks[_track_index]; }
			
			/**
			* @method track get track info with index
			*/
			inline const TrackInfo& track(uint32_t index) const { return _tracks[index]; }
			
			/**
			* @method host
			*/
			inline MediaSource* host() const { return _host; }
			
			/**
			* @method type
			*/
			inline MediaType type() const { return _type; }
			
			/**
			* @method frame_interval
			* */
			inline uint32_t frame_interval() const { return _tracks[0].frame_interval; }
			
			/**
			* @method select_track
			*/
			bool select_track(uint32_t index);
			
			/**
			* @method sample_time
			* */
			inline uint64_t sample_time() const { return _sample_data.time; }
			
			/**
			* @method sample_d_time
			* */
			inline uint64_t sample_d_time() const { return _sample_data.d_time; }
			
			/**
			* @method sample_data
			* */
			inline WeakBuffer sample_data() const {
				return WeakBuffer(_sample_data.data, _sample_data.size);
			}
			
			/**
			* @method sample_size
			* */
			inline uint32_t sample_size() const { return _sample_data.size; }

			/**
			* @method presentation_time
			* */
			inline int sample_flags() const { return _sample_data.flags; }

			/**
			* @method eof_flags
			* */
			inline bool eof_flags() const { return _eof_flags; }

			/**
			* @method deplete_sample
			* */
			uint32_t deplete_sample(Char* out, uint32_t size);

			/**
			* @method deplete_sample
			* */
			uint32_t deplete_sample(Buffer& out);
			
			/**
			* @method deplete_sample
			* */
			uint32_t deplete_sample(uint32_t size);
			
			/**
			* @method deplete_sample
			* */
			inline uint32_t deplete_sample() {
				return deplete_sample(_sample_data.size);
			}
			
			/**
			* @method advance
			* */
			bool advance();

			/**
			* @method set_disable
			*/
			inline void set_disable(bool value) {  _disable = value; }

			/**
			* @method is_disable default true
			* */
			inline bool is_disable() const { return _disable; }
			
		private:
			Extractor(MediaType type, MediaSource* host, Array<TrackInfo>&& tracks);

			struct SampleData {
				Buffer  buf;
				Char*   data;
				uint32_t size;
				uint64_t time;
				uint64_t d_time;
				int      flags;
			};
			MediaSource*      _host;
			MediaType         _type;
			uint32_t          _track_index;
			Array<TrackInfo>  _tracks;
			Array<SampleData> _sample_data_cache;
			uint32_t          _sample_index_cache;
			uint32_t          _sample_count_cache;
			SampleData        _sample_data;
			bool              _eof_flags, _disable;
			friend class MediaSource;
			friend class MediaSource::Inl;
		};

		MediaSource(cString& uri, RunLoop* loop = RunLoop::current());
		~MediaSource() override;

		/**
		* @method set_delegate # Setting media delegate
		*/
		void set_delegate(Delegate* delegate);

		/**
		* @method disable_wait_buffer
		*/
		void disable_wait_buffer(bool value);

		/**
		* @method source # Getting media source path
		*/
		const URI& uri() const;

		/**
		* @method status # Getting current work status
		*/
		MediaSourceStatus status() const;

		/**
		* @method duration
		*/
		uint64_t duration() const;

		/**
		* @method bit_rate_index
		*/
		uint32_t bit_rate_index() const;

		/**
		* @method bit_rate
		*/
		const Array<BitRateInfo>& bit_rate() const;

		/**
		* @method select_bit_rate
		*/
		bool select_bit_rate(int index);

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
		*/
		void start();

		/**
		* @method stop
		*/
		void stop();
		
		/**
		* @method is_active
		*/
		bool is_active();
		
		/**
		* @method get_stream
		*/
		AVStream* get_stream(const TrackInfo& track);
		
	private:
		Inl*         _inl;
		friend class Extractor;
		friend class MediaCodec;
	};

	class Qk_EXPORT MediaCodec: public Object {
		Qk_HIDDEN_ALL_COPY(MediaCodec);
	public:
		typedef MediaSource::Extractor Extractor;

		struct OutputBuffer {
			uint8_t*  data[8] = {0};      /* 数据Buffer */
			uint32_t  linesize[8] = {0};  /* 数据大小 */
			uint32_t  total = 0;        /* 数据总大小 */
			uint64_t  time = 0;         /* 演示时间 */
			int       index = 0;        /* 数据Buffer在解码器中的索引 */
		};

		class Qk_EXPORT Delegate {
		 public:
			virtual void media_decoder_eof(MediaCodec* de, uint64_t timeUs) {}
			virtual void media_decoder_error(MediaCodec* de, cError& err) {}
		};

		/**
		* @method type
		* */
		inline MediaType type() const { return _extractor->type(); }

		/**
		* @method set_delegate
		*/
		void set_delegate(Delegate* delegate);

		/**
		* @method source # return decoder source path
		*/
		inline MediaSource* source() const { return _extractor->host(); }

		/**
		* @method extractor
		*/
		inline Extractor* extractor() const { return _extractor; }

		/**
		* @method color_format decode output video color format 
		*/
		inline VideoColorFormat color_format() const { return _color_format; }

		/**
		* @method channel_count
		* */
		inline uint32_t channel_count() const { return _channel_count; }

		/**
		* @method channel_layout
		* */
		inline uint64_t channel_layout() const { return _channel_layout; }

		/**
		* @method frame_interval
		* */
		inline uint32_t frame_interval() const { return _frame_interval; }

		/**
		* @method open
		*/
		virtual bool open() = 0;

		/**
		* @method close
		*/
		virtual bool close() = 0;

		/**
		* @method flush
		* */
		virtual bool flush() = 0;

		/**
		* @method advance frame
		* */
		virtual bool advance() = 0;

		/**
		* @method output frame buffer
		*/
		virtual OutputBuffer output() = 0;

		/**
		* @method release frame buffer
		* */
		virtual void release(OutputBuffer& buffer) = 0;

		/**
		* @method set_frame_size set audio frame buffer size
		* */
		virtual void set_frame_size(uint32_t size) = 0;

		/**
		* @method set_threads set soft multi thread run
		* */
		virtual void set_threads(uint32_t value) = 0;

		/**
		* @method set_background_run
		* */
		virtual void set_background_run(bool value) = 0;

		/**
		* @method convert_sample_data_to_nalu
		* */
		static bool convert_sample_data_to_nalu(Buffer& buffer);

		/**
		* @method convert_sample_data_to_mp4_style
		* */
		static bool convert_sample_data_to_mp4_style(Buffer& buffer);

		/**
		* @method parse_psp_pps
		* */
		static bool parse_avc_psp_pps(cBuffer& extradata, Buffer& out_psp, Buffer& out_pps);

		/**
		* @method create decoder
		* */
		static MediaCodec* create(MediaType type, MediaSource* source);

		/**
		* @method create create hardware decoder
		*/
		static MediaCodec* hardware(MediaType type, MediaSource* source);

		/**
		* @method software create software decoder
		* */
		static MediaCodec* software(MediaType type, MediaSource* source);

	protected:
		MediaCodec(Extractor* extractor);

		Extractor*  _extractor;
		Delegate*   _delegate;
		VideoColorFormat _color_format;
		uint64_t      _channel_layout;
		uint32_t      _channel_count;
		uint32_t      _frame_interval;
	};

}
#endif