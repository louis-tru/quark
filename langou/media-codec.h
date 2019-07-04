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

#ifndef __langou__media_codec__
#define __langou__media_codec__

#include "langou/utils/util.h"
#include "langou/utils/string.h"
#include "langou/utils/buffer.h"
#include "langou/utils/http.h"
#include "langou/image-codec.h"
#include "langou/media.h"

typedef struct AVStream AVStream;

XX_NS(langou)

enum MultimediaSourceStatus {
	MULTIMEDIA_SOURCE_STATUS_UNINITIALIZED = 0,
	MULTIMEDIA_SOURCE_STATUS_READYING,
	MULTIMEDIA_SOURCE_STATUS_READY,
	MULTIMEDIA_SOURCE_STATUS_WAIT,
	MULTIMEDIA_SOURCE_STATUS_FAULT,
	MULTIMEDIA_SOURCE_STATUS_EOF,
};

enum MediaType {
	MEDIA_TYPE_AUDIO,
	MEDIA_TYPE_VIDEO,
};

enum AudioChannelMask {
	CH_INVALID                = 0,
	CH_FRONT_LEFT             = 0x00000001,
	CH_FRONT_RIGHT            = 0x00000002,
	CH_FRONT_CENTER           = 0x00000004,
	CH_LOW_FREQUENCY          = 0x00000008,
	CH_BACK_LEFT              = 0x00000010,
	CH_BACK_RIGHT             = 0x00000020,
	CH_FRONT_LEFT_OF_CENTER   = 0x00000040,
	CH_FRONT_RIGHT_OF_CENTER  = 0x00000080,
	CH_BACK_CENTER            = 0x00000100,
	CH_SIDE_LEFT              = 0x00000200,
	CH_SIDE_RIGHT             = 0x00000400,
	CH_TOP_CENTER             = 0x00000800,
	CH_TOP_FRONT_LEFT         = 0x00001000,
	CH_TOP_FRONT_CENTER       = 0x00002000,
	CH_TOP_FRONT_RIGHT        = 0x00004000,
	CH_TOP_BACK_LEFT          = 0x00008000,
	CH_TOP_BACK_CENTER        = 0x00010000,
	CH_TOP_BACK_RIGHT         = 0x00020000,
};

enum VideoColorFormat {
	VIDEO_COLOR_FORMAT_YUV420P = PixelData::YUV420P,
	VIDEO_COLOR_FORMAT_YUV420SP = PixelData::YUV420SP,
	VIDEO_COLOR_FORMAT_YUV411P = PixelData::YUV411P,
	VIDEO_COLOR_FORMAT_YUV411SP = PixelData::YUV411SP,
	VIDEO_COLOR_FORMAT_INVALID = PixelData::INVALID,
};

/**
 * @class MultimediaSource
 */
class XX_EXPORT MultimediaSource: public Object {
	XX_HIDDEN_ALL_COPY(MultimediaSource);
	XX_DEFINE_INLINE_CLASS(Inl);
 public:
	
	struct XX_EXPORT TrackInfo {
		TrackInfo();
		TrackInfo(const TrackInfo&);
		uint        track;            /* 轨道在源中的索引 */
		MediaType   type;             /* type */
		String      mime;             /* mime类型 */
		int         codec_id;         /* codec id */
		uint        codec_tag;        /* codec tag */
		int         format;           /* format */
		int         profile;          /* profile */
		int         level;            /* level */
		uint        width;            /* 输出图像宽度 */
		uint        height;           /* 输出图像高度 */
		String      language;         /* 语言 */
		uint        bitrate;          /* 码率 */
		uint        sample_rate;      /* 声音采样率 */
		uint        channel_count;    /* 声音声道数量 */
		uint64      channel_layout;   /* channel_layout */
		uint        frame_interval;   /* 图像帧时间间隔 */
		Buffer      extradata;        /* extradata */
	};
	
	struct XX_EXPORT BitRateInfo {  /* 码率 */
		int     bandwidth;
		uint    width;
		uint    height;
		String  codecs;
		Array<TrackInfo>  tracks;
	};
	
	class XX_EXPORT Delegate {
	 public:
		virtual void multimedia_source_ready(MultimediaSource* source) = 0;
		virtual void multimedia_source_wait_buffer(MultimediaSource* source, float process) = 0;
		virtual void multimedia_source_eof(MultimediaSource* source) = 0;
		virtual void multimedia_source_error(MultimediaSource* source, cError& err) = 0;
	};
	
