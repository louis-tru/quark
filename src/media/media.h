/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK *****/

#ifndef __quark__media__media__
#define __quark__media__media__

#include "../util/util.h"
#include "../util/http.h"
#include "../render/pixel.h"

typedef struct AVPacket AVPacket;
typedef struct AVFrame AVFrame;
typedef struct AVCodecParameters AVCodecParameters;

namespace qk {

	enum MediaType {
		kVideo_MediaType = 1,
		kAudio_MediaType,
	};

	enum MediaSourceStatus {
		kNormal_MediaSourceStatus = 0,
		kOpening_MediaSourceStatus,
		kPlaying_MediaSourceStatus,
		kPaused_MediaSourceStatus,
		kError_MediaSourceStatus,
		kEOF_MediaSourceStatus,
	};

	class MediaCodec;

	class Qk_EXPORT MediaSource: public Object {
		Qk_DISABLE_COPY(MediaSource);
		Qk_DEFINE_INLINE_CLASS(Inl);
	public:

		struct StreamExtra {
			StreamExtra();
			~StreamExtra();
			StreamExtra(const StreamExtra& Extra);
			void set_codecpar(AVCodecParameters *codecpar);
			Array<char>        extradata; /* extradata */
			AVCodecParameters *codecpar;
		};

		struct Stream { // av stream info
			MediaType   type;             /* type */
			String      mime;             /* mime type */
			String      language;         /* language */
			int         codec_id;         /* codec id */
			uint32_t    codec_tag;        /* codec tag */
			int         format;           /* codec av format output */
			int         profile;          /* profile */
			int         level;            /* level */
			int         width;            /* video width */
			int         height;           /* video height */
			int         bitrate;          /* bit rate */
			int         sample_rate;      /* audio sample rate */
			int         channels;         /* audio channel count */
			int         channel_layout;   /* audio channel layout to enum AudioChannelLayoutMask */
			int64_t     duration;         /* duration of the stream, in stream time base. */
			uint32_t    avg_framerate[2]; /* video frame average framerate */
			uint32_t    time_base[2];     /* Unit of pts,dts on Packet,Frame by Numerator/Denominator (seconds) */
			uint32_t    index;            /* stream index in source */
			uint64_t    hash_code;        /* key params hash code */
			StreamExtra extra;
			inline bool operator==(const Stream& s) const {
				return hash_code == s.hash_code; }
			inline bool operator!=(const Stream& s)  const {
				return hash_code != s.hash_code; }
		};

		struct Program { // channel program info
			int         bitrate;
			uint32_t    width;
			uint32_t    height;
			String      codecs; // streams codec info
			Array<Stream> streams;
		};

		struct Packet { // av packet
			AVPacket* avpkt; // ff avpacket
			uint8_t*  data; // packet data
			uint32_t  size; // packet data size
			uint64_t  pts; // Presentation timestamp, (Microseconds)
			uint64_t  dts; // Decompression timestamp, (Microseconds)
			uint64_t  duration; // Duration of this packet, (Microseconds)
			int       flags; // keyframe flags
			~Packet();
			Packet* clone() const;
		};

		class Qk_EXPORT Extractor: public Object {
			Qk_DISABLE_COPY(Extractor);
		public:
			Qk_DEFINE_PROP_GET(MediaSource*, host);
			Qk_DEFINE_PROP_GET(MediaType, type, Const);
			Qk_DEFINE_PROP_GET(uint32_t, stream_index, Const);
			Qk_DEFINE_ACCE_GET(const Stream&, stream, Const);

			~Extractor();

			/**
			* @method streams() get streams
			*/
			inline cArray<Stream>& streams() const { return _streams; }

			/**
			* @method switch_stream
			*/
			bool switch_stream(uint32_t index);

			/**
			* @method advance
			*/
			Packet* advance();

		private:
			Extractor(MediaType type, MediaSource* host, Array<Stream>&& streams);
			void flush();

			Array<Stream>     _streams;
			List<Packet*>     _packets;
			uint64_t          _before_duration, _after_duration;
			List<Packet*>::Iterator _pkt; // next presentation packet

			friend class MediaSource;
			friend class MediaSource::Inl;
		};

		class Delegate {
		public:
			virtual void media_source_open(MediaSource* source) = 0;
			virtual void media_source_advance(MediaSource* source) = 0;
			virtual void media_source_eof(MediaSource* source) = 0;
			virtual void media_source_error(MediaSource* source, cError& err) = 0;
			virtual void media_source_switch(MediaSource* source, Extractor *ex) = 0;
		};

