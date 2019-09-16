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
#include "niutils/android-jni.h"
#include "ngui/media-codec-1.h"

#ifndef USE_FFMPEG_MEDIACODEC
#define USE_FFMPEG_MEDIACODEC 0
#endif

#if USE_FFMPEG_MEDIACODEC
extern "C" {
#include <libavcodec/jni.h>
#include <libavcodec/mediacodec_wrapper.h>
}
const char* AMEDIAFORMAT_KEY_AAC_PROFILE = "aac-profile";
const char* AMEDIAFORMAT_KEY_BIT_RATE = "bitrate";
const char* AMEDIAFORMAT_KEY_CHANNEL_COUNT = "channel-count";
const char* AMEDIAFORMAT_KEY_CHANNEL_MASK = "channel-mask";
const char* AMEDIAFORMAT_KEY_COLOR_FORMAT = "color-format";
const char* AMEDIAFORMAT_KEY_DURATION = "durationUs";
const char* AMEDIAFORMAT_KEY_FLAC_COMPRESSION_LEVEL = "flac-compression-level";
const char* AMEDIAFORMAT_KEY_FRAME_RATE = "frame-rate";
const char* AMEDIAFORMAT_KEY_HEIGHT = "height";
const char* AMEDIAFORMAT_KEY_IS_ADTS = "is-adts";
const char* AMEDIAFORMAT_KEY_IS_AUTOSELECT = "is-autoselect";
const char* AMEDIAFORMAT_KEY_IS_DEFAULT = "is-default";
const char* AMEDIAFORMAT_KEY_IS_FORCED_SUBTITLE = "is-forced-subtitle";
const char* AMEDIAFORMAT_KEY_I_FRAME_INTERVAL = "i-frame-interval";
const char* AMEDIAFORMAT_KEY_LANGUAGE = "language";
const char* AMEDIAFORMAT_KEY_MAX_HEIGHT = "max-height";
const char* AMEDIAFORMAT_KEY_MAX_INPUT_SIZE = "max-input-size";
const char* AMEDIAFORMAT_KEY_MAX_WIDTH = "max-width";
const char* AMEDIAFORMAT_KEY_MIME = "mime";
const char* AMEDIAFORMAT_KEY_PUSH_BLANK_BUFFERS_ON_STOP = "push-blank-buffers-on-shutdown";
const char* AMEDIAFORMAT_KEY_REPEAT_PREVIOUS_FRAME_AFTER = "repeat-previous-frame-after";
const char* AMEDIAFORMAT_KEY_SAMPLE_RATE = "sample-rate";
const char* AMEDIAFORMAT_KEY_WIDTH = "width";
const char* AMEDIAFORMAT_KEY_STRIDE = "stride";
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

XX_NS(ngui)

#if DEBUG && !USE_FFMPEG_MEDIACODEC

static AMediaExtractor* _TEST_ex = NULL;

static void _TEST_init_AMediaExtractor(cString& path, uint select_track) {
	if (_TEST_ex == NULL) {
		int fd = ::open(Path::fallback_c(path), 0);
		if ( fd <= 0 ) {
			return;
		}
		_TEST_ex = AMediaExtractor_new();
		AMediaExtractor_setDataSourceFd(_TEST_ex, fd, 0, FileStat(path).size());
		::close(fd);
	}
	int err = AMediaExtractor_selectTrack(_TEST_ex, select_track);
	XX_ASSERT(err == 0);
}

static void _TEST_get_sample_data(byte* out, uint size, uint& sample_size) {
	Buffer buf(size);
	ssize_t sample_size2 = AMediaExtractor_readSampleData(_TEST_ex, (byte*)*buf, size);

	uint j[10] = { 0 };
	uint o = 0;
	byte* cmp = (byte*)*buf;

	for (int i = 0; i < sample_size; i++) {
		if (cmp[i] != out[i]) {
			j[o] = i;
			o++;
		}
	}
	XX_DEBUG("cmp: %d|%d|%d|%d|%d, cmp_s: %d, sample_size:%d|%d",
					j[0], j[1], j[2], j[3], j[4], o, sample_size, sample_size2);

	memcpy(out, *buf, sample_size2);
	AMediaExtractor_advance(_TEST_ex);
	sample_size = sample_size2;
}

