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

namespace flare {

	static MediaCodec::Delegate default_media_decoder_delegate;

	// ------------------- MultimediaSource ------------------

	MultimediaSource::MultimediaSource(cString& uri, RunLoop* loop): _inl(nullptr) {
		_inl = new Inl(this, uri, loop);
	}

	MultimediaSource::~MultimediaSource() { 
		Release(_inl); _inl = nullptr; 
	}

	void MultimediaSource::set_delegate(Delegate* delegate) { _inl->set_delegate(delegate); }
	const URI& MultimediaSource::uri() const { return _inl->_uri; }
	MultimediaSourceStatus MultimediaSource::status() const {
		return (MultimediaSourceStatus)(int)_inl->_status;
	}
	uint64_t MultimediaSource::duration() const { return _inl->_duration; }
	uint32_t MultimediaSource::bit_rate_index() const { return _inl->bit_rate_index(); }
	const Array<BitRateInfo>& MultimediaSource::bit_rate()const{ return _inl->bit_rate();}
	bool MultimediaSource::select_bit_rate(int index) { return _inl->select_bit_rate(index); }
	Extractor* MultimediaSource::extractor(MediaType type) { return _inl->extractor(type); }
	bool MultimediaSource::seek(uint64_t timeUs) { return _inl->seek(timeUs); }
	void MultimediaSource::start() { _inl->start(); }
	void MultimediaSource::stop() { _inl->stop(); }
	bool MultimediaSource::is_active() { return _inl->is_active(); }
	void MultimediaSource::disable_wait_buffer(bool value) { _inl->disable_wait_buffer(value); }
	AVStream* MultimediaSource::get_stream(const TrackInfo& t) { return _inl->get_stream(t); }

	// ----------------- MultimediaSource::Extractor -------------------

	Extractor::Extractor(MediaType type, MultimediaSource* host, Array<TrackInfo>&& tracks)
		: _host(host)
		, _type(type)
		, _track_index(0)
		, _tracks(std::move(tracks))
		, _sample_data_cache()
		, _sample_index_cache(0)
		, _sample_count_cache(0)
		, _sample_data({ Buffer(), NULL, 0, 0, 0 })
		, _eof_flags(0)
		, _disable(1)
	{
	}

	/**
	* @func select_track
	*/
	bool Extractor::select_track(uint32_t index) {
		ScopeLock lock(_host->_inl->mutex());
		if ( _track_index != index && index < _tracks.length() ) {
			_host->_inl->extractor_flush(this);
			_track_index = index;
			return true;
		}
		return false;
	}

	/**
	* @func deplete_sample
	* */
	uint32_t Extractor::deplete_sample(Char* out, uint32_t size) {
		if ( _sample_data.size ) {
			size = F_MIN(_sample_data.size, size);
			memcpy(out, _sample_data.data, size);
			_sample_data.data += size;
			_sample_data.size -= size;
			return size;
		}
		return 0;
	}


	/**
	* @func deplete_sample
	* */
	uint32_t Extractor::deplete_sample(Buffer& out) {
		uint32_t size = out.write(_sample_data.data, 0, _sample_data.size);
		_sample_data.size = 0;
		return size;
	}

	/**
	* @func deplete_sample
	* */
	uint32_t Extractor::deplete_sample(uint32_t size) {
		size = F_MIN(size, _sample_data.size);
		_sample_data.size -= size;
		_sample_data.data += size;
		return size;
	}

	/**
	* @func advance
	* */
	bool Extractor::advance() {
		return _host->_inl->extractor_advance(this);
	}

	// ----------------- MediaCodec -------------------

	/**
	* @constructor
	*/
	MediaCodec::MediaCodec(Extractor* extractor)
		: _extractor(extractor)
		, _delegate(&default_media_decoder_delegate)
		, _color_format(VIDEO_COLOR_FORMAT_INVALID)
		, _channel_layout(CH_INVALID)
		, _channel_count(0)
		, _frame_interval(0)
	{
		_frame_interval = extractor->track().frame_interval;
	}