		// @props
		Qk_DEFINE_ACCE_GET(const URI&, uri, Const); //!< Getting media source path
		Qk_DEFINE_ACCE_GET(MediaSourceStatus, status, Const); //!< Getting current work status
		Qk_DEFINE_ACCE_GET(uint64_t, duration, Const); // !< source duration
		Qk_DEFINE_ACCE_GET(uint32_t, programs, Const); // !< Returns the programs count
		Qk_DEFINE_ACCE_GET(Extractor*, video); //!< extractor() must be called first
		Qk_DEFINE_ACCE_GET(Extractor*, audio); //!< extractor() must be called first
		Qk_DEFINE_ACCE_GET(bool, is_open, Const); // !< Getting whether it's open
		Qk_DEFINE_ACCE_GET(bool, is_pause, Const); // !< Getting whether it's pause state
		Qk_DEFINE_ACCESSOR(uint64_t, buffer_pkt_duration); // the length of the packet buffer time before and after, default 10 seconds

		MediaSource(cString& uri);
		~MediaSource() override;

		/**
		* @method open playing on new work thread or resume playing
		*/
		void play();

		/**
		* @method stop playing
		*/
		void stop();

		/**
		* @method pause playing
		*/
		void pause();

		/**
		* @method seek
		*/
		bool seek(uint64_t timeUs);

		/**
		* @method set_delegate # Setting media delegate
		*/
		void set_delegate(Delegate* delegate);

		/**
		* @method switch_program
		*/
		bool switch_program(uint32_t index);

		/**
		* @method extractor
		*/
		Extractor* extractor(MediaType type);

		/**
		 * @method remove_extractor() remove the a extractor by type
		*/
		void remove_extractor(MediaType type);

		/**
		 * @method flush flush all extractors and seek stream packet
		*/
		void flush();

	private:
		Inl*         _inl;
		friend class Extractor;
		friend class MediaCodec;
	};

	class Qk_EXPORT MediaCodec: public Object {
		Qk_DISABLE_COPY(MediaCodec);
	public:
		typedef MediaSource::Extractor Extractor;
		typedef MediaSource::Packet Packet;
		typedef MediaSource::Stream Stream;

		struct Frame {
			AVFrame*  avframe = 0; // ff avframe
			uint8_t** data;     // data items
			uint32_t* linesize; // data a linesize
			uint32_t  dataitems;// items size of data
			int64_t   pts;      // Presentation timestamp
			int64_t   pkt_duration; // Duration of frame on packet, (Microseconds)
			uint32_t  nb_samples; // number of audio samples (per channel) described by this frame
			uint32_t  width, height; // width and height of the video frame
			uint32_t  format; // frame output format, video to ColorType, audio default to signed 16 bits
			~Frame();
			static Frame* Make();
		};
		Qk_DEFINE_PROP_GET(MediaType, type, Const); //!< media type

 		/**
		 * @method stream() Returns stream infomaciton
		 */
		inline const Stream& stream() const { return _stream; }

		/**
		* @method open
		*/
		virtual bool is_open() const = 0;

		/**
		* @method open
		*/
		virtual bool open(const Stream *stream = nullptr) = 0;

		/**
		* @method close
		*/
		virtual void close() = 0;

		/**
		* @method flush
		*/
		virtual void flush() = 0;

		/**
		* @method finished() decoder is finished for work queue
		*/
		virtual bool finished() = 0;

		/**
		 * Success return zero
		 * @method send_packet send packet to codec
		 */
		virtual int send_packet(const Packet *pkt) = 0;

		/**
		* @method send_packet send packet to codec by extractor
		*/
		virtual int send_packet(Extractor *extractor) = 0;

		/**
		* @method receive_frame receive frame data
		*/
		virtual Frame* receive_frame() = 0;

		/**
		* @method set_threads set soft multi thread run
		*/
		virtual void set_threads(uint32_t value) = 0;

		/**
		* @method convert_sample_data_to_nalu
		*/
		static bool convert_sample_data_to_nalu(uint8_t *data, uint32_t size);

		/**
		* @method convert_sample_data_to_mp4_style
		*/
		static bool convert_sample_data_to_mp4_style(uint8_t *data, uint32_t size);

		/**
		* @method parse_psp_pps
		*/
		static bool parse_avc_psp_pps(cArray<char>& extradata, Buffer& out_psp, Buffer& out_pps);

		/**
		* @method create decoder
		*/
		static MediaCodec* create(MediaType type, MediaSource* source);

		/**
		 * @method frameToPixel frame convert to pixel and lose holding
		*/
		static Array<Pixel> frameToPixel(Frame *useFrame);

	protected:
		MediaCodec(const Stream &stream);
		Stream _stream; // media source stream info
	};

}
#endif