#else
# define _TEST_init_AMediaExtractor(path, select_track) (void*)(0)
# define _TEST_get_sample_data(out, size, sample_size) (void*)(0)
#endif

// @func init_ffmpeg_jni
static void init_ffmpeg_jni() {
#if USE_FFMPEG_MEDIACODEC
	static bool has_init = false;
	if (!has_init) {
		has_init = true;
		if ( xx_jni_set_java_vm(JNI::jvm(), NULL) != 0 ) {
			XX_ERR( "x_jni_set_java_vm(), unsuccessful." );
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
		, m_format(format)
		, m_codec(codec)
		, m_eof_flags(false)
		, m_video_width(0)
		, m_video_height(0)
		, m_audio_frame_size(0)
		, m_presentation_time(0)
		, m_is_open(true)
	{
		const TrackInfo& track = extractor->track();

		if ( type() == MEDIA_TYPE_VIDEO ) {
			fetch_video_color_format();
			m_video_width = track.width;
			m_video_height = track.height;
		} else {
			m_channel_layout = track.channel_layout;
			m_channel_count  = track.channel_count;
		}
	}

	/**
	 * @destructor
	 */
	virtual ~AndroidHardwareMediaCodec() {
		AMediaCodec_delete(m_codec);   m_codec = nullptr;
		AMediaFormat_delete(m_format); m_format = nullptr;
	}

	/**
	 * @func fetch_video_color_format
	 * */
	void fetch_video_color_format() {
		// AMediaFormat* format = AMediaCodec_getOutputFormat(m_codec);
		int num;
		if ( AMediaFormat_getInt32(m_format, AMEDIAFORMAT_KEY_COLOR_FORMAT, &num) ) {
			switch (num) {
				case 17: m_color_format = VIDEO_COLOR_FORMAT_YUV411P; break;
				case 19: m_color_format = VIDEO_COLOR_FORMAT_YUV420P; break;
				case 21: m_color_format = VIDEO_COLOR_FORMAT_YUV420SP; break;
				default: m_color_format = VIDEO_COLOR_FORMAT_INVALID; break;
			}
		}
	}

	/**
	 * @overwrite
	 */
	virtual bool open() {
		if ( !m_is_open ) {
			int result = AMediaCodec_configure(m_codec, m_format, nullptr, nullptr, 0);
			if ( result == 0 && AMediaCodec_start(m_codec) == 0 ) {
				m_is_open = true;
			}
			bool ok = flush();
		}
		return m_is_open;
	}

	/**
	 * @overwrite
	 */
	virtual bool close() {
		if ( m_is_open ) {
			bool ok = flush();
			if ( AMediaCodec_stop(m_codec) == 0 ) {
				m_is_open = false;
			}
		}
		return !m_is_open;
	}
	
	/**
	 * @overwrite
	 */
	virtual bool flush() {
		m_presentation_time = 0;
		return AMediaCodec_flush(m_codec) == 0;
	}

	/**
	 * @overwrite
	 * */
	virtual bool advance() {

		if ( m_extractor->advance() ) {
			m_eof_flags = false;

			ssize_t bufidx = AMediaCodec_dequeueInputBuffer(m_codec, 0);
			if ( bufidx >= 0 ) {
				size_t bufsize;
				uint8_t* buf = AMediaCodec_getInputBuffer(m_codec, bufidx, &bufsize);
				uint sample_size = m_extractor->deplete_sample((char*)buf, bufsize);
				int sample_flags = m_extractor->eof_flags() ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0;
				uint64 sample_time = m_extractor->sample_time();

				if ( sample_time == 0 ) {
					XX_DEBUG("advance:0");
				}
				if (sample_flags) {
					XX_DEBUG("%s", "eos flags");
				}
				if ( sample_size ) {
					if ( type() == MEDIA_TYPE_VIDEO ) {
						WeakBuffer buffer((char*)buf, sample_size);
						MediaCodec::convert_sample_data_to_nalu(buffer);
					}
					AMediaCodec_queueInputBuffer( m_codec, bufidx, 0,
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
		if ( ! m_eof_flags ) {
			AMediaCodecBufferInfo info;
			ssize_t status = AMediaCodec_dequeueOutputBuffer(m_codec, &info, 0);

			if ( status >= 0 ) {
				if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
					XX_DEBUG("output EOS");
					m_eof_flags = true;
					m_delegate->media_decoder_eof(this, info.presentationTimeUs);
				}
				int64_t presentation = info.presentationTimeUs;

				if ( presentation == 0 ) {
					XX_DEBUG("output:0");
				}

				size_t size;
				uint8_t* buffer = AMediaCodec_getOutputBuffer(m_codec, status, &size);

				if ( size ) {
					OutputBuffer out;
					out.total = info.size;
					out.time = presentation;
					out.index = status;

					m_presentation_time = out.time;
					if ( type() == MEDIA_TYPE_AUDIO ) {
						if (m_audio_frame_size == 0) {
							inl_set_frame_size(info.size);
						}
						out.data[0] = buffer;
						out.linesize[0] = info.size;
					} else {
						if (m_color_format == VIDEO_COLOR_FORMAT_YUV420P) {
							out.linesize[0] = m_video_width * m_video_height;
							out.linesize[1] = out.linesize[0] / 4;
							out.linesize[2] = out.linesize[1];
							out.data[0] = buffer;                                     // y
							out.data[1] = buffer + out.linesize[0];                   // u
							out.data[2] = buffer + out.linesize[0] + out.linesize[1]; // v
						} else { // YUV420SP
							out.linesize[0] = m_video_width * m_video_height;
							out.linesize[1] = out.total - out.linesize[0];
							out.data[0] = buffer;                                     // y
							out.data[1] = buffer + out.linesize[0];                   // uv
						}
						if ( out.time == Uint64::max ) { //  Unknown time frame
							out.time = m_presentation_time + m_frame_interval;
						}
					}
					return out;
				} else {
					AMediaCodec_releaseOutputBuffer(m_codec, status, true);
				}
			} else if ( status == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED ) {
				XX_DEBUG("output buffers changed");
			} else if ( status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED ) {
				AMediaFormat* format = AMediaCodec_getOutputFormat(m_codec);
				XX_DEBUG("format changed to: %s", AMediaFormat_toString(format));
				AMediaFormat_delete(format);
			} else if ( status == AMEDIACODEC_INFO_TRY_AGAIN_LATER ) {
				// XX_DEBUG("no output buffer right now");
			} else {
				XX_ERR("unexpected info code: %d", status);
			}
		}
		return OutputBuffer();
	}

	/**
	 * @overwrite
	 */
	virtual void release(OutputBuffer& buffer) {
		if ( buffer.total ) {
			AMediaCodec_releaseOutputBuffer(m_codec, buffer.index, true);
			memset(&buffer, 0, sizeof(OutputBuffer));
		}
	}

	void inl_set_frame_size(uint size) {
		m_audio_frame_size = size;
		// compute audio frame interval
		const TrackInfo &track = extractor()->track();
		uint64 second_size = track.sample_rate * m_channel_count * 2;
		m_frame_interval = uint64(m_audio_frame_size) * 1000 * 1000 / second_size;
	}

	/**
	 * @overwrite
	 * */
	virtual void set_frame_size(uint size) { }

	/**
	 * @overwrite
	 * */
	virtual void set_threads(uint value) { }

	/**
	 * @overwrite
	 * */
	virtual void set_background_run(bool value) { }

	// --------------------- @overwrite end ---------------------

private:
	AMediaFormat* m_format;
	AMediaCodec*  m_codec;
	bool          m_eof_flags;
	uint          m_video_width;
	uint          m_video_height;
	uint          m_audio_frame_size;
	uint64        m_presentation_time;
	bool          m_is_open;
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
		cchar* mime = NULL;

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

			XX_DEBUG("%s", AMediaFormat_toString(format));
			int result = AMediaCodec_configure(codec, format, nullptr, nullptr, 0);

			if ( result == 0 && AMediaCodec_start(codec) == 0 ) {
				rv = new AndroidHardwareMediaCodec(ex, codec, format);
			} else {
				XX_ERR("Unable to configure and run the decoder");
				AMediaCodec_delete(codec);
				AMediaFormat_delete(format);
			}
		} else {
			XX_ERR("cannot create decoder");
		}
	}
	return rv;
}

XX_END