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

#ifndef __ngui__audio_player__
#define __ngui__audio_player__

#include "ngui/event.h"
#include "ngui/media.h"
#include "ngui/pcm-player.h"
#include "ngui/media-codec.h"

/**
 * @ns ngui
 */

NX_NS(ngui)

/**
 * @class AudioPlayer
 */
class NX_EXPORT AudioPlayer: public Notification<Event<>, GUIEventName>,
                             public MultimediaSource::Delegate {
  NX_HIDDEN_ALL_COPY(AudioPlayer);
 public:

  typedef MultimediaSource::TrackInfo TrackInfo;
  typedef MediaCodec::OutputBuffer    OutputBuffer;

  AudioPlayer(cString& uri = String());

  /**
   * @destructor
   */
  virtual ~AudioPlayer();

  /**
   * @overwrite
   */
  virtual void multimedia_source_ready(MultimediaSource* src);
  virtual void multimedia_source_wait_buffer(MultimediaSource* src, float process);
  virtual void multimedia_source_eof(MultimediaSource* src);
  virtual void multimedia_source_error(MultimediaSource* src, cError& err);

  /**
   * @func src get src
   */
  NX_MEDIA_DYNAMIC String src();
  
  /**
   * @func set_src set src
   */
  NX_MEDIA_DYNAMIC void set_src(cString& value);

  /**
   * @func auto_play
   */
  inline bool auto_play() const { return m_auto_play; }

  /**
   * @func set_auto_play setting auto play
   */
  NX_MEDIA_DYNAMIC void set_auto_play(bool value);

  /**
   * @func source_status
   * */
  NX_MEDIA_DYNAMIC MultimediaSourceStatus source_status();

  /**
   * @func status getting play status
   */
  NX_MEDIA_DYNAMIC PlayerStatus status();

  /**
   * @func mute getting mute status
   * */
  inline bool mute() const { return m_mute; }

  /**
   * @func set_mute setting mute status
   * */
  NX_MEDIA_DYNAMIC void set_mute(bool value);

  /**
   * @func volume
   */
  inline uint volume() { return m_volume; }

  /**
   * @func set_volume
   */
  NX_MEDIA_DYNAMIC void set_volume(uint value);

  /**
   * @func time
   * */
  NX_MEDIA_DYNAMIC uint64 time();

  /**
   * @func duration
   * */
  NX_MEDIA_DYNAMIC uint64 duration();

  /**
   * @func audio_track_count
   */
  NX_MEDIA_DYNAMIC uint track_count();

  /**
   * @func audio_track
   */
  NX_MEDIA_DYNAMIC uint track_index();

  /**
   * @func audio_track
   */
  NX_MEDIA_DYNAMIC const TrackInfo* track();

  /**
   * @func audio_track
   */
  NX_MEDIA_DYNAMIC const TrackInfo* track(uint index);

  /**
   * @func select_audio_track
   * */
  NX_MEDIA_DYNAMIC void select_track(uint index);

  /**
   * @func start play
   */
  NX_MEDIA_DYNAMIC void start();

  /**
   * @func seek to target time
   */
  NX_MEDIA_DYNAMIC bool seek(uint64 timeUs);

  /**
   * @func pause play
   * */
  NX_MEDIA_DYNAMIC void pause();

  /**
   * @func resume play
   * */
  NX_MEDIA_DYNAMIC void resume();

  /**
   * @func stop play
   * */
  NX_MEDIA_DYNAMIC void stop();

  /**
   * @func disable_wait_buffer
   */
  NX_MEDIA_DYNAMIC void disable_wait_buffer(bool value);

  /**
   * @func disable_wait_buffer
   */
  inline bool disable_wait_buffer() const { return m_disable_wait_buffer; }

 private:

  MultimediaSource* m_source;
  PCMPlayer*    m_pcm;
  MediaCodec*   m_audio;
  KeepLoop*     m_keep;
  PlayerStatus  m_status;
  OutputBuffer  m_audio_buffer;
  uint64  m_duration, m_time;
  uint64  m_uninterrupted_play_start_time;
  uint64  m_uninterrupted_play_start_systime;
  uint64  m_prev_presentation_time;
  Mutex   m_audio_loop_mutex, m_mutex;
  uint    m_task_id;
  uint    m_volume;
  bool    m_mute;
  bool    m_auto_play;
  bool    m_disable_wait_buffer;
  bool    m_waiting_buffer;

  NX_DEFINE_INLINE_CLASS(Inl);
};

NX_END
#endif
