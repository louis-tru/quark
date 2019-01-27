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


extern "C" {
	#include <libavutil/imgutils.h>
	#include <libavutil/samplefmt.h>
	#include <libavutil/timestamp.h>
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	// #include <libavcodec/fft.h>
}

#include <shark/utils/util.h>
#include <shark/utils/fs.h>

using namespace shark;

static AVFormatContext *fmt_ctx = NULL;
static AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx;
static int width, height;
static enum AVPixelFormat pix_fmt;
static AVStream *video_stream = NULL, *audio_stream = NULL;
static const char *src_filename = NULL;
static const char *video_dst_filename = NULL;
static const char *audio_dst_filename = NULL;
static FILE *video_dst_file = NULL;
static FILE *audio_dst_file = NULL;

static uint8_t *video_dst_data[4] = {NULL};
static int      video_dst_linesize[4];
static int video_dst_bufsize;

static int video_stream_idx = -1, audio_stream_idx = -1;
static AVFrame *frame = NULL;
static AVPacket pkt;
static int video_frame_count = 0;
static int audio_frame_count = 0;

/* Enable or disable frame reference counting. You are not supposed to support
 * both paths in your application but pick the one most appropriate to your
 * needs. Look for the use of refcount in this example to see what are the
 * differences of API usage between them. */
static int refcount = 0;

static uint64 systemtime() {
	timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	int64_t r = now.tv_sec * 1000000LL + now.tv_nsec / 1000LL;
	return r;
}

static int decode_packet(int *got_frame, int cached)
{
	
	//FFTSample sample = ff_cos_16[0];
	
	int ret = 0;
	int decoded = pkt.size;
	
	*got_frame = 0;
	
	uint64 st = systemtime();
	
	if (pkt.stream_index == video_stream_idx) {
		/* decode video frame */
		
		ret = avcodec_send_packet(video_dec_ctx, &pkt); uint64 st2 = systemtime();
		
		if (ret < 0) {
			// fprintf(stderr, "Error decoding video frame (%s)\n", av_err2str(ret));
			return ret;
		}
		
		ret = avcodec_receive_frame(video_dec_ctx, frame); uint64 st3 = systemtime();
		// LOG("-----------------st:%ld, st2:%ld", st2 - st, st3 - st2);
		
		if (ret >= 0 || *got_frame) {
			
			if (frame->width != width || frame->height != height ||
					frame->format != pix_fmt) {
				/* To handle this change, one could call av_image_alloc again and
				 * decode the following frames into another rawvideo file. */
				fprintf(stderr, "Error: Width, height and pixel format have to be "
								"constant in a rawvideo file, but the width, height or "
								"pixel format of the input video changed:\n"
								"old: width = %d, height = %d, format = %s\n"
								"new: width = %d, height = %d, format = %s\n",
								width, height, av_get_pix_fmt_name(pix_fmt),
								frame->width, frame->height,
								av_get_pix_fmt_name((AVPixelFormat)frame->format));
				return -1;
			}
			
			printf("video_frame%s n:%d coded_n:%d\n",
						 cached ? "(cached)" : "",
						 video_frame_count++, frame->coded_picture_number);
			
			/* copy decoded frame to destination buffer:
			 * this is required since rawvideo expects non aligned data */
			av_image_copy(video_dst_data, video_dst_linesize,
										(const uint8_t **)(frame->data), frame->linesize,
										pix_fmt, width, height);
			
			/* write to rawvideo file */
			fwrite(video_dst_data[0], 1, video_dst_bufsize, video_dst_file);
		}
	} else if (pkt.stream_index == audio_stream_idx) {
		/* decode audio frame */
		ret = avcodec_decode_audio4(audio_dec_ctx, frame, got_frame, &pkt);
		if (ret < 0) {
			// fprintf(stderr, "Error decoding audio frame (%s)\n", av_err2str(ret));
			return ret;
		}
		/* Some audio decoders decode only part of the packet, and have to be
		 * called again with the remainder of the packet data.
		 * Sample: fate-suite/lossless-audio/luckynight-partial.shn
		 * Also, some decoders might over-read the packet. */
		decoded = FFMIN(ret, pkt.size);
		
		if (*got_frame) {
			size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)frame->format);
			/*
			printf("audio_frame%s n:%d nb_samples:%d pts:%s\n",
						 cached ? "(cached)" : "",
						 audio_frame_count++, frame->nb_samples,
						 av_ts2timestr(frame->pts, &audio_dec_ctx->time_base));
			*/
			
			/* Write the raw audio data samples of the first plane. This works
			 * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
			 * most audio decoders output planar audio, which uses a separate
			 * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
			 * In other words, this code will write only the first audio channel
			 * in these cases.
			 * You should use libswresample or libavfilter to convert the frame
			 * to packed data. */
			fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);
		}
	}
	
	/* If we use frame reference counting, we own the data and need
	 * to de-reference it when we don't use it anymore */
	if (*got_frame && refcount)
		av_frame_unref(frame);
	
	return decoded;
}

