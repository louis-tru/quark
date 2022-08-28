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

#include <unistd.h>
#include <fcntl.h>
#include "noug/util/platform/android_jni.h"
#include "noug/media/media_codec_inl.h"

#ifndef USE_FFMPEG_MEDIACODEC
#define USE_FFMPEG_MEDIACODEC 0
#endif

#if USE_FFMPEG_MEDIACODEC
extern "C" {
#include <libavcodec/jni.h>
#include <libavcodec/mediacodec_wrapper.h>
}
cChar* AMEDIAFORMAT_KEY_AAC_PROFILE = "aac-profile";
cChar* AMEDIAFORMAT_KEY_BIT_RATE = "bitrate";
cChar* AMEDIAFORMAT_KEY_CHANNEL_COUNT = "channel-count";
cChar* AMEDIAFORMAT_KEY_CHANNEL_MASK = "channel-mask";
cChar* AMEDIAFORMAT_KEY_COLOR_FORMAT = "color-format";
cChar* AMEDIAFORMAT_KEY_DURATION = "durationUs";
cChar* AMEDIAFORMAT_KEY_FLAC_COMPRESSION_LEVEL = "flac-compression-level";
cChar* AMEDIAFORMAT_KEY_FRAME_RATE = "frame-rate";
cChar* AMEDIAFORMAT_KEY_HEIGHT = "height";
cChar* AMEDIAFORMAT_KEY_IS_ADTS = "is-adts";
cChar* AMEDIAFORMAT_KEY_IS_AUTOSELECT = "is-autoselect";
cChar* AMEDIAFORMAT_KEY_IS_DEFAULT = "is-default";
cChar* AMEDIAFORMAT_KEY_IS_FORCED_SUBTITLE = "is-forced-subtitle";
cChar* AMEDIAFORMAT_KEY_I_FRAME_INTERVAL = "i-frame-interval";
cChar* AMEDIAFORMAT_KEY_LANGUAGE = "language";
cChar* AMEDIAFORMAT_KEY_MAX_HEIGHT = "max-height";
cChar* AMEDIAFORMAT_KEY_MAX_INPUT_SIZE = "max-input-size";
cChar* AMEDIAFORMAT_KEY_MAX_WIDTH = "max-width";
cChar* AMEDIAFORMAT_KEY_MIME = "mime";
cChar* AMEDIAFORMAT_KEY_PUSH_BLANK_BUFFERS_ON_STOP = "push-blank-buffers-on-shutdown";
cChar* AMEDIAFORMAT_KEY_REPEAT_PREVIOUS_FRAME_AFTER = "repeat-previous-frame-after";
cChar* AMEDIAFORMAT_KEY_SAMPLE_RATE = "sample-rate";
cChar* AMEDIAFORMAT_KEY_WIDTH = "width";
cChar* AMEDIAFORMAT_KEY_STRIDE = "stride";
enum {
	AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM = 4,
	AMEDIACODEC_CONFIGURE_FLAG_ENCODE = 1,
	AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED = -3,
	AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED = -2,
	AMEDIACODEC_INFO_TRY_AGAIN_LATER = -1
};
#define AMediaExtractor FFAMediaExtractor
#define AMediaCodec FFAMediaCodec
#define AMediaCodecBufferInfo FFAMediaCodecBufferInfo
#define AMediaFormat FFAMediaFormat
#define AMediaCodec_getOutputFormat ff_AMediaCodec_getOutputFormat
#define AMediaCodec_flush ff_AMediaCodec_flush
#define AMediaCodec_dequeueInputBuffer ff_AMediaCodec_dequeueInputBuffer
#define AMediaCodec_getInputBuffer ff_AMediaCodec_getInputBuffer
#define AMediaCodec_queueInputBuffer ff_AMediaCodec_queueInputBuffer
#define AMediaCodec_dequeueOutputBuffer ff_AMediaCodec_dequeueOutputBuffer
#define AMediaCodec_getOutputBuffer ff_AMediaCodec_getOutputBuffer
#define AMediaCodec_releaseOutputBuffer ff_AMediaCodec_releaseOutputBuffer
#define AMediaCodec_configure ff_AMediaCodec_configure
#define AMediaCodec_start ff_AMediaCodec_start
#define AMediaCodec_stop ff_AMediaCodec_stop
#define AMediaCodec_delete ff_AMediaCodec_delete
#define AMediaCodec_createDecoderByType ff_AMediaCodec_createDecoderByType
#define AMediaFormat_toString ff_AMediaFormat_toString
#define AMediaFormat_new ff_AMediaFormat_new
#define AMediaFormat_delete ff_AMediaFormat_delete
#define AMediaFormat_getInt32 ff_AMediaFormat_getInt32
#define AMediaFormat_setString ff_AMediaFormat_setString
#define AMediaFormat_setInt64 ff_AMediaFormat_setInt64
#define AMediaFormat_setInt32 ff_AMediaFormat_setInt32
#define AMediaFormat_setBuffer ff_AMediaFormat_setBuffer
#else
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaExtractor.h>
#endif

