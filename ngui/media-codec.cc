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

#include "media-codec-1.h"

XX_NS(ngui)

static MediaCodec::Delegate default_media_decoder_delegate;

MultimediaSource::TrackInfo::TrackInfo(const TrackInfo& info)
: track(info.track) 
, type(info.type)
, mime(info.mime)
, codec_id(info.codec_id)
, codec_tag(info.codec_tag)
, format(info.format)
, profile(info.profile)
, level(info.level)
, width(info.width)
, height(info.height)
, language(info.language)
, bitrate(info.bitrate)
, sample_rate(info.sample_rate)
, channel_count(info.channel_count)
, channel_layout(info.channel_layout)
, frame_interval(info.frame_interval)
, extradata(info.extradata.copy())
{

}

MultimediaSource::TrackInfo::TrackInfo() {

}

// ------------------- MultimediaSource ------------------

MultimediaSource::MultimediaSource(cString& uri, RunLoop* loop): m_inl(nullptr) {
  m_inl = new Inl(this, uri, loop);
}
MultimediaSource::~MultimediaSource() { Release(m_inl); m_inl = nullptr; }
void MultimediaSource::set_delegate(Delegate* delegate) { m_inl->set_delegate(delegate); }
const URI& MultimediaSource::uri() const { return m_inl->m_uri; }
MultimediaSourceStatus MultimediaSource::status() const {
  return (MultimediaSourceStatus)(int)m_inl->m_status;
}
uint64 MultimediaSource::duration() const { return m_inl->m_duration; }
uint MultimediaSource::bit_rate_index() const { return m_inl->bit_rate_index(); }
const Array<BitRateInfo>& MultimediaSource::bit_rate()const{ return m_inl->bit_rate();}
bool MultimediaSource::select_bit_rate(int index) { return m_inl->select_bit_rate(index); }
Extractor* MultimediaSource::extractor(MediaType type) { return m_inl->extractor(type); }
bool MultimediaSource::seek(uint64 timeUs) { return m_inl->seek(timeUs); }
void MultimediaSource::start() { m_inl->start(); }
void MultimediaSource::stop() { m_inl->stop(); }
bool MultimediaSource::is_active() { return m_inl->is_active(); }
void MultimediaSource::disable_wait_buffer(bool value) { m_inl->disable_wait_buffer(value); }
AVStream* MultimediaSource::get_stream(const TrackInfo& t) { return m_inl->get_stream(t); }

// ----------------- MultimediaSource::Extractor -------------------

Extractor::Extractor(MediaType type, MultimediaSource* host, Array<TrackInfo>&& tracks)
: m_host(host)
, m_type(type)
, m_tracks(move(tracks))
, m_track_index(0)
, m_sample_data_cache()
, m_sample_index_cache(0)
, m_sample_count_cache(0)
, m_sample_data({ Buffer(), NULL, 0, 0, 0 })
, m_eof_flags(0)
, m_disable(1)
{
  
}

/**
 * @func select_track
 */
bool Extractor::select_track(uint index) {
  ScopeLock lock(m_host->m_inl->mutex());
  if ( m_track_index != index && index < m_tracks.length() ) {
    m_host->m_inl->extractor_flush(this);
    m_track_index = index;
    return true;
  }
  return false;
}

/**
 * @func deplete_sample
 * */
uint Extractor::deplete_sample(char* out, uint size) {
  if ( m_sample_data.size ) {
    size = XX_MIN(m_sample_data.size, size);
    memcpy(out, m_sample_data.data, size);
    m_sample_data.data += size;
    m_sample_data.size -= size;
    return size;
  }
  return 0;
}


/**
 * @func deplete_sample
 * */
uint Extractor::deplete_sample(Buffer& out) {
  uint size = out.write(m_sample_data.data, 0, m_sample_data.size);
  m_sample_data.size = 0;
  return size;
}

/**
 * @func deplete_sample
 * */
uint Extractor::deplete_sample(uint size) {
  size = XX_MIN(size, m_sample_data.size);
  m_sample_data.size -= size;
  m_sample_data.data += size;
  return size;
}

/**
 * @func advance
 * */
bool Extractor::advance() {
  return m_host->m_inl->extractor_advance(this);
}

// ----------------- MediaCodec -------------------

/**
 * @constructor
 */
MediaCodec::MediaCodec(Extractor* extractor)
: m_extractor(extractor)
, m_delegate(&default_media_decoder_delegate)
, m_color_format(VIDEO_COLOR_FORMAT_INVALID)
, m_channel_layout(CH_INVALID)
, m_channel_count(0)
, m_frame_interval(0) {
  m_frame_interval = extractor->track().frame_interval;
}

/**
 * @func set_delegate
 */
void MediaCodec::set_delegate(Delegate* delegate) {
  XX_ASSERT(delegate);
  m_delegate = delegate;
}

inline static bool is_nalu_start(byte* str) {
  return str[0] == 0 && str[1] == 0 && str[2] == 0 && str[3] == 1;
}

static bool find_nalu_package(cBuffer& buffer, uint start, uint& end) {
  uint length = buffer.length();
  if ( start < length ) {
    cchar* c = *buffer + start;
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
  byte* buf = (byte*)*extradata;
  
  if ( is_nalu_start(buf) ) { // nalu
    uint start = 4, end = 0;
    while (find_nalu_package(extradata, start, end)) {
      int nalu_type = buf[start] & 0x1F;
      if (nalu_type == 0x07) {        // SPS
        out_psp.write((char*)buf + start - 4, 0, end - start + 4);
      } else if (nalu_type == 0x08) { // PPS
        out_pps.write((char*)buf + start - 4, 0, end - start + 4);
      }
      if (out_psp.length() && out_pps.length()) {
        return true;
      }
      start = end + 4; // 0x0 0x0 0x0 0x1
    }
  } else { // mp4 style
    uint sps_size = buf[7];
    uint numOfPictureParameterSets = buf[8 + sps_size];
    if (numOfPictureParameterSets == 1) {
      uint pps_size = buf[10 + sps_size];
      if (sps_size + pps_size < extradata.length()) {
        char csd_s[4] = {0, 0, 0, 1};
        out_psp.write(csd_s, 0, 4);
        out_pps.write(csd_s, 0, 4);
        out_psp.write((char*)buf + 8, 4, sps_size);
        out_pps.write((char*)buf + 11 + sps_size, 4, pps_size);
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
  uint size = buffer.length();
  if (size) {
    byte* buf = (byte*)*buffer;
    if ( !is_nalu_start(buf) ) {
      uint i = 0;
      while ( i + 4 < size ) {
        uint len = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8)) + buf[3];
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
  uint size = buffer.length();
  if (size) {
    byte* buf = (byte*)*buffer;
    if ( is_nalu_start(buf) ) {
      uint start = 4, end = 0;
      while( find_nalu_package(buffer, start, end) ) {
        int s = end - start;
        byte header[4] = { (byte)(s >> 24), (byte)(s >> 16), (byte)(s >> 8), (byte)s };
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

MediaCodec::OutputBuffer::OutputBuffer() {
  memset(this, 0, sizeof(OutputBuffer));
}

MediaCodec::OutputBuffer::OutputBuffer(const OutputBuffer& buffer) {
  memcpy(this, &buffer, sizeof(OutputBuffer));
}

XX_END