	/**
	* @func set_delegate
	*/
	void MediaCodec::set_delegate(Delegate* delegate) {
		F_ASSERT(delegate);
		_delegate = delegate;
	}

	inline static bool is_nalu_start(uint8_t* str) {
		return str[0] == 0 && str[1] == 0 && str[2] == 0 && str[3] == 1;
	}

	static bool find_nalu_package(cBuffer& buffer, uint32_t start, uint32_t& end) {
		uint32_t length = buffer.length();
		if ( start < length ) {
			cChar* c = *buffer + start;
			while(1) {
				size_t size = strlen(c);
				start += size;
				c     += size;
				if ( start + 4 < length ) {
					if (c[1] == 0 && c[2] == 0 && c[3] == 1) {
						end = start;
						return true;
					} else {
						start++; c++;
					}
				} else {
					end = length; return true;
				}
			}
		}
		return false;
	}

	/**
	* @func parse_psp_pps
	* */
	bool MediaCodec::parse_avc_psp_pps(cBuffer& extradata, Buffer& out_psp, Buffer& out_pps) {
		// set sps and pps
		uint8_t* buf = (uint8_t*)*extradata;
		
		if ( is_nalu_start(buf) ) { // nalu
			uint32_t start = 4, end = 0;
			while (find_nalu_package(extradata, start, end)) {
				int nalu_type = buf[start] & 0x1F;
				if (nalu_type == 0x07) {        // SPS
					out_psp.write((Char*)buf + start - 4, 0, end - start + 4);
				} else if (nalu_type == 0x08) { // PPS
					out_pps.write((Char*)buf + start - 4, 0, end - start + 4);
				}
				if (out_psp.length() && out_pps.length()) {
					return true;
				}
				start = end + 4; // 0x0 0x0 0x0 0x1
			}
		} else { // mp4 style
			uint32_t sps_size = buf[7];
			uint32_t numOfPictureParameterSets = buf[8 + sps_size];
			if (numOfPictureParameterSets == 1) {
				uint32_t pps_size = buf[10 + sps_size];
				if (sps_size + pps_size < extradata.length()) {
					Char csd_s[4] = {0, 0, 0, 1};
					out_psp.write(csd_s, 0, 4);
					out_pps.write(csd_s, 0, 4);
					out_psp.write((Char*)buf + 8, 4, sps_size);
					out_pps.write((Char*)buf + 11 + sps_size, 4, pps_size);
					return true;
				}
			}
		}
		return false;
	}

	/**
	* @func convert_sample_data_to_nalu
	* */
	bool MediaCodec::convert_sample_data_to_nalu(Buffer& buffer) {
		uint32_t size = buffer.length();
		if (size) {
			uint8_t* buf = (uint8_t*)*buffer;
			if ( !is_nalu_start(buf) ) {
				uint32_t i = 0;
				while ( i + 4 < size ) {
					uint32_t len = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8)) + buf[3];
					buf[0] = 0;
					buf[1] = 0;
					buf[2] = 0;
					buf[3] = 1;
					i += len + 4;
					buf += len + 4;
				}
			}
			return true;
		}
		return false;
	}

	/**
	* @func convert_sample_data_to_mp4_style
	* */
	bool MediaCodec::convert_sample_data_to_mp4_style(Buffer& buffer) {
		uint32_t size = buffer.length();
		if (size) {
			uint8_t* buf = (uint8_t*)*buffer;
			if ( is_nalu_start(buf) ) {
				uint32_t start = 4, end = 0;
				while( find_nalu_package(buffer, start, end) ) {
					int s = end - start;
					uint8_t header[4] = { (uint8_t)(s >> 24), (uint8_t)(s >> 16), (uint8_t)(s >> 8), (uint8_t)s };
					memcpy(buf + start - 4, header, 4);
					start = end + 4;
				}
			}
			return true;
		}
		return false;
	}

	/**
	* @func create decoder
	* */
	MediaCodec* MediaCodec::create(MediaType type, MultimediaSource* source) {
		MediaCodec* rv = hardware(type, source);
		if ( ! rv ) {
			rv = software(type, source);
		}
		return rv;
	}

}