// ------------------------------------------------------------------------------------------------

namespace noug {

	#if DEBUG && !USE_FFMPEG_MEDIACODEC

		static AMediaExtractor* _TEST_ex = NULL;

		static void _TEST_init_AMediaExtractor(cString& path, uint32_t select_track) {
			if (_TEST_ex == NULL) {
				int fd = ::open(fs_fallback_c(path), 0);
				if ( fd <= 0 ) {
					return;
				}
				_TEST_ex = AMediaExtractor_new();
				AMediaExtractor_setDataSourceFd(_TEST_ex, fd, 0, FileStat(path).size());
				::close(fd);
			}
			int err = AMediaExtractor_selectTrack(_TEST_ex, select_track);
			N_Assert(err == 0);
		}

		static void _TEST_get_sample_data(uint8_t* out, uint32_t size, uint& sample_size) {
			Buffer buf(size);
			ssize_t sample_size2 = AMediaExtractor_readSampleData(_TEST_ex, (uint8_t*)*buf, size);

			uint32_t j[10] = { 0 };
			uint32_t o = 0;
			uint8_t* cmp = (uint8_t*)*buf;

			for (int i = 0; i < sample_size; i++) {
				if (cmp[i] != out[i]) {
					j[o] = i;
					o++;
				}
			}
			N_DEBUG("cmp: %d|%d|%d|%d|%d, cmp_s: %d, sample_size:%d|%d",
							j[0], j[1], j[2], j[3], j[4], o, sample_size, sample_size2);

			memcpy(out, *buf, sample_size2);
			AMediaExtractor_advance(_TEST_ex);
			sample_size = sample_size2;
		}

		#else
			#define _TEST_init_AMediaExtractor(path, select_track) (void*)(0)
			#define _TEST_get_sample_data(out, size, sample_size) (void*)(0)
		#endif

		// @func init_ffmpeg_jni
		static void init_ffmpeg_jni() {
		#if USE_FFMPEG_MEDIACODEC
			static bool has_init = false;
			if (!has_init) {
				has_init = true;
				if ( fx_jni_set_java_vm(JNI::jvm(), NULL) != 0 ) {
					N_ERR("x_jni_set_java_vm(), unsuccessful." );
				}
			}
		#endif 
	}

	/**
	* @class AndroidHardwareMediaCodec
	* */
	class AndroidHardwareMediaCodec: public MediaCodec {
		public:

			/**
			* @constructor
			*/
			AndroidHardwareMediaCodec(Extractor* extractor, AMediaCodec* codec, AMediaFormat* format)
				: MediaCodec(extractor)
				, _format(format)
				, _codec(codec)
				, _eof_flags(false)
				, _video_width(0)
				, _video_height(0)
				, _audio_frame_size(0)
				, _presentation_time(0)
				, _is_open(true)
			{
				const TrackInfo& track = extractor->track();

				if ( type() == MEDIA_TYPE_VIDEO ) {
					fetch_video_color_format();
					_video_width = track.width;
					_video_height = track.height;
				} else {
					_channel_layout = track.channel_layout;
					_channel_count  = track.channel_count;
				}
			}

			/**
			* @destructor
			*/
			virtual ~AndroidHardwareMediaCodec() {
				AMediaCodec_delete(_codec);   _codec = nullptr;
				AMediaFormat_delete(_format); _format = nullptr;
			}

			/**
			* @func fetch_video_color_format
			* */
			void fetch_video_color_format() {
				// AMediaFormat* format = AMediaCodec_getOutputFormat(_codec);
				int num;
				if ( AMediaFormat_getInt32(_format, AMEDIAFORMAT_KEY_COLOR_FORMAT, &num) ) {
					switch (num) {
						case 17: _color_format = VIDEO_COLOR_FORMAT_YUV411P; break;
						case 19: _color_format = VIDEO_COLOR_FORMAT_YUV420P; break;
						case 21: _color_format = VIDEO_COLOR_FORMAT_YUV420SP; break;
						default: _color_format = VIDEO_COLOR_FORMAT_INVALID; break;
					}
				}
			}