	/**
	 * @class Extractor
	 */
	class XX_EXPORT Extractor: public Object {
		XX_HIDDEN_ALL_COPY(Extractor);
	 public:
		
		/**
		 * @func track_count
		 */
		inline uint track_count() const { return m_tracks.length(); }
		
		/**
		 * @func track_index current
		 */
		inline uint track_index() const { return m_track_index; }
		
		/**
		 * @func track current
		 * */
		inline const TrackInfo& track() const { return m_tracks[m_track_index]; }
		
		/**
		 * @func track get track info with index
		 */
		inline const TrackInfo& track(uint index) const { return m_tracks[index]; }
		
		/**
		 * @func host
		 */
		inline MultimediaSource* host() const { return m_host; }
		
		/**
		 * @func type
		 */
		inline MediaType type() const { return m_type; }
		
		/**
		 * @func frame_interval
		 * */
		inline uint frame_interval() const { return m_tracks[0].frame_interval; }
		
		/**
		 * @func select_track
		 */
		bool select_track(uint index);
		
		/**
		 * @func sample_time
		 * */
		inline uint64 sample_time() const { return m_sample_data.time; }
		
		/**
		 * @func sample_d_time
		 * */
		inline uint64 sample_d_time() const { return m_sample_data.d_time; }
		
		/**
		 * @func sample_data
		 * */
		inline WeakBuffer sample_data() const {
			return WeakBuffer(m_sample_data.data, m_sample_data.size);
		}
		
		/**
		 * @func sample_size
		 * */
		inline uint sample_size() const { return m_sample_data.size; }

		/**
		 * @func presentation_time
		 * */
		inline int sample_flags() const { return m_sample_data.flags; }

		/**
		 * @func eof_flags
		 * */
		inline bool eof_flags() const { return m_eof_flags; }

		/**
		 * @func deplete_sample
		 * */
		uint deplete_sample(char* out, uint size);

		/**
		 * @func deplete_sample
		 * */
		uint deplete_sample(Buffer& out);
		
		/**
		 * @func deplete_sample
		 * */
		uint deplete_sample(uint size);
		
		/**
		 * @func deplete_sample
		 * */
		inline uint deplete_sample() {
			return deplete_sample(m_sample_data.size);
		}
		
		/**
		 * @func advance
		 * */
		bool advance();

		/**
		 * @func set_disable
		 */
		inline void set_disable(bool value) {  m_disable = value; }

		/**
		 * @func is_disable default true
		 * */
		inline bool is_disable() const { return m_disable; }
		
	 private:
		
		Extractor(MediaType type, MultimediaSource* host, Array<TrackInfo>&& tracks);

		struct SampleData {
			Buffer  _buf;
			char*   data;
			uint    size;
			uint64  time;
			uint64  d_time;
			int     flags;
		};
		
		MultimediaSource* m_host;
		MediaType         m_type;
		uint              m_track_index;
		Array<TrackInfo>  m_tracks;
		Array<SampleData> m_sample_data_cache;
		uint              m_sample_index_cache;
		uint              m_sample_count_cache;
		SampleData        m_sample_data;
		bool              m_eof_flags;
		bool              m_disable;
		friend class MultimediaSource;
		friend class MultimediaSource::Inl;
		// @end
	};
	
	MultimediaSource(cString& uri, RunLoop* loop = RunLoop::current());
	
	/**
	 * @destructor
	 */
	virtual ~MultimediaSource();
	
	/**
	 * @func set_delegate # Setting media delegate
	 */
	void set_delegate(Delegate* delegate);
	
	/**
	 * @func disable_wait_buffer
	 */
	void disable_wait_buffer(bool value);
	
	/**
	 * @func source # Getting media source path
	 */
	const URI& uri() const;
	
	/**
	 * @func status # Getting current work status
	 */
	MultimediaSourceStatus status() const;
	
	/**
	 * @func duration
	 */
	uint64 duration() const;
	
	/**
	 * @func bit_rate_index
	 */
	uint bit_rate_index() const;
	
	/**
	 * @func bit_rate
	 */
	const Array<BitRateInfo>& bit_rate() const;
	
	/**
	 * @func select_bit_rate
	 */
	bool select_bit_rate(int index);
	
