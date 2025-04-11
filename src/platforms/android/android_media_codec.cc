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

#include "../../media/media_inl.h"
#include "../../util/jni.h"
#include "../../util/fs.h"

#include <unistd.h>
#include <fcntl.h>

#ifndef USE_FFMPEG_MEDIACODEC
#define USE_FFMPEG_MEDIACODEC 0
#endif

#if USE_FFMPEG_MEDIACODEC
extern "C" {
# include <libavcodec/jni.h>
# include <libavcodec/mediacodec_wrapper.h>
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
# include <media/NdkMediaCodec.h>
# include <media/NdkMediaExtractor.h>
#endif

// // ------------------------------------------------------------------------------------------------

namespace qk {

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
		Qk_ASSERT(err == 0);
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
		Qk_DLog("cmp: %d|%d|%d|%d|%d, cmp_s: %d, sample_size:%d|%d",
						j[0], j[1], j[2], j[3], j[4], o, sample_size, sample_size2);

		memcpy(out, *buf, sample_size2);
		AMediaExtractor_advance(_TEST_ex);
		sample_size = sample_size2;
	}

#else
#define _TEST_init_AMediaExtractor(path, select_track) (void*)(0)
#define _TEST_get_sample_data(out, size, sample_size) (void*)(0)
#endif

	static void init_ffmpeg_jni() {
#if USE_FFMPEG_MEDIACODEC
		static bool has_init = false;
		if (!has_init) {
			has_init = true;
			if ( av_jni_set_java_vm(JNI::jvm(), nullptr) != 0 ) {
				Qk_ELog("av_jni_set_java_vm(), unsuccessful." );
			}
		}
#endif
	}

	static void deleteAMediaFormat(AMediaFormat *p){ AMediaFormat_delete(p); }
	static void deleteAMediaCodec(AMediaCodec *p){ AMediaCodec_delete(p); }

	typedef Sp<AMediaFormat, object_traits_from<AMediaFormat, deleteAMediaFormat>> AMediaFormatAuto;
	typedef Sp<AMediaCodec, object_traits_from<AMediaCodec, deleteAMediaCodec>> AMediaCodecAuto;

	class AndroidHardwareMediaCodec: public MediaCodec {
	public:
		AndroidHardwareMediaCodec(const Stream &stream)
			: MediaCodec(stream)
			, _isOpen(false)
		{
		}

		~AndroidHardwareMediaCodec() override {
			close();
		}

		bool is_open() const override {
			return _isOpen;
		}

		bool open(const Stream *stream = nullptr) override {
			if (_isOpen) return true;
			if (!stream) stream = &_stream;

			cChar* mime = nullptr;
			auto type = stream->type;

			if (type == kAudio_MediaType) {
				if (stream->mime == "audio/aac" && stream->extra.extradata.length()) { // aac
					mime = "audio/mp4a-latm";
				} else {
					return false;
				}
			} else if (type == kVideo_MediaType) {
				if (stream->mime == "video/avc" || stream->mime == "video/h264") { // h.264
					mime = "video/avc";
				} else if (stream->mime == "video/hevc" || stream->mime == "video/h265") { // h.265
					mime = "video/hevc";
				} else {
					return false;
				}
			} else {
				return false;
			}

			if (!_codec || stream->mime != _stream.mime) {
				_codec = AMediaCodec_createDecoderByType(mime);
			}
			if (!_codec) {
				return Qk_ELog("Cannot create AMediaCodec"), false;
			}

			if (!_format || *stream != _stream) {
				_format = AMediaFormat_new();

				AMediaFormat_setString(_format.get(), AMEDIAFORMAT_KEY_MIME, mime);
				AMediaFormat_setInt64(_format.get(), AMEDIAFORMAT_KEY_DURATION, stream->duration);

				if (type == kAudio_MediaType) {
					//_TEST_init_AMediaExtractor("/sdcard/Download/av.1.0.ts", 1);
					//format = AMediaExtractor_getTrackFormat(_TEST_ex, 1);
					AMediaFormat_setInt32(_format.get(), AMEDIAFORMAT_KEY_SAMPLE_RATE, stream->sample_rate);
					AMediaFormat_setInt32(_format.get(), AMEDIAFORMAT_KEY_CHANNEL_COUNT, stream->channels);
					AMediaFormat_setBuffer(_format.get(), "csd-0",
					stream->extra.extradata.val(), stream->extra.extradata.length());
				} else {
					AMediaFormat_setInt32(_format.get(), AMEDIAFORMAT_KEY_WIDTH, stream->width);
					AMediaFormat_setInt32(_format.get(), AMEDIAFORMAT_KEY_HEIGHT, stream->height);
					AMediaFormat_setInt32(_format.get(), AMEDIAFORMAT_KEY_COLOR_FORMAT, 19);
					Buffer csd_0, csd_1;
					if ( MediaCodec::parse_avc_psp_pps(stream->extra.extradata, csd_0, csd_1) ) {
						AMediaFormat_setBuffer(_format.get(), "csd-0", *csd_0, csd_0.length());
						AMediaFormat_setBuffer(_format.get(), "csd-1", *csd_1, csd_1.length());
					}
					// _TEST_init_AMediaExtractor(source->uri().href(), 0);
					// format = AMediaExtractor_getTrackFormat(_TEST_ex, 0);
				}
			}

			if (type == kVideo_MediaType) {
				int num;
				if (AMediaFormat_getInt32(_format.get(), AMEDIAFORMAT_KEY_COLOR_FORMAT, &num)) {
					switch (num) {
						// case 17: _colorType = kYUV411P_ColorType; break;
						case 19: _colorFormat = AV_PIX_FMT_YUV420P; break; // yuv420p
						case 21: _colorFormat = AV_PIX_FMT_NV12; break; // yuv420sp
						default: return false;
					}
				}
			}

			Qk_DLog("%s", AMediaFormat_toString(_format.get()));

			if (AMediaCodec_configure(_codec.get(), _format.get(), nullptr, nullptr, 0) == 0 &&
					AMediaCodec_start(_codec.get()) == 0
			) {
				if (stream != &_stream) {
					_stream = *stream;
				}
				_isOpen = true;
				flush();
			}
			return _isOpen;
		}

		void close() override {
			if ( _isOpen ) {
				flush();
				Qk_ASSERT_EQ(AMediaCodec_stop(_codec.get()), 0);
				_isOpen = false;
			}
		}

		void flush() override {
			if (_isOpen) {
				Qk_ASSERT_EQ(AMediaCodec_flush(_codec.get()), 0);
				if (_stream.type == kVideo_MediaType)
					_need_keyframe = true;
				Releasep(_packet);
			}
		}

		int send_packet(const Packet *pkt) override {
			if (!_isOpen || !pkt) {
				return AVERROR(ENOMEM);
			}
			if ( _need_keyframe ) { // need key frame
				if ( pkt->flags & AV_PKT_FLAG_KEY ) { // i frame
					_need_keyframe = false; // start
				} else { // Discard
					return 0;
				}
			}
			ssize_t bufidx = AMediaCodec_dequeueInputBuffer(_codec.get(), 0);
			if ( bufidx < 0 ) {
				return AVERROR(EAGAIN); // Program is busy
			}

			media_status_t rc;
			uint32_t flags = 0;//AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM;
			size_t bufsize;
			auto buff = AMediaCodec_getInputBuffer(_codec.get(), bufidx, &bufsize);
			Qk_ASSERT(bufsize >= pkt->size);
			memcpy(buff, pkt->data, pkt->size);

			if (_stream.type == kVideo_MediaType) {
				MediaCodec::convert_sample_data_to_nalu(buff, pkt->size);
			}

			rc = AMediaCodec_queueInputBuffer(_codec.get(), bufidx, 0, pkt->size, pkt->pts, flags);
			if (rc == AMEDIA_OK) {
				_pending++;
			}
			return rc;
		}

		int send_packet(Extractor *extractor) override {
			if (!extractor || !_isOpen) {
				return AVERROR(EINVAL);
			}
			Qk_ASSERT_EQ(type(), extractor->type());

			if (!_packet) {
				_packet = extractor->advance();
			}
			if (!_packet) {
				return AVERROR(EAGAIN);
			}
			int rc = send_packet(_packet);
			if (rc == 0) {
				Releasep(_packet);
			}
			return rc;
		}

		Frame* receive_frame() override {
			AMediaCodecBufferInfo info;
			ssize_t status = AMediaCodec_dequeueOutputBuffer(_codec.get(), &info, 0);
			Frame *f = nullptr;

			if ( status >= 0 ) {
				if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
					Qk_DLog("output EOS");
				}
				int64_t pts = Int64::max(info.presentationTimeUs, 0);

				if (pts == 0) {
					Qk_DLog("presentationTimeUs: 0");
				}
				size_t size;
				uint8_t *buffer = AMediaCodec_getOutputBuffer(_codec.get(), status, &size);

				if (size) {
					f = Frame::Make();
					auto avf = f->avframe;
					auto unit = 1000000.0f * _stream.time_base[0] / _stream.time_base[1];

					if ( _stream.type == kAudio_MediaType ) {
						auto nb_samples = info.size / _stream.channels / 2;
						auto pkt_duration = (nb_samples / float(_stream.sample_rate)) * 1000000U;
						auto buf = av_buffer_alloc(info.size);
						memcpy(buf->data, buffer, info.size);
						Qk_ASSERT_EQ(_stream.sample_rate, _stream.time_base[1]);

						avf->buf[0] = buf;
						avf->extended_data = nullptr;
						avf->format = AV_SAMPLE_FMT_S16;
						avf->data[0] = buf->data;
						avf->linesize[0] = info.size;
						avf->pts = pts / unit;
						avf->pkt_duration = nb_samples;
						avf->nb_samples = nb_samples;
						f->data = avf->data;
						f->linesize = reinterpret_cast<uint32_t*>(avf->linesize);
						f->dataitems = 1;
						f->pts = pts;
						f->nb_samples = nb_samples;
						f->pkt_duration = pkt_duration;
						f->format = AV_SAMPLE_FMT_S16;
					} else {
						auto w = _stream.width, h = _stream.height;
						auto buf = av_buffer_alloc(av_image_get_buffer_size(_colorFormat, w, h, 1));
						Qk_ASSERT_EQ(buf->size,
							av_image_fill_arrays(avf->data, avf->linesize, buf->data, _colorFormat, w, h, 1)
						);
						Qk_ASSERT_EQ(buf->size, size);
						memcpy(buf->data, buffer, size);

						avf->buf[0] = buf;
						avf->format = _colorFormat;
						avf->flags = 0; // flags
						avf->key_frame = 0;
						avf->width = w;
						avf->height = h;
						avf->pts = pts / unit;
						avf->pkt_duration = _stream.avg_framerate[1];
						f->data = avf->data;
						f->linesize = reinterpret_cast<uint32_t*>(avf->linesize);
						f->dataitems = _colorFormat == AV_PIX_FMT_YUV420P ? 3: 2;
						f->pts = pts;
						f->pkt_duration = avf->pkt_duration * unit;
						f->width = w;
						f->height = h;
						f->format = _colorFormat == AV_PIX_FMT_YUV420P ? kYUV420P_ColorType: kYUV420SP_ColorType;
					}
				}
				_pending--;
				Qk_ASSERT(_pending >= 0);
				AMediaCodec_releaseOutputBuffer(_codec.get(), status, true);
			} else if (status == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
				Qk_DLog("output buffers changed");
			} else if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
				auto format = AMediaCodec_getOutputFormat(_codec.get());
				Qk_DLog("format changed to: %s", AMediaFormat_toString(format));
				//Qk_ASSERT_EQ(format, _format.get());
			} else if (status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
				// Qk_DLog("no output buffer right now");
			} else {
				Qk_DLog("unexpected info code: %d", status);
			}

			return f;
		}

		void set_threads(uint32_t value) override {}

		bool finished() override {
			return _pending == 0;
		}

	private:
		AMediaFormatAuto _format;
		AMediaCodecAuto _codec;
		Packet       *_packet;
		AVPixelFormat _colorFormat;
		int           _pending;
		bool          _need_keyframe, _isOpen;
	};

	MediaCodec* MediaCodec_hardware(MediaType type, Extractor* ex) {
		return nullptr;
		init_ffmpeg_jni();

		AndroidHardwareMediaCodec* rv = nullptr;
		if ( ex ) {
			Sp<AndroidHardwareMediaCodec> codec = new AndroidHardwareMediaCodec(ex->stream());
			if (codec->open()) {
				return codec.collapse();
			}
		}
		return nullptr;
	}

}