static int open_codec_context(int *stream_idx,
															AVCodecContext **dec_ctx,
															AVFormatContext *fmt_ctx, enum AVMediaType type)
{
	int ret, stream_index;
	AVStream *st;
	AVCodec *dec = NULL;
	AVDictionary *opts = NULL;
	
	ret = av_find_best_stream(fmt_ctx, type, -1, 2, NULL, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not find %s stream in input file '%s'\n",
						av_get_media_type_string(type), src_filename);
		return ret;
	} else {
		stream_index = ret;
		st = fmt_ctx->streams[stream_index];
		
		/* find decoder for the stream */
		dec = avcodec_find_decoder(st->codecpar->codec_id);
		
		if (!dec) {
			fprintf(stderr, "Failed to find %s codec\n",
							av_get_media_type_string(type));
			return AVERROR(EINVAL);
		}
		
		/* Allocate a codec context for the decoder */
		*dec_ctx = avcodec_alloc_context3(dec);
		if (!*dec_ctx) {
			fprintf(stderr, "Failed to allocate the %s codec context\n",
							av_get_media_type_string(type));
			return AVERROR(ENOMEM);
		}
		
		/*
		if (((*dec_ctx)->codec->capabilities & CODEC_CAP_FRAME_THREADS)
				&& !((*dec_ctx)->flags & CODEC_FLAG_TRUNCATED)
				&& !((*dec_ctx)->flags & CODEC_FLAG_LOW_DELAY)
				&& !((*dec_ctx)->flags2 & CODEC_FLAG2_CHUNKS)) {
			(*dec_ctx)->thread_count = 4;
			(*dec_ctx)->active_thread_type = FF_THREAD_FRAME;
		}*/
		
		/* Copy codec parameters from input stream to output codec context */
		if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
			fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
							av_get_media_type_string(type));
			return ret;
		}
		
		/* Init the decoders, with or without reference counting */
		av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
		if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
			fprintf(stderr, "Failed to open %s codec\n",
							av_get_media_type_string(type));
			return ret;
		}

		*stream_idx = stream_index;
	}
	
	return 0;
}

static int get_format_from_sample_fmt(const char **fmt,
																			enum AVSampleFormat sample_fmt)
{
	int i;
	struct sample_fmt_entry {
		enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
	} sample_fmt_entries[] = {
		{ AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
		{ AV_SAMPLE_FMT_S16, "s16be", "s16le" },
		{ AV_SAMPLE_FMT_S32, "s32be", "s32le" },
		{ AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
		{ AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
	};
	*fmt = NULL;
	
	for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
		struct sample_fmt_entry *entry = &sample_fmt_entries[i];
		if (sample_fmt == entry->sample_fmt) {
			*fmt = AV_NE(entry->fmt_be, entry->fmt_le);
			return 0;
		}
	}
	
	fprintf(stderr,
					"sample format %s is not supported as output format\n",
					av_get_sample_fmt_name(sample_fmt));
	return -1;
}

int test_ffmpeg (int argc, char **argv)
{
	int ret = 0, got_frame;
	
	src_filename = "http://111.202.85.144/vipzjhls.tc.qq.com/mp4/33/qUL265hYAO3TkQqGhBicN5IzSXXUTqcfJDetSNMwjcKaL-GFIHMCwA/TxH54-BdsMklq4rMM_ACe5essCBtzWwRGq-y6sZYTv668hrxWffpUYu5tNUb14hmK1Qk-69UPfmgErMeSNR1nRvA7xVslsXcyr1FQ0THj7YI64E1Af50Yg/d00161yroup.p201.mp4/d00161yroup.p201.mp4.av.m3u8?fn=p201&amp;bw=800&amp;st=0&amp;et=0&amp;iv=&amp;ivfn=&amp;ivfc=&amp;ivt=&amp;ivs=&amp;ivd=&amp;ivl=&amp;ftype=mp4&amp;fbw=93&amp;type=m3u8&amp;drm=0&amp;sdtfrom=v3000&amp;platform=10403&amp;appver=5.3.0.16792&amp;projection=dlna";
	
	String video_dst = Path::fallback_c(Path::temp("q0021regv7h.320092.ts.m3u8.video.dst"));
	String audio_dst = Path::fallback_c(Path::temp("q0021regv7h.320092.ts.m3u8.audio.dst"));
	video_dst_filename = *video_dst;
	audio_dst_filename = *audio_dst;
	
	/* register all formats and codecs */
	av_register_all();
	avformat_network_init();
	
	/* open input file, and allocate format context */
	if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) {
		fprintf(stderr, "Could not open source file %s\n", src_filename);
		::exit(1);
	}
	
	/* retrieve stream information */
	if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
		fprintf(stderr, "Could not find stream information\n");
		::exit(1);
	}
	
