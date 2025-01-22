/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
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
 * ***** END LICENSE BLOCK *****/

#include "./media_inl.h"

namespace qk {
	typedef MediaSource::StreamExtra StreamExtra;
	typedef MediaSource::Packet Packet;
	typedef MediaCodec::Frame Frame;

	struct MakingPixel {
		struct Body: Pixel::Body {
			Body(MakingPixel *host, uint32_t idx)
				: _host(host), _val(host->frame->data[idx])
			{
				_len = idx ? host->frame->height >> 1: host->frame->height;
				_len *= host->frame->linesize[idx];
			}
			virtual void release() override {
				if (--_host->bodyLen == 0) {
					delete _host; _host = nullptr;
				}
			}
			uint8_t* val() override { return _val; }
			uint32_t len() override { return _len; }
			MakingPixel *_host;
			uint8_t     *_val;
			uint32_t     _len;
		};

		Frame   *frame;
		Body     body[3];
		int      bodyLen;

		MakingPixel(Frame *frame): frame(frame)
			, body{ Body(this, 0), Body(this, 1), Body(this, 2) }, bodyLen(frame->dataitems) {
		}
		~MakingPixel() {
			delete frame; frame = nullptr;
		}
		static Array<Pixel> make(Frame *frame) {
			Array<Pixel> pixel;
			auto format = frame->avframe->format;
			if (frame->dataitems && (format == AV_PIX_FMT_YUV420P || format == AV_PIX_FMT_NV12)) {
				auto avf = frame->avframe;
				int width = avf->width, height = avf->height;
				auto pf = new MakingPixel(frame);
				pixel.push(Pixel(PixelInfo(width, height, kYUV420P_Y_8_ColorType), pf->body));
				width >>= 1; height >>= 1;

				if (format == AV_PIX_FMT_YUV420P) {
					Qk_ASSERT_EQ(frame->dataitems, 3);
					pixel.push(Pixel(PixelInfo(width, height, kYUV420P_U_8_ColorType), pf->body+1));
					pixel.push(Pixel(PixelInfo(width, height, kYUV420P_V_8_ColorType), pf->body+2));
				} else { // yuv420sp
					Qk_ASSERT_EQ(frame->dataitems, 2);
					pixel.push(Pixel(PixelInfo(width, height, kYUV420SP_UV_88_ColorType), pf->body+1));
				}
			}
			Qk_ReturnLocal(pixel);
		}
	};

	Array<Pixel> MediaCodec::frameToPixel(Frame *useFrame) {
		return MakingPixel::make(useFrame);
	}

	StreamExtra::StreamExtra() {
		codecpar = (AVCodecParameters*)::malloc(sizeof(AVCodecParameters));
	}

	StreamExtra::StreamExtra(const StreamExtra& extra): StreamExtra() {
		*codecpar = *extra.codecpar;
		set_codecpar(extra.codecpar);
	}

	StreamExtra::~StreamExtra() {
		::free(codecpar); codecpar = nullptr;
	}

	void StreamExtra::set_codecpar(AVCodecParameters *par) {
		extradata = WeakBuffer((Char *) par->extradata, par->extradata_size).buffer().copy();
		*codecpar = *par;
		codecpar->extradata = (uint8_t*)*extradata;
		codecpar->extradata_size = extradata.length();
	}

	Packet::~Packet() {
		av_packet_unref(avpkt); avpkt = nullptr;
	}

	Packet* Packet::clone() const {
		auto pkt = new (::malloc(sizeof(Packet) + sizeof(AVPacket))) Packet{
			nullptr,
			nullptr,
			size,
			pts,
			dts,
			duration,
			flags,
		};
		pkt->avpkt = reinterpret_cast<AVPacket*>(pkt + 1);
		// av_init_packet(pkt->avpkt);
		av_copy_packet(pkt->avpkt, avpkt);
		pkt->data = pkt->avpkt->data;
		return pkt;
	}

	Frame::~Frame() {
		av_frame_unref(avframe);
	}

	Extractor::Extractor(MediaType type, MediaSource* host, Array<Stream>&& streams)
		: _host(host)
		, _type(type)
		, _stream_index(0)
		, _streams(std::move(streams))
		, _before_duration(0), _after_duration(0)
		, _pkt(_packets.end())
	{
	}

	Extractor::~Extractor() {
		flush();
	}

	bool Extractor::switch_stream(uint32_t index) {
		return _host->_inl->switch_stream(this, index);
	}

	MediaSource::Packet* Extractor::advance() {
		return _host->_inl->advance(this);
	}

	const MediaSource::Stream& Extractor::stream() const {
		return _streams[_stream_index];
	}

	void Extractor::flush() {
		for (auto pkt: _packets) {
			delete pkt;
		}
		_packets.clear();
		_before_duration = 0;
		_after_duration = 0;
		_pkt = _packets.end();
	}
}