			/**
			* @overwrite
			*/
			virtual bool open() {
				if ( !_is_open ) {
					int result = AMediaCodec_configure(_codec, _format, nullptr, nullptr, 0);
					if ( result == 0 && AMediaCodec_start(_codec) == 0 ) {
						_is_open = true;
					}
					bool ok = flush();
				}
				return _is_open;
			}

			/**
			* @overwrite
			*/
			virtual bool close() {
				if ( _is_open ) {
					bool ok = flush();
					if ( AMediaCodec_stop(_codec) == 0 ) {
						_is_open = false;
					}
				}
				return !_is_open;
			}
			
			/**
			* @overwrite
			*/
			virtual bool flush() {
				_presentation_time = 0;
				return AMediaCodec_flush(_codec) == 0;
			}

			/**
			* @overwrite
			* */
			virtual bool advance() {

				if ( _extractor->advance() ) {
					_eof_flags = false;

					ssize_t bufidx = AMediaCodec_dequeueInputBuffer(_codec, 0);
					if ( bufidx >= 0 ) {
						size_t bufsize;
						uint8_t * buf = AMediaCodec_getInputBuffer(_codec, bufidx, &bufsize);
						uint32_t sample_size = _extractor->deplete_sample((Char*)buf, bufsize);
						int sample_flags = _extractor->eof_flags() ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0;
						uint64_t sample_time = _extractor->sample_time();

						if ( sample_time == 0 ) {
							N_DEBUG("advance:0");
						}
						if (sample_flags) {
							N_DEBUG("%s", "eos flags");
						}
						if ( sample_size ) {
							if ( type() == MEDIA_TYPE_VIDEO ) {
								WeakBuffer buffer((Char*)buf, sample_size);
								MediaCodec::convert_sample_data_to_nalu(buffer);
							}
							AMediaCodec_queueInputBuffer( _codec, bufidx, 0,
																						sample_size, sample_time, sample_flags);
							return true;
						}
					}
				}
				return false;
			}

			/**
			* @overwrite
			*/
			virtual OutputBuffer output() {
				if ( ! _eof_flags ) {
					AMediaCodecBufferInfo info;
					ssize_t status = AMediaCodec_dequeueOutputBuffer(_codec, &info, 0);

					if ( status >= 0 ) {
						if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
							N_DEBUG("output EOS");
							_eof_flags = true;
							_delegate->media_decoder_eof(this, info.presentationTimeUs);
						}
						int64_t presentation = info.presentationTimeUs;

						if ( presentation == 0 ) {
							N_DEBUG("output:0");
						}

						size_t size;
						uint8_t * buffer = AMediaCodec_getOutputBuffer(_codec, status, &size);

						if ( size ) {
							OutputBuffer out;
							out.total = info.size;
							out.time = presentation;
							out.index = status;

							_presentation_time = out.time;
							if ( type() == MEDIA_TYPE_AUDIO ) {
								if (_audio_frame_size == 0) {
									inl_set_frame_size(info.size);
								}
								out.data[0] = buffer;
								out.linesize[0] = info.size;
							} else {
								if (_color_format == VIDEO_COLOR_FORMAT_YUV420P) {
									out.linesize[0] = _video_width * _video_height;
									out.linesize[1] = out.linesize[0] / 4;
									out.linesize[2] = out.linesize[1];
									out.data[0] = buffer;                                     // y
									out.data[1] = buffer + out.linesize[0];                   // u
									out.data[2] = buffer + out.linesize[0] + out.linesize[1]; // v
								} else { // YUV420SP
									out.linesize[0] = _video_width * _video_height;
									out.linesize[1] = out.total - out.linesize[0];
									out.data[0] = buffer;                                     // y
									out.data[1] = buffer + out.linesize[0];                   // uv
								}
								if ( out.time == Uint64::max ) { //  Unknown time frame
									out.time = _presentation_time + _frame_interval;
								}
							}
							return out;
						} else {
							AMediaCodec_releaseOutputBuffer(_codec, status, true);
						}
					} else if ( status == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED ) {
						N_DEBUG("output buffers changed");
					} else if ( status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED ) {
						AMediaFormat* format = AMediaCodec_getOutputFormat(_codec);
						N_DEBUG("format changed to: %s", AMediaFormat_toString(format));
						AMediaFormat_delete(format);
					} else if ( status == AMEDIACODEC_INFO_TRY_AGAIN_LATER ) {
						// N_DEBUG("no output buffer right now");
					} else {
						N_DEBUG("unexpected info code: %d", status);
					}
				}
				return OutputBuffer();
			}