	//  fmt_ctx->streams[0]->discard = AVDISCARD_ALL;
	//  fmt_ctx->streams[1]->discard = AVDISCARD_ALL;
	//  fmt_ctx->programs[0]->discard = AVDISCARD_ALL;
	
	/* dump input information to stderr */
	av_dump_format(fmt_ctx, 0, src_filename, 0);
	
	char b[256];
	
	cchar* name2 = avcodec_get_name(fmt_ctx->streams[1]->codecpar->codec_id);
	cchar* name3 = avcodec_profile_name(fmt_ctx->streams[1]->codecpar->codec_id, fmt_ctx->streams[1]->codecpar->profile);
	av_get_codec_tag_string(b, 255, fmt_ctx->streams[0]->codecpar->codec_tag);
	avcodec_string(b, 255, fmt_ctx->streams[0]->codec, 0);
	
	if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
		video_stream = fmt_ctx->streams[video_stream_idx];
		
		video_dst_file = fopen(video_dst_filename, "wb");
		if (!video_dst_file) {
			fprintf(stderr, "Could not open destination file %s\n", video_dst_filename);
			ret = 1;
			goto end;
		}
		
		/* allocate image where the decoded image will be put */
		width = video_dec_ctx->width;
		height = video_dec_ctx->height;
		pix_fmt = video_dec_ctx->pix_fmt;
		ret = av_image_alloc(video_dst_data, video_dst_linesize,
												 width, height, pix_fmt, 1);
		if (ret < 0) {
			fprintf(stderr, "Could not allocate raw video buffer\n");
			goto end;
		}
		video_dst_bufsize = ret;
	}
	
	if (open_codec_context(&audio_stream_idx, &audio_dec_ctx, fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
		audio_stream = fmt_ctx->streams[audio_stream_idx];
		audio_dst_file = fopen(audio_dst_filename, "wb");
		if (!audio_dst_file) {
			fprintf(stderr, "Could not open destination file %s\n", audio_dst_filename);
			ret = 1;
			goto end;
		}
	}
	
	if (!audio_stream && !video_stream) {
		fprintf(stderr, "Could not find audio or video stream in the input, aborting\n");
		ret = 1;
		goto end;
	}
	
	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate frame\n");
		ret = AVERROR(ENOMEM);
		goto end;
	}
	
	/* initialize packet, set data to NULL, let the demuxer fill it */
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	
	if (video_stream)
		printf("Demuxing video from file '%s' into '%s'\n", src_filename, video_dst_filename);
	if (audio_stream)
		printf("Demuxing audio from file '%s' into '%s'\n", src_filename, audio_dst_filename);
	
	/* read frames from the file */
	while (1) {
		ret = av_read_frame(fmt_ctx, &pkt);
		if (ret < 0) break;
		AVPacket orig_pkt = pkt;
		LOG("----------------------------------------------------stream_index:%d", pkt.stream_index);
		do {
			ret = decode_packet(&got_frame, 0);
			if (ret < 0)
				break;
			pkt.data += ret;
			pkt.size -= ret;
		} while (pkt.size > 0);
		av_packet_unref(&orig_pkt);
	}
	
	/* flush cached frames */
	pkt.data = NULL;
	pkt.size = 0;
	do {
		decode_packet(&got_frame, 1);
	} while (got_frame);
	
	printf("Demuxing succeeded.\n");
	
	if (video_stream) {
		printf("Play the output video file with the command:\n"
					 "ffplay -f rawvideo -pix_fmt %s -video_size %dx%d %s\n",
					 av_get_pix_fmt_name(pix_fmt), width, height,
					 video_dst_filename);
	}
	
	if (audio_stream) {
		enum AVSampleFormat sfmt = audio_dec_ctx->sample_fmt;
		int n_channels = audio_dec_ctx->channels;
		const char *fmt;
		
		if (av_sample_fmt_is_planar(sfmt)) {
			const char *packed = av_get_sample_fmt_name(sfmt);
			printf("Warning: the sample format the decoder produced is planar "
						 "(%s). This example will output the first channel only.\n",
						 packed ? packed : "?");
			sfmt = av_get_packed_sample_fmt(sfmt);
			n_channels = 1;
		}
		
		if ((ret = get_format_from_sample_fmt(&fmt, sfmt)) < 0)
			goto end;
		
		printf("Play the output audio file with the command:\n"
					 "ffplay -f %s -ac %d -ar %d %s\n",
					 fmt, n_channels, audio_dec_ctx->sample_rate,
					 audio_dst_filename);
	}
	
end:
	avcodec_free_context(&video_dec_ctx);
	avcodec_free_context(&audio_dec_ctx);
	avformat_close_input(&fmt_ctx);
	if (video_dst_file)
		fclose(video_dst_file);
	if (audio_dst_file)
		fclose(audio_dst_file);
	av_frame_free(&frame);
	av_free(video_dst_data[0]);
	
	return ret < 0;
}