	/**
	 * @func extractor
	 */
	Extractor* extractor(MediaType type);
	
	/**
	 * @func seek
	 * */
	bool seek(uint64 timeUs);
	
	/**
	 * @func start
	 */
	void start();

	/**
	 * @func stop
	 */
	void stop();
	
	/**
	 * @func is_active
	 */
	bool is_active();
	
	/**
	 * @func get_stream
	 */
	AVStream* get_stream(const TrackInfo& track);
	
 private:
	friend class Extractor;
	friend class MediaCodec;
	Inl*         m_inl;
};

/**
 * @class MediaCodec
 */
class XX_EXPORT MediaCodec: public Object {
	XX_HIDDEN_ALL_COPY(MediaCodec);
 public:

	typedef MultimediaSource::Extractor Extractor;
	
	struct XX_EXPORT OutputBuffer {
		OutputBuffer();
		OutputBuffer(const OutputBuffer& buffer);
		byte*   data[8];      /* 数据Buffer */
		uint    linesize[8];  /* 数据大小 */
		uint    total;        /* 数据总大小 */
		uint64  time;         /* 演示时间 */
		int     index;        /* 数据Buffer在解码器中的索引 */
	};
	
	class XX_EXPORT Delegate {
	 public:
		virtual void media_decoder_eof(MediaCodec* de, uint64 timeUs) { }
		virtual void media_decoder_error(MediaCodec* de, cError& err) { }
	};
	
	/**
	 * @func type
	 * */
	inline MediaType type() const { return m_extractor->type(); }
	
	/**
	 * @func set_delegate
	 */
	void set_delegate(Delegate* delegate);
	
	/**
	 * @func source # return decoder source path
	 */
	inline MultimediaSource* source() const { return m_extractor->host(); }
	
	/**
	 * @func extractor
	 */
	inline Extractor* extractor() const { return m_extractor; }
	
	/**
	 * @func color_format decode output video color format 
	 */
	inline VideoColorFormat color_format() const { return m_color_format; }

	/**
	 * @func channel_count
	 * */
	inline uint channel_count() const { return m_channel_count; }

	/**
	 * @func channel_layout
	 * */
	inline uint64 channel_layout() const { return m_channel_layout; }

	/**
	 * @func frame_interval
	 * */
	inline uint frame_interval() const { return m_frame_interval; }
	
	/**
	 * @func open
	 */
	virtual bool open() = 0;
	
	/**
	 * @func close
	 */
	virtual bool close() = 0;
	
	/**
	 * @func flush
	 * */
	virtual bool flush() = 0;

	/**
	 * @func advance frame
	 * */
	virtual bool advance() = 0;
	
	/**
	 * @func output frame buffer
	 */
	virtual OutputBuffer output() = 0;
	
	/**
	 * @func release frame buffer
	 * */
	virtual void release(OutputBuffer& buffer) = 0;

	/**
	 * @func set_frame_size set audio frame buffer size
	 * */
	virtual void set_frame_size(uint size) = 0;

	/**
	 * @func set_threads set soft multi thread run
	 * */
	virtual void set_threads(uint value) = 0;

	/**
	 * @func set_background_run
	 * */
	virtual void set_background_run(bool value) = 0;
	
	/**
	 * @func convert_sample_data_to_nalu
	 * */
	static bool convert_sample_data_to_nalu(Buffer& buffer);

	/**
	 * @func convert_sample_data_to_mp4_style
	 * */
	static bool convert_sample_data_to_mp4_style(Buffer& buffer);
	
	/**
	 * @func parse_psp_pps
	 * */
	static bool parse_avc_psp_pps(cBuffer& extradata, Buffer& out_psp, Buffer& out_pps);
	
	/**
	 * @func create decoder
	 * */
	static MediaCodec* create(MediaType type, MultimediaSource* source);
	
	/**
	 * @func create create hardware decoder
	 */
	static MediaCodec* hardware(MediaType type, MultimediaSource* source);
	
	/**
	 * @func software create software decoder
	 * */
	static MediaCodec* software(MediaType type, MultimediaSource* source);
	
 protected:

	MediaCodec(Extractor* extractor);
	
	Extractor*  m_extractor;
	Delegate*   m_delegate;
	VideoColorFormat m_color_format;
	uint64      m_channel_layout;
	uint        m_channel_count;
	uint        m_frame_interval;
};

XX_END
#endif