			/**
			* @overwrite
			*/
			virtual void release(OutputBuffer& buffer) {
				if ( buffer.total ) {
					AMediaCodec_releaseOutputBuffer(_codec, buffer.index, true);
					memset(&buffer, 0, sizeof(OutputBuffer));
				}
			}

			void inl_set_frame_size(uint32_t size) {
				_audio_frame_size = size;
				// compute audio frame interval
				const TrackInfo &track = extractor()->track();
				uint64_t second_size = track.sample_rate * _channel_count * 2;
				_frame_interval = uint64(_audio_frame_size) * 1000 * 1000 / second_size;
			}

			/**
			* @overwrite
			* */
			virtual void set_frame_size(uint32_t size) { }

			/**
			* @overwrite
			* */
			virtual void set_threads(uint32_t value) { }

			/**
			* @overwrite
			* */
			virtual void set_background_run(bool value) { }

			// --------------------- @overwrite end ---------------------

		private:
			AMediaFormat* _format;
			AMediaCodec*  _codec;
			bool          _eof_flags;
			uint32_t          _video_width;
			uint32_t          _video_height;
			uint32_t          _audio_frame_size;
			uint64_t        _presentation_time;
			bool          _is_open;
	};

	/**
	* @func hardware
	*/
	MediaCodec* MediaCodec::hardware(MediaType type, MultimediaSource* source) {
		init_ffmpeg_jni();
		
		AndroidHardwareMediaCodec* rv = NULL;
		Extractor* ex = source->extractor(type);

		if ( ex ) {
			const TrackInfo& track = ex->track();
			cChar* mime = NULL;

			if (type == MEDIA_TYPE_AUDIO) {
				if (track.mime == "audio/aac" && track.extradata.length()) { // aac
					mime = "audio/mp4a-latm";
				} else {
					return NULL;
				}
			} else if (type == MEDIA_TYPE_VIDEO) {
				if (track.mime == "video/h264" || track.mime == "video/avc") { // h.264
					mime = "video/avc";
				} else if (track.mime == "video/hevc" || track.mime == "video/h265") { // h.265
					mime = "video/hevc";
				} else {
					return NULL;
				}
			} else {
				return NULL;
			}

			AMediaCodec* codec = AMediaCodec_createDecoderByType( mime );

			if ( codec ) {
				AMediaFormat* format = AMediaFormat_new();
				AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, mime);
				AMediaFormat_setInt64(format, AMEDIAFORMAT_KEY_DURATION, source->duration());

				if (type == MEDIA_TYPE_AUDIO) {
					//_TEST_init_AMediaExtractor("/sdcard/Download/av.1.0.ts", 1);
					//format = AMediaExtractor_getTrackFormat(_TEST_ex, 1);
					AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_SAMPLE_RATE, track.sample_rate);
					AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_CHANNEL_COUNT, track.channel_count);
					AMediaFormat_setBuffer(format, "csd-0", (void*)*track.extradata, track.extradata.length());
				} else {
					AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, track.width);
					AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, track.height);
					AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, 19);
					Buffer csd_0, csd_1;
					if ( parse_avc_psp_pps(track.extradata, csd_0, csd_1) ) {
						AMediaFormat_setBuffer(format, "csd-0", *csd_0, csd_0.length());
						AMediaFormat_setBuffer(format, "csd-1", *csd_1, csd_1.length());
					}
					// _TEST_init_AMediaExtractor(source->uri().href(), 0);
					// format = AMediaExtractor_getTrackFormat(_TEST_ex, 0);
				}

				N_DEBUG("%s", AMediaFormat_toString(format));
				int result = AMediaCodec_configure(codec, format, nullptr, nullptr, 0);

				if ( result == 0 && AMediaCodec_start(codec) == 0 ) {
					rv = new AndroidHardwareMediaCodec(ex, codec, format);
				} else {
					N_ERR("Unable to configure and run the decoder");
					AMediaCodec_delete(codec);
					AMediaFormat_delete(format);
				}
			} else {
				N_ERR("cannot create decoder");
			}
		}
		return rv;
	}

}